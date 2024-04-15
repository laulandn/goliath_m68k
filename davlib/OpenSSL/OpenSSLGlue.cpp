/*
	OpenSSLGlue.cpp
	Copyright (C) 2000 i-drive.com

	Copyright (C) 2001-2002 Thomas Bednarz <tombednarz@hotmail.com>
	
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "OpenSSLGlue.h"
#include "UOpenTptSupport.h"

extern int errno;

CsslManager *USecureNetworkFactory::mCsslManager = nil;
LVariableArray	*USecureNetworkFactory::mCertificates = nil;
CRandomizer	*USecureNetworkFactory::mRandomizer = nil;
LRandomizer *USecureNetworkFactory::mRandomizerPeriodical = nil;

static USecureNetworkFactory gSecureNetworkFactory;

extern const char* gCertArray[];
extern const int gCertSizeArray[];
extern const int gCertCount;


//------------------------- Utility class ---------------------------

class StToggle
{
public:
	StToggle (bool &value) : mValue (&value)	{ *mValue = true; }
	~StToggle ()								{ *mValue = false; }
private:
	bool *mValue;
};

//------------------------- Glue Routines -----------------------------

long MacSocket_close (long socket)
{
	if (!socket)
		return -1;
	return ((LSecureOpenTptTCPEndpoint*)socket)->InheritedClose();
}

long MacSocket_recv (long socket, void *inBuff, long inBuffLength, bool inBlock)
{
#pragma unused (inBlock)
	if (!socket)
		return -1;
	return ((LSecureOpenTptTCPEndpoint*)socket)->InheritedReceive (inBuff, inBuffLength);
}

long MacSocket_send (long socket, void *outBuff, long outBuffLength)
{
	if (!socket)
		return -1;
	return ((LSecureOpenTptTCPEndpoint*)socket)->InheritedSend (outBuff, outBuffLength);
}

//--------------------- USecureNetworkFactory ------------------------

USecureNetworkFactory::USecureNetworkFactory (void)
{
	if (!mRandomizer)
		mRandomizer = new CRandomizer;
		
	if (!mCertificates)
	{
		mCertificates = new LVariableArray;
		SetCertificates ( gCertCount, gCertArray, gCertSizeArray );
	}
	
	if (!mRandomizerPeriodical)
		mRandomizerPeriodical = new LRandomizer;
		
	if (!mCsslManager)
	{
#ifdef DEBUG
		mCsslManager = new CsslManager(true);
#else
		mCsslManager = new CsslManager(false);
#endif	
	}
	
}

void USecureNetworkFactory::PeriodicAction (void)
{
	if (mRandomizer)
 		mRandomizer->PeriodicAction();
}
 
LSecureOpenTptTCPEndpoint* USecureNetworkFactory::CreateSecureTCPEndpoint (UInt32 inReceiveBufferSize)
{
#pragma unused (inReceiveBufferSize)

	if (!HasOpenTransport()) 
		return nil;
		
	UOpenTptSupport::StartOpenTransport();
	return new LSecureOpenTptTCPEndpoint();
}

void USecureNetworkFactory::SetCertificates ( int count, const char **certDataArray, const int *certDataLen )
{
	mCertificates->RemoveAllItemsAfter (0);
	for (int i = 0; i<count; i++)
		mCertificates->AddItem (certDataArray[i], certDataLen[i]);
}

void USecureNetworkFactory::AddCertificate(const char* certData, const int certLen) 
{
   mCertificates->AddItem (certData, certLen);
}
	
//------------------ Periodical for PowerPlant applications --------------------------
LRandomizer::LRandomizer (void)
{
	StartRepeating();
}

void LRandomizer::SpendTime ( const EventRecord& inMacEvent )
{
#pragma unused (inMacEvent)
	USecureNetworkFactory::PeriodicAction ();
}

//-------------------- LSecureTCPEndpoint methods ------------------------

// ---------------------------------------------------------------------------
//		¥ 
// ---------------------------------------------------------------------------
//	
LSecureOpenTptTCPEndpoint::LSecureOpenTptTCPEndpoint (void) : mSSL(this)
{
	mTimeout = Timeout_None;
	mSSLSending = false;
	mSSLReceiving = false;
	mSSLConnecting = false;

	const LVariableArray *certs = USecureNetworkFactory::GetCertificates();
	certs->Lock();
	for (int i=1; i<=certs->GetCount(); i++)
		mSSL.AddCertificate ( (char*)certs->GetItemPtr(i), certs->GetItemSize(i) );
	certs->Unlock();
}

// ---------------------------------------------------------------------------
//		¥ 
// ---------------------------------------------------------------------------
//	
void LSecureOpenTptTCPEndpoint::Connect ( LInternetAddress& inRemoteAddress, UInt32 inTimeoutSeconds )
{
	
	if (mSSLConnecting)
		LOpenTptTCPEndpoint::Connect ( inRemoteAddress, inTimeoutSeconds );
	else
	{
		StToggle toggle (mSSLConnecting);
		mTimeout = inTimeoutSeconds;
		LOpenTptTCPEndpoint::Connect ( inRemoteAddress, inTimeoutSeconds );
        
		if (mSSL.Connect ())
		{
			Disconnect();
			throw LSSLException (-1);
		}
	}
}

// ---------------------------------------------------------------------------
//		¥ 
// ---------------------------------------------------------------------------
//	
void LSecureOpenTptTCPEndpoint::Disconnect (void)
{
	LOpenTptTCPEndpoint::Disconnect();			// This isn't necessary. It's here just to be explicit.
}

// ---------------------------------------------------------------------------
//		¥ 
// ---------------------------------------------------------------------------
//	
void LSecureOpenTptTCPEndpoint::ReceiveData ( void* outDataBuffer, UInt32& ioDataSize, Boolean& outExpedited, UInt32 inTimeoutSeconds )
{
	if (mSSLSending || mSSLReceiving || mSSLConnecting)
		LOpenTptTCPEndpoint::ReceiveData ( outDataBuffer, ioDataSize, outExpedited, inTimeoutSeconds );
	else
	{
		StToggle toggle (mSSLReceiving);
		int bytesRead;
		int err;
		
		outExpedited = false;
		mTimeout = inTimeoutSeconds;
		
		bytesRead = mSSL.Receive (outDataBuffer, ioDataSize, &err);
		
		if (bytesRead > 0)
			ioDataSize = bytesRead;
			
		else
		{
			ioDataSize = 0;
			if (errno)
			{
				int temp = errno;
				errno = 0;
				throw LException (temp);
			}
			else if (err)
				throw LSSLException (err);
			else
				throw LSSLException (OrderlyDisconnect_Error);
		}
	}
}

// ---------------------------------------------------------------------------
//		¥ 
// ---------------------------------------------------------------------------
//	
void LSecureOpenTptTCPEndpoint::SendData ( void* inData, UInt32 inDataSize, Boolean inExpedited, UInt32 inTimeoutSeconds )
{
	if (mSSLSending || mSSLReceiving || mSSLConnecting)
		LOpenTptTCPEndpoint::SendData ( inData, inDataSize, inExpedited, inTimeoutSeconds );
	else
	{
		StToggle toggle (mSSLSending);
		int bytesSent;
		int err;
		
		mTimeout = inTimeoutSeconds;
		
		bytesSent = mSSL.Send (inData, inDataSize, &err);
		
		if (bytesSent < 0)
		{
			if (errno)
			{
				int temp = errno;
				errno = 0;
				throw (temp);
			}
			else
				throw LSSLException (err);
		}
	}
}


//---------------------- Inherited TCP methods -----------------------
// These will only be called from inside OpenSSL

long LSecureOpenTptTCPEndpoint::InheritedClose (void)
{
// release socket
	errno = 0;
	try
	{
		LOpenTptTCPEndpoint::Disconnect();
	}
	catch (int err) { errno = err;}
	catch (LException &ex) { errno = ex.GetErrorCode(); }
	catch (...) { errno = -1; }
	return errno;
}

long LSecureOpenTptTCPEndpoint::InheritedReceive (void *buf, long bufSize)
{
// return -1 if error
// return 0 if connection is closed
// set errno
	unsigned long received = (unsigned long)bufSize;
	Boolean expedited;
	int tempErr = 0;
	
	errno = 0;
	
	try
	{
		LOpenTptTCPEndpoint::ReceiveData (buf, received, expedited, mTimeout);
	}
	catch (int err) { tempErr = err; }
	catch (LException &ex) { tempErr = ex.GetErrorCode(); }
	catch (...) { errno = -1; }
	
	if (tempErr)
	{
		errno = tempErr;
		if (tempErr == Disconnect_Error || tempErr == OrderlyDisconnect_Error)
			received = 0;
		else
			received = -1;
	}
	
	int temperr = errno;
	return received;
}

long LSecureOpenTptTCPEndpoint::InheritedSend (void *buf, long bufSize)
{
// return -1 if error
// set errno
	errno = 0;
	try
	{
		LOpenTptTCPEndpoint::SendData (buf, bufSize, false, mTimeout);
	}
	catch (int err) { errno = err;}
	catch (LException &ex) { errno = ex.GetErrorCode(); }
	catch (...) { errno = -1; }
	
	return errno ? -1 : bufSize;
}

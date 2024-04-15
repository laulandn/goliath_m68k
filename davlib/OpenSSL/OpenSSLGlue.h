/*
	OpenSSLGlue.h
	Copyright (C) 2000 i-drive.com
	Copyright (C) 2001-2002 Thomas Bednarz Jr. <tombednarz@hotmail.com>
	
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

#ifndef __OPENSSLGLUE_H__
#define __OPENSSLGLUE_H__

#pragma once

#include <UNetworkFactory.h>
#include <LOpenTptTCPEndpoint.h>
#include <LPeriodical.h>
#include "SSLWrapper.h"
#include "Randomizer.h"

#include <LVariableArray.h>

class LSecureOpenTptTCPEndpoint;

#if !USE_OPENTPT
#error no MacTCP support, only OpenTransport
#endif

extern "C"
{
	long MacSocket_close (long socket);
	long MacSocket_recv (long socket, void *outBuff, long outBuffLength, bool inBlock);
	long MacSocket_send (long socket, void *outBuff, long outBuffLength);
}

PP_Begin_Namespace_PowerPlant


//-----------------------------------------------------------------

class LRandomizer : public LPeriodical
{
public:
	LRandomizer (void);
	virtual void SpendTime ( const EventRecord&	inMacEvent );
};

//-----------------------------------------------------------------

class USecureNetworkFactory : public UNetworkFactory
{
public:
	USecureNetworkFactory (void);
	static LSecureOpenTptTCPEndpoint* CreateSecureTCPEndpoint (UInt32 inReceiveBufferSize = 0);
	static void	SetCertificates ( int count, const char **certDataArray, const int *certDataLen);
	static void AddCertificate(const char* certData, const int certLen);
	static const LVariableArray *GetCertificates (void) {return mCertificates;}
	static void PeriodicAction (void);

private:
	static LVariableArray *mCertificates;
	static CRandomizer	*mRandomizer;
	static LRandomizer *mRandomizerPeriodical;
	static CsslManager *mCsslManager;
};

//-----------------------------------------------------------------

class LSSLException : public LException
{
public:
	LSSLException(
			SInt32			inErrorCode,
			ConstStringPtr	inErrorString = nil) : LException (inErrorCode, inErrorString) {}
};

//-----------------------------------------------------------------

class LSecureOpenTptTCPEndpoint : public LOpenTptTCPEndpoint
{
public:
	LSecureOpenTptTCPEndpoint (void);

	virtual void Connect ( LInternetAddress& inRemoteAddress, UInt32 inTimeoutSeconds = Timeout_None );
	virtual void Disconnect (void);
	virtual void ReceiveData ( void* outDataBuffer, UInt32& ioDataSize, Boolean& outExpedited, UInt32 inTimeoutSeconds = Timeout_None );
	virtual void SendData ( void* inData, UInt32 inDataSize, Boolean inExpedited = false, UInt32 inTimeoutSeconds = Timeout_None );

	long	InheritedClose (void);
	long	InheritedReceive (void *buf, long bufSize);
	long	InheritedSend (void *buf, long bufSize);
	
private:
	Cssl			mSSL;
	unsigned long	mTimeout;
	
// These flags are to assure that recursive calls by the corresponding LOpenTptTCPEndpoint
// calls will be correctly dispatched.

	bool			mSSLSending;
	bool			mSSLReceiving;
	bool			mSSLConnecting;
};

PP_End_Namespace_PowerPlant

#endif

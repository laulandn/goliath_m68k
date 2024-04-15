/* ===========================================================================
 *	CDAVMessage.cpp			   
 *
 *  This file is part of the DAVLib package
 *  Copyright (C) 1999-2000  Thomas Bednarzär
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  For questions, suggestions, bug-reports, enhancement-requests etc.
 *  I may be contacted at:
 *
 *  tombednarz@hotmail.com
 */
 
#include "DAVTypes.h"
#include "CDAVMessage.h"
#include <UInternet.h>

#include <LArray.h>
#include <LArrayIterator.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <LComparator.h>

#include "CDAVLibUtils.h"

#define kFieldHost "Host"
#define kFieldIIfHeader "If"

#define kFieldDepth "Depth" 
#define kFieldDestination "Destination"
#define kFieldOverwrite "Overwrite"
#define kFieldTimeout "Timeout"
#define kFieldLockToken "Lock-Token"

//#define kFieldContentType "Content-Type"

#define kFieldProxyAuthorization "Proxy-Authorization"

using namespace std;

// ---------------------------------------------------------------------------
//		¥ CDAVMessage
// ---------------------------------------------------------------------------
//	Constructor
//
// Creates a new empty message.

CDAVMessage::CDAVMessage(): mHasDepth(false), mDepth(DAVTypes::ZERO),
             mHasDestination(false), mHasOverwrite(false), mHasTimeout(false),
             mHasLockToken(false), mHasIfHeader(false)
{
	InitMembers();
}

// ---------------------------------------------------------------------------
//		¥ CDAVMessage
// ---------------------------------------------------------------------------
//	Alternate Constructor
//
//	This constructor takes a char * to a string containing a RFC822 style message
//	consisting of a properly formatted header and a message body seperated by a 
//	CRLF pair. 

//	The header will be copied to mHeader and parsed for standard info
//	like: MIME, Content-Type, Mime Boundary, etc.. Other fields will
//	be accessible via the "Arbitrary Field" accessors.
//
//	The body will be copied into mBody and no transformations will be applied to it.

CDAVMessage::CDAVMessage(const char * inMessage):LHTTP11Message(inMessage)
{
}

// ---------------------------------------------------------------------------
//		¥ ~CDAVMessage
// ---------------------------------------------------------------------------
//	Destructor
//

CDAVMessage::~CDAVMessage()
{

}

// ---------------------------------------------------------------------------
//		¥ SetDepth
// ---------------------------------------------------------------------------
//

void CDAVMessage::SetDepth(DAVTypes::PropertyDepth depth) {
   mDepth = depth;
   mHasDepth = true;
}

// ---------------------------------------------------------------------------
//		¥ GetDepth
// ---------------------------------------------------------------------------
//

DAVTypes::PropertyDepth CDAVMessage::GetDepth() {
   return mDepth;
}

// ---------------------------------------------------------------------------
//		¥ GetHasDepth
// ---------------------------------------------------------------------------
//

Boolean CDAVMessage::GetHasDepth() {
   return mHasDepth;
}

// ---------------------------------------------------------------------------
//		¥ SetDestination
// ---------------------------------------------------------------------------
//

void CDAVMessage::SetDestination(const char* destination) {
    mHasDestination = true;
    mDestination = destination;      

}

// ---------------------------------------------------------------------------
//		¥ SetDestination
// ---------------------------------------------------------------------------
//
void CDAVMessage::SetDestination(ConstStr255Param destination) {
	mHasDestination = true;
	mDestination.assign ((const char*)destination + 1, destination[0]);
}


// ---------------------------------------------------------------------------
//		¥ GetDestination
// ---------------------------------------------------------------------------
//

const char *CDAVMessage::GetDestination() {
   return mDestination.c_str();
}


// ---------------------------------------------------------------------------
//		¥ GetHasDestination
// ---------------------------------------------------------------------------
//

Boolean CDAVMessage::GetHasDestination() {
   return mHasDestination;
}
	

// ---------------------------------------------------------------------------
//		¥ SetOverwrite
// ---------------------------------------------------------------------------
//

void CDAVMessage::SetOverwrite(DAVTypes::Overwrite overwrite) {
   mHasOverwrite = true;
   mOverwrite = overwrite;
}


// ---------------------------------------------------------------------------
//		¥ GetOverwrite
// ---------------------------------------------------------------------------
//

DAVTypes::Overwrite CDAVMessage::GetOverwrite() {
   return mOverwrite;
}


// ---------------------------------------------------------------------------
//		¥ GetHasOverwrite
// ---------------------------------------------------------------------------
//
Boolean CDAVMessage::GetHasOverwrite() {
   return mHasOverwrite;
}

// ---------------------------------------------------------------------------
//		¥ SetTimeout
// ---------------------------------------------------------------------------
//
void CDAVMessage::SetTimeout(const char* timeout) {
   mHasTimeout = true;
   mTimeout = timeout;
}

// ---------------------------------------------------------------------------
//		¥ GetTimeout
// ---------------------------------------------------------------------------
//
const char* CDAVMessage::GetTimeout() {
   return mTimeout.c_str();
}
    
// ---------------------------------------------------------------------------
//		¥ GetHasTimeout
// ---------------------------------------------------------------------------
//
Boolean CDAVMessage::GetHasTimeout() {
   return mHasTimeout;
}


// ---------------------------------------------------------------------------
//		¥ SetLockToken
// ---------------------------------------------------------------------------
//
void CDAVMessage::SetLockToken(const char* lockToken) {
   mLockToken = lockToken;
   mHasLockToken=true;
}


// ---------------------------------------------------------------------------
//		¥ GetLockToken
// ---------------------------------------------------------------------------
//
const char* CDAVMessage::GetLockToken() {
   return mLockToken.c_str();
}

// ---------------------------------------------------------------------------
//		¥ GetHasLockToken
// ---------------------------------------------------------------------------
//
Boolean CDAVMessage::GetHasLockToken() {
   return mHasLockToken;
} 


// ---------------------------------------------------------------------------
//		¥ SetIfHeader
// ---------------------------------------------------------------------------
//
void CDAVMessage::SetIfHeader(const char* ifhdr) {
   mHasIfHeader = true;
   mIfHeader = ifhdr;
}


// ---------------------------------------------------------------------------
//		¥ GetIfHeader
// ---------------------------------------------------------------------------
//
const char* CDAVMessage::GetIfHeader() {
   return mIfHeader.c_str();
}

// ---------------------------------------------------------------------------
//		¥ GetHasIfHeader
// ---------------------------------------------------------------------------
//
Boolean CDAVMessage::GetHasIfHeader() {
   return mHasIfHeader;
}

// ---------------------------------------------------------------------------
//		¥ SetProxyPassword
// ---------------------------------------------------------------------------
// 
void CDAVMessage::SetProxyPassword(const char * inPassword) {
	mProxyPassword = inPassword;
}

// ---------------------------------------------------------------------------
//		¥ GetProxyPassword
// ---------------------------------------------------------------------------
// 
const char * CDAVMessage::GetProxyPassword() {
	return mPassword.c_str();
}

// ---------------------------------------------------------------------------
//		¥ SetProxyUserName
// ---------------------------------------------------------------------------
// 
void CDAVMessage::SetProxyUserName(const char * inUserName) {
	mProxyUserName = inUserName;
}

// ---------------------------------------------------------------------------
//		¥ GetProxyUserName
// ---------------------------------------------------------------------------
// 
const char * CDAVMessage::GetProxyUserName() {
	return mProxyUserName.c_str();
}

  
// ---------------------------------------------------------------------------
//		¥ BuildHeader
// ---------------------------------------------------------------------------
//
void CDAVMessage::BuildHeader(LDynamicBuffer * outHeader)
{		
	char tempString[1024];
	//LMIMEMessage::BuildHeader(outHeader);
	
    //Handle the custom header
	//LInternetMessage::BuildHeader(outHeader);
    LHTTP11Message::BuildHeader(outHeader);
    
	//If header is custom we use it "as is"
	if (mCustomHeader)
		return;
		
	//Proxy Authentication if necessary
	if ((mProxyUserName.length() > 0) && (mProxyPassword.length() > 0)) {
		char encodedAuthStr[256];
		sprintf(tempString, "%s:%s", mProxyUserName.c_str(), mProxyPassword.c_str());
		
		HTUU_encode((unsigned char *)tempString, strlen(tempString), encodedAuthStr);

		//7/23/98 PFV - To prevent folding of this field we build it
		//				here directly.
		sprintf(tempString, "%s: Basic %s\r\n", kFieldProxyAuthorization, encodedAuthStr);
		*outHeader += tempString;
	}
	

    if (GetHasDepth()) {
       DAVTypes::PropertyDepth depth = GetDepth();
       if (DAVTypes::ZERO == depth) {
          sprintf(tempString, "0", GetDepth());
          AddFieldToBuffer(kFieldDepth, tempString, outHeader);	       
       } else if (DAVTypes::ONE == depth) {
          sprintf(tempString, "1", GetDepth());
          AddFieldToBuffer(kFieldDepth, tempString, outHeader);	       
       } else if (DAVTypes::PROPINFINITY == depth) {
          sprintf(tempString, "infinity", GetDepth());
    	  AddFieldToBuffer(kFieldDepth, tempString, outHeader);	       
       }
    }
    
    if (GetHasDestination()) {
       sprintf(tempString, "%s", GetDestination());
       //AddFieldToBuffer(kFieldDestination, tempString, outHeader);
       outHeader->ConcatenateBuffer(kFieldDestination);
       outHeader->ConcatenateBuffer(": ");
       outHeader->ConcatenateBuffer(tempString);
       outHeader->ConcatenateBuffer(CRLF);
    }
    
    if (GetHasOverwrite()) {
       DAVTypes::Overwrite overwrite = GetOverwrite();
       if (DAVTypes::T == overwrite) {
          sprintf(tempString, "T");
          AddFieldToBuffer(kFieldOverwrite, tempString, outHeader);	       
       } else if (DAVTypes::F == overwrite) {
          sprintf(tempString, "F");
          AddFieldToBuffer(kFieldOverwrite, tempString, outHeader);	       
       }
    }
    
    if (GetHasTimeout()) {
       AddFieldToBuffer(kFieldTimeout, GetTimeout(), outHeader);	      
    }	
    
    if (GetHasLockToken()) {
       //AddFieldToBuffer(kFieldLockToken, GetLockToken(), outHeader);
       outHeader->ConcatenateBuffer(kFieldLockToken);
       outHeader->ConcatenateBuffer(": ");
       outHeader->ConcatenateBuffer(GetLockToken());
       outHeader->ConcatenateBuffer(CRLF);
    }
    
    if (GetHasIfHeader()) {
       outHeader->ConcatenateBuffer(kFieldIIfHeader);
       outHeader->ConcatenateBuffer(": ");
       outHeader->ConcatenateBuffer(GetIfHeader());
       outHeader->ConcatenateBuffer(CRLF);
    }    
}

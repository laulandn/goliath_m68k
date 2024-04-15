/*
	LHTTPResponse.cp
	Copyright (C) 2000 i-drive.com (email: opensource@mail.idrive.com)

	This library is free software; you can redistribute it and/or
	modify it under the terms of version 2.1 of the GNU Lesser General
	Public License as published by the Free Software Foundation.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// ===========================================================================
//
//   Portions of this file are derived from the file LHTTPResponse.cp
//   by Metrowerks, and are Copyrighted (C) 1996-2000 Metrowerks Inc.
//
// ===========================================================================

// ===========================================================================
//	LHTTPResponse.cp			PowerPlant 2.0		
// ===========================================================================
//
// Class for handling HTTP style responses.
//
// NOTE: Since HTTP responses always include the requested data or an error
//		description (usually in HTML) this class also includes the actual
//		LHTTPMessage return from the server.

#include <LHTTPResponse.h>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <UInternet.h>

PP_Begin_Namespace_PowerPlant
using namespace std;

// ===========================================================================

#pragma mark === LHTTPResponse ===

// ---------------------------------------------------------------------------
//		¥ LHTTPResponse()
// ---------------------------------------------------------------------------
//	Constructor

LHTTPResponse::LHTTPResponse()
{
	mExternalStream = NULL;
}

// ---------------------------------------------------------------------------
//		¥ LHTTPResponse()
// ---------------------------------------------------------------------------
//	Alternate Constructor

LHTTPResponse::LHTTPResponse(char * inResponse)
{
	SetResponse(inResponse);
	mExternalStream = NULL;
}

// ---------------------------------------------------------------------------
//		¥ LHTTPResponse()
// ---------------------------------------------------------------------------
//	Copy Constructor

LHTTPResponse::LHTTPResponse(const LHTTPResponse& copyResponse)
	:LInternetResponse(copyResponse)
{
	mMessage = copyResponse.mMessage;
	mFields = copyResponse.mFields;
	mExternalStream = copyResponse.mExternalStream;
}

// ---------------------------------------------------------------------------
//		¥ ~LHTTPResponse
// ---------------------------------------------------------------------------
//	Destructor

LHTTPResponse::~LHTTPResponse()
{
}
 
// ---------------------------------------------------------------------------
//		¥ SetResponse
// ---------------------------------------------------------------------------
//
void
LHTTPResponse::SetResponse(const char * inResponse) {
	SetResponse(inResponse, strlen(inResponse));
}

// ---------------------------------------------------------------------------
//		¥ SetResponse
// ---------------------------------------------------------------------------
//
void
LHTTPResponse::SetResponse(const char * inResponse, UInt32 inLength)
{	
	char * p, *q;
	
	p = strchr(inResponse, LF);
	if (!p)
		return;
		
	mResponseText.assign(inResponse, (p - inResponse)); 	//-1 removes the CR
	
	//Pull code out of response
	q = strchr(inResponse, ' ');
	if (q)
		mResponseCode = atol(++q);
	
	p++;		//+1 walks past the LF
	if (*p)
		mMessage.SetMessage(p, inLength - (p - inResponse));
	Parse ();
}

// ---------------------------------------------------------------------------
//		¥ SetResponse
// ---------------------------------------------------------------------------
//

void
LHTTPResponse::SetResponse(const char * inResponse, LDynamicBuffer * inResponseData)
{	
	StPointerBlock tempResponse(strlen(inResponse) + 1, true);
	strcpy(tempResponse, inResponse);
	char * p;
	
	//strip CRLF
	p = strchr(tempResponse, LF);
	if (p)
		*p = '\0';
	p = strchr(tempResponse, CR);
	if (p)
		*p = '\0';
	mResponseText = tempResponse;

	//Pull code out of response
	p = strchr(inResponse, ' ');
	if (p)
		mResponseCode = atol(++p);
	
	//Set the data, we just grab the stream to prevent having to
	//	duplicate a potentially large amount of data
	mMessage.SetMessage(inResponseData);
	Parse ();
}

// ---------------------------------------------------------------------------
//		¥ SetResponseText
// ---------------------------------------------------------------------------
//

void
LHTTPResponse::SetResponseText (const char * inResponseText, UInt32 inLength)
{
	string::size_type p;
	const static SInt32 kHTTPRequestOK = 200;
	
	mResponseText.assign (inResponseText, inLength);
	if ((p = mResponseText.find (CR)) != string::npos)
		mResponseText.resize (p);
	if ((p = mResponseText.find (' ')) != string::npos)
		mResponseCode = atol (mResponseText.c_str() + p + 1);
	else
		mResponseCode = 0;
		
	if (mExternalStream && mResponseCode == kHTTPRequestOK)
		mMessage.GetInternalMessageBody()->SetStream (mExternalStream, false);
}

// ---------------------------------------------------------------------------
//		¥ Parse
// ---------------------------------------------------------------------------
//

void
LHTTPResponse::Parse (void)
{
	mFields.clear();
	const string& header = *mMessage.GetInternalHeaderString();
	
	string name;
	string contents;
	
	string::size_type titleEnd = 0;
	string::size_type lineEnd = 0;
	
	for (int pos = 0; pos < header.size(); pos = lineEnd + 2)
	{
		lineEnd = header.find ("\r\n", pos);						// Find the end of the line
		if (lineEnd == string::npos || lineEnd == pos)
			break;
		titleEnd = header.find (':', pos);							// Find first colon in line
		if (titleEnd == string::npos)
			continue;
		name.assign (header, pos, titleEnd - pos);					// Name is everything up to colon
		titleEnd++;													// Go past the colon
		while (titleEnd < lineEnd && isspace((header)[titleEnd]))	// Go past blank spaces
			titleEnd++;
		contents.assign (header, titleEnd, lineEnd - titleEnd);		// Field contents is everything to end of line
		mFields.insert ( pair<const string, string> (name, contents) );
	}
}

// ---------------------------------------------------------------------------
//		¥ GetField
// ---------------------------------------------------------------------------
//

const string *LHTTPResponse::GetField (const char* fieldName, UInt32 index)
{
	pair<stringMap::iterator, stringMap::iterator> range = mFields.equal_range (string(fieldName));
	stringMap::iterator iter = range.first;
	
	if (distance (range.first, range.second) < index)
		return NULL;
		
	for (int i=0; iter != range.second; iter++, i++)
		if (i==index)
			return &(iter->second);
			
	return NULL;
}

// ---------------------------------------------------------------------------
//		¥ SetExternalStream
// ---------------------------------------------------------------------------
//

void LHTTPResponse::SetExternalStream (LStream* inStream)
{
	mExternalStream = inStream;
}

// ---------------------------------------------------------------------------
//		¥ CaseFreeCompare
// ---------------------------------------------------------------------------
//

bool
LHTTPResponse::CaseFreeLess::operator() (const string& s1, const string& s2) const
{
	if (s1.size() != s2.size())
		return s1.size() < s2.size();
		
	const char *p1 = s1.c_str() - 1;
	const char *p2 = s2.c_str() - 1;
	char c1, c2;
	
	while ( (c1=tolower(*++p1)) == (c2=tolower(*++p2)) )
	{
		if (!c1)
			return false;		// Strings are equal
	}
			
	return (c1 < c2);
}

PP_End_Namespace_PowerPlant

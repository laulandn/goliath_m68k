/*
	LHTTPResponse.h
	Copyright (C) 2000 i-drive.com

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
//   Portions of this file are derived from the file LHTTPResponse.h
//   by Metrowerks, and are Copyrighted (C) 1996-2000 Metrowerks Inc.
//
// ===========================================================================

// ===========================================================================
//	LHTTPResponse.h				PowerPlant 2.0
// ===========================================================================

#ifndef _H_LHTTPResponse
#define _H_LHTTPResponse
#pragma once

#include <LInternetResponse.h>
#include <LHTTPMessage.h>
#include <map>

#if PP_Uses_Pragma_Import
	#pragma import on
#endif

PP_Begin_Namespace_PowerPlant

// ===========================================================================
//		¥ LHTTPResponse
// ===========================================================================

class LHTTPResponse : public LInternetResponse {

public:
							LHTTPResponse();
							LHTTPResponse(char * inResponse);
							LHTTPResponse(const LHTTPResponse& copyResponse);
	virtual					~LHTTPResponse();
	
	virtual void			SetResponse(const char* inResponse);
	virtual void			SetResponse(const char * inResponse, UInt32 inLength);
	virtual void			SetResponse(const char * inResponse, LDynamicBuffer * inResponseData);
	void					SetResponseText (const char * inResponseText, UInt32 inLength);
							//Accept all 2xx responses as valid
	inline Boolean			GetStatus() {return (GetResponseCode()/100 == 2);}

	virtual inline LHTTPMessage*	GetReturnMessage() {return &mMessage;}

	virtual void			ResetResponse() {
								LInternetResponse::ResetResponse();
								mMessage.ResetMembers();
							}
	const std::string *		GetField (const char* fieldName, UInt32 index = 0);
	void					Parse (void);
	void					SetExternalStream (LStream* inStream);

protected:
	LHTTPMessage			mMessage;
	
	struct CaseFreeLess
	{
		bool operator() (const std::string& a, const std::string& b) const;
	};
	
// multimap, because some fields (e.g. Set-Cookie) may have the same name.
	typedef std::multimap< std::string, std::string, CaseFreeLess > stringMap;
	
	LStream*	mExternalStream;
	stringMap	mFields;
};

PP_End_Namespace_PowerPlant

#if PP_Uses_Pragma_Import
	#pragma import reset
#endif

#endif

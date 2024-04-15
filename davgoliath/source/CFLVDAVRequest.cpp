/* ==================================================================================================
 * CFLVDAVRequest.cp															   
 *    Goliath - a Finder like application that implements WebDAV
 *    Copyright (C) 1999-2001  Thomas Bednarz
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
 * ===========================================================================
 */
 
#include <UModalDialogs.h>
#include <LWindow.h>
#include <PP_Messages.h>

#include <map>
#include "CDAVRequest.h"
#include "CDAVProperty.h"
#include "parsexml.h"
#include "CFLVDAVRequest.h"
#include <LStaticText.h>
#include "CDAVTableApp.h"
#include "CClientLockManager.h"
#ifndef __CDAVERRORUTILS_h__
#include "CDAVErrorUtils.h"
#endif
#include <string.h>

// ---------------------------------------------------------------------------
//		¥ CFLVDAVRequest()
// ---------------------------------------------------------------------------
//	Constructor
CFLVDAVRequest::CFLVDAVRequest(CDAVContext *ctx): CDAVRequest(ctx), mSuppressDlog(false)
{
   mApp = GetApplicationInstance();
}



// ---------------------------------------------------------------------------
//		¥ CFLVDAVRequest()
// ---------------------------------------------------------------------------
//	Constructor
CFLVDAVRequest::CFLVDAVRequest(LThread & inThread, CDAVContext *ctx): CDAVRequest(inThread, ctx),
mSuppressDlog(false)
{
   mApp = GetApplicationInstance();
}


// ---------------------------------------------------------------------------
//		¥ CFLVDAVRequest()
// ---------------------------------------------------------------------------
//	Constructor
CFLVDAVRequest::CFLVDAVRequest(CFLVDAVRequest &rhs):CDAVRequest(rhs) {
   mApp = rhs.mApp;
   mSuppressDlog = rhs.mSuppressDlog;
}


// ---------------------------------------------------------------------------
//		¥ ~CFLVDAVRequest()
// ---------------------------------------------------------------------------
//	Destructor
CFLVDAVRequest::~CFLVDAVRequest() {

}

// ---------------------------------------------------------------------------
//		¥ _OnDavRequestError()
// ---------------------------------------------------------------------------
// 
void CFLVDAVRequest::_OnDavRequestError(LHTTPResponse &theResponse, const char* displayString) {
    if (mSuppressDlog)
       return;
    CDAVErrorUtils::DisplayDAVError(&theResponse, displayString);
    
/*	PP_PowerPlant::StDialogHandler dialog(1300,  mApp);
	Assert_(dialog.GetDialog() != nil);
    PP_PowerPlant::LStaticText* errorField = dynamic_cast<PP_PowerPlant::LStaticText*>
									(dialog.GetDialog()->FindPaneByID('ERRT'));
	
	ThrowIfNil_ (errorField);
	const char* theError = theResponse.GetResponse();
	
    LStr255 errUIPString(str_UIStrings, str_UnknownConnectionErr); 
    std::string errUICstring;
    errUICstring.assign(errUIPString.ConstTextPtr(), errUIPString.Length());

	
	//against Apache 1.3.x on Win32 systems, the connection fails and returns a
	//status of zero.  No time to track this one down right now, unfortunately.
	if (((NULL==theError) || (strcmp(theError,"")==0)) && (NULL != displayString))
	   theError = displayString;
    else if (theResponse.GetResponseCode() == 401)
    	theError = "Unauthorized User; Please Verify User Name and Password";
    else if (theResponse.GetResponseCode() == 403)
    	theError = "Error: Operation Forbidden";
    else if (theResponse.GetResponseCode() == 404)
    	theError = "Error: URL not found on this server";
    else if (theResponse.GetResponseCode() == 405)
       theError = "Error: Operation not allowed.";//***teb - move to STR resource
    else
	   theError = "Unknown connection error.";//***teb - move to STR resource
	   
	errorField->SetText(const_cast<char*>(theError), strlen(theError));
	dialog.GetDialog()->Show();
	while (true) {
		PP_PowerPlant::MessageT hitMessage = dialog.DoDialog();
	    if (hitMessage == PP_PowerPlant::msg_OK)
			break;
	}
	*/
}

// ---------------------------------------------------------------------------
//		¥ _OnDavRequestError()
// ---------------------------------------------------------------------------
//
void CFLVDAVRequest::_HandleSSLexception(LHTTPResponse &theResponse, LSSLException& e) {

   LStr255 errUnkUIPString(str_UIStrings, str_UnknownSSLErr);
   std::string errUnkUICString;
   errUnkUICString.assign(errUnkUIPString.ConstTextPtr(), errUnkUIPString.Length());

   LStr255 errCAUIPString(str_UIStrings, str_SSLCertErr);
   std::string errCAUICString;
   errCAUICString.assign(errCAUIPString.ConstTextPtr(), errCAUIPString.Length());

   std::string errStr = errUnkUICString;
  
   SInt32 errCode = e.GetErrorCode();
   if (-1 == errCode) {
      errStr = errCAUICString;
   }
   
   _OnDavRequestError(theResponse, errStr.c_str());	    
}

// ---------------------------------------------------------------------------
//		¥ _OnDavRequestError()
// ---------------------------------------------------------------------------
// 
void CFLVDAVRequest::_OnDavItemCreated(CDAVItem& theItem) {
   std::string dummy;
   if (mApp->GetLockManager()->GetLockToken(m_context, &theItem, dummy)) {
      theItem.SetIsLocalLock(true);
   }
}

// ---------------------------------------------------------------------------
//		¥ _OnDavItemDataChange()
// ---------------------------------------------------------------------------
// 
void CFLVDAVRequest::_OnDavItemDataChange(CDAVItem& theItem) {
   std::string lockToken;
   if (mApp->GetLockManager()->GetLockToken(m_context, &theItem, lockToken)) {
      if ((theItem.GetIsLocked()==false) || (strcmp(lockToken.c_str(), theItem.GetLockToken().c_str())!=0)) {
         mApp->GetLockManager()->ItemUnlocked(m_context, &theItem);
      } 
   }
}

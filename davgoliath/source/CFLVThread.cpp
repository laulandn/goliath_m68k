/* ==================================================================================================
 * CFLVThread.cpp														   
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
#include <CDAVItem.h>
#include "CDAVTableApp.h"
#include "CDAVTableWindow.h"
#include <LThread.h>
#include <LProgressBar.h>
#include <CDAVRequest.h>
#include <LOutlineItem.h>
#include "CDAVTableItem.h"
#include "CFLVThread.h"
#include <CWindowManager.h>
#include <LStaticText.h>
#include <string.h>
#include <CGoliathPreferences.h>

// ---------------------------------------------------------------------------
//		¥ CFLVThread()
// ---------------------------------------------------------------------------
//	
CFLVThread::CFLVThread(CDAVContext *ctx, CDAVTableWindow *wnd) :LThread(	false,/*pre-emptive*/
					PP_PowerPlant::thread_DefaultStack,
					PP_PowerPlant::LThread::threadOption_Default,
					nil), 
					mContext(*ctx), 
					mWnd(wnd), 
					mRequest ( *this, &mContext) 
{
   Init();
   mEncodingDisabled = !(wnd->EncodeResources());
}

// ---------------------------------------------------------------------------
//		¥ CFLVThread()
// ---------------------------------------------------------------------------
//	
CFLVThread::CFLVThread(class CDAVContext *ctx, class CDAVTableWindow *wnd, bool encodingDisbled) :
			LThread(	false,/*pre-emptive*/
					PP_PowerPlant::thread_DefaultStack,
					PP_PowerPlant::LThread::threadOption_Default, nil), 
			mContext(*ctx), 
			mWnd(wnd), 
			mRequest ( *this, &mContext),
			mEncodingDisabled(encodingDisbled)
{
   Init();
}

// ---------------------------------------------------------------------------
//		¥ ~CFLVThread()
// ---------------------------------------------------------------------------
//			
CFLVThread::~CFLVThread() {

}
		
// ---------------------------------------------------------------------------
//		¥ Init()
// ---------------------------------------------------------------------------
//			
void CFLVThread::Init() {
   mRezForksAreHidden = true;
   std::string showDotFiles = GetApplicationInstance()->GetPreferencesManager()->GetPrefValue(CGoliathPreferences::SHOWDOTFILES);
   if (strcmp(showDotFiles.c_str(), CGoliathPreferences::TRUE_VALUE)==0)
        mRezForksAreHidden = false;
}
			
// ---------------------------------------------------------------------------
//		¥  Run()
// ---------------------------------------------------------------------------
//			
void* CFLVThread::Run() {

   CWindowManager *winMgr = GetApplicationInstance()->GetWindowManager();
   void* retVal;
   
   msg_HTTPStartStruct startStruct;
   startStruct.originatingWindow = mWnd;
   winMgr->BroadcastMessage(msg_HTTPStart, &startStruct);

	try
	{
		retVal = _executeDAVTransaction();
	}
	catch (...)
	{
	}

   msg_HTTPEndStruct endStruct;
   endStruct.originatingWindow = mWnd;
   winMgr->BroadcastMessage(msg_HTTPEnd, &endStruct);
   
   return retVal;
}

// ---------------------------------------------------------------------------
//		¥ _OnXMLRequestError()
// ---------------------------------------------------------------------------
// 
void CFLVThread::_OnXMLRequestError() {
	PP_PowerPlant::StDialogHandler dialog(1300,  GetApplicationInstance());
	Assert_(dialog.GetDialog() != nil);
    PP_PowerPlant::LStaticText* errorField = dynamic_cast<PP_PowerPlant::LStaticText*>
									(dialog.GetDialog()->FindPaneByID('ERRT'));
	
	ThrowIfNil_ (errorField);
	const char* theError = "The server returned an invalid response.";
		   
	errorField->SetText(const_cast<char*>(theError), strlen(theError));
	dialog.GetDialog()->Show();
	while (true) {
		PP_PowerPlant::MessageT hitMessage = dialog.DoDialog();
	    if (hitMessage == PP_PowerPlant::msg_OK)
			break;
	}
}

// ---------------------------------------------------------------------------
//		¥ _RezForksAreHidden()
// ---------------------------------------------------------------------------
// 
bool CFLVThread::_RezForksAreHidden() {
   return mRezForksAreHidden;
}

// ---------------------------------------------------------------------------
//		¥ _EncodingDisabled()
// ---------------------------------------------------------------------------
// 
bool CFLVThread::_EncodingDisabled() {
   return mEncodingDisabled;
}
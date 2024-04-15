/* ==================================================================================================
 * CDAVTableApp.h															   
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
 
#ifndef __CDAVTableApp_h__ 
#define __CDAVTableApp_h__
#pragma once

#include <LApplication.h>
#include <CDAVItem.h>
#include <LCookie.h>
#include <LDocApplication.h>
#include <LListener.h>

#if PP_Debug
	class LDebugMenuAttachment;
#endif

class CDAVTableApp : public LDocApplication, LListener 
{
	public:
						CDAVTableApp();
		virtual 		~CDAVTableApp();
	
		virtual Boolean	ObeyCommand(CommandT inCommand, void* ioParam);	
		virtual void	FindCommandStatus(CommandT inCommand, Boolean &outEnabled, Boolean &outUsesMark, UInt16 &outMark, Str255 outName);
        
        class CWindowManager       * GetWindowManager();
        class CGoliathPreferences  *GetPreferencesManager();
        class CClientLockManager   *GetLockManager();
        
        virtual void HandleAppleEvent(const AppleEvent& inAppleEvent,
	                                  AppleEvent& outAEReply, AEDesc& outResult, long inAENumber);

        void DoAEOpenDoc(const AppleEvent& inAppleEvent, AppleEvent& outAEReply,SInt32 inAENumber);
        void OpenDocList(const AEDescList&	inDocList, SInt32 inAENumber);
        virtual Boolean AttemptQuitSelf( SInt32 inSaveOption);
		virtual void ListenToMessage(MessageT inMessage, void* ioParam);

		virtual void		ProcessNextEvent();
 
        void DisplayErrorDialog(LStr255& inErrMsg);
		
		LCookieList& GetCookieList() {return mCookieList;};
	protected:
	    virtual void    Initialize();
		virtual void	ShowAboutBox();
		virtual void	StartUp();
		virtual void    MakeNewWindow();
		virtual void    OpenDocument(FSSpec *inSpec);

		virtual void	EventResume(const EventRecord& inMacEvent);
	
		
#if DAVLIB_SSL
		void _InitDAVLibSSL();
#endif		
		void _OnPreferences();
		void _OnLogHTTPRequests();
		void _OnOpeniDisk();
		void _OnEditClientCertificate();
		
		class CWindowManager* mWindowManager;
		class CIconSuiteManager *mIconSuiteManager;
		class CGoliathPreferences *mPreferences;
		class CClientLockManager  *mLockManager;

        UInt32 mActiveConnections;		
		
		LCookieList mCookieList;
		
		Boolean mIsLogging;
		bool	mIsCheckingEditState;
		bool    mStartedUp;
#if PP_Debug
	LDebugMenuAttachment*	mDebugAttachment;
#endif
};


CDAVTableApp *GetApplicationInstance();
class UMainThread *GetMainThreadInstance();


const ResIDT	win_FinderListView		= 202;
const ResIDT	win_About				= 201;
const ResIDT	icn_SortDown			= 300;
const ResIDT	icn_SortUp				= 301;
const ResIDT	icn_folder				= 400;
const ResIDT	icn_file				= 401;

const MessageT	msg_RowSingleClicked	= 1000;
const MessageT	msg_RowDoubleClicked	= 1001;
const MessageT  msg_TableViewScrolled   = 1002;



const ResIDT    str_UIStrings           = 1000;
const ResIDT    str_ColumnHdrStrings    = 1001;

const ResIDT    str_Collection          = 1;
const ResIDT    str_File                = 2;
const ResIDT    str_Warning             = 3;
const ResIDT    str_DeleteConfirmation  = 4;
const ResIDT    str_Ok                  = 5;
const ResIDT    str_Cancel              = 6;
const ResIDT    str_DownloadSelFolder   = 7;
const ResIDT    str_SelUploadFile       = 8;
const ResIDT    str_SelStateDocument    = 9;

const ResIDT    str_NameHeader          = 10;
const ResIDT    str_LastModHeader       = 11;
const ResIDT    str_SizeHeader          = 12;
const ResIDT    str_KindHeader          = 13;
const ResIDT    str_LockedByHeader      = 14;
const ResIDT	str_FileAlreadyExists	= 15;

const ResIDT    str_SelSaveStateDocument      = 19;
const ResIDT    str_FileToLargeToEdit      = 20;
const ResIDT    str_DeleteConfirmText    = 21;
const ResIDT    str_SelectLogFile        = 22;
const ResIDT    str_ConfirmEnableLog     = 23;
const ResIDT    str_GoliathCAFolderName  = 24;
const ResIDT    str_UnknownConnectionErr = 25;
const ResIDT    str_UnknownSSLErr        = 26;
const ResIDT    str_SSLCertErr           = 27;
const ResIDT	str_FolderAlreadyExists	 = 28;
const ResIDT	str_InternalErrorNoParamText	= 29;
const ResIDT	str_InternalError		 = 30;
const ResIDT	str_ConnectionDocParseError		 = 31;
const ResIDT	str_DefaultLockUserName		 = 32;
const ResIDT	str_CantLockRezForkErr		 = 33;
const ResIDT	str_CantLockRezForkAndNoUnlockDataErr		 = 33;
const ResIDT	str_CantUnlockRezForkAndNoUnlockDataErr		 = 34;
const ResIDT	str_EditConnectionSettingsDlogTitle = 35;
const ResIDT	str_OpenClientCertFile	= 36;
const ResIDT	str_IncorrectPassword	= 37;
const ResIDT	str_BadDataFile	= 38;
const ResIDT    str_CertFileIOErr = 39;
const ResIDT    str_CertFileUnknownErr = 40;



const ResIDT    str_ErrorStrings        = 1003;
const ResIDT	str_errRequestUnauthorized = 1;
const ResIDT	str_errRequestForbidden    = 2;
const ResIDT	str_errNotFound            = 3;
const ResIDT	str_errNotAllowed          = 4;


extern const char* GoliathAppVersion;
const ResIDT    str_HeaderTitles        = 1001;


const ResIDT    pane_TableHeader        = 'phdr';
const ResIDT    pane_DAVTable           = 'flvt';
const ResIDT    pane_BusyArrows         = 'prg1';

const OSType    AppCreatorCode          = 'DAV1';
const OSType    DocTypeCode             = 'DDOC';

#endif

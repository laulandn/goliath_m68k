/* ==================================================================================================
 * CDAVTableApp.cp															   
 *    Goliath - a Finder like application that implements WebDAV
 *    Copyright (C) 1999-2002  Thomas Bednarz
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

#ifndef __APPEARANCE__
#include <Appearance.h>
#endif
#include <InternetConfig.h>
#include <LGrowZone.h>
#include <LStaticText.h>
#include <UStandardDialogs.h>
#include <UThread.h>
#include <LCleanupTask.h>
#include <LURL.h>
#include <CDAVLibUtils.h>
#include <CDAVContext.h>
#include "CFLVDAVRequest.h"
#include "CWindowManager.h"
#include "CIconSuiteManager.h"
#include "CNavServicesUtils.h"
#include "ParseConnectionDocument.h"
#include "CDAVTableAppConstants.h"
#include "CGoliathPreferences.h"
#include "CClientLockManager.h"
#include "CDAVTableApp.h"
#include "CDAVTableWindow.h"
#include "CAboutBox.h"
#ifndef __PreferencesDialog_h__
#include "PreferencesDialog.h"
#endif
#ifndef __ConnectionSettingsDialog_h__
#include "ConnectionSettingsDialog.h"
#endif
#ifndef __ClassRegistration_h__
#include "ClassRegistration.h"
#endif
#ifndef __CDAVERRORUTILS_h__
#include "CDAVErrorUtils.h"
#endif
#ifndef __GoliathSSLCertCallback_h__
#include "GoliathSSLCertCallback.h"
#endif
#ifndef __SSLCertificateDialog_h__
#include "SSLClientCertDialog.h"
#endif
#ifndef __ExternalEditorManager_h__
#include "ExternalEditorManager.h"
#endif
#ifndef __ExternalEditorUI_h__
#include "ExternalEditorUI.h"
#endif

#if DAVLIB_SSL
#include <OpenSSLGlue.h>
#endif

#if PP_Target_Carbon
#include <CScrollWheelAttachment.h>
#endif

#if PP_Debug
	#include <LDebugMenuAttachment.h>
	#include <LCommanderTree.h>
	#include <LPaneTree.h>
	#include <LTreeWindow.h>
	#include <PP_DebugConstants.h>
#endif
#include <UDebugUtils.h>
#include <UMemoryEater.h>
#include <PP_DebugMacros.h>

#if GOLIATH_PROFILE
#include <Profiler.h>
#endif

#include <exception>
#include <string.h>
#include <stdio.h>



CDAVTableApp *mAppInstance=NULL;
UMainThread  *mMainThread=NULL;

const char* GoliathAppVersion="1.0";

AEEventHandlerUPP ShowPreferencesUUP = NULL;



#if !PP_Target_Carbon
static bool IsICAvail() {
	SInt32 response;
	ComponentDescription ICDesc = {kICComponentType, kICComponentSubType,
									0L, 0L, 0L};

	// Gestalt check for Component Mgr and InternetConfig
	bool ICAvailable = false;
	OSErr err = ::Gestalt(gestaltComponentMgr, &response);
		
	if (err == noErr) {
		ICAvailable = (::CountComponents(&ICDesc) > 0);
	}
						
	if (ICAvailable) {
	#ifdef __POWERPC__
		ICAvailable = ((Ptr)ICStart != (Ptr) kUnresolvedCFragSymbolAddress);
    #else
        ICAvailable=true;
    #endif
	}
	return ICAvailable;
}
#endif

// ---------------------------------------------------------------------------
//		¥ main()
// ---------------------------------------------------------------------------
//
int main(void)
{
	
	UEnvironment::InitEnvironment();

#if 0//PP_Debug

		// If under the debugger, use the debugger for everything
		// (throw, signal, DebugNew), else use alerts.
	
	if (AmIBeingMWDebugged()) {

		UDebugging::SetDebugThrow(debugAction_Debugger);
		UDebugging::SetDebugSignal(debugAction_Debugger);

	} else {

			// Not under the MW Debug, so use alerts. If you use
			// another debugger, you could set this up the same way. Point
			// is to use the debugger's facilities when under the debugger,
			// else alerts.
		
		UDebugging::SetDebugThrow(debugAction_Alert);
		UDebugging::SetDebugSignal(debugAction_Alert);

			// Use our own error handler for DebugNew which uses alerts
			// instead of DebugStr calls.
	
		PP_DebugNewInstallPPErrorHandler_();
	}

#else
		// In final build mode so nothing should be seen. These are
		// commented out because gDebugThrow and gDebugSignal are
		// initialized to debugAction_Nothing -- assignment here is
		// unnecessary (but left in as a comment for illustration).
	
	UDebugging::SetDebugThrow(debugAction_Nothing);
	UDebugging::SetDebugSignal(debugAction_Nothing);
	
#endif	

		// Clean up any "leaks" that might have occured at static
		// initialization time.
	{
		SLResetLeaks_();
		//DebugNewForgetLeaks_();
	}

	InitializeHeap(15);

#if PP_Target_Carbon
    UQDGlobals::InitializeToolbox();
#else
	UQDGlobals::InitializeToolbox(&qd);
#endif

	::InitCursor();
	::FlushEvents(everyEvent, nil);

#if !PP_Target_Carbon
	if (!IsICAvail()) {
		LStr255 errMsg("InternetConfig version 2.0 or greater required");

		::ParamText(errMsg, Str_Empty, Str_Empty, Str_Empty);

		::CautionAlert(129, NULL);
		return 1;
	}
#endif

	// Check Debugging environment
#if PP_Debug
//	UDebugUtils::CheckEnvironment();
#endif

	LGrowZone* theGZ = new LGrowZone(20000);
	ValidateObject_(theGZ);
	SignalIf_(theGZ->MemoryIsLow());
	printf("Staring mainthread\n");
	mMainThread = new PP_PowerPlant::UMainThread;

#if GOLIATH_PROFILE
   if (ProfilerInit(collectDetailed,
		bestTimeBase, 100, 20) == noErr) {
#endif

    {
    printf("creating theApp\n");
		CDAVTableApp theApp;
	   	mAppInstance = &theApp;
		theApp.AddAttachment(new PP_PowerPlant::LYieldAttachment());
printf("Going to run\n");
		theApp.Run();
		printf("after run\n");
    }
	// Make sure async tasks get cleaned up. This call is VERY IMPORTANT.
	// (Note: LCleanupTask patches ExitToShell, so things get cleaned up
	// appropriately if you kill the application.)
	PP_PowerPlant::LCleanupTask::CleanUpAtExit();	
	if(ShowPreferencesUUP != nil){
		AERemoveEventHandler (kCoreEventClass,
									kAEShowPreferences ,
									ShowPreferencesUUP,
									false);
	#if (UNIVERSAL_INTERFACES_VERSION < 0x0330)
		DisposeRoutineDescriptor (ShowPreferencesUUP);
	#else
		DisposeAEEventHandlerUPP (ShowPreferencesUUP);
	#endif
	}

#if GOLIATH_PROFILE
    ProfilerDump("\pGoliath-profile.out");
	ProfilerTerm();
	}
#endif

#if PP_Debug

		// This cleanup isn't necessary (they are items that are to
		// remain for the duration of the application's run time. When
		// the app quits and the Process Manager reclaims the heap,
		// the memory occupied by these items will be released). This
		// is just done to keep things like DebugNew and Spotlight
		// quiet.
	
	LMenuBar*	theBar = LMenuBar::GetCurrentMenuBar();
	delete theBar;

	URegistrar::DisposeClassTable();

	LPeriodical::DeleteIdlerAndRepeaterQueues();

	UMemoryEater::DeleteMemoryLists();

	LModelDirector*	theDirector = LModelDirector::GetModelDirector();
	delete theDirector;

	LModelObject::DestroyLazyList();

	UScreenPort::Dispose();
	
	DisposeOf_(theGZ);
	
	LComparator*	theCompare = LComparator::GetComparator();
	delete theCompare;
	
	LLongComparator*	theLongCompare = LLongComparator::GetComparator();
	delete theLongCompare;

#endif

	//DebugNewReportLeaks_();
printf("End of main\n");
	return 0;
}


// ---------------------------------------------------------------------------------
//		¥ CDAVTableApp
// ---------------------------------------------------------------------------------
//	Constructor
CDAVTableApp::CDAVTableApp():mActiveConnections(0), mIsLogging(false), 
   mIsCheckingEditState(false), mStartedUp(false) {
	if (UEnvironment::HasFeature(env_HasAppearance))
		::RegisterAppearanceClient();

	RegisterCustomClasses();
	
		// Preload facilities for the Standard Dialogs
	PP_StandardDialogs::Load();
		// Require at least Navigation Services 1.1. See comments
		// above SetTryNavServices in UConditionalDialogs.cp for why
		// you might wish to do this.
#if PP_StdDialogs_Option == PP_StdDialogs_Conditional
	UConditionalDialogs::SetTryNavServices(0x01108000);
#endif

		// Initialize contextual menus
	//UCMMUtils::Initialize();
	//AddAttachment(NEW LCMAttachment);

	SetSleepTime(1);					// increase responsiveness for Networking
    
    mWindowManager    = new CWindowManager();
    mIconSuiteManager = new CIconSuiteManager();
    printf("Going to create preferences\n");
    mPreferences      = new CGoliathPreferences(LStr255(str_FileNameStrings, str_GoliathPreferences));
    if (mPreferences) {
    printf("Going to init prefs\n");
    mPreferences->Init();
    printf("after prefs init\n");
    }
    
    mLockManager      = new CClientLockManager();
    if (mLockManager) mLockManager->Init();    
    
	new LClipboard();
    
#if DAVLIB_SSL
    _InitDAVLibSSL();
#endif

	std::string goliathUserAgent = "Goliath/";
	goliathUserAgent += GoliathAppVersion;
#if PP_Target_Carbon
	goliathUserAgent += " (Macintosh-Carbon; ";
#else
	goliathUserAgent += " (Macintosh; ";
#endif	
	goliathUserAgent += "PPC)";
	
	CDAVRequest::SetUserAgent(goliathUserAgent.c_str());
	
#if PP_Debug
		// Add the SIOUX console Attachment for debugging purposes
//	AddAttachment(NEW LSIOUXAttachment);
	
	mDebugAttachment = nil;
#endif
	printf("end of app constructor\n");
}




// ---------------------------------------------------------------------------------
//		¥ ~CDAVTableApp
// ---------------------------------------------------------------------------------
//	Destructor

CDAVTableApp::~CDAVTableApp() {
		// Clean up after Standard Dialogs
	PP_StandardDialogs::Unload();

#if PP_Debug
	DisposeOf_(mDebugAttachment);
#endif

}


// ===========================================================================
//		¥ Preferences Menu event handler
// ===========================================================================
static pascal OSErr ShowPreferences( const AppleEvent * /*message*/,
											AppleEvent * /* reply */,
#if (UNIVERSAL_INTERFACES_VERSION < 0x0337)
											UInt32 /* refcon */)
#else
											long /* refcon */)
#endif
{
	DisplayPreferencesDialog();
	return noErr;
}


// ---------------------------------------------------------------------------
//	¥ Initialize												   [protected]
// ---------------------------------------------------------------------------
//	Last chance to initialize Application before processing events

void CDAVTableApp::Initialize() {
	MenuHandle		menu;
	OSErr			err;
	
	//LApplication::Initialize();
	LDocApplication::Initialize();
	menu = GetMenuHandle( kWebMenuID );
	if ( menu == nil ) return;
	
	SetMenuItemModifiers( menu, 1, kMenuShiftModifier);	
	
	SetItemCmd( menu, kDeleteMenuItemID, 0x08 ); // delete key
	err = SetMenuItemKeyGlyph( menu, kDeleteMenuItemID, 0x0A ); // delete key glyph in font

    mWindowManager->SetWindowMenuID(kWindowMenuResID);
    AddAttachment( mWindowManager, nil, false);
    mWindowManager->AddListener(this);
    
   short  vRefNum;
   long   dirID;

   //locate our preferences file; if it doesn't exist, create it
   err = ::FindFolder( kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
                       &vRefNum, &dirID);
                       
   if (err == noErr) {
      LStr255 appListName(str_FileNameStrings, str_GoliathExternalAppListFileName);

      FSSpec extAppSpec;
      FSSpec extFileListSpec;
      
      extAppSpec.vRefNum = vRefNum;
      extAppSpec.parID   = dirID;
      LString::CopyPStr(appListName, extAppSpec.name, sizeof(StrFileName));

      LStr255 editFileListName(str_FileNameStrings, str_GoliathExternalEditListFileName);
      extFileListSpec.vRefNum = vRefNum;
      extFileListSpec.parID   = dirID;
      LString::CopyPStr(editFileListName, extFileListSpec.name, sizeof(StrFileName));

 
      LStr255 externalEditRootName(str_FileNameStrings ,str_GoliathExternalAppEditRootName);
      ExternalEditorManager::InitializeExternalEditMgr(extAppSpec, EDIT_IN_APP_START, EDIT_IN_APP_RANGE, EDITITEM_SUBMENU, externalEditRootName, extFileListSpec);
    }
#if PP_Debug
		// Build the Debug menu. The SDebugInfo structure is
		// manually filled out here to allow ease of customization
		// (this is project stationery ;-)

	SDebugInfo	theDebugInfo;
	
	theDebugInfo.debugMenuID			= MENU_DebugMenu;
	theDebugInfo.debugMenuIconID		= icsX_DebugMenuTitle;

	theDebugInfo.commanderTreePPobID	= PPob_AMLCommanderTreeWindow;
	theDebugInfo.commanderTreePaneID	= TreeWindow_Tree;
	theDebugInfo.commanderTreeThreshold	= 1;	// Every 1 second
	
	theDebugInfo.paneTreePPobID			= PPob_AMLPaneTreeWindow;
	theDebugInfo.paneTreePaneID			= TreeWindow_Tree;
	theDebugInfo.paneTreeThreshold		= 0;	// Don't autoupdate
	
	theDebugInfo.validPPobDlogID		= PPob_AMDialogValidatePPob;
	theDebugInfo.validPPobEditID		= ValidatePPob_EditResIDT;
	
	theDebugInfo.eatMemPPobDlogID		= PPob_AMEatMemoryDialog;
	theDebugInfo.eatMemRadioHandleID	= EatMemoryDialog_RadioHandle;
	theDebugInfo.eatMemRadioPtrID		= EatMemoryDialog_RadioPtr;
	theDebugInfo.eatMemEditID			= EatMemoryDialog_EditAmount;
	theDebugInfo.eatMemRadioGroupID		= EatMemoryDialog_RadioGroup;
	
//	mDebugAttachment = NEW LDebugMenuAttachment(theDebugInfo);
//	ValidateObject_(mDebugAttachment);
//	mDebugAttachment->InitDebugMenu();
			
//	AddAttachment(mDebugAttachment);

#endif

#if PP_Target_Carbon
	SInt32	result;
	err = Gestalt(gestaltMenuMgrAttr, &result);
	if(!err && (result & gestaltMenuMgrAquaLayoutMask)){
		MenuItemIndex item;
		MenuRef menu;
		LMenu	*theMenu = LMenuBar::GetCurrentMenuBar()->FetchMenu(129);	// File Menu
		if(theMenu){
			theMenu->RemoveItem(theMenu->IndexFromCommand(cmd_Quit)-1);
  			theMenu->RemoveItem(theMenu->IndexFromCommand(cmd_Quit));
		}
		GetIndMenuItemWithCommandID( NULL, kHICommandQuit, 1, &menu, &item );
		SetMenuItemCommandKey(menu, item, false, 'Q');
		
		
		GetIndMenuItemWithCommandID( NULL, kHICommandPreferences, 1, &menu, &item );	

	#if (UNIVERSAL_INTERFACES_VERSION < 0x0330)
		ShowPreferencesUUP = NewAEEventHandlerProc (ShowPreferences);
	#else
		ShowPreferencesUUP = NewAEEventHandlerUPP (ShowPreferences);
	#endif
		OSErr err = AEInstallEventHandler(kCoreEventClass, kAEShowPreferences , ShowPreferencesUUP, NULL, false);
		if(err != noErr){
			ShowPreferencesUUP = nil;
		} else {
			theMenu = LMenuBar::GetCurrentMenuBar()->FetchMenu(130);	// Edit Menu
			if(theMenu){
				theMenu->RemoveItem(theMenu->IndexFromCommand(cmd_Preferences)-1);
  				theMenu->RemoveItem(theMenu->IndexFromCommand(cmd_Preferences));
			}
		}
		EnableMenuCommand( menu, kHICommandPreferences );
		SetMenuItemCommandKey(menu, item, false, ';');
	}
	AddAttachment(new CScrollWheelAttachment);
#endif
}



#if DAVLIB_SSL
// ---------------------------------------------------------------------------
//	¥ _InitDAVLibSSL												   [protected]
// ---------------------------------------------------------------------------
//
void CDAVTableApp::_InitDAVLibSSL() {
   OSErr  err;
   short  vRefNum;
   long   dirID;

   //locate our preferences file; if it doesn't exist, create it
   err = ::FindFolder( kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
                       &vRefNum, &dirID);
                       
   if (err != noErr) 
	   return;   

   LStr255 certDBName(str_FileNameStrings, str_GoliathSSLCertDBFileName);

   FSSpec certFile;
   certFile.vRefNum = vRefNum;
   certFile.parID   = dirID;
   LString::CopyPStr(certDBName, certFile.name, sizeof(StrFileName));
   DAVLibSSLCertificateCallback::SetCertificateDatabaseFile(&certFile);

   GoliathSSLCertCallback::RegisterSSLCertCallback();
   GoliathSSLClientCertCallback::RegisterSSLCertCallback();

//   DAVLibClientCertificateCallback::InstallDAVLibSSLClientCertCallback(&gUselessCallback);
}
#endif

// ---------------------------------------------------------------------------------
//		¥ ShowAboutBox()
// ---------------------------------------------------------------------------------
//	
void CDAVTableApp::ShowAboutBox()
{
	LWindow::CreateWindow(win_About, this);
}

// ---------------------------------------------------------------------------------
//		¥ ListenToMessage()
// ---------------------------------------------------------------------------------
//	
void CDAVTableApp::ListenToMessage(MessageT inMessage, void* /*ioParam*/) {
   if ( msg_HTTPStart == inMessage) 
      mActiveConnections++;
   else if ( msg_HTTPEnd == inMessage) 
      mActiveConnections--;
}


// ---------------------------------------------------------------------------------
//		¥ StartUp()
// ---------------------------------------------------------------------------------
//	
void CDAVTableApp::StartUp() {

   
   mIsCheckingEditState = true;
   
   ExternalEditorUI::CheckForExternalEdits();
   
   mIsCheckingEditState = false;
   mStartedUp = true;

   std::string strtAction = GetPreferencesManager()->GetPrefValue(CGoliathPreferences::STARTUP_ACTION);
   if (strcmp(strtAction.c_str(), CGoliathPreferences::STARTUP_ACTION_NEW)==0)
	  ObeyCommand(cmd_New, nil);
   else if (strcmp(strtAction.c_str(), CGoliathPreferences::STARTUP_ACTION_OPEN)==0)
	  ObeyCommand(cmd_Open, nil);
   else if (strcmp(strtAction.c_str(), CGoliathPreferences::STARTUP_ACTION_IDISK)==0)
      ObeyCommand(OPENIDISKCMD, nil);
}


// ---------------------------------------------------------------------------------
//		¥ MakeNewWindow()
// ---------------------------------------------------------------------------------
//	
void CDAVTableApp::MakeNewWindow() {
   DisplaySettingsData inOutData;
   if (!DisplayConnectionSettingsDialog(inOutData))
      return;

   CDAVTableWindow *win = (CDAVTableWindow*)LWindow::CreateWindow(win_FinderListView, this);
   win->SetDisplaySettings(&inOutData);
}

// ---------------------------------------------------------------------------------
//		¥ DisplayErrorDialog
// ---------------------------------------------------------------------------------
//	
void CDAVTableApp::DisplayErrorDialog(LStr255& inErrMsg) {
	PP_PowerPlant::StDialogHandler dialog(1300,  GetApplicationInstance());
	Assert_(dialog.GetDialog() != nil);
    PP_PowerPlant::LStaticText* errorField = dynamic_cast<PP_PowerPlant::LStaticText*>
									(dialog.GetDialog()->FindPaneByID('ERRT'));
	
	ThrowIfNil_ (errorField);
	const char* theError = "The server returned an invalid response.";
		   
	errorField->SetText(inErrMsg.ConstTextPtr(), inErrMsg.Length());
	dialog.GetDialog()->Show();
	while (true) {
		PP_PowerPlant::MessageT hitMessage = dialog.DoDialog();
	    if (hitMessage == PP_PowerPlant::msg_OK)
			break;
	}
}

// ---------------------------------------------------------------------------------
//		¥ LoadConnection
// ---------------------------------------------------------------------------------
//	
void CDAVTableApp::OpenDocument(FSSpec *inSpec) {
   ConnectionDocumentData docData;
   InitConnectionDocumentData(docData);
   
   XML_Error err = ParseConnectionDocument(inSpec, &docData);
   if (XML_ERROR_NONE != err) {
      LStr255 errMsg(str_UIStrings, str_ConnectionDocParseError);
      DisplayErrorDialog(errMsg);
      return;
   }
    
   CDAVTableWindow *win = (CDAVTableWindow*)LWindow::CreateWindow(win_FinderListView, this);
   win->SetDisplaySettings(&docData, inSpec);
}

// ---------------------------------------------------------------------------------
//		¥ GetWindowManager
// ---------------------------------------------------------------------------------
//	
CWindowManager* CDAVTableApp::GetWindowManager() {
   return  mWindowManager;
}

// ---------------------------------------------------------------------------------
//		¥ GetPreferencesManager
// ---------------------------------------------------------------------------------
//
CGoliathPreferences* CDAVTableApp::GetPreferencesManager() {
   return mPreferences;
}

// ---------------------------------------------------------------------------------
//		¥ GetLockManager
// ---------------------------------------------------------------------------------
//
CClientLockManager* CDAVTableApp::GetLockManager() {
   return mLockManager;
}

// ---------------------------------------------------------------------------------
//		¥ _OnPreferences
// ---------------------------------------------------------------------------------
//	
void CDAVTableApp::_OnPreferences() {
	DisplayPreferencesDialog();
}

// ---------------------------------------------------------------------------------
//		¥ ProcessNextEvent
// ---------------------------------------------------------------------------------
//	
void CDAVTableApp::ProcessNextEvent() {
	try {
		LApplication::ProcessNextEvent();
	} catch (const LException& e) {
		CDAVErrorUtils::ReportInternalError(e.GetErrorString());
	} catch (const std::exception& e) {
		CDAVErrorUtils::ReportInternalError();
	} catch (ExceptionCode e) {
		CDAVErrorUtils::ReportInternalError();
	} catch (...) {
		CDAVErrorUtils::ReportInternalError();
	}
}

// ---------------------------------------------------------------------------------
//		¥ ObeyCommand
// ---------------------------------------------------------------------------------
//
void CDAVTableApp::_OnLogHTTPRequests() {


   MenuHandle menu = GetMenuHandle( kFileMenuID );


   if (false == mIsLogging) {
      FSSpec outSpec;

      AlertStdAlertParamRec param;

      param.movable 		= false/* ***teb - maybe someday - true*/;
      param.filterProc 	= nil;
      param.defaultText 	= "\pOk";
      param.cancelText 	= "\pCancel";
      param.otherText 	= nil;
      param.helpButton 	= false;
      param.defaultButton = kAlertStdAlertOKButton;
      param.cancelButton 	= kAlertStdAlertCancelButton;
      param.position 		= 0;
   
      SInt16			itemHit;
		
      LStr255 desc;
      desc.Assign(str_UIStrings, str_ConfirmEnableLog);
      OSErr err=StandardAlert( kAlertNoteAlert/*kAlertCautionAlert*/, desc, "\p"/*nil*/, &param, &itemHit );
   
      if ((noErr != err) || (itemHit ==2))
         return;

      if (CNavServicesUtils::savefileNavSrv(&outSpec, str_UIStrings, str_SelectLogFile,
                                             'TEXT', 'ttxt')) {
         mIsLogging = true;
         LHTTP11Connection::SetLoggingStream(&outSpec);    
         if (nil != menu) {
#if PP_Target_Carbon
            ::CheckMenuItem(menu, kLogMenuResId, mIsLogging);
#else
            ::CheckItem(menu, kLogMenuResId, mIsLogging);
#endif
         }
      }
      
   } else {
         mIsLogging = false;
         LHTTP11Connection::SetLoggingStream(NULL);    
         if (nil != menu) {
#if PP_Target_Carbon
            ::CheckMenuItem(menu, kLogMenuResId, mIsLogging);
#else
            ::CheckItem(menu, kLogMenuResId, mIsLogging);
#endif
         }
   }                      
}                                

// ---------------------------------------------------------------------------------
//		¥ _OnOpeniDisk
// ---------------------------------------------------------------------------------
//	
void CDAVTableApp::_OnOpeniDisk() {
   DisplaySettingsData inOutData;
   if (!DisplayIDiskSettingsDialog(inOutData))
      return;
   
   //***teb - iBackup customization
   if (strstr(inOutData.mHost.c_str(), "ibackup.com")!= NULL)
      inOutData.mPort = 443;
      
   CDAVTableWindow *win = (CDAVTableWindow*)LWindow::CreateWindow(win_FinderListView, this);
   win->SetDisplaySettings(&inOutData);
}

// ---------------------------------------------------------------------------------
//		¥ _OnEditClientCertificate
// ---------------------------------------------------------------------------------
//	
void CDAVTableApp::_OnEditClientCertificate() {
	DisplayClientCertificateDialog();
}

// ---------------------------------------------------------------------------------
//		¥ ObeyCommand
// ---------------------------------------------------------------------------------
//	
Boolean CDAVTableApp::ObeyCommand(CommandT inCommand, void *ioParam)
{
	Boolean cmdHandled = true;

	switch (inCommand)
	{
		case cmd_New:
			MakeNewWindow();
			break;
	    case cmd_Open:
           FSSpec theSpec;
           OSType type = DocTypeCode, type2 = 'DRVW';
           if (!CNavServicesUtils::getfileNavSrv(&theSpec,str_UIStrings, str_SelStateDocument, &type, const_cast<OSType*>(&AppCreatorCode), &type2 ))
              return cmdHandled;
           OpenDocument(&theSpec);
	       break;
	    case cmd_Preferences:
	        _OnPreferences();
	        break;
		case LOGHTTPREQUESTSCMD:
		    _OnLogHTTPRequests();
		    break;
		case OPENIDISKCMD:
			_OnOpeniDisk();
			break;
		case EDITCLIENTCERTIFICATES:
			_OnEditClientCertificate();
			break;
		case EDIT_IN_APP_CFG:
			ExternalEditorUI::EditHelperApps();
			break;
		case EDIT_IN_APP_FILES:
			ExternalEditorUI::ViewLocalEdits();
			break;		
		case EDIT_IN_APP_CHECK_EDT:
		    ExternalEditorUI::CheckForExternalEdits();
		   break;
		default:
			cmdHandled = LApplication::ObeyCommand(inCommand, ioParam);
			break;
	}
	return cmdHandled;
}


// ---------------------------------------------------------------------------
//	¥ HandleAppleEvent												 
// ---------------------------------------------------------------------------
//	Respond to an AppleEvent

void CDAVTableApp::HandleAppleEvent(const AppleEvent& inAppleEvent,
	AppleEvent& outAEReply, AEDesc& outResult, long inAENumber) {
	switch (inAENumber) {
		case ae_OpenDoc:
			DoAEOpenDoc(inAppleEvent, outAEReply, inAENumber);
			break;
			
		default:
			/*LApplication*/LDocApplication::HandleAppleEvent(inAppleEvent, outAEReply,
								outResult, inAENumber);
			break;
	}
}

// ---------------------------------------------------------------------------
//	¥ DoAEOpenDoc											  
// ---------------------------------------------------------------------------
//	
Boolean CDAVTableApp::AttemptQuitSelf( SInt32	/* inSaveOption */) {
	if (mActiveConnections > 0) {
      AlertStdAlertParamRec param;

      param.movable 		= false/* ***teb - maybe someday - true*/;
      param.filterProc 	= nil;
      param.defaultText 	= "\pOk";
      param.cancelText 	= "\pCancel";
      param.otherText 	= nil;
      param.helpButton 	= false;
      param.defaultButton = kAlertStdAlertOKButton;
      param.cancelButton 	= kAlertStdAlertCancelButton;
      param.position 		= 0;
      SInt16			itemHit;
		
      LStr255 desc;
      desc.Assign(str_UIStrings, str_DeleteConfirmText);
      OSErr err=StandardAlert( kAlertCautionAlert, desc, "\p"/*nil*/, &param, &itemHit );
   
      if ((noErr != err) || (itemHit ==2))
         return false;
	}	
   return true;
}

// ---------------------------------------------------------------------------
//	¥ DoAEOpenDoc											  
// ---------------------------------------------------------------------------
//	Respond to an AppleEvent to open a Document
void CDAVTableApp::DoAEOpenDoc(const AppleEvent& inAppleEvent, AppleEvent&/* outAEReply */,
	                         SInt32 inAENumber) {
	StAEDescriptor	docList;
	OSErr		err = ::AEGetParamDesc(&inAppleEvent, keyDirectObject,
							typeAEList, docList);
	ThrowIfOSErr_(err);
	
	OpenDocList(docList, inAENumber);
}

// ---------------------------------------------------------------------------
//	¥ OpenDocList											  
// ---------------------------------------------------------------------------
//	Openthe documents specified in a AE descriptor list
void CDAVTableApp::OpenDocList(const AEDescList&	inDocList, SInt32 inAENumber) {
	SInt32 numDocs;
	OSErr err = ::AECountItems(&inDocList, &numDocs);
	ThrowIfOSErr_(err);
	
	for (SInt32 i = 1; i <= numDocs; i++) {
		AEKeyword	theKey;
		DescType	theType;
		FSSpec		theFileSpec;
		Size		theSize;
		err = ::AEGetNthPtr(&inDocList, i, typeFSS, &theKey, &theType,
							(Ptr) &theFileSpec, sizeof(FSSpec), &theSize);
		ThrowIfOSErr_(err);
		
		if (inAENumber == ae_OpenDoc) {
			OpenDocument(&theFileSpec);
		} 
	}
}


// ---------------------------------------------------------------------------------
//		¥ FindCommandStatus
// ---------------------------------------------------------------------------------
//	
void CDAVTableApp::FindCommandStatus(CommandT inCommand, Boolean &outEnabled, Boolean &outUsesMark, UInt16 &outMark, Str255 outName)
{
	switch (inCommand)
	{
		case cmd_New:
			outEnabled = true;
			break;
	    case cmd_Open:
	        outEnabled = true;
	        break;
	    case cmd_Preferences:
	        outEnabled = true;
	        break;
		case LOGHTTPREQUESTSCMD:
		    outEnabled = true;
		    break;
		case OPENIDISKCMD:
			outEnabled = true;
			break;
		case EDITCLIENTCERTIFICATES:
			outEnabled = true;
			break;
		case EDIT_IN_APP_CFG:
			outEnabled = true;
			break;		
		case EDIT_IN_APP_FILES:
			outEnabled = true;
			break;	
		case EDIT_IN_APP_CHECK_EDT:
		   outEnabled = ExternalEditorUI::HaveAtLeastOneEdit();
		   break;	
		case EDITITEM_SUBMENU:
			outEnabled = true;
			break;
		case LOCKS_SUBMENU:
			outEnabled = true;
			break;
		default:
			LDocApplication::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
			break;
	}
}

// ---------------------------------------------------------------------------------
//		¥ GetApplicationInstance
// ---------------------------------------------------------------------------------
//	
void CDAVTableApp::EventResume(const EventRecord& inMacEvent) {
   LDocApplication::EventResume(inMacEvent);
   
   if (mIsCheckingEditState || !mStartedUp)
      return;
     
   mIsCheckingEditState = true;
   
   ExternalEditorUI::CheckForExternalEdits();
   
   mIsCheckingEditState = false;
}

// ---------------------------------------------------------------------------------
//		¥ GetApplicationInstance
// ---------------------------------------------------------------------------------
//	
CDAVTableApp *GetApplicationInstance() {
   return mAppInstance;
}

// ---------------------------------------------------------------------------------
//		¥ GetMainThreadInstance
// ---------------------------------------------------------------------------------
//	
UMainThread *GetMainThreadInstance() {
   return mMainThread;
}


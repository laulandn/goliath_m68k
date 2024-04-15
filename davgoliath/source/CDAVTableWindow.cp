/* ==================================================================================================
 * CDAVTableWindow.cp															   
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

#include <string.h> 
#include <stdio.h> 
#include <CDAVItem.h>
#include "CDAVTableApp.h"
#include "CDAVTableWindow.h"
#include <UAttachments.h>
#include <UAMModalDialogs.h>
#include <LInPlaceOutlineKeySelector.h>
#include <UReanimator.h>
#include <LBevelButton.h>
#include <LStaticText.h>
#include <UGAColorRamp.h>
#include <LChasingArrows.h>
#include <LProgressBar.h>
#include <CFLVDAVRequest.h>
#include <UThread.h>
#include <CDAVTableComparators.h>
#include "CListDirThread.h"
#include "CUploadFileThread.h"
#include <CCreateCollectionThread.h>
#include "CDAVTableAppConstants.h"
#include <UInternetConfig.h>
#include <CDAVContext.h>
#include "CValidatingListDirThread.h"
#include "CDeleteItemsThread.h"
#include <CWindowManager.h>
#include <CDAVTableItem.h>
#include <CDownloadFileThread.h>
#include <CLockItemsThread.h>
#include <CUnlockItemsThread.h>
#include <CDownloadAndLockItemsThread.h>
#include <LEditText.h>
#include <LCheckBox.h>
#include <LStaticText.h>
#include <LIconControl.h>
#include <UDesktop.h>
#include <LClipboard.h>
#include <UModalDialogs.h>
#include <CDAVTranscodeUtils.h>

#include "ParseConnectionDocument.h"
#include <CDAVHeadingTable.h>
#include <CNavServicesUtils.h>
#include <CHeadingTable.h>
#include <CClientLockManager.h>
#include <CPropDisplayTable.h>
#include "CEditPropertiesThread.h"
#include "CDuplicateItemsThread.h"
#include "CMoveItemsToTrashThread.h"
#include "CCopyMoveThread.h"
#include "CSetItemPropsThread.h"
#include "CDAVItemUtils.h"
#include "CEditItemsThread.h"
#include <CDAVLibUtils.h>
#include <assert.h>
#ifndef __ConnectionSettingsDialog_h__
#include "ConnectionSettingsDialog.h"
#endif
#include "CLiveResizeAttachment.h"
#include "CDAVLibUtils.h"

// ---------------------------------------------------------------------------------
//		¥ CreateFromStream()
// ---------------------------------------------------------------------------------
//
CDAVTableWindow* CDAVTableWindow::CreateFromStream(LStream* inStream) {
	return (new CDAVTableWindow(inStream));

}

// ---------------------------------------------------------------------------------
//		¥ CDAVTableWindow()
// ---------------------------------------------------------------------------------
//
CDAVTableWindow::CDAVTableWindow() : mContext(NULL), mThreadCnt(0), mApp(NULL), mOpeningPropsDlog(false), mEncodeResources(true), mFile(NULL), mChanged(false) {
   _init();
}


// ---------------------------------------------------------------------------------
//		¥ CDAVTableWindow()
// ---------------------------------------------------------------------------------
//
CDAVTableWindow::CDAVTableWindow(LStream* inStream) : LWindow(inStream), mContext(NULL), mThreadCnt(0), mApp(NULL), mOpeningPropsDlog(false), mEncodeResources(true), mFile(NULL),
mChanged(false)  {
   _init();
}

// ---------------------------------------------------------------------------------
//		¥ _init()
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::_init() {
   GetApplicationInstance()->GetWindowManager()->AddListener(this);

   AddToRequiredProperties(LastModified);
   AddToRequiredProperties(ContentLength);
   AddToRequiredProperties(ResourceType);
   AddToRequiredProperties(ContentType);
   AddToRequiredProperties(LockDiscovery);
   //AddToRequiredProperties(mod_davExecutable);   
      
}

// ---------------------------------------------------------------------------------
//		¥ ~CDAVTableWindow()
// ---------------------------------------------------------------------------------
//
CDAVTableWindow::~CDAVTableWindow() {
  if (mContext)
      delete mContext;
   GetApplicationInstance()->GetWindowManager()->RemoveListener(this);
   if (mFile) {
      delete mFile;
   }
}


// ---------------------------------------------------------------------------------
//		¥ SetDisplaySettings()
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::SetWindowName(std::string& inHost, SInt32 inPort, std::string& inPath, bool inForceSecure) {
   LStr255 winName;

   if ((sHTTPSPort == inPort) || (inForceSecure))
      winName.Append(sHTTPSProtocol);
   else
      winName.Append(sHTTPProtocol); 
   
   winName.Append(inHost.c_str());
   if ((inPort != sHTTPPort) && (inPort != sHTTPSPort)) {
      winName.Append(':');
      winName.Append(inPort);
   }  
      
   winName.Append(inPath.c_str());
   SetDescriptor(winName);
}

// ---------------------------------------------------------------------------------
//		¥ SetDisplaySettings()
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::SetDisplaySettings(DisplaySettingsData* inDisplaySettings) {
   SetWindowName(inDisplaySettings->mHost, inDisplaySettings->mPort, inDisplaySettings->mPath, inDisplaySettings->mForceSecure);
   
   SetAppPtr(GetApplicationInstance());
   Show();
   
   if (inDisplaySettings->mDisableAppleDoubleEncoding) {
      DisableAppleEncoding();
   }
   
   CDAVContext *ctx = new CDAVContext (inDisplaySettings->mHost.c_str(), inDisplaySettings->mPort, inDisplaySettings->mForceSecure);
   ctx->SetCookieList (&(GetApplicationInstance()->GetCookieList()));
   
   if (inDisplaySettings->mUseProxy) {
      ctx->SetProxyServer(inDisplaySettings->mProxyServer, inDisplaySettings->mProxyPort);
   } 
	
   if (inDisplaySettings->mUseProxyAuth) {
      ctx->SetProxyCredentials(inDisplaySettings->mProxyUser.c_str(), inDisplaySettings->mProxyPass.c_str());
   }

   if ((inDisplaySettings->mUserName.size() > 0) &&  (inDisplaySettings->mPassWord.size() > 0))
      ctx->SetUserCredentials(inDisplaySettings->mUserName.c_str(), inDisplaySettings->mPassWord.c_str());
	
   SetDAVContext(ctx);
   LoadInitialResource(inDisplaySettings->mPath);

}

// ---------------------------------------------------------------------------------
//		¥ SetDisplaySettings()
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::SetDisplaySettings(ConnectionDocumentData* inDisplaySettings, FSSpec *inSpec) {   
   SetWindowName(inDisplaySettings->mHost, inDisplaySettings->mPort, inDisplaySettings->mResource, inDisplaySettings->mForceSecure);

   SetAppPtr(GetApplicationInstance());
   if (inDisplaySettings->mHasFrameInformation) {
      ResizeWindowTo(inDisplaySettings->mFrameSize.width, inDisplaySettings->mFrameSize.height);
      MoveWindowTo(inDisplaySettings->mFrameLocation.h,  inDisplaySettings->mFrameLocation.v);
   }
   if (inDisplaySettings->mHasColumnInformation)
      SetColumnWidths(inDisplaySettings->mColWidths);
   
   Show();
   CDAVContext *ctx = new CDAVContext (inDisplaySettings->mHost.c_str(), inDisplaySettings->mPort, inDisplaySettings->mForceSecure);
   ctx->SetCookieList (&(GetApplicationInstance()->GetCookieList()));
   
   // Make a new empty document.
   if (inDisplaySettings->mHasProxy) {
	     ctx->SetProxyServer(std::string (inDisplaySettings->mProxyHost.TextPtr(), inDisplaySettings->mProxyHost.Length()), inDisplaySettings->mProxyPort);
   } 
   if (inDisplaySettings->mHasUserCredentials) {
	   ctx->SetUserCredentials(inDisplaySettings->mUser, inDisplaySettings->mPassword);
	}
   if (inDisplaySettings->mHasProxyCredentials) {
       std::string plogin, ppassword;
	   plogin.assign ((const char*) inDisplaySettings->mProxyUser.ConstTextPtr(), inDisplaySettings->mProxyUser.Length());
	   ppassword.assign ((const char*) inDisplaySettings->mProxyPassword.ConstTextPtr(), inDisplaySettings->mProxyPassword.Length());
	   ctx->SetProxyCredentials(plogin.c_str(), ppassword.c_str());
	}
	if (!inDisplaySettings->mEncodeAppleDouble) 
	   DisableAppleEncoding();
	
	assert(mFile == NULL);
	if (mFile)
	   delete mFile;
	mFile = new LFile(*inSpec);
	SetDAVContext(ctx);
	LoadInitialResource(inDisplaySettings->mResource);
}

// ---------------------------------------------------------------------------------
//		¥ UpdatePort()
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::UpdatePort() {
	LWindow::UpdatePort();
	
	if (mFLVTable)
		mFLVTable->DrawOverhang();
}

// ---------------------------------------------------------------------------------
//		¥ SetAppPtr()
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::SetAppPtr(LApplication *app) {
   mApp = app;
}

// ---------------------------------------------------------------------------------
//		¥ FinishCreateSelf()
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::FinishCreateSelf() {
	// set the widths of the table columns by getting the widths of the Bevel buttons
	UInt16 columnWidths[5];
	MessageT valueMsgs[5];

	mHeaderText = reinterpret_cast<LStaticText*>(FindPaneByID('head'));
    
	CDAVHeadingTable *heading = reinterpret_cast<CDAVHeadingTable*>(FindPaneByID( kDAVWindowHeading ));
	mFLVTableHeader = heading;

	mFLVTable = reinterpret_cast<CDAVTable*>(FindPaneByID(kDAVTable));
	
    columnWidths[0] = kDefaultNameColumnWidth;
    columnWidths[1] = kDefaultDateColumnWidth;
    columnWidths[2] = kDefaultSizeColumnWidth;
    columnWidths[3] = kDefaultKindColumnWidth;
    columnWidths[4] = kDefaultLockOwnerColumnWidth;
    
    valueMsgs[0]=kNameColMessage;
    valueMsgs[1]=kDateColMessage;
    valueMsgs[2]=kSizeColMessage;
    valueMsgs[3]=kKindColMessage;
    valueMsgs[4]=kLockOwnerColMessage;

	AddAttachment(new LInPlaceOutlineKeySelector(mFLVTable, msg_AnyMessage, true));
	mFLVTable->Setup(this, 19, 5, columnWidths);
    heading->InsertCols(5, 0, nil, nil, false);

    for (int i=0; i<5; i++) {
        LStr255 hdrText(str_HeaderTitles, i+1);
		// We really should do a GetDrawContents on each of the
		heading->SetColumnHeading( i+1, hdrText, kControlBevelButtonAlignTextCenter, 0);
		heading->SetColWidth(columnWidths[i], i + 1, i + 1);
		heading->SetColumnHeadingValueMessage(i+1, valueMsgs[i]);
		heading->SetColumnListener(i+1, this);
	}

	mFLVTable->AddListener(this);
	mFLVTable->SetRowColor(UGAColorRamp::GetColor(1));
	mFLVTable->SetSortColor(UGAColorRamp::GetColor(2));

    mLockIcon = reinterpret_cast<LIconControl*>(FindPaneByID(kRootLockIcon));
    
	mSortIcon = icn_SortUp;
	
	mArrows = reinterpret_cast<LChasingArrows*>(FindPaneByID(kActivityArrows));
	if (NULL != mArrows)
	   mArrows->Hide();
	
    UReanimator::LinkListenerToBroadcasters(this, this, win_FinderListView);
	_updateHeaderText();

#if PP_Target_Carbon
	AddAttachment( new CLiveResizeAttachment(this) );
#endif
}


// ---------------------------------------------------------------------------------
//		¥ ListenToMessage()
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::ListenToMessage(MessageT inMessage, void *ioParam)
{
	CDAVTableComparator* comp = (CDAVTableComparator*)mFLVTable->GetComparator();
	CDAVTableItem* dItem = NULL;
	std::string origResource;
	switch(inMessage)
	{
	    case msg_TableViewScrolled:
	       if (NULL == mFLVTableHeader)
	          return;
	       ScrollImageArgsStruct* sias =  reinterpret_cast<ScrollImageArgsStruct*>(ioParam);
	       
	       mFLVTableHeader->ScrollImageBy(sias->leftDelta, 0, sias->refresh);  
	       break;
		case kSortMessage:
			LBevelButton *button = (LBevelButton*)FindPaneByID('sort');
			ControlButtonContentInfo controlInfo;
			button->GetContentInfo(controlInfo);
			if (controlInfo.u.resID == icn_SortUp)
				controlInfo.u.resID = icn_SortDown;
			else
				controlInfo.u.resID = icn_SortUp;
			mSortIcon = controlInfo.u.resID;
			button->SetContentInfo(controlInfo);
			CDAVTableComparator::SortDirection sDir = CDAVTableComparator::Ascending;
			
			if (mSortIcon != icn_SortUp)
			   sDir = CDAVTableComparator::Descending;
			
			comp->setSortDirection(sDir);
			mFLVTable->Sort(true/*refresh*/);
			break;
		case msg_RowSingleClicked:
			break;
		case msg_RowDoubleClicked:
               _OnRowDoubleClick(ioParam);
			break;		
		case kNameColMessage:
			comp->setSortColumn(CDAVTableComparator::Name);
			mFLVTable->SetSortColumn(kNameColumn);
			mFLVTable->Sort(true/*refresh*/);
			break;
		case kDateColMessage:
			comp->setSortColumn(CDAVTableComparator::Date);
			mFLVTable->SetSortColumn(kDateColumn);
			mFLVTable->Sort(true/*refresh*/);
			break;
		case kSizeColMessage:
			comp->setSortColumn(CDAVTableComparator::Size);
			mFLVTable->SetSortColumn(kSizeColumn);
			mFLVTable->Sort(true/*refresh*/);
			break;
		case kKindColMessage:
			comp->setSortColumn(CDAVTableComparator::Kind);
			mFLVTable->SetSortColumn(kKindColumn);
			mFLVTable->Sort(true/*refresh*/);
			break;
		case kLockOwnerColMessage:
			comp->setSortColumn(CDAVTableComparator::LockOwner);
			mFLVTable->SetSortColumn(kLockOwnerColumn);
			mFLVTable->Sort(true/*refresh*/);
			break;
	    case msg_DAVItemDeleted:
	           _OnDavItemDeleted(ioParam);
	           _updateHeaderText();
	        break;
	    case msg_HTTPStart:
	       msg_HTTPStartStruct *ss = reinterpret_cast<msg_HTTPStartStruct*>(ioParam);
	       if (ss->originatingWindow == this) 
	          _BeginHTTPTransaction();
	       break;
	    case msg_DAVItemsAdded:
	          msg_DAVItemsAddedStruct* iaStruct= reinterpret_cast<msg_DAVItemsAddedStruct*>(ioParam);
	          if (iaStruct->originatingWindow == this)
	             mFLVTable->AddItems(*iaStruct->items, iaStruct->parent);
	          _updateHeaderText();
	       break;
	    case msg_DAVResetTransaction:
	          msg_DAVResetTransactionStruct *resetStruct = reinterpret_cast<msg_DAVResetTransactionStruct*>(ioParam);
	          if (resetStruct->originatingWindow == this) {
	             TableIndexT idx =  mFLVTable->FindRowForItem(resetStruct->activeItem);
	             if ( LArray::index_Bad == idx)
	                return;   //item is no longer in display
	                
	             resetStruct->activeItem->ResetTransaction();
                 mFLVTable->RefreshItem(resetStruct->activeItem);
	          }
	       break;   
	    case msg_DAVItemChanged:
           msg_DAVItemChangedStruct *changeStruct = reinterpret_cast<msg_DAVItemChangedStruct*>(ioParam);
   
          //check to see if it is possible that this resource is displayed in this
          // window; could be a little more intelligent.
          
          //if (this != changeStruct->originatingWindow)
          //   return;
          dItem = mFLVTable->GetTableItemForResource(&(changeStruct->item->GetHREF()));
          if (NULL != dItem) {
              dItem->SetItem(*(changeStruct->item));

	          mFLVTable->RefreshItem(dItem);
	       }
	       break;
	    case msg_DAVItemRenamed:
           msg_DAVItemRenamedStruct *renameStruct = reinterpret_cast<msg_DAVItemRenamedStruct*>(ioParam);
           dItem = mFLVTable->GetTableItemForResource(&renameStruct->originalHREF);
           if (NULL != dItem) {
              dItem->SetItem(*(renameStruct->item));

	          mFLVTable->RefreshItem(dItem);
	       }
          
	       break;
		case msg_HTTPEnd:
	       msg_HTTPEndStruct *es = reinterpret_cast<msg_HTTPEndStruct*>(ioParam);
	       if (es->originatingWindow == this) 
	          _EndHTTPTransaction();
	        break;
	    case msg_DAVConnectionInfo:
	        msg_DAVConnectionInfoStruct *dcs = reinterpret_cast<msg_DAVConnectionInfoStruct*>(ioParam);
	        assert(dcs);
	        if (dcs->originatingWindow == this)
	        	SetDAVInfo(*dcs->connectionInfo);
	        break;
		default:
			break;
	}
}

// ---------------------------------------------------------------------------------
//		¥ DisableAppleEncoding()
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::DisableAppleEncoding() {
	mEncodeResources = false;
}

// ---------------------------------------------------------------------------------
//		¥ DisableAppleEncoding()
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::EnableAppleEncoding() {
	mEncodeResources = true;
}

// ---------------------------------------------------------------------------------
//		¥ EncodeResources()
// ---------------------------------------------------------------------------------
//
bool CDAVTableWindow::EncodeResources() const {
   return mEncodeResources;
}

// ---------------------------------------------------------------------------------
//		¥ SetDAVContext()
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::SetDAVContext(class CDAVContext *ctx) {
   if (mContext)
      delete mContext;
   mContext = ctx;
}

// ---------------------------------------------------------------------------------
//		¥ GetDAVRequestor()
// ---------------------------------------------------------------------------------
//
CDAVContext *CDAVTableWindow::GetDAVContext() {
   return mContext;
}

// ---------------------------------------------------------------------------------
//		¥ SetDAVInfo()
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::SetDAVInfo(CDAVInfo& info) {
   mDavInfo = info;
}


// ---------------------------------------------------------------------------------
//		¥ GetRequiredProperties()
// ---------------------------------------------------------------------------------
//
CDAVPropertyVector& CDAVTableWindow::GetRequiredProperties() {
   return mRequiredProperties;
}

// ---------------------------------------------------------------------------------
//		¥ AddToRequiredProperties()
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::AddToRequiredProperties(CDAVProperty& newProp) {
   mRequiredProperties.push_back(newProp);
}

 
 
// ---------------------------------------------------------------------------------
//		¥ LoadInitialResource()
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::LoadInitialResource(std::string& res) {
   
   mResource = res;
   if (mResource[mResource.size()-1] != '/')
      mResource+='/';
   
   OnFolderExpansion(mResource, NULL, true);
}

// ---------------------------------------------------------------------------------
//		¥ OnFolderExpansion()
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::OnFolderExpansion(std::string& res, LOutlineItem *pnt, Boolean isInitExpansion) {
   std::string resStr = res;
   
   if (resStr[resStr.size()-1] != '/')
      resStr += '/';
   
   CListDirThread *thread;
   
   if (NULL == pnt)
      thread = new CValidatingListDirThread(mContext, this, resStr, pnt);
   else
      thread = new CListDirThread(mContext, this, resStr, pnt);
   if (NULL == thread)
      return;   
   
   if (!isInitExpansion) {
   CDAVTableWindow *win = GetApplicationInstance()->GetWindowManager()->FindWindowForURI(mContext, resStr);
   if (NULL != win) {
      win->DoClose();      
   }   
   }
   
   thread->Resume();
}

// ---------------------------------------------------------------------------------
//		¥ _BeginHTTPTransaction()
// ---------------------------------------------------------------------------------
//                
void CDAVTableWindow::_BeginHTTPTransaction() {
   StMutex mutex(mAccess);   
   mThreadCnt++;
   if (NULL != mArrows)
      mArrows->Show();
}

// ---------------------------------------------------------------------------------
//		¥ _EndHTTPTransaction
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::_EndHTTPTransaction() {
	StMutex mutex(mAccess);
   mThreadCnt--;
   if (0 == mThreadCnt) {
      if (NULL != mArrows)
         mArrows->Hide();

    // since some menu bar enablement is based on thread status
    // force a menu bar update here
	   if (IsOnDuty() /*&& GetUpdateCommandStatus()*/) {
	       Activate();
			   mApp->UpdateMenus();
	   }
    }
}

// ---------------------------------------------------------------------------------
//		¥ GetBaseResource()
// ---------------------------------------------------------------------------------
//
std::string& CDAVTableWindow::GetBaseResource() {
   return mResource;
}

// ---------------------------------------------------------------------------------
//		¥ UploadFSSpec
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::UploadFSSpecVector(FSSpecVector& theSpecs, CDAVTableItem *pntItem) {
   std::string resource;
   if (NULL == pntItem) {
      resource=mResource;
      if (resource[resource.size()-1]!='/')
         resource+='/';
   } else {
      resource=pntItem->GetItem().GetHREF();
      if (resource[resource.size()-1]!='/')
         resource+='/';
   }
      
   CUploadFileThread *thread = new CUploadFileThread(mContext, this, resource, theSpecs);
   if (NULL == thread)
      return;   //***teb - signal error condition here
   if (NULL != pntItem)
      thread->SetParentOutlineItem(pntItem);
      
   thread->Resume();
}


// ---------------------------------------------------------------------------------
//		¥ UploadFSSpec
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::UploadFSSpec(FSSpec *theSpec, CDAVTableItem *pntItem) {
   std::string resource;
   if (NULL == pntItem) {
      resource=mResource;
      if (resource[resource.size()-1]!='/')
         resource+='/';
   } else {
      resource=pntItem->GetItem().GetHREF();
      if (resource[resource.size()-1]!='/')
         resource+='/';
   }

   CUploadFileThread *thread = new CUploadFileThread(mContext, this, resource, *theSpec);
   if (NULL == thread)
      return;   //***teb - signal error condition here

   if (NULL != pntItem)
      thread->SetParentOutlineItem(pntItem);

   thread->Resume();
}

// ---------------------------------------------------------------------------------
//		¥ SetColumnWidths
// ---------------------------------------------------------------------------------
//	
void CDAVTableWindow::SetColumnWidths(std::vector<UInt16> colWidths) {
  std::vector<UInt16>::iterator iter = colWidths.begin();
  int i=1;
  while (iter != colWidths.end()) {
     reinterpret_cast<CDAVHeadingTable*>(mFLVTableHeader)->SetColWidth(*iter, i, i);
     iter++;
     i++;
  }
}

// ---------------------------------------------------------------------------------
//		¥ ObeyCommand
// ---------------------------------------------------------------------------------
//	
Boolean CDAVTableWindow::ObeyCommand(CommandT inCommand, void *ioParam)
{
	Boolean cmdHandled = true;

   if (EDIT_IN_APP_START <= inCommand && inCommand < EDIT_IN_APP_START + EDIT_IN_APP_RANGE) {
       _OnEditItems(inCommand);   
      return true;
   }

   switch (inCommand) {
   case NEWFOLDERCMD:
       cmdHandled=true;
       _OnNewFolderCommand();
   break; 
   case UPLOADITEMCMD:
       cmdHandled=true;
       _OnUploadNewFile();
   break; 
   case REFRESHVIEWCMD:
       cmdHandled=true;
       _OnRefreshView();
   break; 
   case DELETEITEMSCMD:
       cmdHandled=true;
       _OnDeleteItems();    
   break; 
   case DOWNLOADITEMSCMD:
       cmdHandled=true;
        _OnDownloadItems();
   break; 
   case cmd_Save:
       cmdHandled=true;
       _OnSaveConnection(false);
   break; 
   case cmd_SaveCopyAs:
       cmdHandled=true;
       _OnSaveConnection();
   break; 
   case LOCKITEMSCMD:
       cmdHandled=true;
       _OnLockItems();     
   break; 
   case UNLOCKITEMSCMD:
       cmdHandled=true;
       _OnUnlockItems();     
   break; 
   case LOCKANDDOWNLOADITEMSCMD:
       cmdHandled=true;
       _OnLockAndDownload();    
   break; 
   case DOWNLOADITEMWITHOPTCMD:
       cmdHandled=true;
        _OnDownloadItemWithOptions();    
   break; 
   case UNLOCKANDUPLOADITEMSCMD:
       cmdHandled=true;
       _OnUnlockAndUpload();
   break; 
   case CLAIMLOCKCMD:
       cmdHandled=true;
       _OnClaimLockCommand();
   break; 
   case VIEWLOCKINFOCMD:
       cmdHandled=true;
       _OnViewLockInfoCommand();
   break; 
   case SYNCHRONIZEITEMCMD:
       cmdHandled=true;
      _OnSychronizeItem();
   break; 
   case EDITPROPSCMD:
       cmdHandled=true;
       mOpeningPropsDlog = true;
	   _OnEditProperties();    
   break; 
   case DUPLICATEITEMCMD:
       cmdHandled=true;
       _OnDuplicateItemCmd();       
   break; 
   case MODDAV_SETEXECUTABLETRUE:
       cmdHandled = true;
       _OnSetExecutable(true/*executable*/);
   break;
   case MODDAV_SETEXECUTABLEFALSE:
       cmdHandled = true;
       _OnSetExecutable(false/*not executable*/);
   break;
   case EDITITEM_SUBMENU:
       //cmdHandled = true;
       //_OnEditItems();   
   break;
   case CMD_COPYITEMSURL:
       cmdHandled = true;
       _OnCopyItemURLS();
   break;
   case cmd_Copy:
       cmdHandled=true;
       _OnCutCopy(false);
   break;
   case cmd_Cut:
       cmdHandled=true;
       _OnCutCopy(true);
   break;
   case cmd_Paste:
       cmdHandled=true;
       _OnPaste();
   break;
   case EDITCONNECTIONSETTINGS:
      cmdHandled = true;
      _OnEditConnectionSettings();
   break;
   default:
		cmdHandled = LWindow::ObeyCommand(inCommand, ioParam);
   break;
   } 
	return cmdHandled;
}



// ---------------------------------------------------------------------------------
//		¥ FindCommandStatus
// ---------------------------------------------------------------------------------
//	
void CDAVTableWindow::FindCommandStatus(CommandT inCommand, Boolean &outEnabled, Boolean &outUsesMark, 
                                        UInt16 &outMark, Str255 outName) {
   if (EDIT_IN_APP_START <= inCommand && inCommand < EDIT_IN_APP_START + EDIT_IN_APP_RANGE) {
      outEnabled = mFLVTable->atLeastOneItemSelectedIs(CDAVTable::FILE);
      return;
   }
   
   switch (inCommand) {
      case NEWFOLDERCMD:
         outEnabled = true;
	  break;
      case UPLOADITEMCMD:
         outEnabled = true;
      break;
      case REFRESHVIEWCMD:
         outEnabled = (mThreadCnt==0);
      break;
      case DELETEITEMSCMD:
         outEnabled = mFLVTable->atLeastOneItemSelected();
      break;
      case DOWNLOADITEMSCMD:
         outEnabled = mFLVTable->atLeastOneItemSelected();
      break;
      case DOWNLOADITEMWITHOPTCMD:
         outEnabled = mFLVTable->atLeastOneItemSelected();
      break;      
      case LOCKITEMSCMD:
         outEnabled = (((CDAVInfo::DAV_Lock & mDavInfo.getSupportedOperations())!=0) && 
                        (mFLVTable->atLeastOneItemSelectedIs(CDAVTable::UNLOCKED)));
      break;	
      case LOCKANDDOWNLOADITEMSCMD:
         outEnabled = (((CDAVInfo::DAV_Lock & mDavInfo.getSupportedOperations())!=0) && 
                        (mFLVTable->atLeastOneItemSelectedIs(CDAVTable::UNLOCKED)));
      break;	
      case UNLOCKITEMSCMD:
         outEnabled = (((CDAVInfo::DAV_Unlock & mDavInfo.getSupportedOperations())!=0) && 
                      (mFLVTable->atLeastOneItemSelectedIs(CDAVTable::LOCKED)));
      break;		
      case UNLOCKANDUPLOADITEMSCMD:
         outEnabled = (((CDAVInfo::DAV_Unlock & mDavInfo.getSupportedOperations())!=0) && 
                      ( mFLVTable->OnlyOneItemSelected() && mFLVTable->atLeastOneItemSelectedIs(CDAVTable::LOCKED_LOCAL)));
      break;		
      case cmd_Save:
         outEnabled = (NULL == mFile || mChanged);
      break;
      case cmd_SaveCopyAs:
         outEnabled = true;
      break;
      case VIEWLOCKINFOCMD:
         outEnabled = (mFLVTable->OnlyOneItemSelected() && mFLVTable->atLeastOneItemSelectedIs(CDAVTable::LOCKED));      
      break;
      case CLAIMLOCKCMD:
         outEnabled = mFLVTable->atLeastOneItemSelectedIs(CDAVTable::LOCKED_NOTLOCAL);      
      break;
      case SYNCHRONIZEITEMCMD:
         outEnabled = (mFLVTable->OnlyOneItemSelected() && mFLVTable->atLeastOneItemSelectedIs(CDAVTable::LOCKED));
      break;
      case EDITPROPSCMD:
         outEnabled = (mFLVTable->OnlyOneItemSelected() && mOpeningPropsDlog==false);
      break;
      case DUPLICATEITEMCMD:
        outEnabled = mFLVTable->OnlyOneItemSelected();
      break;
      case MODDAV_SETEXECUTABLETRUE:
         outEnabled = (mDavInfo.getHasExecutableSupport() && mFLVTable->atLeastOneItemSelectedIs(CDAVTable::FILE));
      break;
      case MODDAV_SETEXECUTABLEFALSE:
         outEnabled = (mDavInfo.getHasExecutableSupport() && mFLVTable->atLeastOneItemSelectedIs(CDAVTable::FILE));
      break;
      case CMD_COPYITEMSURL:
         outEnabled = mFLVTable->atLeastOneItemSelected();
      break;

      case cmd_Copy:
        outEnabled = mFLVTable->atLeastOneItemSelected();
      break;
      case cmd_Cut:
        outEnabled = mFLVTable->atLeastOneItemSelected();
      break;
      /* 
      //***teb - future feature work disabled for the time being
      case cmd_Paste:			
         outEnabled = false;
         LClipboard* theClipboard = LClipboard::GetClipboard();
         if (theClipboard)
            outEnabled = (theClipboard->GetData(cDAVItemType, nil) != 0);
      break;
         */
      case EDITCONNECTIONSETTINGS:
         outEnabled = (mThreadCnt==0);
      break;
      default:
			LWindow::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
      break;
   }
}


// ---------------------------------------------------------------------------------
//		¥ _OnDavItemDeleted()
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::_OnDavItemDeleted(void *ioParam) {
   msg_DAVItemDeletedStruct *delStruct = reinterpret_cast<msg_DAVItemDeletedStruct*>(ioParam);
   
   //check to see if it is possible that this resource is displayed in this
   // window; could be a little more intelligent.
   if (this != delStruct->originatingWindow)
      if (delStruct->href.find(mResource)!=0)
         return;
      
   CDAVTableItem*  dItem = mFLVTable->GetTableItemForResource(&(delStruct->href));
   if (NULL != dItem)
      mFLVTable->RemoveItem(reinterpret_cast<LOutlineItem*>(dItem), true, true);

}


// ---------------------------------------------------------------------------------
//		¥ SetDAVContext()
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::_OnRowDoubleClick(void *ioParam) {
   CDAVItem *item = (CDAVItem*)ioParam;
   if (item->GetItemType() == CDAVItem::COLLECTION) {
      std::string href = item->GetHREF();
      LStr255 winName = "http://";
      CDAVTableItem*  dItem = mFLVTable->GetTableItemForResource(&href);
      if ((NULL != dItem) && (dItem->IsExpanded())) {
         dItem->Collapse();
      }

      CDAVTableWindow *win = GetApplicationInstance()->GetWindowManager()->FindWindowForURI(mContext, href);
      if (NULL != win) {
         win->Show();
         UDesktop::SelectDeskWindow(win);
      } else {         
         win = (CDAVTableWindow*)LWindow::CreateWindow(win_FinderListView, GetApplicationInstance());
      
         winName.Append(mContext->GetServerName().c_str());
         SInt32 thePort = mContext->GetPort();
         if (80 != thePort) {
            winName.Append(':');
            winName.Append(thePort);
         }  
         winName.Append(href.c_str());
         //***teb - clean up this nonsense
         win->SetDescriptor(winName);
         win->SetAppPtr(mApp);
         win->SetDAVContext(new CDAVContext(*mContext));
         win->mEncodeResources = this->mEncodeResources;
         win->Show();
         win->LoadInitialResource(href);
      }
   } else {
      std::string href = item->GetHREF();
      std::string host=mContext->GetServerName();
      SInt32 port = mContext->GetPort();
	  std::string urlToOpen = (port == 443) ? "https://" : "http://";
	  urlToOpen += mContext->GetServerName();
	  if (port != 80  && port != 443) {
			char tmp[16];
			sprintf (tmp, ":%d", port);
			urlToOpen += tmp;
	  }
      //href should be fully qualified here
      urlToOpen.append(href);
			  
      long tmp=0,tmp2=urlToOpen.length();
      /*ICError*/OSErr err = UInternetConfig::PP_ICLaunchURL("\p", const_cast<char*>(urlToOpen.c_str()), urlToOpen.length(), &tmp, &tmp2);
   }
}

// ---------------------------------------------------------------------------------
//		¥ _OnNewFolderCommand
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::_OnNewFolderCommand() {
   LStr255 folderName;
   Boolean resp;
       
   resp = UAMModalDialogs::AskForOneString(this, NEWFOLDERNAMEDLOG, NEWFOLDEREDITFIELD, folderName);
   if (1 != resp) 
      return ;
       
   
   std::string resource = mResource;

   if (resource[resource.size()-1]!='/')
      resource+='/';

   CDAVItemVector items = mFLVTable->GetAllSelectedItems();
   CDAVItem &item = *items.begin();
   CDAVTableItem *pntItem=NULL;
   
   if ((items.size() == 1) && (item.GetItemType()==CDAVItem::COLLECTION)) {
      pntItem = mFLVTable->GetTableItemForResource(&(item.GetHREF()));
      std::string fname;
      item.GetFileName(fname);
      resource.assign(item.GetHREF());      
      if (resource[resource.size()-1]=='/')
         resource+='/';
   }
   
   std::string folderNameCstr;
   
   folderNameCstr.append(folderName.TextPtr(), folderName.Length());
   folderNameCstr = CDAVTranscodeUtils::TranscodeSystemScriptToUTF8(folderNameCstr);

   resource += folderNameCstr;

   CCreateCollectionThread *thread = new CCreateCollectionThread(mContext, this, resource);
   if (NULL == thread)
      return;    //***teb - signal error condition here
   
   std::string lockToken;
   std::string tmpPntRes = mResource;
   if (tmpPntRes[tmpPntRes.size()-1]!='/')
      tmpPntRes += '/';
   if (GetApplicationInstance()->GetLockManager()->GetLockToken(mContext, tmpPntRes.c_str(), lockToken)) {
      thread->SetParentLock(lockToken.c_str());
   }
   
   thread->SetParentOutlineItem(pntItem);
   
   thread->Resume();
}



// ---------------------------------------------------------------------------------
//		¥ _OnUploadNewFile
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::_OnUploadNewFile() {
   FSSpec theSpec;
              
   if (!CNavServicesUtils::getfileNavSrv(&theSpec,str_UIStrings, str_SelUploadFile, NULL, NULL))
      return;

   CDAVItemVector items = mFLVTable->GetAllSelectedItems();
   CDAVItem &item = *items.begin();
   CDAVTableItem *pntItem=NULL;
   
   if (items.size() == 1 && items.begin()->GetItemType() == CDAVItem::COLLECTION) {
      pntItem = mFLVTable->GetTableItemForResource(&(item.GetHREF()));
   }
   
   UploadFSSpec(&theSpec, pntItem);
}



// ---------------------------------------------------------------------------------
//		¥ _OnRefreshView
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::_OnRefreshView() {
   
   mFLVTable->RemoveAllOutlineItems();
   _updateHeaderText();
   
   std::string tmpCstr(mResource);
   CListDirThread *thread = new CListDirThread(mContext, this, tmpCstr, NULL);
   if (NULL == thread)
      return;   //***teb - signal error condition here
   
   thread->Resume();
}

// ---------------------------------------------------------------------------------
//		¥ _OnDeleteItems
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::_OnDeleteItems() {
   AlertStdAlertParamRec	param;

   CDAVItemVector items;   
   items = mFLVTable->GetAllSelectedItems();

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
		
   LStr255 text, desc;
   text.Assign(str_UIStrings, str_Warning);
   desc.Assign(str_UIStrings, str_DeleteConfirmation);
   OSErr err=StandardAlert( kAlertCautionAlert, desc, "\p"/*nil*/, &param, &itemHit );
   
   if ((noErr != err) || (itemHit ==2))
      return;

   CDeleteItemsThread *thread = new CDeleteItemsThread(mContext, this, items);
   if (NULL == thread)
      return;  //***teb - signal error condition here
      
   thread->Resume();
}



// ---------------------------------------------------------------------------------
//		¥ _OnDownloadItems
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::_OnDownloadItems() {
   FSSpec theSpec;
   CDAVItemVector items;
   
   if (!CNavServicesUtils::getfolderNavSrv(&theSpec, str_UIStrings, str_DownloadSelFolder))
      return;

   items = mFLVTable->GetAllSelectedItems();
   CDownloadFileThread *thread = new CDownloadFileThread(mContext, this, theSpec,items);
   if (NULL == thread)
      return;   //***teb - signal error condition here
      
   thread->Resume();

}

// ---------------------------------------------------------------------------------
//		¥ _OnDownloadItemWithOptions
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::_OnDownloadItemWithOptions() {
   FSSpec theSpec;
   CDAVItemVector items;
   LStr255 urlopts;
   Boolean resp;
   
   if (!CNavServicesUtils::getfolderNavSrv(&theSpec, str_UIStrings, str_DownloadSelFolder))
      return;
      
   //***teb - make this edit field part of the NavSrvcs dlog
   resp = UAMModalDialogs::AskForOneString(this, URLOPTSDLOG, URLOPTSEDITFIELD, urlopts);
   if (1 != resp) 
      return ;

   items = mFLVTable->GetAllSelectedItems();
   CDownloadFileThread *thread = new CDownloadFileThread(mContext, this, theSpec,items);
   if (NULL == thread)
      return;   //***teb - signal error condition here
   
   std::string tmpCstr(urlopts.TextPtr(), urlopts.Length());
   thread->SetURLParameter(tmpCstr);
   thread->Resume();

}

// ---------------------------------------------------------------------------------
//		¥ _OnLockItems
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::_OnLockItems() {
   CDAVItemVector items;
   
   items = mFLVTable->GetAllSelectedItems();
   CLockItemsThread *thread = new CLockItemsThread(mContext, this,items);
   if (NULL == thread)
      return;   //***teb - signal error condition here
      
   thread->Resume();
}




// ---------------------------------------------------------------------------------
//		¥ _OnUnlockItems
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::_OnUnlockItems() {
   CDAVItemVector items;
   
   items = mFLVTable->GetAllSelectedItems();
   CUnlockItemsThread *thread = new CUnlockItemsThread(mContext, this,items);
   if (NULL == thread)
      return;   //***teb - signal error condition here
      
   thread->Resume();
}



// ---------------------------------------------------------------------------------
//		¥ _OnSaveConnection
// ---------------------------------------------------------------------------------
//
void CDAVTableWindow::_OnSaveConnection(bool inSaveAs) {
   SDimension16 frameSize;
   std::vector<UInt16> colWidths;
   Point pt;
   pt.v=pt.h=0;

#if PP_Target_Carbon
   GrafPtr gp = GetMacPort();
   Rect bounds;
   ::GetPortBounds(gp, &bounds);

   frameSize.width = (SInt16)(bounds.right - bounds.left);
   frameSize.height =  (SInt16) (bounds.bottom - bounds.top);
   pt.h = bounds.left;
   pt.v =  bounds.top;
   LocalToGlobal(&pt);
#else   
   GrafPtr gp = GetMacPort();
   frameSize.width = (SInt16)(gp->portRect.right - gp->portRect.left);
   frameSize.height =  (SInt16) (gp->portRect.bottom - gp->portRect.top);
   pt.h = gp->portRect.left;
   pt.v =  gp->portRect.top;
   LocalToGlobal(&pt);
#endif

   colWidths.push_back(mFLVTable->GetColWidth(kNameColumn));
   colWidths.push_back(mFLVTable->GetColWidth(kDateColumn));
   colWidths.push_back(mFLVTable->GetColWidth(kSizeColumn));
   colWidths.push_back(mFLVTable->GetColWidth(kKindColumn));
   colWidths.push_back(mFLVTable->GetColWidth(kLockOwnerColumn));
 
   FSSpec theSpec;
   if (inSaveAs || NULL == mFile) {
      if (! CNavServicesUtils::savefileNavSrv(&theSpec,str_UIStrings, str_SelSaveStateDocument,DocTypeCode, AppCreatorCode))
         return;
      if (mFile) {
         delete mFile;
      }
      mFile = new LFile(theSpec);
   } else {
      mFile->GetSpecifier(theSpec);
   }
   
   WriteConnectionDocument(mResource, mContext, frameSize, pt, &theSpec, &colWidths, mEncodeResources);
}

// ---------------------------------------------------------------------------------
//		¥ _OnCutCopy
// ---------------------------------------------------------------------------------
//	
void CDAVTableWindow::_OnCutCopy(Boolean isCut) {
   LClipboard* theClipboard = LClipboard::GetClipboard();
   SignalIf_(theClipboard == nil);

   CDAVItemVector items;   
   items = mFLVTable->GetAllSelectedItems();
   items = CDAVItemUtils::PruneItemVector(items);
   

   std::string clipData;
   DAVItemData::DAVItemsToClip(items, GetDAVContext(), isCut, clipData);
   SInt32 len = clipData.length();
   Ptr clipdata = const_cast<Ptr>(clipData.c_str());
   theClipboard->SetData(cDAVItemType, clipdata, 
                         len, true/*reset*/);
}




// ---------------------------------------------------------------------------------
//		¥ _OnPaste
// ---------------------------------------------------------------------------------
//	
void CDAVTableWindow::_OnPaste() {
   LClipboard* theClipboard = LClipboard::GetClipboard();
   SignalIf_(theClipboard == nil);
	
   Handle theDataH = ::NewHandle(0);
   SInt32 sz = theClipboard->GetData(cDAVItemType, theDataH);
   StHandleLocker handleLock(theDataH);

   DAVItemData** davItemHandle = reinterpret_cast<DAVItemData**>(theDataH);

   CCopyMoveThread * thread = new CCopyMoveThread(mContext, this,
                                 mResource, reinterpret_cast<DAVItemData*>(*theDataH));
   if (NULL == thread)
      return;  //***teb - signal error here
   
   //***teb - shouldn't handle be released?
   thread->Resume();	
}

// ---------------------------------------------------------------------------------
//		¥ _OnLockAndDownload
// ---------------------------------------------------------------------------------
//	
void CDAVTableWindow::_OnLockAndDownload() {
   CDAVItemVector items;
   FSSpec theSpec;
   
   if (!CNavServicesUtils::getfolderNavSrv(&theSpec, str_UIStrings, str_DownloadSelFolder))
      return;
   items = mFLVTable->GetAllSelectedItems();
   CDownloadAndLockItemsThread *thread = new CDownloadAndLockItemsThread(mContext, this, theSpec, items);
   if (NULL == thread)
      return;  //***teb - signal error here
      
   thread->Resume();

}

// ---------------------------------------------------------------------------------
//		¥ _OnUnlockAndUpload
// ---------------------------------------------------------------------------------
//	
void CDAVTableWindow::_OnUnlockAndUpload() {
   FSSpec theSpec;
              
   if (!CNavServicesUtils::getfileNavSrv(&theSpec,str_UIStrings, str_SelUploadFile, NULL, NULL))
      return;

   CDAVItemVector items = mFLVTable->GetAllSelectedItems();
   CDAVItem &item = *items.begin();
    
   std::string resource(item.GetHREF());

   CUploadFileThread *thread = new CUploadFileThread(mContext, this, resource, theSpec);
   if (NULL == thread)
      return;   //***teb - signal error condition here
   
   if (item.GetIsLocalLock()) {
      std::string token = item.GetLockToken();
      thread->SetLockToken(token, item.GetIsLocalLock(), true);
   }
   
   thread->SetIsResourceFullyQualified(true);
   thread->Resume();
}

// ---------------------------------------------------------------------------------
//		¥ _OnUnlockAndUpload
// ---------------------------------------------------------------------------------
//	
void CDAVTableWindow::_OnClaimLockCommand() {
   CDAVItemVector items;
   CDAVItemVector::iterator iter;
   msg_DAVItemChangedStruct iaStruct;

   items = mFLVTable->GetAllSelectedItems();
   for (iter = items.begin(); iter<items.end(); iter++) {
      if ((iter->GetIsLocked()) && (iter->GetIsLocalLock() == false)) {
         GetApplicationInstance()->GetLockManager()->ItemLocked(mContext, &*iter);
         iter->SetIsLocalLock(true);
         iaStruct.originatingWindow = this;
         iaStruct.item = &*iter;
         GetApplicationInstance()->GetWindowManager()->BroadcastMessage(msg_DAVItemChanged, &iaStruct);
      }
   }
}

// ---------------------------------------------------------------------------------
//		¥ _OnViewLockInfoCommand
// ---------------------------------------------------------------------------------
//	
void CDAVTableWindow::_OnViewLockInfoCommand() {
	PP_PowerPlant::StDialogHandler dialog(VIEWLOCKSDLOG, this);
	Assert_(dialog.GetDialog() != nil);
	
	CDAVItemVector items = mFLVTable->GetAllSelectedItems();
    CDAVItem &item = *items.begin();
    
    std::string tmpStr = "http://";
    tmpStr.append (mContext->GetServerName().c_str());
    tmpStr.append (item.GetHREF().c_str());

    PP_PowerPlant::LEditText* uri = dynamic_cast<PP_PowerPlant::LEditText*>
								(dialog.GetDialog()->FindPaneByID(VIEWLOCKSDLOGURLFIELD));
    Assert_(uri != nil);
    uri->SetText(tmpStr.c_str(), tmpStr.size());

    PP_PowerPlant::LStaticText* locktype = dynamic_cast<PP_PowerPlant::LStaticText*>
								(dialog.GetDialog()->FindPaneByID(VIEWLOCKSDLOGTYPE));
    Assert_(locktype != nil);
    tmpStr="Write";
    locktype->SetText(const_cast<char*>(tmpStr.c_str()), tmpStr.size());

    PP_PowerPlant::LStaticText* lockscope = dynamic_cast<PP_PowerPlant::LStaticText*>
								(dialog.GetDialog()->FindPaneByID(VIEWLOCKSDLOGSCOPE));
    Assert_(lockscope != nil);
    if (CDAVItem::EXCLUSIVE == item.GetLockScope())
       tmpStr = "Exclusive";
    else
       tmpStr = "Shared";
       
    lockscope->SetText(const_cast<char*>(tmpStr.c_str()), tmpStr.size());

    PP_PowerPlant::LStaticText* duration = dynamic_cast<PP_PowerPlant::LStaticText*>
								(dialog.GetDialog()->FindPaneByID(VIEWLOCKSDLOGDURATION));
    Assert_(duration != nil);
    const char* dur = item.GetLockTimeout().c_str();
    tmpStr.assign(dur);
    duration->SetText(const_cast<char*>(tmpStr.c_str()), tmpStr.size());

    PP_PowerPlant::LStaticText* token = dynamic_cast<PP_PowerPlant::LStaticText*>
								(dialog.GetDialog()->FindPaneByID(VIEWLOCKSDLOGTOKEN));
    Assert_(token != nil);
    tmpStr.assign(item.GetLockToken().c_str());
    token->SetText(const_cast<char*>(tmpStr.c_str()), tmpStr.size());

    PP_PowerPlant::LStaticText* owner = dynamic_cast<PP_PowerPlant::LStaticText*>
								(dialog.GetDialog()->FindPaneByID(VIEWLOCKSDLOGOWNER));
    Assert_(owner != nil);
    std::string tmp = item.GetLockOwner();
    if (tmp.length()==0)
       tmpStr.assign("Unknown");
    else
       tmpStr.assign(tmp.c_str());
    owner->SetText(const_cast<char*>(tmpStr.c_str()), tmpStr.size());


	dialog.GetDialog()->Show();
	while (true) {
		PP_PowerPlant::MessageT hitMessage = dialog.DoDialog();
		if (hitMessage == PP_PowerPlant::msg_Cancel)
			return;
		else if (hitMessage == PP_PowerPlant::msg_OK)
			break;
	}

}

// ---------------------------------------------------------------------------------
//		¥ _OnSychronizeItem
// ---------------------------------------------------------------------------------
//	
void CDAVTableWindow::_OnSychronizeItem() {
   FSSpec theSpec;
              
   if (!CNavServicesUtils::getfileNavSrv(&theSpec,str_UIStrings, str_SelUploadFile, NULL, NULL))
      return;

   CDAVItemVector items = mFLVTable->GetAllSelectedItems();
   CDAVItem &item = *items.begin();
    
   std::string resource(item.GetHREF());

   CUploadFileThread *thread = new CUploadFileThread(mContext, this, resource, theSpec);
   if (NULL == thread)
      return;   //***teb - signal error condition here
   
   if (item.GetIsLocalLock()) {
      std::string token = item.GetLockToken();
      thread->SetLockToken(token, item.GetIsLocalLock());
   }
   thread->SetIsResourceFullyQualified(true);
   thread->Resume();
   
}

// ---------------------------------------------------------------------------------
//		¥ _OnEditProperties
// ---------------------------------------------------------------------------------
//	
void CDAVTableWindow::_OnEditProperties() {
   CDAVItemVector items = mFLVTable->GetAllSelectedItems();
   CDAVItem &item = *items.begin();
   CEditPropertiesThread *thread = new CEditPropertiesThread(mContext, this, item);
   if (NULL == thread)
      return;   //***teb - signal error condition here
   
   thread->Resume();
}


// ---------------------------------------------------------------------------------
//		¥ _OnDuplicateItemCmd
// ---------------------------------------------------------------------------------
//	
void CDAVTableWindow::_OnDuplicateItemCmd() {
   CDAVItemVector items = mFLVTable->GetAllSelectedItems();
   CDAVItem &item = *items.begin();
   
   std::string pntHref;
   item.GetParentPath(pntHref);
   std::string tmpPntHref(pntHref.c_str());
   
   CDAVTableItem *pntItem = mFLVTable->GetTableItemForResource(&tmpPntHref);
   
   CDuplicateItemsThread *thread = new CDuplicateItemsThread(mContext, this, item, pntItem);
   if (NULL == thread)
      return;   //***teb - signal error condition here
   
   thread->Resume();
}

// ---------------------------------------------------------------------------------
//		¥ _OnCopyItemURLS
// ---------------------------------------------------------------------------------
//	
void CDAVTableWindow::_OnCopyItemURLS() {
   CDAVItemVector items = mFLVTable->GetAllSelectedItems();
   CDAVItemVector::iterator iter = items.begin();
   std::string cbData, hostInfo;
   hostInfo="http://";
   hostInfo+=mContext->GetServerName();
   if (mContext->GetPort() != 80) {
      char buf[10];
      hostInfo+=":";
      sprintf(buf, "%u", mContext->GetPort());
      hostInfo+=buf;
   }
   
   while (iter != items.end() ) {
       cbData+=hostInfo;
       cbData+=CHTTPStringUtils::URLEncodeString(iter->GetURI());
       iter++;
       if (items.end() != iter) 
         cbData+='\r';    
   }
   Boolean clearCB = true;
   ResType rt = FOUR_CHAR_CODE('TEXT');
   Ptr data = const_cast<Ptr>(cbData.c_str());
   SInt32 dataLen = cbData.length();
   LClipboard::GetClipboard()->SetData(rt, data, dataLen, clearCB);

}

// ---------------------------------------------------------------------------------
//		¥ _OnSetExecutable
// ---------------------------------------------------------------------------------
//	
void CDAVTableWindow::_OnSetExecutable(Boolean exec) {
   CDAVItemVector items = mFLVTable->GetAllSelectedItems();
   std::string propVal;
   if (exec)
      propVal="T";
   else
      propVal="F";
      
   CSetItemPropsThread *thread = new CSetItemPropsThread(mContext, this, items,
                                 &mod_davExecutable, propVal);
   if (NULL == thread)
      return;   //***teb - signal error condition here
   
   thread->Resume();

}


// ---------------------------------------------------------------------------------
//		¥ _OnEditItems
// ---------------------------------------------------------------------------------
//	
void CDAVTableWindow::_OnEditItems(CommandT inEditCmd) {
   CDAVItemVector items;
   
   items = mFLVTable->GetAllSelectedItems();
   CEditItemsThread *thread = new CEditItemsThread(mContext, this, items, inEditCmd);
      if (NULL == thread)
      return;  //***teb - signal error here
      
   thread->Resume();


}

// ---------------------------------------------------------------------------------
//		¥ _OnEditConnectionSettings
// ---------------------------------------------------------------------------------
//	
void CDAVTableWindow::_OnEditConnectionSettings() {
	DisplaySettingsData inOutData;
	
	inOutData.mHost = mContext->GetServerName();
	inOutData.mPort = mContext->GetPort();
	inOutData.mForceSecure = mContext->GetForceSecure();
	inOutData.mPath = mResource;

	inOutData.mUserName = mContext->GetLogin();
	inOutData.mPassWord = mContext->GetPassword();
	inOutData.mUseProxy = mContext->GetUsesProxy();
	
	mContext->GetProxyServer(inOutData.mProxyServer);
	inOutData.mProxyPort = mContext->GetProxyPort();
	inOutData.mUseProxyAuth = mContext->GetHasProxyCredentials();
	inOutData.mProxyUser = mContext->GetProxyLogin();
	inOutData.mProxyPass = mContext->GetProxyPassword();
	
	inOutData.mDisableAppleDoubleEncoding = !EncodeResources();
	
	if (!DisplayConnectionSettingsDialog(inOutData, true))
		return;

	mFLVTable->RemoveAllOutlineItems();
	mChanged = true;
	SetDisplaySettings(&inOutData);
}

// ---------------------------------------------------------------------------------
//		¥ _updateHeaderText
// ---------------------------------------------------------------------------------
//	
void CDAVTableWindow::_updateHeaderText() {
   char buf[50];
   
   UInt32 num = mFLVTable->GetItems().GetCount();
   sprintf(buf, "%u items", num);
   mHeaderText->SetText(buf, strlen(buf));
}


	                            
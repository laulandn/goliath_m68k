/* ==================================================================================================
 * CUploadFileThread.cp															   
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
#include <Dialogs.h>
#include <CDAVItem.h>
#include "CDAVTableApp.h"
#include "CDAVTableWindow.h"
#include <LThread.h>
#include <LProgressBar.h>
#include <CDAVRequest.h>
#include <LOutlineItem.h>
#include "CDAVTableItem.h"
#include "CUploadFileThread.h"

#include <CProgressDialog.h>
#include <CWindowManager.h>
#include <CClientLockManager.h> 
#include "CDAVLibUtils.h"
#include "CDAVItemUtils.h"
#include "FileForkUtils.h"
#include <CDAVTranscodeUtils.h>


static bool _isDirectory(FSSpec* theSpec) {
   CInfoPBRec cpb;
   cpb.hFileInfo.ioNamePtr = theSpec->name;
   cpb.hFileInfo.ioVRefNum = theSpec->vRefNum;
   cpb.hFileInfo.ioFDirIndex = 0;
   cpb.hFileInfo.ioDirID = theSpec->parID;

   if (noErr != PBGetCatInfoSync((CInfoPBPtr) &cpb))
      return false;

   return (cpb.hFileInfo.ioFlAttrib & ioDirMask);
}

// ---------------------------------------------------------------------------
//		¥ CUploadFileThread()
// ---------------------------------------------------------------------------
//
CUploadFileThread::CUploadFileThread(CDAVContext *ctx, CDAVTableWindow *wnd, std::string &resource, FSSpec &theSpec)
 :CFLVThread(ctx, wnd), mResource(resource), mSpec(theSpec), mCancel(false), mResourceFullyQualfied(false),
 mHasLockToken(false), mParentItem(NULL), mTotalItemsToUpload(0) {
    mSpecVector.push_back(theSpec);
    mLockMgr = GetApplicationInstance()->GetLockManager();
}


// ---------------------------------------------------------------------------
//		¥ CUploadFileThread()
// ---------------------------------------------------------------------------
//
CUploadFileThread::CUploadFileThread(CDAVContext *ctx, CDAVTableWindow *wnd, std::string &resource, FSSpecVector &theSpecs)
  :CFLVThread(ctx, wnd), mResource(resource), mCancel(false), mResourceFullyQualfied(false), mHasLockToken(false), mHasLocalLock(false),
   mUnlockItem(false), mParentItem(NULL), mTotalItemsToUpload(0) {
   mSpecVector =  theSpecs;
   mLockMgr = GetApplicationInstance()->GetLockManager();
}


// ---------------------------------------------------------------------------
//		¥ CUploadFileThread()
// ---------------------------------------------------------------------------
//
void CUploadFileThread::SetIsResourceFullyQualified(Boolean fq) {
   mResourceFullyQualfied = fq;
}

// ---------------------------------------------------------------------------
//		¥ SetLockToken()
// ---------------------------------------------------------------------------
//
void CUploadFileThread::SetLockToken(std::string& locktoken, Boolean isLocal, Boolean unlockItem) {
   mHasLockToken = true;
   mLockToken = locktoken;
   mHasLocalLock = isLocal;
   mUnlockItem = unlockItem;
}


// ---------------------------------------------------------------------------
//		¥ cancelOperation()
// ---------------------------------------------------------------------------
//
void CUploadFileThread::cancelOperation() {
   mCancel = true;
   if (1 == mTotalItemsToUpload)
      mRequest.CancelTransaction();
}



// ---------------------------------------------------------------------------
//		¥ _executeDAVTransaction()
// ---------------------------------------------------------------------------
//		
void* CUploadFileThread::_executeDAVTransaction() {
   OSErr result = noErr;
   FSSpec *tmpSpec;
   FSSpecVector::iterator iter = mSpecVector.begin(), checkiter = mSpecVector.begin();

   CProgressDialog *progwin = NULL;
   progwin = reinterpret_cast<CProgressDialog*>(LWindow::CreateWindow(1303, GetApplicationInstance()));
   reinterpret_cast<CProgressDialog*>(progwin)->setParams(this, mSpecVector.size());

   
   Boolean willOverwrite = false;
   while ((checkiter != mSpecVector.end()) && (false == mCancel)) {
      CDAVRequest tmpRequest(mRequest); //since there may be a modal dialog displayed before the 
                                        //uploads begin, use a temp here to avoid a socket timeout.
                                        //better soln would be to implement re-connects
      
      Boolean exists;
      std::string resStr;
      if (mResourceFullyQualfied) {
         resStr = mResource;
      } else {
         resStr = mResource.c_str();
         if (resStr[resStr.length()-1]!='/')
       	    resStr +='/';
      
         LStr255 tmpPstr(checkiter->name);
         std::string resName;
         
         resName.append(tmpPstr.ConstTextPtr(), tmpPstr.Length());
         resName = CDAVTranscodeUtils::TranscodeSystemScriptToUTF8(resName);

         resStr += resName;
      }
      
      FSSpec tmpSpec = *checkiter;
      mTotalItemsToUpload += CDAVItemUtils::CalculateTotalTransactions(&tmpSpec);
            
      if ((!willOverwrite) && (CDAVRequest::SUCCESS == tmpRequest.GetResourceExists(*this, resStr, _isDirectory(&tmpSpec), exists))) {
         if (exists) {
             willOverwrite = true;
             //break;
         }                                          
      }
      checkiter++;

   }
   
   if (mCancel)
      return nil;
      
   if (willOverwrite) {
      if (!_doAlert())
         return nil;
   }
   
   if (1 == mTotalItemsToUpload) 
      progwin->StartListening();
      
   progwin->setParams(this, mTotalItemsToUpload);
   
   iter = mSpecVector.begin();
   while ((iter != mSpecVector.end()) && (false == mCancel)) {
      tmpSpec = &*iter;
      iter++;
      
      std::string baseResource;
   	  baseResource = mResource.c_str();
      if (!mResourceFullyQualfied && baseResource[baseResource.size()-1]!='/')
       	 baseResource.append("/");
      
      _uploadFSSpec(tmpSpec, baseResource, true, progwin);
      
      if (mCancel)
         break;
   }   

   if (progwin) {
      progwin->Hide();
      delete progwin;
   }
   
   LCommander *cmdr = GetApplicationInstance()->GetTarget();
   if (cmdr)
      cmdr->RestoreTarget(); 
      
   return nil;
}


// ---------------------------------------------------------------------------
//		¥ _ProcessFolder()
// ---------------------------------------------------------------------------
//		
Boolean CUploadFileThread::_ProcessFolder(FSSpec* theSpec, const std::string& inResource, Boolean message, CProgressDialog* progwin) {
   CInfoPBRec cpb;
   cpb.hFileInfo.ioNamePtr = theSpec->name;
   cpb.hFileInfo.ioVRefNum = theSpec->vRefNum;
   cpb.hFileInfo.ioFDirIndex = 0;
   cpb.hFileInfo.ioDirID = theSpec->parID;

   if (noErr != PBGetCatInfoSync((CInfoPBPtr) &cpb))
      return false;

   OSErr result = noErr;

   CDAVItem item;
   std::string fldrName = inResource;
   LStr255 nameTemp(theSpec->name);
   fldrName.append(nameTemp.ConstTextPtr(), nameTemp.Length());
   fldrName = CDAVTranscodeUtils::TranscodeSystemScriptToUTF8(fldrName);

   Boolean exists, doNext=false;
   CDAVRequest::ReqStatus reqStat = mRequest.GetResourceExists(*this, fldrName, _isDirectory(theSpec), exists);
   if ((CDAVRequest::SUCCESS == reqStat) && (exists))
      doNext = true;

   if ((CDAVRequest::SUCCESS == reqStat && !exists) && (CDAVRequest::SUCCESS == mRequest.CreateDirectory(*this, fldrName))){
      item.SetItemType(CDAVItem::COLLECTION);
      std::string href(fldrName);
      href.append("/");
      item.SetHREF(href);

      CDAVPropertyVector props = mWnd->GetRequiredProperties();
   
      mRequest.GetItemProperties(*this, item, props);
    
      if (progwin)
         progwin->incrementProgBar();
      if (mCancel)
         return false;

      if (message)
         _itemAdded(item, true);
      doNext = true;
   }

   if (doNext) {
      fldrName.append("/");
      short index = 1;
      long dirID =  cpb.dirInfo.ioDrDirID;
      do {
         cpb.dirInfo.ioFDirIndex = index;
	     cpb.dirInfo.ioDrDirID = dirID;
	     result = PBGetCatInfoSync(&cpb);
	     
	     if (result == noErr) {
	        FSSpec nestSpec;
	        LString::CopyPStr(cpb.dirInfo.ioNamePtr, nestSpec.name, sizeof(StrFileName));
	        nestSpec.vRefNum = cpb.dirInfo.ioVRefNum;
	        nestSpec.parID = cpb.dirInfo.ioDrParID;
	        _uploadFSSpec(&nestSpec, fldrName, false, progwin);
	     }
         if (mCancel)
            return false;
	     ++index;	
	  } while (noErr == result);
      return true;
   } else {
      return false;
   }
}



// ---------------------------------------------------------------------------
//		¥ _ProcessItem()
// ---------------------------------------------------------------------------
//		
Boolean CUploadFileThread::_ProcessFile(FSSpec* theSpec, const std::string& inResource, Boolean message, CProgressDialog* progwin) {
   CDAVItem item;
   OSErr result = noErr;
   Boolean resourceCreated;
    
   std::string resName = inResource;   
 
 
   if (false == mResourceFullyQualfied)  {
      LStr255 nameTmp(theSpec->name);
      std::string tempResName;
      tempResName.append(nameTmp.ConstTextPtr(), nameTmp.Length());
      tempResName = CDAVTranscodeUtils::TranscodeSystemScriptToUTF8(tempResName);

      resName += tempResName;
   }
   
   std::string tmpRes=resName;
   char *lockToken=NULL, *pntLockToken = NULL;
   std::string tmpToken;
   if ( mHasLockToken) {
      lockToken = const_cast<char*>(mLockToken.c_str());
   } else if (mLockMgr->GetLockToken(&mContext, tmpRes.c_str(),tmpToken)) {
      lockToken = const_cast<char*>(tmpToken.c_str());  
      mHasLockToken = true;    
   }
   
   std::string parentHREF;
   std::string pntLockToken_bs;
   if (CURLStringUtils::GetURIParent(resName, parentHREF)) {
      if (GetApplicationInstance()->GetLockManager()->GetLockToken(&mContext, parentHREF.c_str(), pntLockToken_bs))
         pntLockToken = const_cast<char*>(pntLockToken_bs.c_str());      
   }

   short tmpRefNum;
   long fileSize=0;
   OSErr err = ::FSpOpenRF (theSpec, fsRdPerm, &tmpRefNum);
   if (!err)
      ::GetEOF (tmpRefNum, &fileSize);
   if (0 != tmpRefNum)
      ::FSClose(tmpRefNum);
      
   CDAVRequest::ReqStatus reqStat =  mRequest.PutResource(*this, resName, *theSpec, false, const_cast<const char*>(lockToken),
                                                     const_cast<const char*>(pntLockToken), &resourceCreated, 
                                                     progwin);
                                                     
   if (CDAVRequest::SUCCESS !=reqStat) {
      if (CDAVRequest::CANCEL == reqStat) {
         mRequest.CDAVRequest::DeleteResource(*this, resName);
      }
      return false;
   } 

   if (mUnlockItem) {
      mRequest.UnlockResource(*this, resName.c_str(), mLockToken.c_str());
   }
   
   std::string href(resName);
   
   //***teb - ok, some API brittleness here.  CDAVItem::SetItemType must
   // be  called prior to calling CDAVItem::SetHREF.  Not pretty and
   // should be fixed
   item.SetItemType(CDAVItem::FILE);
   item.SetHREF(href);

   if (!_EncodingDisabled()) {
   
      std::string outRezForkURI;
      CDAVItemUtils::GetURIForAppleDoubleResourceFork( item, outRezForkURI);
      bool unlockRezFork = false;
      lockToken = NULL;
      if (mLockMgr->GetLockToken(&mContext, outRezForkURI.c_str(), tmpToken)) {
         lockToken = const_cast<char*>(tmpToken.c_str());     
         if (mUnlockItem)
      	   unlockRezFork = true;
      } else {
      	pntLockToken = NULL;
      }
   
       FSSpec tempSpec;
       CDAVItemUtils::GetATempFile(tempSpec);
       ::FSpCreate (&tempSpec, '????', '????', smSystemScript);
       Boolean rezResourceCreated;
       
	   BuildAppleDoubleRezFile(*theSpec, tempSpec);
       reqStat =  mRequest.PutResource(*this, outRezForkURI, tempSpec, false, const_cast<const char*>(lockToken),
                                                        const_cast<const char*>(pntLockToken), &rezResourceCreated, 
                                                        progwin);
	   ::FSpDelete(&tempSpec);
       if (CDAVRequest::SUCCESS !=reqStat) {
         if (CDAVRequest::CANCEL == reqStat) {
            mRequest.CDAVRequest::DeleteResource(*this, resName);
            mRequest.CDAVRequest::DeleteResource(*this, outRezForkURI);
         }
         return false;
      } 

      if (unlockRezFork) {
         mRequest.UnlockResource(*this, outRezForkURI.c_str(), lockToken);
      }
      if (!_RezForksAreHidden() && message) {
         CDAVItem rezItem;
         rezItem.SetItemType(CDAVItem::FILE);
         rezItem.SetHREF(outRezForkURI);
         CDAVPropertyVector rezprops = mWnd->GetRequiredProperties();
   
         mRequest.GetItemProperties(*this, rezItem, rezprops);
         _itemAdded(rezItem, rezResourceCreated);
      }
   }							
   if (progwin)
      progwin->incrementProgBar();
   if (mCancel)
      return false;

   
   CDAVPropertyVector props = mWnd->GetRequiredProperties();
   
   CDAVRequest::ReqStatus putStat = mRequest.GetItemProperties(*this, item, props);
   if (CDAVRequest::XMLERROR == putStat) {
       _OnXMLRequestError();
       return false;
   }

   if (!mUnlockItem)
      item.SetIsLocalLock(mHasLockToken);
      
   if (message)
      _itemAdded(item, resourceCreated);
   
   return true;
}

// ---------------------------------------------------------------------------
//		¥ _uploadFSSpec()
// ---------------------------------------------------------------------------
//		
Boolean CUploadFileThread::_uploadFSSpec(FSSpec *theSpec, const std::string& inResource, Boolean message, CProgressDialog* progwin) {

   if (_isDirectory(theSpec)) {
      return _ProcessFolder(theSpec, inResource, message, progwin);
   } else {
   	  return _ProcessFile(theSpec, inResource, message, progwin);
   }
}

// ---------------------------------------------------------------------------
//		¥ _itemAdded()
// ---------------------------------------------------------------------------
//		
void CUploadFileThread::_itemAdded(CDAVItem &item, Boolean newResource) {
   
   if ((false == mResourceFullyQualfied) && (newResource)) {
      CDAVItemVector items;
      items.push_back(item);
      msg_DAVItemsAddedStruct iaStruct;
      
      iaStruct.originatingWindow = mWnd;
      iaStruct.items = &items;
      iaStruct.parent = mParentItem;
      GetApplicationInstance()->GetWindowManager()->BroadcastMessage(msg_DAVItemsAdded, &iaStruct);
   } else {
      msg_DAVItemChangedStruct iaStruct;
      
      iaStruct.originatingWindow = mWnd;
      iaStruct.item = &item;
      GetApplicationInstance()->GetWindowManager()->BroadcastMessage(msg_DAVItemChanged, &iaStruct);
   
   }

}

// ---------------------------------------------------------------------------
//		¥ _doAlert()
// ---------------------------------------------------------------------------
//
Boolean CUploadFileThread::_doAlert() {
   LStr255 AlertText(str_UIStrings, str_FileAlreadyExists);
   AlertStdAlertParamRec	param;
   SInt16			itemHit;
   param.movable 		= true;
   param.filterProc 	= nil;
   param.defaultText 	= "\pOK";
   param.cancelText 	= "\pCancel";
   param.helpButton 	= false;
   param.otherText = nil;
   param.defaultButton = kAlertStdAlertOKButton;
   param.cancelButton 	= kAlertStdAlertCancelButton;
   param.position 		= 0;
   OSErr err = StandardAlert( kAlertCautionAlert, StringPtr(AlertText), nil, &param, &itemHit );
   
   if (kAlertStdAlertOKButton != itemHit)
      return false;
      
   return true;
}

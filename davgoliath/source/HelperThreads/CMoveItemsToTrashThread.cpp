/* ==================================================================================================
 * CDeleteItemsThread.cpp															   
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
#include <CDAVRequest.h>
#include <LOutlineItem.h>
#include "CDAVTableItem.h"
#include "CMoveItemsToTrashThread.h"
#include <CWindowManager.h>

#include <CProgressDialog.h>
#include <CDAVItemUtils.h>

#ifndef __FileForkUtils_h__
#include "FileForkUtils.h"
#endif


// ---------------------------------------------------------------------------
//		¥ CMoveItemsToTrashThread()
// ---------------------------------------------------------------------------
//
CMoveItemsToTrashThread::CMoveItemsToTrashThread(CDAVContext *ctx, CDAVTableWindow *wnd, FSSpec &docroot, CDAVItemVector items)
 :CFLVThread(ctx, wnd), mItems(items), mCancel(false), mRootSpec(docroot) {
 
}
		
// ---------------------------------------------------------------------------
//		¥ Run()
// ---------------------------------------------------------------------------
//
void CMoveItemsToTrashThread::cancelOperation() {
   mCancel = true;
}

// ---------------------------------------------------------------------------
//		¥ _executeDAVTransaction()
// ---------------------------------------------------------------------------
//
void* CMoveItemsToTrashThread::_executeDAVTransaction() {
   //SInt16			itemHit;

   LStr255 ok, cancel;
   ok.Assign(str_UIStrings, str_Ok);
   cancel.Assign(str_UIStrings, str_Cancel);
   AlertStdAlertParamRec	param;
   param.movable 		= false/* ***teb - maybe someday - true*/;
   param.filterProc 	= nil;
   param.defaultText 	= "\pOk";
   param.cancelText 	= "\pCancel";
   param.otherText 	= nil;
   param.helpButton 	= false;
   param.defaultButton = kAlertStdAlertOKButton;
   param.cancelButton 	= kAlertStdAlertCancelButton;
   param.position 		= 0;
		
   //***teb - move to resource
   //LStr255 colTmp;
   //            colTmp.Assign(str_UIStrings, str_Collection);
/*   LStr255 text, desc;
   text.Assign(str_UIStrings, str_Warning);
   desc.Assign(str_UIStrings, str_DeleteConfirmation);
   OSErr err=StandardAlert( kAlertCautionAlert, desc, nil, &param, &itemHit );
   
   if ((noErr != err) || (itemHit ==2))
      return nil;
*/
   CDAVItemVector prunedVector = CDAVItemUtils::PruneItemVector(mItems);
   
   CProgressDialog *progwin = NULL;
   if (prunedVector.size() > 1) {
      progwin = reinterpret_cast<CProgressDialog*>(LWindow::CreateWindow(1305, GetApplicationInstance()));
      reinterpret_cast<CProgressDialog*>(progwin)->setParams(this, prunedVector.size());
   }
   
   CWindowManager *winMgr = GetApplicationInstance()->GetWindowManager();


   CDAVItemVector::iterator iter = prunedVector.begin();
   msg_DAVItemDeletedStruct itemDelStruct;
      
   while ((iter != prunedVector.end()) && (false == mCancel)) {
      const std::string &ref = iter->GetHREF();
      if (_downloadResource(&mRootSpec, &*iter)) 
      {
         CDAVRequest::ReqStatus reqStat = mRequest.DeleteResource(*this, *iter);
         if (CDAVRequest::SUCCESS == reqStat) {
            itemDelStruct.href.assign(ref);
            itemDelStruct.originatingWindow = mWnd;
            winMgr->BroadcastMessage(msg_DAVItemDeleted, &itemDelStruct);
         } else if (CDAVRequest::XMLERROR == reqStat){
            _OnXMLRequestError();      
         }
      }
      
      if (progwin)
         progwin->incrementProgBar();
      iter++;
   }
   
   if (progwin) {
      progwin->Hide();
      delete progwin;
   }

   LCommander *elGuapo = GetApplicationInstance()->GetTarget();
   if (elGuapo)
      elGuapo->RestoreTarget(); //eet's a sweater!
      
   return nil;
}


// ---------------------------------------------------------------------------
//		¥ _downloadResource()
// ---------------------------------------------------------------------------
//	
Boolean CMoveItemsToTrashThread::_downloadResource(FSSpec *rootSpec, CDAVItem* inItem) {
   std::string itemHref = inItem->GetHREF();
   UInt8 lastDirDelim;

   if (CDAVItem::COLLECTION == inItem->GetItemType()) {
      itemHref.resize(itemHref.size()-1);
   }

   lastDirDelim = itemHref.rfind('/');
   LStr255 itemName;
   itemName.Assign(itemHref.c_str() + lastDirDelim +1 , itemHref.size() - lastDirDelim - 1);

   FSSpec newSpec(*rootSpec);
   LStr255 uniqItemName = CDAVItemUtils::GetUniqueFileName(itemName, *rootSpec);
   LString::CopyPStr(uniqItemName, newSpec.name, sizeof(StrFileName));

   if (CDAVItem::COLLECTION == inItem->GetItemType()) {
      FSSpec folderSpec(*rootSpec);
      
      long folderID;
      OSErr err = ::FSpDirCreate(&newSpec, smSystemScript, &folderID);
      if (err != noErr)
         return false;
      
      //Recurse the folder
      CDAVItemVector children;
      CDAVItemVector::iterator iter;
      CDAVPropertyVector props;
      
      FSSpec newRoot;
      newRoot.parID = folderID;
      newRoot.vRefNum = rootSpec->vRefNum;
      
      if (CDAVRequest::SUCCESS ==
          mRequest.ListDirectory(*this, inItem->GetHREF(), children, props, NULL)) {
          
          for (iter = children.begin(); iter != children.end(); iter++) {
             _downloadResource(&newRoot, &*iter);
          }
          return true;
      }                      
      return true;
   }
   
   CDAVRequest::ReqStatus reqStat;
   std::string outRezForkURI;
   CDAVItemUtils::GetURIForAppleDoubleResourceFork( *inItem, outRezForkURI);
   Boolean rezForkExists = false;                   
   if (_EncodingDisabled() == false)
      mRequest.GetResourceExists(*this, outRezForkURI, false/*isCollection*/, rezForkExists);
      
   reqStat = mRequest.GetResource(*this, itemHref, newSpec);

   if ((CDAVRequest::SUCCESS != reqStat) && (CDAVRequest::WARNING != reqStat))
      return false;

   if (rezForkExists) {
         FSSpec tmpSpec;
         CDAVItemUtils::GetATempFile(tmpSpec);
         reqStat = mRequest.GetResource(*this, outRezForkURI, tmpSpec, false);
         OSErr err =  ::MergeFlattenedForks(newSpec, tmpSpec); 
         ::FSpDelete(&tmpSpec);
         if (reqStat != CDAVRequest::SUCCESS) {
            ::FSpDelete(&newSpec);
            return false;
         }

   }
   return true;

}


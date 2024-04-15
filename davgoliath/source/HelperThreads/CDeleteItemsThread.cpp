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
#include "CDeleteItemsThread.h"
#include <CWindowManager.h>
#include <CClientLockManager.h>
#include <CProgressDialog.h>
#include <CMoveItemsToTrashThread.h>
#include "CDAVLibUtils.h"
#include "CDAVItemUtils.h"


// ---------------------------------------------------------------------------
//		¥ CDeleteItemsThread()
// ---------------------------------------------------------------------------
//
CDeleteItemsThread::CDeleteItemsThread(CDAVContext *ctx, CDAVTableWindow *wnd, CDAVItemVector items)
 :CFLVThread(ctx, wnd), mItems(items), mCancel(false) {
 
}
		
// ---------------------------------------------------------------------------
//		¥ Run()
// ---------------------------------------------------------------------------
//
void CDeleteItemsThread::cancelOperation() {
   mCancel = true;
}

// ---------------------------------------------------------------------------
//		¥ _executeDAVTransaction()
// ---------------------------------------------------------------------------
//
void* CDeleteItemsThread::_executeDAVTransaction() {
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
		
   LStr255 text, desc;
   text.Assign(str_UIStrings, str_Warning);
   desc.Assign(str_UIStrings, str_DeleteConfirmation);
   /*
   OSErr err=StandardAlert( kAlertCautionAlert, desc, "\p", &param, &itemHit );
   
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
      std::string lockToken;
      const char* ltPtr = NULL;
      if (iter->GetIsLocalLock()) {
         lockToken = iter->GetLockToken();
         ltPtr = lockToken.c_str();
      }
      
      
      std::string pntLockToken;
      char* pntLT = NULL;
      CDAVItem* item = &*iter;
      std::string origHref = item->GetHREF(), tmpPntRes;
      
      if (CURLStringUtils::GetURIParent(origHref, tmpPntRes)) {
          if (GetApplicationInstance()->GetLockManager()->GetLockToken(&mContext, tmpPntRes.c_str(), pntLockToken))
             pntLT = const_cast<char*>(pntLockToken.c_str());      
      }
      
      CDAVRequest::ReqStatus reqStat = mRequest.DeleteResource(*this, *item, ltPtr, const_cast<const char*>(pntLT));
      if (CDAVRequest::SUCCESS == reqStat) {
         if (item->GetIsLocalLock())
            GetApplicationInstance()->GetLockManager()->ItemUnlocked(&mContext, item);
      
         itemDelStruct.href.assign(item->GetHREF());
         itemDelStruct.originatingWindow = mWnd;
         winMgr->BroadcastMessage(msg_DAVItemDeleted, &itemDelStruct);
      } else if (CDAVRequest::XMLERROR == reqStat){
         _OnXMLRequestError();      
      }
      
      if (_RezForksAreHidden() && !_EncodingDisabled()) {
         std::string outRezForkURI;
         CDAVItemUtils::GetURIForAppleDoubleResourceFork( *item, outRezForkURI);
         Boolean rezForkExists = false;                   
         mRequest.GetResourceExists(*this, outRezForkURI, false/*isCollection*/, rezForkExists);
         if (rezForkExists) {
            std::string rezLockToken;
            CDAVItem tmpRezItem(outRezForkURI);
            GetApplicationInstance()->GetLockManager()->GetLockToken(&mContext, outRezForkURI.c_str(), rezLockToken);
            char* rezLTPtr = NULL;
            if (rezLockToken.size() > 0) 
               rezLTPtr = const_cast<char*>(rezLockToken.c_str());
            mRequest.DeleteResource(*this, tmpRezItem, rezLTPtr, const_cast<const char*>(pntLT));
            if (rezLockToken.size() == 0) {
               GetApplicationInstance()->GetLockManager()->ItemUnlocked(&mContext, item);
            }
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


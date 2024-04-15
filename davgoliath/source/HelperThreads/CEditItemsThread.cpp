/* ==================================================================================================
 * CEditItemsThread.cp															   
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

#include <CDAVItem.h>
#include "CDAVTableApp.h"
#include "CDAVTableWindow.h"
#include <LThread.h>
#include <LProgressBar.h>
#include <CDAVRequest.h>
#include <LOutlineItem.h>
#include "CDAVTableItem.h"
#include "CEditItemsThread.h"

#include <CProgressDialog.h>
#include <CWindowManager.h>
#include <CGoliathPreferences.h>
#include <CClientLockManager.h> 
#include <string.h>
#include "CDAVTableWindow.h"

#include "CDAVItemUtils.h"
#include "CDAVErrorUtils.h"

#ifndef __ExternalEditorManager_h__
#include "ExternalEditorManager.h"
#endif

// ---------------------------------------------------------------------------
//		¥ CEditItemsThread()
// ---------------------------------------------------------------------------
//
CEditItemsThread::CEditItemsThread(CDAVContext *ctx, CDAVTableWindow *wnd, 
                                                    CDAVItemVector &theItems, CommandT inEditCmd):
  CFLVThread(ctx, wnd),
  mEditCmd(inEditCmd)
{
  mSupportsLocking = ((wnd->GetCurrentInfo().getSupportedOperations() & CDAVInfo::DAV_Lock) != 0);
   mItemVector =  theItems;
   mOwnerPrefVal = GetApplicationInstance()->GetPreferencesManager()->GetPrefValue(CGoliathPreferences::LOCK_OWNER);
   std::string setowner(GetApplicationInstance()->GetPreferencesManager()->GetPrefValue(CGoliathPreferences::SET_LOCK));   
   if (strcmp(setowner.c_str(), CGoliathPreferences::TRUE_VALUE)==0) {
      mOwnerStr = mOwnerPrefVal.c_str();
   } else
      mOwnerStr = NULL;
   
   mLockMgr = GetApplicationInstance()->GetLockManager();
   
}

// ---------------------------------------------------------------------------
//		¥ _lockResource()
// ---------------------------------------------------------------------------
//
Boolean CEditItemsThread::_lockResource(CDAVItem *theItem) {


   std::string dummy;
   if (mLockMgr->GetLockToken(&mContext, theItem, dummy)) {
      return true;
   }
   
   if (CDAVRequest::SUCCESS ==
          mRequest.LockResource(*this, *theItem, CDAVItem::EXCLUSIVE, mOwnerStr)) {

      CDAVPropertyVector props = mWnd->GetRequiredProperties();

      if (CDAVRequest::SUCCESS == mRequest.GetItemProperties(*this, *theItem, props)) {
         mLockMgr->ItemLocked(&mContext, theItem);
         msg_DAVItemChangedStruct iaStruct;
      
         iaStruct.originatingWindow = mWnd;
         iaStruct.item = theItem;
         GetApplicationInstance()->GetWindowManager()->BroadcastMessage(msg_DAVItemChanged, &iaStruct);
         if (!_EncodingDisabled() && _RezForksAreHidden()) {
         	mRequest.SetSuppressErrorDialog(true);
         	
            std::string outRezForkURI;
            CDAVItemUtils::GetURIForAppleDoubleResourceFork( *theItem, outRezForkURI);
            Boolean rezForkExists = false;           
            CDAVRequest::ReqStatus reqStat;        
			reqStat = mRequest.GetResourceExists(*this, outRezForkURI, false/*isCollection*/, rezForkExists);
            
            if (!rezForkExists)
               return true;
            
            bool rollback = false;
            
            CDAVItem rezForkItem(outRezForkURI);
            rezForkItem.SetItemType(CDAVItem::FILE);
            reqStat = mRequest.LockResource(*this, rezForkItem, CDAVItem::EXCLUSIVE, mOwnerStr);

            if (CDAVRequest::SUCCESS != reqStat) {
            	rollback = true;
            }

			if (!rollback)        
				reqStat = mRequest.GetItemProperties(*this, rezForkItem, props);
				
			if (CDAVRequest::SUCCESS != reqStat) {
            	rollback = true;
            }
            
            if (!rollback) {
            	mLockMgr->ItemLocked(&mContext, &rezForkItem);
            } else {
            	reqStat = mRequest.UnlockResource(*this, *theItem);
           		theItem->ResetLockStatus();

            	ResIDT errStrRes = str_CantLockRezForkAndNoUnlockDataErr;
            	
            	if ((CDAVRequest::SUCCESS == reqStat)|| (CDAVRequest::WARNING == reqStat)) {
            		mLockMgr->ItemUnlocked(&mContext, theItem);
            		errStrRes = str_CantLockRezForkErr;
            	} 
            	
      	        GetApplicationInstance()->GetWindowManager()->BroadcastMessage(msg_DAVItemChanged, &iaStruct);

            	CDAVErrorUtils::DisplayDAVError(str_UIStrings, errStrRes);
          		return false;  	
            }
         }
         return true;
      }
   }
   return false;
}

// ---------------------------------------------------------------------------
//		¥ _downloadResource()
// ---------------------------------------------------------------------------
//	
void* CEditItemsThread::_executeDAVTransaction() {
   CDAVItem *tmpItem;
   CDAVItemVector::iterator iter = mItemVector.begin();
   ExternalEditorManager* editMgr = ExternalEditorManager::GetExternalEditsMgr();
   
   while ((iter != mItemVector.end()) ) {
      tmpItem = &*iter;
      iter++;
      FSSpec editSpec;
      std::string itemHref = tmpItem->GetHREF();

      if (editMgr->GetFSSpecForEditedItem(mContext.GetServerName(), itemHref, editSpec)) {
         std::pair<std::string, FSSpec> thePair(itemHref, editSpec);
         mURLToFSpecMap.push_back(thePair);      
      } else {
         _downloadResource(tmpItem);
      }
   }   
   
   editMgr->EditItems(mEditCmd, &mContext, mURLToFSpecMap, mWnd);
  
   return nil;
}

// ---------------------------------------------------------------------------
//		¥ _downloadResource()
// ---------------------------------------------------------------------------
//	
Boolean CEditItemsThread::_downloadResource(CDAVItem* inItem) {
   Boolean retVal = true;

   if (CDAVItem::COLLECTION == inItem->GetItemType()) 
      return true;

   std::string itemHref = inItem->GetHREF();

   FSSpec tmpSpec;
   ExternalEditorManager::GetExternalEditsMgr()->GetFSSpecForLocalEdit(mContext.GetServerName(), itemHref, tmpSpec);

   if (CDAVItem::COLLECTION == inItem->GetItemType()) {
      itemHref.erase(itemHref.size(), 1);
   }

   if (mSupportsLocking) {
      retVal = _lockResource(inItem);
   } 

   if (retVal) {
      if (CDAVRequest::SUCCESS == mRequest.GetResource(*this, itemHref, tmpSpec)) {
         std::pair<std::string, FSSpec> thePair(itemHref, tmpSpec);
         mURLToFSpecMap.push_back(thePair);
       } else {
         retVal = false;
       }
   }
   
   return retVal;
}


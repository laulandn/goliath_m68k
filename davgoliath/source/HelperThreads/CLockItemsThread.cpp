/* ==================================================================================================
 * CLockItemsThread.cpp															   
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

#ifndef __CDAVITEM_H__
#include <CDAVItem.h>
#endif
#ifndef __CDAVTableApp_h__ 
#include "CDAVTableApp.h"
#endif
#ifndef __CDAVTableWindow_h__
#include "CDAVTableWindow.h"
#endif
#ifndef _H_LThread
#include <LThread.h>
#endif
#ifndef _H_LProgressBar
#include <LProgressBar.h>
#endif
#ifndef __CDAVREQUEST_H__
#include <CDAVRequest.h>
#endif
#ifndef _H_LOutlineItem
#include <LOutlineItem.h>
#endif
#ifndef __CDAVTABLEITEM_H__
#include "CDAVTableItem.h"
#endif
#ifndef __CLOCKITEMSTHREAD_H__
#include "CLockItemsThread.h"
#endif
#ifndef __CDAVITEMUITILS_H__
#include "CDAVItemUtils.h"
#endif
#ifndef __CDAVCONTEXT_H__
#include <CDAVContext.h>
#endif
#ifndef __CWINDOWMANAGER_H__
#include <CWindowManager.h>
#endif
#ifndef __CGOLIATHPREFERENCES_H__
#include <CGoliathPreferences.h>
#endif
#ifndef __CCLIENTLOCKMANAGER_H__
#include <CClientLockManager.h> 
#endif
#ifndef __CDAVERRORUTILS_h__
#include "CDAVErrorUtils.h"
#endif
#include <string.h>

// ---------------------------------------------------------------------------
//		¥ CLockItemsThread()
// ---------------------------------------------------------------------------
// Constructor
//  Parameters
//  ----------
//    ctx - Context object to use in server transaction
//    wnd - window containing the item
CLockItemsThread::CLockItemsThread(CDAVContext *ctx, CDAVTableWindow *wnd, CDAVItemVector& items) 
 :CFLVThread(ctx, wnd), mItemVector(items) {
    mLockMgr = GetApplicationInstance()->GetLockManager();
}
		
		
// ---------------------------------------------------------------------------
//		¥ _executeDAVTransaction()
// ---------------------------------------------------------------------------
//
void* CLockItemsThread::_executeDAVTransaction() {

   CDAVItem *tmpItem;
   CDAVItemVector::iterator iter = mItemVector.begin();
   std::string ownerstr(GetApplicationInstance()->GetPreferencesManager()->GetPrefValue(CGoliathPreferences::LOCK_OWNER));
   std::string setowner(GetApplicationInstance()->GetPreferencesManager()->GetPrefValue(CGoliathPreferences::SET_LOCK));

   char* theOwner=NULL;
   
   if (strcmp(setowner.c_str(), CGoliathPreferences::TRUE_VALUE)==0) {
      theOwner = const_cast<char*>(ownerstr.c_str());
   }
   
   while ((iter != mItemVector.end()) /*&& (false == mCancel)*/) {
      tmpItem = &*iter;
      iter++;
      if (!tmpItem->GetIsLocked())
         _lockResource(tmpItem, theOwner);
   }
   
   return nil;
}

// ---------------------------------------------------------------------------
//		¥ _lockResource()
// ---------------------------------------------------------------------------
//
void CLockItemsThread::_lockResource(CDAVItem* theItem, const char* ownerstr) {
   if (CDAVRequest::SUCCESS ==
          mRequest.LockResource(*this, *theItem, CDAVItem::EXCLUSIVE, ownerstr)) {

      CDAVPropertyVector props = mWnd->GetRequiredProperties();

      if (CDAVRequest::SUCCESS ==mRequest.GetItemProperties(*this, *theItem, props)) {
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
            if (CDAVRequest::SUCCESS != reqStat) {
            	return;
            }
            
            if (!rezForkExists)
               return;
            
            bool rollback = false;
            
            CDAVItem rezForkItem(outRezForkURI);
            rezForkItem.SetItemType(CDAVItem::FILE);
            reqStat = mRequest.LockResource(*this, rezForkItem, CDAVItem::EXCLUSIVE, ownerstr);

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
            	
            }
         }
      }
   }
}
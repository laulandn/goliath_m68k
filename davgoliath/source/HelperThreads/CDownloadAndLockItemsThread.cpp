/* ==================================================================================================
 * CDownloadFileThread.cp															   
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

#include <CDAVItem.h>
#include "CDAVTableApp.h"
#include "CDAVTableWindow.h"
#include <LThread.h>
#include <LProgressBar.h>
#include <CDAVRequest.h>
#include <LOutlineItem.h>
#include "CDAVTableItem.h"
#include "CDownloadAndLockItemsThread.h"
#include "CDAVErrorUtils.h"

#include <CProgressDialog.h>
#include <CWindowManager.h>
#include <CGoliathPreferences.h>
#include <CClientLockManager.h> 
#include "CDAVItemUtils.h"
#include <string.h>

// ---------------------------------------------------------------------------
//		¥ CDownloadAndLockItemsThread()
// ---------------------------------------------------------------------------
//
CDownloadAndLockItemsThread::CDownloadAndLockItemsThread(CDAVContext *ctx, CDAVTableWindow *wnd, 
                                                         FSSpec &rootSpec, CDAVItemVector &theItems)
  :CDownloadFileThread(ctx, wnd, rootSpec, theItems) {

   mOwnerPrefVal = GetApplicationInstance()->GetPreferencesManager()->GetPrefValue(CGoliathPreferences::LOCK_OWNER);
   std::string setowner(GetApplicationInstance()->GetPreferencesManager()->GetPrefValue(CGoliathPreferences::SET_LOCK));   
   if (strcmp(setowner.c_str(), CGoliathPreferences::TRUE_VALUE)==0) {
      mOwnerStr = mOwnerPrefVal.c_str();
   } else
      mOwnerStr = NULL;
   
   mLockMgr = GetApplicationInstance()->GetLockManager();
}



// ---------------------------------------------------------------------------
//		¥ _downloadResource()
// ---------------------------------------------------------------------------
//	
Boolean CDownloadAndLockItemsThread::_downloadResource(FSSpec *rootSpec, CDAVItem* theItem, CProgressDialog *progwin) {

	if (CDAVRequest::SUCCESS == mRequest.LockResource(*this, *theItem, CDAVItem::EXCLUSIVE, mOwnerStr)) {

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
            
				if (rezForkExists) {
            
					bool rollback = false;
            
					CDAVItem rezForkItem(outRezForkURI);
					rezForkItem.SetItemType(CDAVItem::FILE);
					reqStat = mRequest.LockResource(*this, rezForkItem, CDAVItem::EXCLUSIVE, mOwnerStr);

					if (CDAVRequest::SUCCESS != reqStat) 
						rollback = true;
            	
					if (!rollback)        
						reqStat = mRequest.GetItemProperties(*this, rezForkItem, props);
				
					if (CDAVRequest::SUCCESS != reqStat) 
						rollback = true;

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
			}
			return CDownloadFileThread::_downloadResource(rootSpec, theItem, progwin);
		}
	}
	return false;
}


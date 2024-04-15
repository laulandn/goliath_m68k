/* ==================================================================================================
 * CSaveToWebThread.cp															   
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
#include <Dialogs.h>
#include <CDAVItem.h>
#include "CDAVTableApp.h"
#include <LThread.h>
#include <LProgressBar.h>
#include <CDAVRequest.h>
#include <LOutlineItem.h>
#include "CDAVTableItem.h"
#include "CSaveToWebThread.h"

#include <CWindowManager.h>
#include <CClientLockManager.h> 
#include "CDAVLibUtils.h"
#include "CDAVItemUtils.h"
#include "FileForkUtils.h"
#include "CDAVTableWindow.h"
#include "ExternalEditorManager.h"

// ---------------------------------------------------------------------------
//		¥ CSaveToWebThread()
// ---------------------------------------------------------------------------
//
CSaveToWebThread::CSaveToWebThread(CDAVContext *ctx, const CDAVTableWindow *wnd, const std::string &resource, const FSSpec &theSpec, const bool inEncodeResources, const bool inRelinquishEdit)
 :CFLVThread(ctx, const_cast<CDAVTableWindow*>(wnd), inEncodeResources), mResource(resource), mSpec(theSpec), mHasLockToken(false),
 mMutex(NULL),
 mRelinquishLock(inRelinquishEdit)
{
    mLockMgr = GetApplicationInstance()->GetLockManager();
    SetLockToken();
}

// ---------------------------------------------------------------------------
//		¥ SetLockToken()
// ---------------------------------------------------------------------------
//
void CSaveToWebThread::SetLockToken() {
   mHasLockToken = mLockMgr->GetLockToken(&mContext, mResource.c_str(), mLockToken);
}



// ---------------------------------------------------------------------------
//		¥ _executeDAVTransaction()
// ---------------------------------------------------------------------------
//		
void* CSaveToWebThread::_executeDAVTransaction() {

   std::string resName = mResource;   
 
   Boolean resourceCreated;
 
   std::string tmpRes=resName;
   char *lockToken=NULL, *pntLockToken = NULL;
   std::string tmpToken;
   if ( mHasLockToken) {
      lockToken = const_cast<char*>(mLockToken.c_str());
   } else if (mLockMgr->GetLockToken(&mContext, tmpRes.c_str(), tmpToken)) {
      lockToken = const_cast<char*>(tmpToken.c_str());  
      mHasLockToken = true;    
   }
   
   std::string parentHREF;
   std::string pntLockToken_bs;
   if (CURLStringUtils::GetURIParent(resName, parentHREF)) {
      if (GetApplicationInstance()->GetLockManager()->GetLockToken(&mContext, parentHREF.c_str(), pntLockToken_bs))
         pntLockToken = const_cast<char*>(pntLockToken_bs.c_str());      
   }

												
   CDAVRequest::ReqStatus reqStat =  mRequest.PutResource(*this, resName, mSpec, false, const_cast<const char*>(lockToken),
                                                     const_cast<const char*>(pntLockToken), &resourceCreated, 
                                                     NULL);                                                      
   if (CDAVRequest::SUCCESS !=reqStat) {
      if (mMutex)
         mMutex->Signal();
         
      return nil; //***teb handle error here
   }
   
   if (mRelinquishLock && mHasLockToken) {
      reqStat = mRequest.UnlockResource(*this, tmpRes.c_str(), lockToken);
      mLockMgr->ItemUnlocked(&mContext, tmpRes.c_str());      
   }
   
   CDAVItem tmpItem(tmpRes);
   CDAVPropertyVector props;
   props.push_back(LastModified);
   props.push_back(ContentLength);
   props.push_back(ResourceType);
   props.push_back(ContentType);
   props.push_back(LockDiscovery);

   mRequest.GetItemProperties(*this, tmpItem, props);
   msg_DAVItemChangedStruct iaStruct;
      
   iaStruct.originatingWindow = NULL;
   iaStruct.item = &tmpItem;
   GetApplicationInstance()->GetWindowManager()->BroadcastMessage(msg_DAVItemChanged, &iaStruct);

   if (!_EncodingDisabled()) {
   
      std::string outRezForkURI;
      CDAVItem tmpItem(resName);
      CDAVItemUtils::GetURIForAppleDoubleResourceFork( tmpItem, outRezForkURI);
      bool unlockRezFork = false;
      lockToken = NULL;
      if (mLockMgr->GetLockToken(&mContext, outRezForkURI.c_str(), tmpToken)) {
         lockToken = const_cast<char*>(tmpToken.c_str());     
      } else {
      	pntLockToken = NULL;
      }
   
       FSSpec tempSpec;
       CDAVItemUtils::GetATempFile(tempSpec);
       ::FSpCreate (&tempSpec, '????', '????', smSystemScript);
       Boolean rezResourceCreated;
       
	   BuildAppleDoubleRezFile(mSpec, tempSpec);
       reqStat =  mRequest.PutResource(*this, outRezForkURI, tempSpec, false, const_cast<const char*>(lockToken),
                                                        const_cast<const char*>(pntLockToken), &rezResourceCreated, 
                                                        NULL);
      if (CDAVRequest::SUCCESS !=reqStat) {
         if (mMutex)
            mMutex->Signal();
         
         return nil; //***teb handle error here
      }
      
	  if (mRelinquishLock) {
	     reqStat = mRequest.UnlockResource(*this, outRezForkURI.c_str(), lockToken);
         mLockMgr->ItemUnlocked(&mContext, outRezForkURI.c_str());      
         if (!_RezForksAreHidden()) {
            CDAVItem tmpResItem(outRezForkURI);
            mRequest.GetItemProperties(*this, tmpResItem, props);
            iaStruct.item = &tmpResItem;
            GetApplicationInstance()->GetWindowManager()->BroadcastMessage(msg_DAVItemChanged, &iaStruct);
         }
	  }
	   
	  if (mRelinquishLock) {
	     ::FSpDelete(&tempSpec);
      }
   }							

   
   if (mRelinquishLock) {
      ::FSpDelete(&mSpec);
      ExternalEditorManager::GetExternalEditsMgr()->StopEditingItem(mContext.GetServerName(), resName);
   } else {
      ExternalEditorManager::GetExternalEditsMgr()->UpdateItemEditTime(mContext.GetServerName(), resName);
   }
   
   return nil;
}


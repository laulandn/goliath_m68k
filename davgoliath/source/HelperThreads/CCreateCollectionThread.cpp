/* ==================================================================================================
 * CListDirThread.cp															   
 *    Goliath - a Finder like application that implements WebDAV
 *    Copyright (C) 1999-2001 Thomas Bednarz
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
 
#include <CDAVLibUtils.h>
#include <CDAVItem.h>
#include "CDAVTableApp.h"
#include "CDAVTableWindow.h"
#include <LThread.h>
#include <LProgressBar.h>
#include <CDAVRequest.h>
#include <LOutlineItem.h>
#include "CDAVTableItem.h"
#include "CCreateCollectionThread.h"
#include <CWindowManager.h>

// ---------------------------------------------------------------------------
//		¥ CCreateCollectionThread()
// ---------------------------------------------------------------------------
//
CCreateCollectionThread::CCreateCollectionThread(CDAVContext *ctx, CDAVTableWindow *wnd, std::string &resource)
 :CFLVThread(ctx, wnd), mResource(resource),mHasParentLock(false), mParentItem(NULL) {

	if (mResource[mResource.length()]!='/')
		mResource+='/';
}


// ---------------------------------------------------------------------------
//		¥ SetParentLock()
// ---------------------------------------------------------------------------
//
void CCreateCollectionThread::SetParentLock(const char* pntLock) {
   mHasParentLock = true;
   mParentLock = pntLock;
}


// ---------------------------------------------------------------------------
//		¥ _executeDAVTransaction()
// ---------------------------------------------------------------------------
//
void* CCreateCollectionThread::_executeDAVTransaction() {
   
   const char* pntLock;
   if (mHasParentLock)
      pntLock = mParentLock.c_str();
   else
      pntLock = NULL;
      
   CDAVRequest::ReqStatus reqStat;
      
   Boolean exists;
   reqStat = mRequest.GetResourceExists(*this, mResource, true/*Iscollection*/, exists);
   if ((CDAVRequest::SUCCESS == reqStat) && (exists)) {
   	  _doExistsAlert();
      return nil;
   }
   reqStat = mRequest.CreateDirectory(*this, mResource, pntLock);
   if (CDAVRequest::SUCCESS == reqStat) {
   
   } else {
      if (CDAVRequest::XMLERROR == reqStat)
         _OnXMLRequestError();
      return nil;
   }

   CDAVItem item;
   //***teb - href should probably be c-style null terminated
   
   std::string href(mResource);
   // if not trailing '/', add one.
   if (href[href.size()-1]!='/')
      href+='/';

   //***teb - ok, some API brittleness here.  CDAVItem::SetItemType must
   // be  called prior to calling CDAVItem::SetHREF.  Not pretty and
   // should be fixed
   item.SetItemType(CDAVItem::COLLECTION);
   item.SetHREF(href);
   
   CDAVPropertyVector props = mWnd->GetRequiredProperties();
   
   mRequest.GetItemProperties(*this, item, props);
   CDAVItemVector items;
   items.push_back(item);

   msg_DAVItemsAddedStruct iaStruct;
      
   iaStruct.originatingWindow = mWnd;
   iaStruct.items = &items;
   iaStruct.parent = mParentItem;
   GetApplicationInstance()->GetWindowManager()->BroadcastMessage(msg_DAVItemsAdded, &iaStruct);
   
   return nil;
}

// ---------------------------------------------------------------------------
//		¥ _doExistsAlert()
// ---------------------------------------------------------------------------
//
void CCreateCollectionThread::_doExistsAlert() {
   LStr255 AlertText(str_UIStrings, str_FolderAlreadyExists);
   AlertStdAlertParamRec	param;
   SInt16			itemHit;
   param.movable 		= true;
   param.filterProc 	= nil;
   param.defaultText 	= "\pOK";
   param.cancelText 	= nil;
   param.helpButton 	= false;
   param.otherText = nil;
   param.defaultButton = kAlertStdAlertOKButton;
   param.cancelButton 	= 0;
   param.position 		= 0;
   OSErr err = StandardAlert( kAlertCautionAlert, StringPtr(AlertText), nil, &param, &itemHit );
 }

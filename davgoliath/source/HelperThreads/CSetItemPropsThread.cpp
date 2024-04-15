/* ==================================================================================================
 * CSetItemPropsThread.cpp															   
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
#include "CSetItemPropsThread.h"
#include <CDAVContext.h>
#include <CWindowManager.h>
#include <string.h>

// ---------------------------------------------------------------------------
//		¥ CSetItemPropsThread()
// ---------------------------------------------------------------------------
// Constructor
//  Parameters
//  ----------
//    ctx - Context object to use in server transaction
//    wnd - window containing the item
//    FLVTable - table containin the item
CSetItemPropsThread::CSetItemPropsThread(CDAVContext *ctx, CDAVTableWindow *wnd, CDAVItemVector& items,
                                         CDAVProperty *prop, std::string& propval) 
 :CFLVThread(ctx, wnd), mItemVector(items), mProperty(*prop),  mPropVal(propval) {

}
		
		
// ---------------------------------------------------------------------------
//		¥ _executeDAVTransaction()
// ---------------------------------------------------------------------------
//
void* CSetItemPropsThread::_executeDAVTransaction() {

   CDAVItem *tmpItem;
   CDAVItemVector::iterator iter = mItemVector.begin();
   
   while ((iter != mItemVector.end()) /*&& (false == mCancel)*/) {
      tmpItem = &*iter;
      iter++;
      if (tmpItem->GetItemType()==CDAVItem::FILE)
         _setProp(tmpItem);
   }
   
   return nil;
}

// ---------------------------------------------------------------------------
//		¥ _lockResource()
// ---------------------------------------------------------------------------
//
void CSetItemPropsThread::_setProp(CDAVItem* theItem) {
   if (CDAVRequest::SUCCESS ==
          mRequest.SetItemProperty(*this, *theItem, mProperty,  mPropVal)) {

         msg_DAVItemChangedStruct iaStruct;
      
         iaStruct.originatingWindow = mWnd;
         iaStruct.item = theItem;
         GetApplicationInstance()->GetWindowManager()->BroadcastMessage(msg_DAVItemChanged, &iaStruct);
   }
}
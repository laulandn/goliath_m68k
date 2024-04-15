/* ==================================================================================================
 * CDuplicateItemsThread.cpp															   
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
#include "CDuplicateItemsThread.h"

#include <CProgressDialog.h>
#include <CWindowManager.h>
#include <CDAVHeadingTable.h>
#include <CPropDisplayTable.h>
#include <CDAVTableAppConstants.h>
#include <CDAVContext.h>
#include <CDAVLibUtils.h>

const std::string CopyPrefix = "Copy ";

// ---------------------------------------------------------------------------
//		¥ CDuplicateItemsThread()
// ---------------------------------------------------------------------------
//
CDuplicateItemsThread::CDuplicateItemsThread(CDAVContext *ctx, CDAVTableWindow *wnd, CDAVItem& item,
       CDAVTableItem* parentItem) :CFLVThread(ctx, wnd), mParentItem(parentItem) {
    mItem = item;
}

// ---------------------------------------------------------------------------
//		¥ _resourceExists()
// ---------------------------------------------------------------------------
//	
Boolean CDuplicateItemsThread::_resourceExists(std::string &href, Boolean& exists) {
   CDAVRequest::ReqStatus reqStat;
      
   reqStat = mRequest.GetResourceExists(*this, href, mItem.GetItemType() == CDAVItem::COLLECTION, exists);
   if ((CDAVRequest::SUCCESS == reqStat))
      return true;
   return false;
}

// ---------------------------------------------------------------------------
//		¥ _executeDAVTransaction()
// ---------------------------------------------------------------------------
//		
void* CDuplicateItemsThread::_executeDAVTransaction() {
   std::string baseName, pntHref, copyHref;
   mItem.GetFileName(baseName);
   mItem.GetParentPath(pntHref);
   if (!pntHref[pntHref.size()-1]!= '/' && !baseName[0] != '/')
      pntHref.append("/");

   SInt32 i=0;
   while (true) {
      LStr255 tmpName = CopyPrefix.c_str();
      if (i != 0) {
         tmpName.Append(i);
         tmpName.Append(' ');
      }
      tmpName.Append("of ");
      tmpName.Append(baseName.c_str());
      std::string itemNewName;
      itemNewName.assign(tmpName.ConstTextPtr(), tmpName.Length());
      copyHref=pntHref;
      copyHref.append(itemNewName);
      Boolean exists;
      if (false == _resourceExists(copyHref, exists)) {
         return nil;
      } else if (false == exists) {
         break;
      }
      i++;  
   }
   
   CDAVRequest::ReqStatus reqStat;
   SInt32 port = mContext.GetPort();
   LStr255 destURI=(port == 443) ? "https://" : "http://";
   destURI.Append(mContext.GetServerName().c_str());
   if (80 != port) {
      destURI.Append(':');
      destURI.Append(port);
   }
   destURI.Append(CHTTPStringUtils::URLEncodeString(copyHref).c_str());
   
   std::string destURIstr;
   destURIstr.assign(destURI.ConstTextPtr(), destURI.Length());
   reqStat = mRequest.CopyResource(*this, mItem.GetHREF(), destURIstr);
   if (CDAVRequest::SUCCESS != reqStat) {
      return nil;
   }

   CDAVItem item;
   //***teb - href should probably be c-style null terminated
   
   std::string href(copyHref);

   //***teb - ok, some API brittleness here.  CDAVItem::SetItemType must
   // be  called prior to calling CDAVItem::SetHREF.  Not pretty and
   // should be fixed
   item.SetItemType(mItem.GetItemType());
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
/* ==================================================================================================
 * CCopyMoveThread.cpp															   
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
#include "CCopyMoveThread.h"

#include <CProgressDialog.h>
#include <CWindowManager.h>
#include "CDAVLibUtils.h"
#include <CDAVContext.h>

// ---------------------------------------------------------------------------
//		¥ CCopyMoveThread()
// ---------------------------------------------------------------------------
//
CCopyMoveThread::CCopyMoveThread(CDAVContext *ctx, CDAVTableWindow *wnd, 
                                 std::string& baseResource, DAVItemData* cbData):
                                 CFLVThread(ctx, wnd), mCancel(false),
                                 mBaseResource(baseResource) {
                                
   mServer = cbData->server;
   mPort = cbData->port;
   
   mHasProxy = cbData->hasProxy;
   mProxyServer = cbData->proxyserver;
   mProxyPort = cbData->proxyport;
   mHasAuth = cbData->hasAuth;
   mLogin = cbData->login;
   mPassword = cbData->password;
   mIsCut = cbData->isCut;

   mHasProxyAuth;
   mProxyLogin;
   mProxyPassword;   
   
   for (std::list<std::string>::iterator iter = cbData->sourceURIs.begin();
        iter != cbData->sourceURIs.end(); ++iter)
      mResources.push_back(*iter);

}




// ---------------------------------------------------------------------------
//		¥ cancelOperation()
// ---------------------------------------------------------------------------
//
void CCopyMoveThread::cancelOperation() {
   mCancel = true;
}

// ---------------------------------------------------------------------------
//		¥ _executeLocalResource()
// ---------------------------------------------------------------------------
//
Boolean CCopyMoveThread::_executeLocalResource(std::string *resource) {
   LStr255 destURI="http://";
   destURI.Append(mContext.GetServerName().c_str());
   SInt32 port = mContext.GetPort();
   if (80 != port) {
      destURI.Append(':');
      destURI.Append(port);
   }
   if (mBaseResource[0]!='/')
      destURI.Append('/');
   destURI.Append(mBaseResource.c_str());
   if (!destURI.EndsWith('/')) 
      destURI.Append('/');
   
   std::string nodeName;
   
   //LStr255 res = resource->c_str();
   std::string res = resource->c_str();
   if (!CURLStringUtils::GetURINodeName(res, nodeName))
      return false;
    destURI.Append(nodeName.c_str());

   CDAVRequest::ReqStatus reqStat;
   std::string dest, src;
   dest.assign(destURI.TextPtr(), destURI.Length());
   if (mIsCut) 
      reqStat = mRequest.MoveResource(*this, res, dest);
   else
      reqStat = mRequest.CopyResource(*this, res, dest);
   
   if (CDAVRequest::SUCCESS != reqStat) {
      return nil;
   }

   CDAVItem item;
   //***teb - href should probably be c-style null terminated
   
   LStr255 newHref(mBaseResource.c_str());
   if (mBaseResource[0]!='/')
      newHref.Append('/');
   newHref.Append(nodeName.c_str());
   LStr255 tmpStr = newHref;
   std::string href(DAVp2cstr(tmpStr));

   //***teb - ok, some API brittleness here.  CDAVItem::SetItemType must
   // be  called prior to calling CDAVItem::SetHREF.  Not pretty and
   // should be fixed
   if (resource->at(resource->size())=='/')
      item.SetItemType(CDAVItem::COLLECTION);
   else
      item.SetItemType(CDAVItem::FILE);
   
   item.SetHREF(href);
   
   CDAVPropertyVector props = mWnd->GetRequiredProperties();
   
   mRequest.GetItemProperties(*this, item, props);
   CDAVItemVector items;
   items.push_back(item);

   msg_DAVItemsAddedStruct iaStruct;
      
   iaStruct.originatingWindow = mWnd;
   iaStruct.items = &items;
   iaStruct.parent = NULL;
   GetApplicationInstance()->GetWindowManager()->BroadcastMessage(msg_DAVItemsAdded, &iaStruct);
   
   if (mIsCut) {
      msg_DAVItemDeletedStruct idStruct;
      idStruct.href=resource->c_str();
      idStruct.originatingWindow = NULL;
       GetApplicationInstance()->GetWindowManager()->BroadcastMessage(msg_DAVItemDeleted, &idStruct);   
   }
   
   return true;
}


// ---------------------------------------------------------------------------
//		¥ _executeDAVTransaction()
// ---------------------------------------------------------------------------
//		
void* CCopyMoveThread::_executeDAVTransaction() {
   std::list<std::string>::iterator iter = mResources.begin();
   while (iter != mResources.end()) {
      std::string tmpStr = *iter;
      _executeLocalResource(&tmpStr);
      iter++;
   }
  return nil;
}


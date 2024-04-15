/* ==================================================================================================
 * CRenameResourceThread.cpp															   
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
#include "DAVMessages.h"
#include "CWindowManager.h"
#include <LThread.h>
#include <LProgressBar.h>
#include <CDAVRequest.h>
#include <LOutlineItem.h>
#include "CDAVTableItem.h"
#include "CRenameResourceThread.h"
#include <CDAVContext.h>
#include <CDAVLibUtils.h>
#include <CDAVItemUtils.h>

// ---------------------------------------------------------------------------
//		¥ CRenameResourceThread()
// ---------------------------------------------------------------------------
// Constructor
//  Parameters
//  ----------
//    ctx - Context object to use in server transaction
//    wnd - window containing the item
//    newFileName - new name of the node (NOT fully qualified path; i.e. renameTest, not /foo/rename test or http://yadda.org/foo/renameTest
//    theItem - the item in question
CRenameResourceThread::CRenameResourceThread(CDAVContext *ctx, CDAVTableWindow *wnd, std::string &newFileName, CDAVItem *theItem) 
 :CFLVThread(ctx, wnd), mNewResourceName(newFileName), mDAVItem(*theItem) {
}
		
		
// ---------------------------------------------------------------------------
//		¥ _executeDAVTransaction()
// ---------------------------------------------------------------------------
//
void* CRenameResourceThread::_executeDAVTransaction() {
   SInt32 port = mContext.GetPort();
   std::string tmpStr;
   std::string pntPath, newHREF;
   
   newHREF.assign( ((443 == port) || mContext.GetForceSecure()) ? "https://" : "http://");
   newHREF += mContext.GetServerName();
   if (80 != port && 443 != port) {
      LStr255 portStr(port);
      newHREF += ':';
      newHREF.append (portStr.TextPtr(), portStr.Length());
   }
   
   mDAVItem.GetParentPath(tmpStr);
   pntPath = tmpStr;
   
   if (pntPath[0] != '/')
      newHREF += '/';
   newHREF += pntPath;
   if (newHREF[newHREF.size()-1] != '/')
      newHREF += '/';
   newHREF += mNewResourceName;

   CDAVItem &theItem = mDAVItem;
   CDAVItem theOrigItem(theItem);
   
   std::string srcHREF;
   mDAVItem.GetParentPath(tmpStr);
   if (tmpStr[tmpStr.size()-1] != '/')
      tmpStr += '/';
   tmpStr += mNewResourceName;
   if (CDAVItem::COLLECTION == mDAVItem.GetItemType())
      tmpStr += '/';
   
   std::string newHref = tmpStr;
   tmpStr = theItem.GetHREF();
   srcHREF=tmpStr;
   theItem.SetHREF(newHref);
   //mFLVTable->Refresh();
   
   msg_DAVItemRenamedStruct iaStruct;
   iaStruct.item = &theItem;
   iaStruct.originalHREF = srcHREF;
   GetApplicationInstance()->GetWindowManager()->BroadcastMessage(msg_DAVItemRenamed, &iaStruct);
   
   CDAVRequest::ReqStatus reqStat =  mRequest.MoveResource(*this, srcHREF, newHREF, DAVTypes::F);
   if (CDAVRequest::SUCCESS != reqStat) {
      theItem.SetHREF(tmpStr);
      iaStruct.originalHREF = newHref;
      GetApplicationInstance()->GetWindowManager()->BroadcastMessage(msg_DAVItemRenamed, &iaStruct);

      //mFLVTable->Refresh();
   } else if (CDAVRequest::XMLERROR == reqStat) {
      _OnXMLRequestError();
   }
   
   std::string outRezForkURI;
   CDAVItemUtils::GetURIForAppleDoubleResourceFork( theOrigItem, outRezForkURI);
   Boolean rezForkExists = false;                   
   if (_EncodingDisabled() == false)
      mRequest.GetResourceExists(*this, outRezForkURI, false/*isCollection*/, rezForkExists);

   if (false == rezForkExists)
      return nil;
      
   newHREF.assign( ((443 == port) || mContext.GetForceSecure()) ? "https://" : "http://");
   newHREF += mContext.GetServerName();
   if (80 != port && 443 != port) {
      LStr255 portStr(port);
      newHREF += ':';
      newHREF.append (portStr.TextPtr(), portStr.Length());
   }
   
   mDAVItem.GetParentPath(tmpStr);
   pntPath = tmpStr;
   
   if (pntPath[0] != '/')
      newHREF += '/';
   newHREF += pntPath;
   if (newHREF[newHREF.size()-1] != '/')
      newHREF += '/';
   newHREF += "._";
   newHREF += mNewResourceName;
   reqStat =  mRequest.MoveResource(*this, outRezForkURI, newHREF, DAVTypes::T);
   
   return nil;
}
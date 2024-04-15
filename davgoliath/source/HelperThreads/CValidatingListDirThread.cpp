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
#include <UModalDialogs.h>
#include <CDAVItem.h>
#include "CDAVTableApp.h"
#include "CDAVTableWindow.h"
#include <LThread.h>
#include <LProgressBar.h>
#include <CDAVRequest.h>
#include <LOutlineItem.h>
#include "CDAVTableItem.h"
#include "CValidatingListDirThread.h"
#include "CDAVTableApp.h"
#include "CWindowManager.h"
#include "CDAVErrorUtils.h"

// ---------------------------------------------------------------------------
//		¥ CValidatingListDirThread()
// ---------------------------------------------------------------------------
//
CValidatingListDirThread::CValidatingListDirThread(CDAVContext *ctx, CDAVTableWindow *wnd, std::string &resource, class LOutlineItem *parent) :
                     CListDirThread(ctx, wnd, resource, parent) {
}
					

// ---------------------------------------------------------------------------
//		¥ _executeDAVTransaction()
// ---------------------------------------------------------------------------
//
void* CValidatingListDirThread::_executeDAVTransaction() {   
   CDAVInfo info;
   CFLVDAVRequest tmpRequest(mRequest);
   tmpRequest.SetSuppressErrorDialog(true);
   
   CDAVRequest::ReqStatus stat = tmpRequest.GetServerOptions(*this, mResource, info, true);
   
   if (CDAVRequest::SUCCESS == stat) {
      msg_DAVConnectionInfoStruct davInfoMsgStruct;
      davInfoMsgStruct.originatingWindow = mWnd;
      davInfoMsgStruct.connectionInfo = &info;
      GetApplicationInstance()->GetWindowManager()->BroadcastMessage(msg_DAVConnectionInfo, &davInfoMsgStruct);
      
      if (info.getHasExecutableSupport()) {
         mWnd->AddToRequiredProperties(mod_davExecutable);  
      }
   } else if (CDAVRequest::FAILURE == stat) {
      return nil;
   } else if (CDAVRequest::WARNING == stat) {
      if (tmpRequest.GetLastResponse() == CDAVConnection::kHTTPRequestUnauthorized) {
      	stat = tmpRequest.GetServerOptions(*this, mResource, info, true);
    	if (CDAVRequest::SUCCESS == stat) {
      		msg_DAVConnectionInfoStruct davInfoMsgStruct;
      		davInfoMsgStruct.originatingWindow = mWnd;
      		davInfoMsgStruct.connectionInfo = &info;
      		GetApplicationInstance()->GetWindowManager()->BroadcastMessage(msg_DAVConnectionInfo, &davInfoMsgStruct);
      
      		if (info.getHasExecutableSupport()) {
         		mWnd->AddToRequiredProperties(mod_davExecutable);  
      		}
   		 }
      }
   }
   mRequest.SetSuppressErrorDialog(true);
   CListDirThread::_executeDAVTransaction();
   SInt32 lastResp = mRequest.GetLastResponse();
   if (CDAVConnection::kHTTPMultiStatus != lastResp) {
      if (CDAVConnection::kHTTPMethodNotAllowed == lastResp) {
         CDAVErrorUtils::DisplayNoDAVSupportDlog();
      } else {
         CDAVErrorUtils::DisplayDAVError(lastResp);
      }
   }
   return nil;
}

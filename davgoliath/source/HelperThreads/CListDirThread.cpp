/* ==================================================================================================
 * CListDirThread.cp															   
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
#include "CListDirThread.h"
#include <CWindowManager.h>
#include "CDAVItemUtils.h"

// ---------------------------------------------------------------------------
//		¥ CListDirThread()
// ---------------------------------------------------------------------------
//
CListDirThread::CListDirThread(CDAVContext *ctx, CDAVTableWindow *wnd,
					std::string &resource, class LOutlineItem *parent) :
                     CFLVThread(ctx, wnd), mResource(resource), mParent(parent),
                     m_listProps(false) {
}
					

// ---------------------------------------------------------------------------
//		¥ _executeDAVTransaction()
// ---------------------------------------------------------------------------
//
void* CListDirThread::_executeDAVTransaction() {   
   CDAVItemVector kids;
   CDAVInfo info;
      
   //mWnd->mProgBar->SetMinValue(0);
   //mWnd->mProgBar->SetMaxValue(100);

   CDAVPropertyVector props = mWnd->GetRequiredProperties(); 

   CDAVRequest::ReqStatus reqStat = mRequest.ListDirectory(*this, mResource, kids, props,this);
   if (CDAVRequest::SUCCESS == reqStat) {
      msg_DAVItemsAddedStruct iaStruct;
      CDAVItemVector procKids;
      CDAVItemUtils::ProcessDotFiles(kids, procKids);
      
      iaStruct.originatingWindow = mWnd;
      iaStruct.items = &procKids;
      iaStruct.parent = mParent;
      GetApplicationInstance()->GetWindowManager()->BroadcastMessage(msg_DAVItemsAdded, &iaStruct);
   } else {
      if (CDAVRequest::XMLERROR == reqStat)
         _OnXMLRequestError();
      if (NULL != mParent) {
         msg_DAVResetTransactionStruct resetStruct;
         resetStruct.originatingWindow = mWnd;
         resetStruct.activeItem = reinterpret_cast<CDAVTableItem*>(mParent);
         GetApplicationInstance()->GetWindowManager()->BroadcastMessage(msg_DAVResetTransaction, &resetStruct);
         
      }
   }
   
   return nil;
}


// ---------------------------------------------------------------------------
//		¥ ListenToMessage()
// ---------------------------------------------------------------------------
//
void CListDirThread::ListenToMessage(MessageT inMessage, void *ioParam)
{
	SProgressMessage * theMsg = (SProgressMessage *) ioParam;
	//char statusMessage[256];
	
	switch (inMessage) {
		case msg_RetrieveItemFailed:
			break;

		case msg_RetrieveItemSuccess:
			break;
		
		case msg_OpeningConnection:
            //mWnd->mProgBar->SetValue(20);
            //mWnd->mProgBar->Refresh();
			break;

		case msg_Connected:
            //mWnd->mProgBar->SetValue(40);

			break;
			
		case msg_Disconnected:
			break;
        case msg_SendingData:
			if (theMsg->totalBytes) {
				float temp1 = theMsg->completedBytes;
				float temp2 = theMsg->totalBytes;
				UInt32 thePercentage = (temp1/temp2) * 100;
				if (thePercentage > 100)
					thePercentage = 100;
		        //mWnd->mProgBar->SetValue(thePercentage+20);			
            }
            // mWnd->mProgBar->SetValue(85);
            break;
            
		case msg_ReceivingData:
			/*if (theMsg->totalBytes) {
				float temp1 = theMsg->completedBytes;
				float temp2 = theMsg->totalBytes;
				UInt32 thePercentage = (temp1/temp2) * 100;
				if (thePercentage > 100)
					thePercentage = 100;
		        mWnd->mProgBar->SetValue(thePercentage+120);

			} else {
			   mWnd->mProgBar->SetIndeterminateFlag(true);
			}*/
		    //mWnd->mProgBar->SetValue(90);

			break;
		
		default:
			break;
	}
}

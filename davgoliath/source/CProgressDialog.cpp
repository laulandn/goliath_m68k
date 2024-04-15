/* ==================================================================================================
 * CProgressDialog.cpp														   
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
 
#include <CFLVThread.h>
#include <CProgressDialog.h>
#include <LProgressBar.h>
#include <LStaticText.h>
#include <LThread.h>
#include <UReanimator.h>

#include <LInternetProtocol.h>


// ---------------------------------------------------------------------------
//		¥ CProgressDialog()
// ---------------------------------------------------------------------------
//
CProgressDialog::CProgressDialog():mCurrItem(0), mNumCompleted(0), mNumRemaining(0),
   mListenToSend(true) {
   ::GetDateTime(&mStartDateTime);
}


// ---------------------------------------------------------------------------
//		¥ CProgressDialog()
// ---------------------------------------------------------------------------
//
CProgressDialog::CProgressDialog(LStream *inStream):LDialogBox(inStream), mCurrItem(0),
 mNumCompleted(0), mNumRemaining(0), mListenToSend(true) {
   ::GetDateTime(&mStartDateTime);
}


// ---------------------------------------------------------------------------
//		¥ ~CProgressDialog()
// ---------------------------------------------------------------------------
//
CProgressDialog::~CProgressDialog() {

}



// ---------------------------------------------------------------------------
//		¥ setParams()
// ---------------------------------------------------------------------------
//
void CProgressDialog::setParams(CFLVThread *thread, int numItems) {
   mNumItems=numItems;
   mThread=thread;
   mProgBar->SetMaxValue(mNumItems);
   mCntText->SetValue(LStr255(mNumItems));
   mNumRemaining=numItems;
}

#include <LPushButton.h>

// ---------------------------------------------------------------------------
//		¥ FinishCreateSelf()
// ---------------------------------------------------------------------------
//
void CProgressDialog::FinishCreateSelf() {
   mProgBar  = reinterpret_cast<LProgressBar*>(FindPaneByID('prog'));
   mProgBar->SetIndeterminateFlag(false, true);
   mProgBar->SetValue(mCurrItem);
   
   mCntText = reinterpret_cast<LStaticText*>(FindPaneByID('icnt'));
   mTimeRemText = reinterpret_cast<LStaticText*>(FindPaneByID('tctr'));
   
   //LPushButton *stopBtn = reinterpret_cast<LPushButton*>(FindPaneByID('stop'));
   LControl *stopBtn = (LControl*)FindPaneByID('stop');
   if (stopBtn)
      stopBtn->AddListener(this);

   //UReanimator::LinkListenerToBroadcasters(this, this, 1304);
}



// ---------------------------------------------------------------------------
//		¥ incrementProgBar()
// ---------------------------------------------------------------------------
//
void CProgressDialog::incrementProgBar() {
   mProgBar->SetValue(++mCurrItem);
   SInt16 curnum=mNumItems-mCurrItem;
   mCntText->SetValue(LStr255(curnum));
   
   unsigned long currTime;
   long remTime, aveTime;
   LStr255 strRemainText;
   
   ::GetDateTime(&currTime);
   mNumCompleted++;
   mNumRemaining--;
   
   aveTime = ((currTime-mStartDateTime)/mNumCompleted);
   remTime = aveTime*mNumRemaining;
   if ((remTime/60) < 1) {
      strRemainText.Append(" Less than a minute");
   } else {
      remTime-=remTime%60;
      remTime /= 60;
      remTime++;
      strRemainText.Assign("Approximately ");
      strRemainText.Append(remTime);
      strRemainText.Append(" minutes");
   }
   mTimeRemText->SetDescriptor(strRemainText);
}


// ---------------------------------------------------------------------------
//		¥ IncrementTotalToBeCompleted()
// ---------------------------------------------------------------------------
//
void CProgressDialog::IncrementTotalToBeCompleted(UInt32 incr) {
   mNumRemaining += incr;
   mNumItems +=incr;
   //mProgBar->SetMaxValue(mNumRemaining+mNumCompleted);
   mProgBar->SetIndeterminateFlag(true, true);
}

// ---------------------------------------------------------------------------
//		¥ ListenToMessage()
// ---------------------------------------------------------------------------
//
void CProgressDialog::ListenToMessage(MessageT inMessage,  void* ioParam) {
#pragma unused(ioParam)
   LDialogBox::ListenToMessage(inMessage, ioParam);
   if (inMessage == 'STOP') {
      if (mThread)
         mThread->cancelOperation();
      return;
   }
   
   if (1 != mNumItems)
      return;

   unsigned long currTime;
   unsigned long remTime, elapsedTime;
   LStr255 strRemainText;
   
   ::GetDateTime(&currTime);

   SProgressMessage *progMsg;

  if (inMessage == msg_ReceivingData) {
      if (!mListenToSend) {
         progMsg = reinterpret_cast<SProgressMessage*>(ioParam);
         if (progMsg->totalBytes) {
            mProgBar->SetMaxValue(progMsg->totalBytes);
            mProgBar->SetValue(progMsg->completedBytes);
            elapsedTime = (currTime-mStartDateTime);
            remTime = elapsedTime * ((progMsg->totalBytes-progMsg->completedBytes)/progMsg->completedBytes);
            LStr255 strRemainText;
            if ((remTime/60) < 1) {
               strRemainText.Append(" Less than a minute");
               strRemainText.Append(" ");
            } else {
               remTime-=remTime%60;
               remTime /= 60;
               remTime++;
               strRemainText.Assign("Approximately ");
               strRemainText.Append((long)remTime);
               strRemainText.Append(" minutes");
            }
            
            mTimeRemText->SetDescriptor(strRemainText);
         } else {
            mProgBar->SetIndeterminateFlag(true);
         }
      }
   } else if (inMessage == msg_SendingData) {
      if (mListenToSend) {
         progMsg = reinterpret_cast<SProgressMessage*>(ioParam);
         if (progMsg->totalBytes) {
            mProgBar->SetMaxValue(progMsg->totalBytes);
            mProgBar->SetValue(progMsg->completedBytes);
            elapsedTime = (currTime-mStartDateTime);
            remTime = elapsedTime * ((progMsg->totalBytes-progMsg->completedBytes)/progMsg->completedBytes);
            LStr255 strRemainText;
            if ((remTime/60) < 1) {
               strRemainText.Append(" Less than a minute");
               strRemainText.Append(" ");
            } else {
               remTime-=remTime%60;
               remTime /= 60;
               remTime++;
               strRemainText.Assign("Approximately ");
               strRemainText.Append((long)remTime);
               strRemainText.Append(" minutes");
            }
            
            mTimeRemText->SetDescriptor(strRemainText);
         } else {
            mProgBar->SetIndeterminateFlag(true);
         }
      }
   }
}

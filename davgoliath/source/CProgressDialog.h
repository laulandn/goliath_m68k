/* ==================================================================================================
 * CProgressDialog.h													   
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
#ifndef __CPROGRESSDIALOG_H__
#define __CPROGRESSDIALOG_H__


#include <LDialogBox.h>

/*
   A simple mode-less dialog that displays delete progress and also
   has a cancel mechanism to stop any queued deletes.
*/
class CProgressDialog : public LDialogBox {
   public:
      enum	{ class_ID = FOUR_CHAR_CODE('DDLG') };
      
      CProgressDialog();
      CProgressDialog(LStream* inStream);

	  virtual ~CProgressDialog();
      
      void setParams(class CFLVThread *thread, int numItems);
	  void incrementProgBar();
	  void IncrementTotalToBeCompleted(UInt32 incr);
	  
	  virtual void FinishCreateSelf();
	  virtual void ListenToMessage(MessageT inMessage,  void* ioParam);
      
      void ListenToReceive() {mListenToSend = false;};
   protected:
      //total number of resources to be deleted
      SInt16               mNumItems;
      //current item
      SInt16               mCurrItem;
      
      class LProgressBar*        mProgBar;
      class LStaticText*         mCntText;
      class LStaticText*         mTimeRemText;

      class CFLVThread*  mThread;
      
      unsigned long mStartDateTime;
      SInt16        mNumCompleted;
      SInt16        mNumRemaining;
      
      Boolean       mListenToSend;
};


#endif

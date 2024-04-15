/* ==================================================================================================
 * CDownloadFileThread.cp															   
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
#include "CDownloadFileThread.h"
#include "CDAVItemUtils.h"
#include "FileForkUtils.h"
#include <CProgressDialog.h>
#include <CWindowManager.h>
#include <CDAVTranscodeUtils.h>
#include <LCheckBox.h>
#include <LEditText.h>
#include <cassert>
#include <stdio.h>
#include <string.h>

const int kHFSMaxFileNameLen = 31;

// ---------------------------------------------------------------------------
//		¥ CUploadFileThread()
// ---------------------------------------------------------------------------
//
CDownloadFileThread::CDownloadFileThread(CDAVContext *ctx, CDAVTableWindow *wnd, FSSpec &rootSpec, CDAVItem &theItem)
 :CFLVThread(ctx, wnd), mCancel(false), mHasUrlParam(false), mTotalToDownload(0), 
    mHaveCompleteTotal(false), mProgressWindow(NULL),
    mAutoTruncate(false) {
    mItemVector.push_back(theItem);
    mRootSpec =rootSpec;
}


// ---------------------------------------------------------------------------
//		¥ CUploadFileThread()
// ---------------------------------------------------------------------------
//
CDownloadFileThread::CDownloadFileThread(CDAVContext *ctx, CDAVTableWindow *wnd, FSSpec &rootSpec, CDAVItemVector &theItems)
  :CFLVThread(ctx, wnd), mCancel(false), mHasUrlParam(false), mTotalToDownload(0), 
   mHaveCompleteTotal(false), mProgressWindow(NULL),
   mAutoTruncate(false) {
   mItemVector =  theItems;
   mRootSpec =rootSpec;
}

// ---------------------------------------------------------------------------
//		¥ ~CUploadFileThread()
// ---------------------------------------------------------------------------
//
CDownloadFileThread::~CDownloadFileThread() {
   if (mProgressWindow)
      delete mProgressWindow;
}


// ---------------------------------------------------------------------------
//		¥ cancelOperation()
// ---------------------------------------------------------------------------
//
void CDownloadFileThread::cancelOperation() {
   mCancel = true;
   if (1 == mTotalToDownload) 
      mRequest.CancelTransaction();   
}

#include "CFLVDAVRequest.h"
#include <CDAVTreeWalker.h>

// ---------------------------------------------------------------------------
//		¥ _executeDAVTransaction()
// ---------------------------------------------------------------------------
//		
void* CDownloadFileThread::_executeDAVTransaction() {

   CProgressDialog *progwin = NULL;
 
   CDAVItemVector::iterator citer = mItemVector.begin();
   CDAVPropertyVector props;
   while ((citer != mItemVector.end()) && (false == mCancel)) {
      if (CDAVItem::COLLECTION == citer->GetItemType()) {
         CDAVItemVector tmp, children;
         reinterpret_cast<CFLVDAVRequest*>(&mRequest)->SetSuppressErrorDialog(true);
         if (CDAVRequest::SUCCESS == mRequest.ListDirectory(*this, 
                              citer->GetHREF(), tmp, props, NULL, DAVTypes::PROPINFINITY)) {
			CDAVItemUtils::ProcessDotFiles(tmp, children);
            mTotalToDownload +=children.size();
            mHaveCompleteTotal = true;
         } else if (mRequest.GetLastResponse() != CDAVConnection::kHTTPForbidden) {
            //Handle case where mod_dav is configured to prohibit PROPFIND-Infinity 
            // requests by walking the tree.  Since we now have HTTP/1.1 keep-alive
            // this isn't as bad as one would think.
            CDAVTreeWalker walker(*this, &mRequest, !_EncodingDisabled());
            if (walker.WalkTree(citer->GetHREF().c_str())) {
               mTotalToDownload +=children.size();
               mHaveCompleteTotal = true;            
            }
         }
      } else {
         mTotalToDownload++;
      }
      citer++;
   }

   reinterpret_cast<CFLVDAVRequest*>(&mRequest)->SetSuppressErrorDialog(false);
   
   progwin = reinterpret_cast<CProgressDialog*>(LWindow::CreateWindow(1304, GetApplicationInstance()));
   reinterpret_cast<CProgressDialog*>(progwin)->setParams(this, mTotalToDownload);
   progwin->ListenToReceive();
   mProgressWindow = progwin;
   
   CDAVItem *tmpItem;
   CDAVItemVector::iterator iter = mItemVector.begin();
   while ((iter != mItemVector.end()) && (false == mCancel)) {
      tmpItem = &*iter;
      iter++;
       
       if (!_downloadResource(&mRootSpec, tmpItem, progwin))
          break;
      
      if (progwin)
         progwin->incrementProgBar();

      if (mCancel)
         break;
   }   

   LCommander *cmdr = GetApplicationInstance()->GetTarget();
   if (cmdr)
      cmdr->RestoreTarget(); 

   if (progwin) {
      progwin->Hide();
      //delete progwin;
   }


   return nil;
}


// ---------------------------------------------------------------------------
//		¥ _downloadResource()
// ---------------------------------------------------------------------------
//	
Boolean CDownloadFileThread::_downloadResource(FSSpec *rootSpec, CDAVItem* inItem, CProgressDialog *progwin) {
   Boolean retval = false;
   std::string itemHref = inItem->GetHREF();
   UInt8 lastDirDelim;

   if (CDAVItem::COLLECTION == inItem->GetItemType()) {
   		if (itemHref [itemHref.size() - 1]=='/')
      		itemHref.resize(itemHref.size() - 1);
   }
   
   lastDirDelim = itemHref.rfind('/');
   std::string itemName;
   itemName.assign (itemHref.c_str() + lastDirDelim + 1);


   itemName = CDAVTranscodeUtils::TranscodeUTF8ToSystemScript(itemName);

   if (mHasUrlParam) {
      itemHref.append(mUrlParam);
   }

   FSSpec newSpec(*rootSpec);
   if (itemName.size() > kHFSMaxFileNameLen) {
      std::string fileName = GenerateTruncatedFileName(itemName, rootSpec);
      if (mAutoTruncate || DisplayFileNameTooLargeDlog(fileName, mAutoTruncate)) {
	      newSpec.name[0] = fileName.size();
   		  BlockMoveData (fileName.c_str(), newSpec.name+1, newSpec.name[0]);
      } else {
         return false;
      }
   } else {
      newSpec.name[0] = itemName.size();
      BlockMoveData (itemName.c_str(), newSpec.name+1, newSpec.name[0]);
   }
	
   if (CDAVItem::COLLECTION == inItem->GetItemType()) {
      FSSpec folderSpec(*rootSpec);
      
      long folderID;
      OSErr err = ::FSpDirCreate(&newSpec, smSystemScript, &folderID);
      if ((err != noErr) && (err != dupFNErr))
         return false;

      if (dupFNErr == err) {
         CInfoPBRec cpb;
         LStr255 iName = itemName.c_str();
         cpb.hFileInfo.ioNamePtr = iName;
         
         cpb.hFileInfo.ioVRefNum = rootSpec->vRefNum;
         cpb.hFileInfo.ioFDirIndex = 0;
         cpb.hFileInfo.ioDirID = rootSpec->parID;
         
         OSErr ioResult = PBGetCatInfoSync((CInfoPBPtr) &cpb);
         if (!ioResult)
            folderID = cpb.dirInfo.ioDrDirID;
      }
      //Recurse the folder
      CDAVItemVector children, tmp;
      CDAVItemVector::iterator iter;
      CDAVPropertyVector props;
      
      FSSpec newRoot;
      newRoot.parID = folderID;
      newRoot.vRefNum = rootSpec->vRefNum;
      
      if (CDAVRequest::SUCCESS ==
          mRequest.ListDirectory(*this, inItem->GetHREF(), tmp, props, NULL)) {
          CDAVItemUtils::ProcessDotFiles(tmp, children);
          if (false == mHaveCompleteTotal) {
             if (progwin) {
                progwin->IncrementTotalToBeCompleted(children.size());
                progwin->incrementProgBar();
             }
          }
          for (iter = children.begin(); iter != children.end(); iter++) {
             _downloadResource(&newRoot, &*iter, progwin);
             if (mCancel)
                return false;
          }
          return true;
      }                      
      return true;
   }
   

   CDAVRequest::ReqStatus reqStat;
   std::string outRezForkURI;
   CDAVItemUtils::GetURIForAppleDoubleResourceFork( *inItem, outRezForkURI);
   Boolean rezForkExists = false;                   
   if (_EncodingDisabled() == false)
      mRequest.GetResourceExists(*this, outRezForkURI, false/*isCollection*/, rezForkExists);
   reqStat = mRequest.GetResource(*this, itemHref, newSpec, false, 0, progwin);

   if ((CDAVRequest::SUCCESS != reqStat) && (CDAVRequest::WARNING != reqStat))
      return false;

   if (CDAVRequest::SUCCESS == reqStat) {
      retval= true;
   } else if (CDAVRequest::CANCEL == reqStat) {
      retval= false;
      ::FSpDelete(&newSpec);
      return false;
   }
   
   if (rezForkExists) {
         FSSpec tmpSpec;
         CDAVItemUtils::GetATempFile(tmpSpec);
         reqStat = mRequest.GetResource(*this, outRezForkURI, tmpSpec, false);
         OSErr err =  ::MergeFlattenedForks(newSpec, tmpSpec); 
         ::FSpDelete(&tmpSpec);
         if (reqStat != CDAVRequest::SUCCESS) {
            ::FSpDelete(&newSpec);
            return false;
         }

   }
   return retval;
}



// ---------------------------------------------------------------------------
//		¥ GenerateTruncatedFileName()
// ---------------------------------------------------------------------------
//	
std::string	CDownloadFileThread::GenerateTruncatedFileName(const std::string& inFileName, const FSSpec* inPntFolder) {
	std::string::size_type pos = inFileName.rfind(".");
	std::string basename, extension;
	if (std::string::npos == pos) {
		basename = inFileName.substr(0, kHFSMaxFileNameLen  );
	} else {
		extension = inFileName.substr(pos);
		basename =  inFileName.substr(0, kHFSMaxFileNameLen - extension.size());
	}
	
	std::string outName;
	bool first = true;
	int i=0;
	char tmpBuf[10];
	while (1) {
	    if (0 == i) {
			outName = basename + extension;
		} else {
			sprintf(&tmpBuf[0], "%d", i);
			outName = basename.substr(0, basename.size() - strlen(tmpBuf));
			outName.append(tmpBuf);
			outName.append(extension);
		}
		LStr255 tempFileName(outName.c_str());
		FSSpec tempFileSpec;
		OSErr theErr = ::FSMakeFSSpec(inPntFolder->vRefNum, inPntFolder->parID, tempFileName, &tempFileSpec);
		if (theErr == fnfErr)
		   break;
		i++;
	}
	
	return outName;
}


// ---------------------------------------------------------------------------
//		¥ DisplayFileNameTooLargeDlog()
// ---------------------------------------------------------------------------
//	
bool CDownloadFileThread::DisplayFileNameTooLargeDlog(std::string& ioFname, bool& outAlwaysTruncate) {
	PP_PowerPlant::StDialogHandler dialog(1420, GetApplicationInstance());
	LDialogBox *dlog = dynamic_cast<LDialogBox*>(dialog.GetDialog());
	
	LEditText *text = dynamic_cast<LEditText*>(dlog->FindPaneByID('fnam'));
	LCheckBox *autotrunc = dynamic_cast<LCheckBox*>(dlog->FindPaneByID('auto'));
	
	assert(text != NULL);
	assert(autotrunc != NULL);
	if (text == NULL || autotrunc == NULL)
		return false;

	text->SetText(ioFname.c_str(), ioFname.size());
	
	while (true) {
		PP_PowerPlant::MessageT hitMessage = dialog.DoDialog();
		if (hitMessage == PP_PowerPlant::msg_Cancel)
			return false;
		else if (hitMessage == PP_PowerPlant::msg_OK) {
			break;
		} 
	}
	
	LStr255 tmpPStr;
	text->GetText(tmpPStr);
	ioFname.assign(tmpPStr.ConstTextPtr(), tmpPStr.Length());
	outAlwaysTruncate = (autotrunc->GetValue()==1);
	return true;
	
}

 

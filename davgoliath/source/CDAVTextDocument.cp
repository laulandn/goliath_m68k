/* ==================================================================================================
 * CDAVTextDocument.cp																   
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
//	A subclass of LSingleDoc for basic management of DAV Text documents.
//  
#include "CDAVTextDocument.h"

#include <LFile.h>
#include <LPlaceHolder.h>
#include <LPrintout.h>
#include <LString.h>
#include <LWindow.h>
#include <PP_Messages.h>
#include <UMemoryMgr.h>
#include <UResourceMgr.h>
#include <UWindows.h>
#include "CDAVTableWindow.h"
#include "CDAVTableAppConstants.h"
#include "CDAVTextView.h"
#include "CUnlockItemsThread.h"
#include "CSaveToWebThread.h"
#include "CClientLockManager.h"

#include <CDAVRequest.h>

// ---------------------------------------------------------------------------------
//	¥ CDAVTextDocument										[public]
// ---------------------------------------------------------------------------------
//	Constructor

CDAVTextDocument::CDAVTextDocument(
	LCommander*		inSuper,
	FSSpec*			inFileSpec, 
	CDAVContext     *context,
	CDAVItem        *theItem,
	Boolean         supportLocks,
	CDAVTableWindow *originatingwnd)

	: LSingleDoc(inSuper), mContext(*context), mItem(*theItem), mOriginatingWnd(originatingwnd),
	  mSupportsLocks(supportLocks), mIsSavingDocument(false), mIsClosingDocuement(false)
{
   mFLVTable =  mOriginatingWnd->GetTable();
		// Create window for our document.
   mWindow = LWindow::CreateWindow(PPob_TextWindow, this );
   ThrowIfNil_(mWindow);
	
	// Specify that the text view should be the
	// target when the window is activated.
	mTextView = dynamic_cast<CDAVTextView*>(mWindow->FindPaneByID(kTextView));
	Assert_(mTextView != nil);
	mWindow->SetLatentSub(mTextView);
	
	
	if (inFileSpec) {
		OpenFile(*inFileSpec);		// Display contents of file in window.
	}
	
	// Make the window visible.
	mWindow->Show();
}


// ---------------------------------------------------------------------------------
//	¥ ~CDAVTextDocument									[public, virtual]
// ---------------------------------------------------------------------------------
//	Destructor

CDAVTextDocument::~CDAVTextDocument()
{
	try {
		TakeOffDuty();
		
		if ((mFile != nil) && (mFileDesignator != nil) && (false == mIsClosingDocuement)) {
			mFile->CloseDataFork();
			FSSpec outFileSpec;
			mFile->GetSpecifier(outFileSpec);
			::FSpDelete(&outFileSpec);
		}
	}
		
	catch (...) { }
}

// ---------------------------------------------------------------------------
//	¥ Close										[public, virtual]
// ---------------------------------------------------------------------------
//
void CDAVTextDocument::Close() {
   if (mSupportsLocks) {
      CDAVItemVector items;
      items.push_back(mItem);
      CUnlockItemsThread *thread = new CUnlockItemsThread(&mContext, mOriginatingWnd, mFLVTable, items);
      if (NULL != thread)
         thread->Resume();
   }
   LSingleDoc::Close();
}



// ---------------------------------------------------------------------------------
//	¥ OpenFile											[protected, virtual]
// ---------------------------------------------------------------------------------
//	Open the specified file

void
CDAVTextDocument::OpenFile(
	FSSpec&		inFileSpec)
{
	mFile = nil;
	
		// Create a new File object, read the entire File contents,
		//  put the contents into the text view, and set the Window
		//  title to the name of the File.
		// We don't close the file (until mFile is deleted) so we
		//  can prevent others from modifying the file.
		
	try {
		StDeleter<LFile>	theFile(new LFile(inFileSpec));
		theFile->OpenDataFork(fsRdWrPerm);
		StHandleBlock textH(theFile->ReadDataFork());
		
		mTextView->SetTextHandle(textH, nil);
				
		mTextView->SetDirty(false);
		mWinName.Assign("http://");
		mWinName.Append(mContext.GetServerName().c_str());
		mWinName.Append(mItem.GetHREF().c_str());
		mWindow->SetDescriptor(mWinName);
		mIsSpecified = true;
		
		mFile = theFile.Release();
		
		mFile->CloseDataFork();
	}
	
	catch (LException& inErr) {

			// If the 32K limit of TextEdit is hit, do not rethrow here.
			// Instead, this would be a good place to notify the user of
			// the situation.				
		if (inErr.GetErrorCode() == err_32kLimit) {
            AlertStdAlertParamRec param;
            param.movable 		= false/* ***teb - maybe someday - true*/;
            param.filterProc 	= nil;
            param.defaultText 	= "\pOK";
            param.cancelText 	= nil;
            param.otherText 	= nil;
            param.helpButton 	= false;
            param.defaultButton = kAlertStdAlertOKButton;
            param.cancelButton 	= kAlertStdAlertCancelButton;
            param.position 		= 0;
            SInt16			itemHit;
		
            LStr255 text, desc;
            desc.Assign(str_UIStrings, str_FileToLargeToEdit);
            StandardAlert( kAlertCautionAlert, desc, "\p"/*nil*/, &param, &itemHit );

		}
	}

}


// ---------------------------------------------------------------------------------
//	¥ IsModified										[public, virtual]
// ---------------------------------------------------------------------------------
//	Return whether the Document is has changed since the last save

Boolean
CDAVTextDocument::IsModified()
{
		// Document has changed if the text view is dirty.
	mIsModified = mTextView->IsDirty();
	return mIsModified;
}


// ---------------------------------------------------------------------------------
//	¥ DoSave											[public, virtual]
// ---------------------------------------------------------------------------------
//	Saves the content of this window back to the originating DAV server
void
CDAVTextDocument::DoSave()
{
		// First, write the data fork.

		// Assumes the data fork is already open for writing
		// (handled in DoAESave).

		// Get the text from the text view.
	Handle	theTextH = mTextView->GetTextHandle();
	ThrowIfNil_(theTextH);
	Size	textSize = ::GetHandleSize(theTextH);
	
		// Lock the text handle.
	StHandleLocker	theLock(theTextH);
	
		// Write the text to the file.
	mFile->OpenDataFork(fsRdWrPerm);
	mFile->WriteDataFork(*theTextH, textSize);
	mFile->CloseDataFork();
	FSSpec tmpSpec;
	mFile->GetSpecifier(tmpSpec);
	
    std::string lockToken;
    Boolean hasLockToken = false;
    if (mSupportsLocks) {
       if (GetApplicationInstance()->GetLockManager()->GetLockToken(&mContext, &mItem, lockToken)) {
          hasLockToken = true;
       }
    }
    
    std::string tmpCstr = mItem.GetHREF();
	CSaveToWebThread *thread = new CSaveToWebThread(&mContext, mOriginatingWnd, mFLVTable, 
	                   tmpCstr, tmpSpec);
	

     if (hasLockToken)
         thread->SetLockToken(lockToken);
     LSemaphore mux;
         
    if (!mIsClosingDocuement) 
       thread->SetDocument(this);
    else {
       thread->SetDeleteOnUpload();
       thread->SetMutex(&mux);
       
    }
    mIsSavingDocument = true;
    
	if (NULL != thread) {
	   thread->Resume();
	}
	 
	
	// Saving makes doc clean.
	mTextView->SetDirty(false);
}


// ---------------------------------------------------------------------------------
//	¥ DoPrint											[public, virtual]
// ---------------------------------------------------------------------------------

void
CDAVTextDocument::DoPrint()
{
		// Create the printout.
	StDeleter<LPrintout>	thePrintout(LPrintout::CreatePrintout(PPob_TextPrintout));
	ThrowIfNil_(thePrintout.Get());
	
		// Set the print record.
//***teb	thePrintout->SetPrintSpec(mPrintSpec);
	
		// Get the text placeholder.
	LPlaceHolder* thePlaceholder = dynamic_cast<LPlaceHolder*>
							(thePrintout->FindPaneByID(kTextPlaceholder));
	ThrowIfNil_(thePlaceholder);
	
		// Install the text view in the placeholder.
	thePlaceholder->InstallOccupant(mTextView, atNone);
	
		// Set the frame size.
	SetPrintFrameSize();
	
		// Print.
	thePrintout->DoPrintJob();
	
	// Delete the printout (handled automatically by the
	// StDeleter object). The text view is returned
	// to the window when the placeholder is destroyed.
}



// ---------------------------------------------------------------------------------
//	¥ SetPrintFrameSize									[protected, virtual]
// ---------------------------------------------------------------------------------

void
CDAVTextDocument::SetPrintFrameSize()
{
		// Get the frame size.
	SDimension16	theFrameSize;
	mTextView->GetFrameSize(theFrameSize);
	
		// Get the text edit record handle.
	TEHandle	theTextEditH = mTextView->GetMacTEH();
	
		// Calculate the number of lines per page.
	SInt16	theLinesPerPage;
	theLinesPerPage = theFrameSize.height / (**theTextEditH).lineHeight;

		// Resize the frame to an integral number of lines.
	mTextView->ResizeFrameTo( theFrameSize.width,
		(**theTextEditH).lineHeight * theLinesPerPage, Refresh_No);
}


// ---------------------------------------------------------------------------
//	¥ GetFileType										[public, virtual]
// ---------------------------------------------------------------------------
//	Return the type (four character code) of the file used for saving
//	the Document. Subclasses should override if they support saving files.

OSType
CDAVTextDocument::GetFileType() const
{
	return ResType_Text;
}


// ---------------------------------------------------------------------------
//	¥ FindCommandStatus									[public, virtual]
// ---------------------------------------------------------------------------
//	Override provided here for convenience.

void
CDAVTextDocument::FindCommandStatus(
	CommandT		inCommand,
	Boolean&		outEnabled,
	Boolean&		outUsesMark,
	UInt16&			outMark,
	Str255			outName)
{
   if (inCommand == cmd_Save) {
      if (mIsSavingDocument) {
         outEnabled = false;
         return;
      }
   }
	LSingleDoc::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);	
}


// ---------------------------------------------------------------------------
//	¥ ObeyCommand										[public, virtual]
// ---------------------------------------------------------------------------
//	Override provided here for convenience.

Boolean
CDAVTextDocument::ObeyCommand(
	CommandT		inCommand,
	void*			ioParam)
{
	Boolean	cmdHandled = LSingleDoc::ObeyCommand(inCommand, ioParam);

	return cmdHandled;
}

// ---------------------------------------------------------------------------
//	¥ AskSaveChanges												  [public]
// ---------------------------------------------------------------------------
//	Ask user whether to save changes before closing the Document or
//	quitting the Application

SInt16
CDAVTextDocument::AskSaveChanges(bool	inQuitting)
{
	MakeCurrent();
	
	LStr255	appName(STRx_Standards, str_ProgramName);
	mIsClosingDocuement = true;
	
	::ParamText(appName, mWinName, Str_Empty, Str_Empty);
	
	ResIDT	alertID = ALRT_SaveChangesClosing;
	if (inQuitting) {
		alertID = 1202;
	}
	
	DialogItemIndex idx = UModalAlerts::CautionAlert(alertID);
    if (inQuitting)
       idx++;
    return idx;
}

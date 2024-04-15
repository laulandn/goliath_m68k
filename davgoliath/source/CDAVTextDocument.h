/* ==================================================================================================
 * CDAVTextDocument.h														   
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
 
#ifndef _H_CDAVTextDocument
#define _H_CDAVTextDocument
#pragma once

#include <LSingleDoc.h>
#include <UStandardDialogs.h>

#include <CDAVContext.h>
#include <CDAVItem.h>
class CDAVTextView;


class CDAVTextDocument : public LSingleDoc {

public:
							CDAVTextDocument(
								LCommander*			inSuper,
								FSSpec*				inFileSpec,
								CDAVContext     *context,
	                            CDAVItem        *theItem, 
	                            Boolean supportsLocks,
	                            class CDAVTableWindow *originatingwnd);

	virtual					~CDAVTextDocument();

	virtual Boolean			IsModified();
	
	virtual void			DoSave();
	virtual void			DoPrint();

	virtual OSType			GetFileType() const;

	virtual Boolean			ObeyCommand(
								CommandT			inCommand,
								void*				ioParam = nil);
								
	virtual void			FindCommandStatus(
								CommandT			inCommand,
								Boolean&			outEnabled,
								Boolean&			outUsesMark,
								UInt16&				outMark,
								Str255				outName);					

    virtual void Close();
    virtual SInt16 AskSaveChanges(bool	inQuitting);
	protected:

	virtual void			OpenFile(
								FSSpec&				inFileSpec);
	virtual void			SetPrintFrameSize();

	CDAVTextView*		mTextView;
	
	Boolean  mIsSavingDocument;
	Boolean  mIsClosingDocuement;
	
	PP_StandardDialogs::LFileDesignator*	mFileDesignator;	

private:
							CDAVTextDocument();
							CDAVTextDocument(const CDAVTextDocument& inOriginal);
			CDAVTextDocument&	operator=(const CDAVTextDocument& inRhs);
			
			CDAVContext     mContext;
			CDAVItem        mItem;
			Boolean mSupportsLocks;
			CDAVTableWindow *mOriginatingWnd;
			class CDAVTable *mFLVTable;
            LStr255 mWinName;
            
            
            friend class CSaveToWebThread;
};

#endif // _H_CDAVTextDocument
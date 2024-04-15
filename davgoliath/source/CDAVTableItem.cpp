/* ==================================================================================================
 * CDAVTableItem.cp															   
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
 
#include <UGAColorRamp.h>
#include <LInPlaceEditField.h>

#include <CDAVRequest.h>
#include <CDAVTranscodeUtils.h>

#include "CDAVTableWindow.h"
#include "CDAVTableItem.h"
#include "CDAVTable.h"
#include <UThread.h>
#include "CRenameResourceThread.h"
#include "CDAVTableAppConstants.h"
#include "CIconSuiteManager.h"

#include <stdlib.h>

// ---------------------------------------------------------------------------
// utility fxn used internally by this class
void BytesToString(long long bytes, LStr255 &text);

// ---------------------------------------------------------------------------
//		¥ CDAVTableItem()
// ---------------------------------------------------------------------------
//	Constructor
CDAVTableItem::CDAVTableItem():  
   mEditField (nil), mTransacting(false), mHandlingClick(false) {
#if !PP_Target_Carbon
   mIcon = NULL;
#endif
}

// ---------------------------------------------------------------------------
//		¥ ~CDAVTableItem()
// ---------------------------------------------------------------------------
//	Destructor
CDAVTableItem::~CDAVTableItem() {
#if PP_Target_Carbon
	::ReleaseIconRef(mIconRef);
#else
	if ( mIcon != nil ) DisposeIconSuite(mIcon, true);/*::DisposeHandle(mIcon);*/
#endif
};

// ---------------------------------------------------------------------------
//		¥ SetItem
// ---------------------------------------------------------------------------
//	Destructor
void CDAVTableItem::SetItem(CDAVItem &item) {
	mItem = item;
	mFolder=(item.GetItemType()==CDAVItem::COLLECTION);
	
#if PP_Target_Carbon
	OSErr err;
	if (mFolder) {
	   err = ::GetIconRef(kOnSystemDisk, kSystemIconsCreator, kGenericFolderIcon, &mIconRef);
	} else {
		err = CIconSuiteManager::GetIconForItem(mItem, &mIconRef);
		if (err != noErr) {
    	  	err = ::GetIconRef(kOnSystemDisk, kSystemIconsCreator, kGenericDocumentIcon, &mIconRef);
		}
	}
#else
	if (mFolder) { 
		::GetIconSuite(&mIcon, kGenericFolderIconResource, kSelectorAllAvailableData);
	} else {
		mIcon = CIconSuiteManager::GetIconForItem(item);
	}
#endif
};

// ---------------------------------------------------------------------------
//		¥ DrawRowAdornments()
// ---------------------------------------------------------------------------
//	
void CDAVTableItem::DrawRowAdornments(const Rect& inLocalRowRect ) {
	RGBColor *background = ((CDAVTable*)mOutlineTable)->GetRowColor(FindRowForItem());
	ShadeRow(*background, inLocalRowRect);
	((CDAVTable*)mOutlineTable)->DrawRowBackground(FindRowForItem(), inLocalRowRect);
}

// ---------------------------------------------------------------------------
//		¥ CanExpand()
// ---------------------------------------------------------------------------
//
Boolean CDAVTableItem::CanExpand()  const {
   return (mFolder&& (!mTransacting));
}

// ---------------------------------------------------------------------------
//		¥ SingleClick()
// ---------------------------------------------------------------------------
//
// tell the table that the cell was clicked on
void CDAVTableItem::SingleClick(const STableCell& inCell, const SMouseDownEvent& inMouseDown, const SOutlineDrawContents& inDrawContents, Boolean inHitText)
{
	LEditableOutlineItem::SingleClick(inCell, inMouseDown, inDrawContents, inHitText);
	((CDAVTable*)mOutlineTable)->CellSingleClicked(this);
}

// ---------------------------------------------------------------------------
//		¥ DoubleClick()
// ---------------------------------------------------------------------------
//
// tell the table that the cell was double clicked
void CDAVTableItem::DoubleClick(const STableCell &inCell, const SMouseDownEvent& inMouseDown, const SOutlineDrawContents& inDrawContents, Boolean inHitText)
{
	LEditableOutlineItem::DoubleClick(inCell, inMouseDown, inDrawContents, inHitText);
	((CDAVTable*)mOutlineTable)->CellDoubleClicked(this);
}

// ---------------------------------------------------------------------------
//		¥ Expand()
// ---------------------------------------------------------------------------
//
void CDAVTableItem::Expand() {
   StMutex mutex(fAccess);
   mTransacting = true;
   LOutlineItem::Expand();
}

// ---------------------------------------------------------------------------
//		¥ Collapse()
// ---------------------------------------------------------------------------
//
void CDAVTableItem::Collapse() {
   StMutex mutex(fAccess);
   LOutlineItem::Collapse();
   ((CDAVTable*)mOutlineTable)->mWnd->_updateHeaderText();
}

// ---------------------------------------------------------------------------
//		¥ ExpandSelf()
// ---------------------------------------------------------------------------
//
void CDAVTableItem::ExpandSelf() {
   StMutex mutex(fAccess);
   std::string href = mItem.GetHREF();
   ((CDAVTable*)mOutlineTable)->mWnd->OnFolderExpansion(href, this);
}

// ---------------------------------------------------------------------------
//		¥ StartInPlaceEdit()
// ---------------------------------------------------------------------------
//        
void CDAVTableItem::StartInPlaceEdit(const STableCell& inCell) {
	LEditableOutlineItem::StartInPlaceEdit( inCell );

	// we need to listen for the edit field to stop editing
	
	mEditField = GetEditField();
	ThrowIfNil_(mEditField);
	
	mEditField->AddListener(this);
	mEditField->SetValueMessage(100);
}

// ---------------------------------------------------------------------------
//		¥ ResetTransaction()
// ---------------------------------------------------------------------------
//
void CDAVTableItem::ResetTransaction() {
   mTransacting = false;
}

// ---------------------------------------------------------------------------
//		¥ ListenToMessage()
// ---------------------------------------------------------------------------
//
void CDAVTableItem::ListenToMessage( MessageT	inMessage, void *ioParam ){
#pragma unused(ioParam)

	switch ( inMessage )
	{
		case 100:
		{
		    CDAVTable* dTable = (CDAVTable*)mOutlineTable;
		    LStr255 text;
			std::string	origName, newName;
		    
		    mEditField->GetDescriptor( text );
		    if ( text[0] == 0 )
		       return;
		    
		    newName.append(text.ConstTextPtr(), text.Length());
		    #ifdef __POWERPC__
			newName = CDAVTranscodeUtils::TranscodeSystemScriptToUTF8(newName);
			#endif
			
			GetItem().GetFileName(origName);
		    //if name hasn't changed, return
		    			
		    if (newName.compare(origName) == 0)
		       return;
		       
			CRenameResourceThread *thread = new CRenameResourceThread(dTable->mWnd->GetDAVContext(), dTable->mWnd, newName, &mItem); 
			if (NULL == thread)
               return;   //***teb - signal error condition here

           thread->Resume();
           break;
		}
	}
}


// ---------------------------------------------------------------------------
//		¥ BytesToString()
// ---------------------------------------------------------------------------
//
void BytesToString(long long bytes, LStr255 &text) {
	long tenths;
	
	if (bytes == 0)
		text = "Ñ";
	else if (bytes <= 512) {
	   text = (long)bytes;
	   text += "\p bytes";
	} else if (bytes < 0x100000)				// less than one megabyte
	{
		tenths = ((bytes * 10) >> 10) % 10;
		if (tenths >= 5)
			text = (SInt32)(1 + (bytes >> 10));
		else
			text = (SInt32)(bytes >> 10);
			
		text += "\p KB";						
	}
	else if (bytes < 0x40000000)			// less than one gigabyte
	{
		text = (SInt32)(bytes >> 20);
		tenths = ((bytes * 10) >> 20) % 10;
		if (tenths)
		{
			text += "\p.";
			text += tenths;
		}
		text += "\p MB";
	}
	else									// greater than one gigabyte
	{
	    text = (SInt32)(bytes >> 30);
	    tenths = ((bytes * 10) >> 30) % 10;
	    if (tenths)
	    {
	    	text += "\p.";
	   		text += tenths;
	    }
		text += "\p GB";
	}
}

#ifndef __CDAVTRANSCODEUTILS_H__
#include "CDAVTranscodeUtils.h"
#endif


// ---------------------------------------------------------------------------
//		¥ GetDrawContentsSelf()
// ---------------------------------------------------------------------------
//
void CDAVTableItem::GetDrawContentsSelf( const STableCell& inCell, SOutlineDrawContents&	ioDrawContents) {
   LStr255 text;
   std::string stdText;   
   std::string pval;
   UInt32 size;
    
   switch (inCell.col) {
         case kNameColumn:						// Name column
            ioDrawContents.outShowSelection = true;
			ioDrawContents.outHasIcon = true;
#if !PP_Target_Carbon
			ioDrawContents.outIconSuite = mIcon;
#endif
			ioDrawContents.outCanDoInPlaceEdit = true;
            GetItem().GetFileName(stdText);
            #ifdef __POWERPC__
            stdText = CDAVTranscodeUtils::TranscodeUTF8ToSystemScript(stdText);
            #endif
            
            text = stdText.c_str();
            LString::CopyPStr(text, ioDrawContents.outTextString);
         break;
         case kDateColumn:						// Date column
            text = GetItem().GetPropertyValue(LastModified).c_str();
		    LString::CopyPStr(text, ioDrawContents.outTextString);
			ioDrawContents.outShowSelection = true;
			ioDrawContents.outHasIcon = false;
			ioDrawContents.outTextTraits.style = 0;
         break;
         case kSizeColumn:						// Size column
			ioDrawContents.outShowSelection = true;
			ioDrawContents.outHasIcon = false;
			ioDrawContents.outTextTraits.style = 0;
            if (GetItem().GetItemType() == CDAVItem::COLLECTION)
               LString::CopyPStr("\p-", ioDrawContents.outTextString);
            else {
               pval = GetItem().GetPropertyValue(ContentLength);
               size = atoi(pval.c_str());
               BytesToString(size, text);
               LString::CopyPStr(text, ioDrawContents.outTextString);
            }
         break;
         case kKindColumn:						// Kind column
            ioDrawContents.outShowSelection = true;
            ioDrawContents.outHasIcon = false;
            ioDrawContents.outTextTraits.style = 0;
            if (GetItem().GetItemType() == CDAVItem::COLLECTION) {
               LStr255 colTmp;
               colTmp.Assign(str_UIStrings, str_Collection);
               LString::CopyPStr(colTmp, ioDrawContents.outTextString);
			} else {
               LStr255 fileTmp;
               //fileTmp.Assign(str_UIStrings, str_File);
               //LString::CopyPStr(fileTmp, ioDrawContents.outTextString);
               pval = GetItem().GetPropertyValue(ContentType);
               text=pval.c_str();
               LString::CopyPStr(text, ioDrawContents.outTextString);
            }
            break;
         case kLockOwnerColumn:						// Kind column
            ioDrawContents.outShowSelection = true;
            ioDrawContents.outHasIcon = false;
            ioDrawContents.outTextTraits.style = 0;{
            LStr255 lockowner(GetItem().GetLockOwner().c_str());
            LString::CopyPStr(lockowner, ioDrawContents.outTextString);}
            break;
		default:
			  //nothing for now
			break;
		}
   
}

// ---------------------------------------------------------------------------
//		¥ AddItem()
// ---------------------------------------------------------------------------
//
void CDAVTableItem::AddItem(CDAVItem &item) {
   StMutex mutex(fAccess);
   
   ((CDAVTable*)mOutlineTable)->AddItem(item, this);
}

// ---------------------------------------------------------------------------
//		¥ AddItems()
// ---------------------------------------------------------------------------
//
void CDAVTableItem::AddItems(CDAVItemVector &items) {
   StMutex mutex(fAccess);
   ((CDAVTable*)mOutlineTable)->AddItems(items,  this);

}

// ---------------------------------------------------------------------------
//		¥ AddItems()
// ---------------------------------------------------------------------------
//
void CDAVTableItem::DrawCell(const STableCell& inCell, const Rect& inLocalCellRect) 
{
   ::PenNormal();
   
   LOutlineItem::DrawCell( inCell, inLocalCellRect);
   Handle icon;
   SOutlineDrawContents ioDrawContents;
   GetDrawContents(inCell, ioDrawContents);

#if PP_Target_Carbon
	PlotIconRef(&ioDrawContents.prIconFrame, (SInt16) ioDrawContents.outIconAlign,
							(SInt16) ioDrawContents.outIconTransform,
							kIconServicesNormalUsageFlag, mIconRef);
#endif

   if (GetItem().GetIsLocked()) {
      SInt16 resId;
      if (mItem.GetIsLocalLock())
         resId = kLocalUserLockIcon;
      else
         resId = kLockIcon;
      ::GetIconSuite(&icon, resId, kSelectorAllAvailableData);
      ::DetachResource(icon);
      ::PlotIconSuite(&ioDrawContents.prIconFrame, (SInt16) ioDrawContents.outIconAlign,
							(SInt16) ioDrawContents.outIconTransform, icon);
   }   

   if (GetItem().GetIsExecutable()) {
      ::GetIconSuite(&icon, kExecutableIcon, kSelectorAllAvailableData);
      ::DetachResource(icon);
      ::PlotIconSuite(&ioDrawContents.prIconFrame, (SInt16) ioDrawContents.outIconAlign,
							(SInt16) ioDrawContents.outIconTransform, icon);
   }
}

// ---------------------------------------------------------------------------
//		¥ AddItems()
// ---------------------------------------------------------------------------
//
void CDAVTableItem::RefreshCell(const STableCell& inCell)
{
	SOutlineDrawContents drawInfo;		// Find out what to draw here
	GetDrawContents(inCell, drawInfo);

	
										// Get port origin so we can convert
										//   from local to  port coords
	Point	portOrigin;
	mOutlineTable->GetPortOrigin(portOrigin);

										// If there's an icon, refresh it
		
#if PP_Target_Carbon
	mOutlineTable->RefreshRect(drawInfo.prIconFrame);
#else
	if (drawInfo.outHasIcon && (drawInfo.outIconSuite != nil)) {
		StRegion	iconRgn;
	
		::IconSuiteToRgn(iconRgn, &drawInfo.prIconFrame,
						 (SInt16) drawInfo.outIconAlign, drawInfo.outIconSuite);
		
		mOutlineTable->RefreshRgn(iconRgn);
	}
#endif		
	// If there's text, refresh it
	mOutlineTable->RefreshRect(drawInfo.prTextFrame);
}

// ---------------------------------------------------------------------------
//		¥ AddItems()
// ---------------------------------------------------------------------------
//
void CDAVTableItem::PrepareDrawContents(const STableCell& inCell, SOutlineDrawContents&	ioDrawContents) {
   LOutlineItem::PrepareDrawContents(inCell,ioDrawContents);	
   if (mHandlingClick)
      return;
      
   /* ***teb - little hack here; the update algorithm for LOutlineTable is too efficient sometimes.
      If an item is unlocked, the owner column is never updated since the text is now empty.  Force
      a redraw of this column.
   */
   if (5 == inCell.col) {
      ioDrawContents.prTextFrame = ioDrawContents.ioCellBounds;
   }
 }
 

// ---------------------------------------------------------------------------
//		¥ GetDescriptor()
// ---------------------------------------------------------------------------
//
void CDAVTableItem::GetDescriptor(Str255 outDescriptor) {
   LStr255 text;
   std::string textTmp;
   GetItem().GetFileName(textTmp); 
   text = textTmp.c_str();
   LString::CopyPStr(text, outDescriptor);
}

// ---------------------------------------------------------------------------
//		¥ ClickCell()
// ---------------------------------------------------------------------------
//
void CDAVTableItem::ClickCell(const STableCell& inCell, const SMouseDownEvent& inMouseDown) {
	mHandlingClick = true;
#if PP_Target_Carbon
	SOutlineDrawContents drawInfo;
	GetDrawContents(inCell, drawInfo);

	UInt16 icnRefCnt=0;
	OSErr err = GetIconRefOwners(mIconRef, &icnRefCnt);
	if (err == noErr && icnRefCnt > 0) {
		if (::PtInIconRef(&inMouseDown.whereLocal, &drawInfo.prIconFrame, (SInt16) drawInfo.outIconAlign, kIconServicesNormalUsageFlag, mIconRef)) {
			TrackContentClick(inCell, inMouseDown, drawInfo, false);
			return;
		}
	}
#endif
	LOutlineItem::ClickCell(inCell, inMouseDown);
	mHandlingClick = false;
}

// ---------------------------------------------------------------------------
//		¥ CellHitByMarquee()
// ---------------------------------------------------------------------------
//
Boolean CDAVTableItem::CellHitByMarquee(const STableCell& inCell, const Rect& inMarqueeLocalRect) {
	mHandlingClick = true;
#if PP_Target_Carbon
	SOutlineDrawContents drawInfo;
	GetDrawContents(inCell, drawInfo);
	UInt16 icnRefCnt=0;
	OSErr err = GetIconRefOwners(mIconRef, &icnRefCnt);
	if (err == noErr && icnRefCnt > 0) {
		if (::RectInIconRef(&inMarqueeLocalRect, &drawInfo.prIconFrame, (SInt16) drawInfo.outIconAlign, kIconServicesNormalUsageFlag, mIconRef)) {
	       return true;
	    }
	}
#endif
	
   bool retVal =  LOutlineItem::CellHitByMarquee(inCell, inMarqueeLocalRect);
   mHandlingClick = false;
   return retVal;
}
/* ==================================================================================================
 * CDAVTable.cp															   
 *    Goliath - a Finder like application that implements WebDAV
 *    Copyright (C) 1999-2002  Thomas Bednarz
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
 
#ifndef __CDAVTABLE_H__
#include "CDAVTable.h"
#endif
#ifndef __CDAVTableApp_h__ 
#include "CDAVTableApp.h"
#endif
#ifndef __CDAVTableWindow_h__
#include "CDAVTableWindow.h"
#endif
#include "CDAVTableComparators.h"
#include "CDAVTableItem.h"
#include <UGAColorRamp.h>
#include <LOutlineItem.h>
#include <LTablePaneHostingGeometry.h>
#include <LTableMultiGeometry.h>
#include <LOutlineRowSelector.h>
#include <LOutlineKeySelector.h>
#include <LFastArrayIterator.h>
#include <UThread.h>
#include <Drag.h>
#include <UDragAndDropUtils.h>
#include <UInternetConfig.h>
#include <CDownloadFileThread.h>
#include <CMoveItemsToTrashThread.h>
#include <CIconSuiteManager.h>
#include <UCursor.h>
#include <UDrawingState.h>
#include <UAttachments.h>
#include <Folders.h>
#include <Dialogs.h>

#include "DataTransfer.h"

const UInt8		kMaxColumns = 50;


// ---------------------------------------------------------------------------
//		¥ CDAVTable()
// ---------------------------------------------------------------------------
//
CDAVTable::CDAVTable(LStream *inStream) : CSortableOutlineTable(inStream, new CDAVTableComparator(), true), mWnd(NULL), mSortColumn(0),
    LDragAndDrop(
#if PP_Target_Carbon
    UQDGlobals::GetCurrentWindowPort(),
#else
    UQDGlobals::GetCurrentPort(),
#endif
    this) {
    
	AddAttachment(new LOutlineKeySelector(this, msg_AnyMessage));
	AddAttachment(new LKeyScrollAttachment(this));

	if (mSuperCommander != nil) {
		mSuperCommander->SetLatentSub(this);
	}
}

// ---------------------------------------------------------------------------
//		¥ ~CDAVTable()
// ---------------------------------------------------------------------------
//
CDAVTable::~CDAVTable() {
}


// ---------------------------------------------------------------------------
//		¥ SetSortColumn()
// ---------------------------------------------------------------------------
//
void CDAVTable::SetSortColumn(TableIndexT sCol) {
   mSortColumn = sCol;
}


// ---------------------------------------------------------------------------
//		¥ Setup()
// ---------------------------------------------------------------------------
//
void CDAVTable::Setup(CDAVTableWindow* wnd, UInt8 rowHeight, UInt8 , UInt16 columnWidths[]) {
   mWnd = wnd;

   SetTableGeometry(new LTablePaneHostingGeometry(this, columnWidths[0], rowHeight));
   SetTableSelector(new LOutlineRowSelector( this ) );

	mRows = 0;	
}


// ---------------------------------------------------------------------------
//		¥ AddItems()
// ---------------------------------------------------------------------------
//
void CDAVTable::AddItems(CDAVItemVector &items, class LOutlineItem *parent) {
   StMutex mutex(fAccess);
   CDAVItemVector::iterator iter = items.begin();
      while (iter != items.end()) {
         CDAVItem &item = *iter;
		 const LStr255 &href = item.GetPHREF();
         Handle icon=NULL;
          
         UInt8 nameStart;
         if (item.GetItemType() == CDAVItem::COLLECTION) {

/*
#if PP_Target_Carbon
            IconRef iconRef;
            IconFamilyHandle iconFamily;
            OSErr err = ::GetIconRef(kOnSystemDisk, kSystemIconsCreator, kGenericFolderIcon, &iconRef);
            err = ::IconRefToIconFamily(iconRef, kSelectorAllAvailableData, &iconFamily);
            err = ::IconFamilyToIconSuite(iconFamily, kSelectorAllAvailableData, &icon);
            ReleaseIconRef(iconRef);
#else
            ::GetIconSuite(&icon, kGenericFolderIconResource, kSelectorAllAvailableData);
#endif
            //::DetachResource(icon);
*/
            nameStart = href.ReverseFind('/', href.Length()-1);
            _AddItem(item, parent);
         } else {            
            //Handle icnSuite = isMgr->GetIconForItem(item);
            nameStart = href.ReverseFind('/');
            //::DetachResource(icnSuite);
            _AddItem(item, parent);
         }
         iter++;
      }
      if (parent != NULL)
         ((CDAVTableItem*)parent)->ResetTransaction();
      Refresh();
}

// ---------------------------------------------------------------------------
//		¥ _AddItem()
// ---------------------------------------------------------------------------
//
void CDAVTable::_AddItem(CDAVItem &item, class LOutlineItem *parent) {
	
	if (NULL != parent) {
	  //parent may have been deleted if a row has been collapsed!  
	  TableIndexT idx =  FindRowForItem(parent);
	  if (( LArray::index_Bad == idx) || (parent->IsExpanded()==false))
	     return;
	}
	
	LOutlineItem *newItem;
    newItem = (LOutlineItem*)new CDAVTableItem();
	
	((CDAVTableItem*)newItem)->SetItem(item);
	
    InsertSortedItem(newItem, parent, false /*refresh*/, true);
}


// ---------------------------------------------------------------------------
//		¥ AddItem()
// ---------------------------------------------------------------------------
//
void CDAVTable::AddItem(CDAVItem &item, class LOutlineItem *parent) {
	StMutex mutex(fAccess);
   _AddItem(item, parent);
}


// ---------------------------------------------------------------------------
//		¥ Sort()
// ---------------------------------------------------------------------------
//
void CDAVTable::Sort(Boolean inRefresh) {
   StMutex mutex(fAccess);   
   CSortableOutlineTable::Sort(inRefresh);
}

// ---------------------------------------------------------------------------
//		¥ SetSortColor()
// ---------------------------------------------------------------------------
//
void CDAVTable::SetSortColor(const RGBColor &color) {
	mSortColor.red = color.red;
	mSortColor.green = color.green;
	mSortColor.blue = color.blue;
}

// ---------------------------------------------------------------------------
//		¥ RemoveItem()
// ---------------------------------------------------------------------------
//
void CDAVTable::RemoveItem(LOutlineItem* inOutlineItem, Boolean inRefresh, Boolean inAdjustImageSize) {
   StMutex mutex(fAccess);   
   LOutlineTable::RemoveItem(inOutlineItem, inRefresh, inAdjustImageSize);
}

// ---------------------------------------------------------------------------
//		¥ DrawOverhang()
// ---------------------------------------------------------------------------
//
void CDAVTable::DrawOverhang() {
    
	int contentHeight = mRows * 19;
	
	Rect tableRect;
	//CalcPortFrameRect(tableRect);
	
	CalcLocalFrameRect(tableRect);
	if (contentHeight >= tableRect.bottom)
		return;
		
	tableRect.right++;
	tableRect.top = contentHeight + 1;
	FocusDraw();
	::RGBBackColor(&mRowColor);
	::EraseRect(&tableRect);
	
	int i;
	UInt16 rightEdge = tableRect.right;
	tableRect.left = 0;
	
	for (i = 1; i < mSortColumn; i++)
		tableRect.left += GetColWidth(i);
	
	tableRect.right = tableRect.left + GetColWidth(i);
	
	// if the sort column isn't off to the right, draw it
	if (tableRect.left < rightEdge)	{
		if (tableRect.right > rightEdge)
			tableRect.right = rightEdge;
		FocusDraw();
		::RGBBackColor(&mSortColor);
		::EraseRect(&tableRect);
	}
}

// ---------------------------------------------------------------------------
//		¥ DrawRowBackground()
// ---------------------------------------------------------------------------
//
void CDAVTable::DrawRowBackground(TableIndexT, const Rect &inLocalRowRect) {
    
	Rect shadeRect = inLocalRowRect;
	shadeRect.top++;					// leave a white separator line between the rows
	
	int i;
	for (i = 1; i < mSortColumn; i++)
		shadeRect.left += GetColWidth(i);
	
	shadeRect.right = shadeRect.left + GetColWidth(i);
	
	FocusDraw();
	::RGBBackColor(&mSortColor);
	::EraseRect(&shadeRect);
}

// ---------------------------------------------------------------------------
//		¥ GetRowColor()
// ---------------------------------------------------------------------------
//
RGBColor* CDAVTable::GetRowColor(TableIndexT) {
	return &mRowColor;
}

// ---------------------------------------------------------------------------
//		¥ SetRowColor()
// ---------------------------------------------------------------------------
//
void CDAVTable::SetRowColor(const RGBColor &color) {
	mRowColor.red = color.red;
	mRowColor.green = color.green;
	mRowColor.blue = color.blue;
}

// ---------------------------------------------------------------------------
//		¥ CellSingleClicked()
// ---------------------------------------------------------------------------
//
void CDAVTable::CellSingleClicked(CDAVTableItem *inCell) {
	BroadcastMessage(msg_RowSingleClicked, (void*)&inCell->GetItem());
}

// ---------------------------------------------------------------------------
//		¥ CellDoubleClicked()
// ---------------------------------------------------------------------------
//
void CDAVTable::CellDoubleClicked(CDAVTableItem *inCell) {
	BroadcastMessage(msg_RowDoubleClicked, (void*)&inCell->GetItem());
}

// ---------------------------------------------------------------------------
//		¥ RemoveAllOutlineItems()
// ---------------------------------------------------------------------------
//
void CDAVTable::RemoveAllOutlineItems() {
	StMutex mutex(fAccess);   
/*	    
    LArray tmpItems(GetItems());
	LArrayIterator indexer(tmpItems, LArrayIterator::index_AfterEnd);
    LOutlineItem				*item;
	
	while (indexer.Previous(&item))
	   LOutlineTable::RemoveItem(item, false, false);
	
	if (mFirstLevelItems.GetCount() > 0) {
		Assert_(0);
		mFirstLevelItems.RemoveAllItemsAfter(0);
	}
*/	
    LArray tmpItems(GetItems());
	
	LTableView::RemoveRows(GetItems().GetCount(), 1, false);
	mOutlineItems.RemoveAllItemsAfter(0);
	mFirstLevelItems.RemoveAllItemsAfter(0);

	LArrayIterator indexer(tmpItems, LArrayIterator::index_AfterEnd);
    LOutlineItem				*item;
	
	while (indexer.Previous(&item))
	   delete item;

    Refresh();
}

// ---------------------------------------------------------------------------
//		¥ ScrollImageBy()
// ---------------------------------------------------------------------------
//
void CDAVTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, Boolean inRefresh) {
   ScrollImageArgsStruct scrollImageArgs;
                             
   // call inherited method
   CSortableOutlineTable::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh); 
                             
   scrollImageArgs.leftDelta = inLeftDelta;
   scrollImageArgs.topDelta = inTopDelta;
   scrollImageArgs.refresh = inRefresh;
   BroadcastMessage(msg_TableViewScrolled, &scrollImageArgs);
}

                            
// ----------------------------------------------------------------------------
//	¥ ItemIsAcceptable
// ----------------------------------------------------------------------------
// Called by the Drag Manager via PowerPlant to determine if we will take what
//   was just dragged into the pane.

Boolean CDAVTable::ItemIsAcceptable(DragReference inDragRef, ItemReference inItemRef)
{
	FlavorFlags		theFlags;

	if (GetFlavorFlags(inDragRef, inItemRef, cDAVItemType, &theFlags) == noErr) {        
		if (CheckIfViewIsAlsoSender(inDragRef))
		   return false;//return _IsDragOverFolder(inDragRef);
		else
		   return true;
    }

	if (GetFlavorFlags(inDragRef, inItemRef, flavorTypeHFS, &theFlags) == noErr) {        
		return true;
    }
    
	return false;
}

// ---------------------------------------------------------------------------
//	¥ _IsDragOverFolder													  
// ---------------------------------------------------------------------------
//	
Boolean CDAVTable::_IsDragOverFolder(DragReference inDragRef) {
	ItemReference	theItemRef;
	Point			mouse,pinnedMouse;
	SPoint32		imagePt;
	STableCell		hitCell;
	
	GetDragItemReferenceNumber(inDragRef, 1, &theItemRef);
	GetDragMouse(inDragRef, &mouse, &pinnedMouse);
	
	GlobalToLocal(&pinnedMouse);
	LocalToImagePoint(pinnedMouse,imagePt);
	CDAVTableItem* pntItem = NULL;
	
	if((GetCellHitBy(imagePt, hitCell))) {
	  CDAVTableItem* ditem =  reinterpret_cast<CDAVTableItem*>(FindItemForRow(hitCell.row));
	  if ((NULL != ditem) && (ditem->GetItem().GetItemType() == CDAVItem::COLLECTION))
         return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
//	¥ _getItemUnderDrag												  
// ---------------------------------------------------------------------------
//	
CDAVTableItem* CDAVTable::_getItemUnderDrag(DragReference inDragRef) {
	ItemReference	theItemRef;
	Point			mouse,pinnedMouse;
	SPoint32		imagePt;
	STableCell		hitCell;
	
	GetDragItemReferenceNumber(inDragRef, 1, &theItemRef);
	GetDragMouse(inDragRef, &mouse, &pinnedMouse);
	
	GlobalToLocal(&pinnedMouse);
	LocalToImagePoint(pinnedMouse,imagePt);
	CDAVTableItem* pntItem = NULL;
	
	if((GetCellHitBy(imagePt, hitCell))) {
	  return reinterpret_cast<CDAVTableItem*>(FindItemForRow(hitCell.row));
    }
    return NULL;
}

// ---------------------------------------------------------------------------
//	¥ DoDragReceive													  [public]
// ---------------------------------------------------------------------------
//	Receive the items from a completed Drag and Drop
//
//	This function gets called after items are dropped into a DropArea.
//	The drag contains items that are acceptable, as defined by the
//	DragIsAcceptable() and ItemIsAcceptable() member functions.
//
//	This function repeatedly calls ReceiveDragItem for each item in the drag.
//	Override if you want to process the dragged items all at once.

void CDAVTable::DoDragReceive(DragReference	inDragRef) {
    FSSpecVector files;

	DragAttributes	dragAttrs;
	::GetDragAttributes(inDragRef, &dragAttrs);
	
	UInt16	itemCount;				// Number of Items in Drag
	::CountDragItems(inDragRef, &itemCount);
	
	for (UInt16 item = 1; item <= itemCount; item++) {
		ItemReference	itemRef;
		::GetDragItemReferenceNumber(inDragRef, item, &itemRef);
		
		Rect	itemBounds;			// Get bounds of Item in Local coords
		LView::OutOfFocus(nil);
		FocusDropArea();
		::GetDragItemBounds(inDragRef, itemRef, &itemBounds);
		::GlobalToLocal(&topLeft(itemBounds));
		::GlobalToLocal(&botRight(itemBounds));
		
	    Size theDataSize;	// How much data there is for us.
	    HFSFlavor theHFS;
        // If this is our own drag ignore it.
        if (!CheckIfViewIsAlsoSender(inDragRef)) {
           ::GetFlavorDataSize(inDragRef, itemRef, flavorTypeHFS, &theDataSize);
           if (theDataSize) {
              ::GetFlavorData(inDragRef, itemRef, flavorTypeHFS, &theHFS, &theDataSize, 0L);
              files.push_back(theHFS.fileSpec);
		   }
		} 
		
/*
		OSErr davDataErr = ::GetFlavorDataSize(inDragRef, itemRef, cDAVItemType, &theDataSize);
        if (davDataErr == noErr && theDataSize) {
           Ptr dataPtr = NewPtrClear(theDataSize);
           ::GetFlavorData(inDragRef, itemRef, cDAVItemType, dataPtr, &theDataSize, 0L);
           DAVItemData theDragData;
           XML_Error xErr = DAVItemData::ParseDAVItemClip(dataPtr, theDataSize, &theDragData);
        }
*/
	}

	if (files.size() == 0)
		return;
		
	CDAVTableItem* pntItem = NULL;
    CDAVTableItem* ditem;
    ditem = _getItemUnderDrag(inDragRef);
    if ((NULL != ditem) && (ditem->GetItem().GetItemType() == CDAVItem::COLLECTION))
       pntItem = ditem;
       
    mWnd->UploadFSSpecVector(files, pntItem);
}


// ----------------------------------------------------------------------------
//	¥ CreateDragEvent
// ----------------------------------------------------------------------------
//
void CDAVTable::CreateDragEvent(SMouseDownEvent	inMouseDown)
{
	CDAVItemVector items;
   items = GetAllSelectedItems();

    LDragTask theDragTask(inMouseDown.macEvent);

    OSErr err;
    PromiseHFSFlavor phfs;
		phfs.fileType = '????';
		phfs.fileCreator = '????';
		phfs.promisedFlavor = 'fssP';
    
    err = ::SetDragSendProc( theDragTask.GetDragReference(),
						   NewDragSendDataUPP(LDropArea::HandleDragSendData),
						   (LDropArea*)this);

    err = ::AddDragItemFlavor(theDragTask.GetDragReference(), 1, flavorTypePromiseHFS,  &phfs, sizeof(phfs), flavorNotSaved);
	err = ::AddDragItemFlavor(theDragTask.GetDragReference(), 1, 'fssP', NULL, 0, flavorNotSaved);			

    std::string clipData;
    DAVItemData::DAVItemsToClip(items, mWnd->GetDAVContext(), false/*isCut*/, clipData);
    err = ::AddDragItemFlavor(theDragTask.GetDragReference(), 1, cDAVItemType, clipData.c_str(), clipData.size(), flavorSenderOnly);			
   	
    mDragItems = GetAllSelectedItems();

    StRegion mDragRegion;
    BuildDragRegionFromSelection(theDragTask, 1, mDragRegion);
	::TrackDrag(theDragTask.GetDragReference(), &inMouseDown.macEvent, mDragRegion);					   							
}

// ----------------------------------------------------------------------------
//	¥ DoDragSendData
// ----------------------------------------------------------------------------
//
void CDAVTable::DoDragSendData(FlavorType /*inFlavor*/, ItemReference inItemRef, DragReference inDragRef ) {
   Size theDataSize;

   ::GetFlavorDataSize(inDragRef, inItemRef, flavorTypePromiseHFS, &theDataSize);
   if (theDataSize) {
      PromiseHFSFlavor theItem;
      ::GetFlavorData(inDragRef, inItemRef, flavorTypePromiseHFS, &theItem, &theDataSize, 0L);
      _onDragSendData(inDragRef);
   }
	   
}

// ----------------------------------------------------------------------------
//	¥ _onDragSendData
// ----------------------------------------------------------------------------
//
Boolean CDAVTable::_onDragSendData(DragReference  inDragRef) {
   Boolean destCreated;
   AEDesc dropLocDesc, targetDirDesc;
   FSSpec destinationDir;
   OSErr err;
   CInfoPBRec cat;
   long destinationDirID;
   CDAVItemVector items;
      
   LStr255 tmpStr;
   /* set up locals */
   //AECreateDesc(typeNull, NULL, 0, NULL);
   //AECreateDesc(typeNull, NULL, 0, NULL);
   AECreateDesc(typeNull, NULL, 0, &dropLocDesc);
   AECreateDesc(typeNull, NULL, 0, &targetDirDesc);
   
   destCreated = false;
	
   /* get the drop location where */
   err = ::GetDropLocation(inDragRef, &dropLocDesc);
   tmpStr = err;
   if (err != noErr) goto bail;

   /* attempt to convert the location record to a FSSpec.  By doing it this way
      instead of looking at the descriptorType field,  we don't need to know what
	  type the location originally was or what coercion handlers are installed.  */
   err = ::AECoerceDesc(&dropLocDesc, typeFSS, &targetDirDesc);
   tmpStr = err;
   if (err != noErr) goto bail;
 
#if PP_Target_Carbon
   //***teb - carbon CHECK THIS
   //Ptr outPtr =NULL;
   //AEGetDescData(&targetDirDesc, outPtr, sizeof(FSSpec));                            
   //::BlockMoveData(outPtr, reinterpret_cast<void*>(&destinationDir), sizeof(FSSpec));
    err = ::AEGetDescData(&targetDirDesc, &destinationDir, sizeof(FSSpec));  
    tmpStr = err;
    if (err != noErr) goto bail;
#else
   BlockMoveData(*targetDirDesc.dataHandle, &destinationDir, sizeof(FSSpec));   
#endif
   
   cat.hFileInfo.ioNamePtr = destinationDir.name;
   cat.hFileInfo.ioVRefNum = destinationDir.vRefNum;
   cat.hFileInfo.ioDirID = destinationDir.parID;
   cat.hFileInfo.ioFDirIndex = 0;
   err = PBGetCatInfoSync(&cat);
   tmpStr = err;
   if (err != noErr) goto bail;
   destinationDirID = cat.hFileInfo.ioDirID;
   destinationDir.parID=destinationDirID;

   AEDisposeDesc(&targetDirDesc);
   //items = GetAllSelectedItems();
   items = mDragItems;
   LThread *thread = NULL;

   if (UDragAndDropUtils::DroppedInTrash(inDragRef)) {
      SInt16 itemHit;
      AlertStdAlertParamRec	param;
      param.movable 		= false;
      param.filterProc 	= nil;
      param.defaultText 	= "\pOk";
      param.cancelText 	= "\pCancel";
      param.otherText 	= nil;
      param.helpButton 	= false;
      param.defaultButton = kAlertStdAlertOKButton;
      param.cancelButton 	= kAlertStdAlertCancelButton;
      param.position 		= 0;
      LStr255 text, desc;
      text.Assign(str_UIStrings, str_Warning);
      desc.Assign(str_UIStrings, str_DeleteConfirmation);
      err=StandardAlert( kAlertCautionAlert, desc, nil, &param, &itemHit );
   
      if ((noErr != err) || (itemHit ==2))
         return false;
      thread = new CMoveItemsToTrashThread(mWnd->GetDAVContext(), mWnd, destinationDir,items);
   } else {
      thread = new CDownloadFileThread(mWnd->GetDAVContext(), mWnd, destinationDir,items);
   }
   if (NULL == thread)
      return false;
     
   thread->Resume();
	
   return true;

bail:
   AEDisposeDesc(&dropLocDesc);
   AEDisposeDesc(&targetDirDesc); 
   return false;   
}



// ----------------------------------------------------------------------------
//	¥ CreateDragEvent
// ----------------------------------------------------------------------------
//
void CDAVTable::HiliteDropArea(DragReference /*inDragRef*/) {
	mPane->ApplyForeAndBackColors();

	Rect	dropRect;
	mPane->CalcLocalFrameRect(dropRect);
    
    //***teb - ok, so here's an awful hack.  ShowDragHilite doesn't seem to want
    // to work with LOutlineTable; has something to do with the way my LOutlineItem
    // derived class shades rows to look all Finder¨ like and all.  I work around this
    // by drawing the highlight manually.  If you think this is bad, check out the 
    // drag leave handler.
    //StRegion	dropRgn(dropRect);
    //::ShowDragHilite(inDragRef, dropRgn, true);

    PenSize(2,2);
    RGBColor color;
    GetDragHiliteColor(
#ifdef PP_Target_Carbon
        mWnd->GetMacWindow(),
#else
        mWnd->GetMacPort(),
#endif
        &color);
	RGBForeColor(&color);
	MacFrameRect(&dropRect);						 	
}

// ----------------------------------------------------------------------------
//	¥ InsideDropArea
// ----------------------------------------------------------------------------
// Called by the Drag Manager via PowerPlant whenever there is drag in our pane
// Here we do the visual feedback

#define kControlKey1					0x3B
#define kControlKey2					0x3E

void CDAVTable::InsideDropArea(	DragReference	inDragRef)
{
	ItemReference	theItemRef;
	Point			mouse,pinnedMouse;
	SPoint32		imagePt;
	STableCell		hitCell;
	
	GetDragItemReferenceNumber(inDragRef, 1, &theItemRef);
	GetDragMouse(inDragRef, &mouse, &pinnedMouse);
	
	GlobalToLocal(&pinnedMouse);
	LocalToImagePoint(pinnedMouse,imagePt);
    
    //temporary hack since we don't handle internal drags
    if (CheckIfViewIsAlsoSender(inDragRef))
    	return;
    
    
    Byte theKeys[16];
#if PP_Target_Carbon
	::GetKeys((SInt32 *)theKeys);
#else
	::GetKeys((UInt32 *)theKeys);
#endif
	if (((theKeys[kControlKey1 >> 3] >> (kControlKey1 & 7)) & 1) ||
				  ((theKeys[kControlKey2 >> 3] >> (kControlKey2 & 7)) & 1)) 
	   UCursor::SetTheCursor(128);
	else
	   UCursor::SetTheCursor(0);

	if((GetCellHitBy(imagePt, hitCell))) {
	  CDAVTableItem* ditem =  reinterpret_cast<CDAVTableItem*>(FindItemForRow(hitCell.row));
	  if (NULL == ditem)
	     return;
	  if (ditem->GetItem().GetItemType() != CDAVItem::COLLECTION) {
	     if (mLastDragSelCell.row==0 && mLastDragSelCell.col==0)
	        return; 
	     UnselectCell(mLastDragSelCell);
         mLastDragSelCell.row=mLastDragSelCell.col=0;
         Draw(nil);
	     HiliteDropArea(inDragRef);
	     return;
	  }
	  if (mLastDragSelCell != hitCell) {
         SelectCell(hitCell);
         SelectionChanged();
         UnselectCell(mLastDragSelCell);
         mLastDragSelCell = hitCell;
         Draw(nil);
      }
      
   } else if ((0 != mLastDragSelCell.row) && (0 != mLastDragSelCell.col)) {
      UnselectCell(mLastDragSelCell);
      mLastDragSelCell.row=0;
      mLastDragSelCell.col=0;
      Draw(nil);
   }
	
	HiliteDropArea(inDragRef);
	
}

// ----------------------------------------------------------------------------
//	¥ ClickSelf
// ----------------------------------------------------------------------------
// 
void CDAVTable::ClickSelf(const SMouseDownEvent& inMouseDown) {
	CSortableOutlineTable::ClickSelf(inMouseDown);
	
	SPoint32 imagePt;
	Point theMouse = inMouseDown.macEvent.where;
	GlobalToLocal(&theMouse);
	LocalToImagePoint(theMouse, imagePt);
	STableCell hitCell;
	if(!GetCellHitBy(imagePt, hitCell)) {
		return;
    }

   if (::StillDown() && GetClickCount() == 1) {
	   Boolean isDrag = ::WaitMouseMoved(inMouseDown.macEvent.where);
	
	   if (isDrag) {
		   CreateDragEvent(inMouseDown);
	   }
	}

}


// ----------------------------------------------------------------------------
//	¥ LeaveDropArea
// ----------------------------------------------------------------------------
// Called by the Drag Manager via PowerPlant whenever the drag leaves our pane
void CDAVTable::LeaveDropArea(DragReference inDragRef) {
	LDropArea::LeaveDropArea(inDragRef);  // call inherited method
	
    //***teb - ok, so here's an awful hack.  ShowDragHilite doesn't seem to want
    // to work with LOutlineTable; has something to do with the way my LOutlineItem
    // derived class shades rows to look all Finder¨ like and all.  I worked around this
    // by drawing the highlight manually.  Now, i'm being lazy and having the entire view
    // repaint itself to remove the highligh.  Gag!  At least LOutlineTable is smart 
    // in it's drawing in that it only repaints the visible cells.  I could get snazzy here
    // and call Draw twice, once with the first visible row's Region and second with the
    // last visible row's Region.  I should do this soon, because this does seem pretty blinky
    //  I'd also have to update the first and last rows as well.
	UnselectCell(mLastDragSelCell);
    Draw(nil);
    mLastDragSelCell.row=0;
    mLastDragSelCell.col=0;
    UCursor::SetTheCursor(0);
}



// ----------------------------------------------------------------------------
//	¥ CheckForTrash
// ----------------------------------------------------------------------------
//	Did we drag to the trash?
//	If so the put a clipping in the trash and return true.

Boolean CDAVTable::CheckForTrash(LDragTask* inDragTask)
{
	OSErr	theErr;
	AEDesc	theDropDestination;
	
	DragReference theDragRef = inDragTask->GetDragReference();
	theErr = ::GetDropLocation(theDragRef, &theDropDestination);
	
	if ((theErr) || (theDropDestination.descriptorType == typeNull))
		return false;
	
	if (theDropDestination.descriptorType == typeAlias) {
		//
		// The drag was to the finder. The question now is whether it
		// was to the trash.
		//
		Boolean	aliasWasChanged;
		FSSpec	theDestinationFSSpec;
		FSSpec	theTrashFSSpec;
		short	theTrashVRefNum;
		long	theTrashDirID;
		//
		// First, build the FSSpec of the destination to which the user dragged
		// the object
		//
		HLock(reinterpret_cast<Handle>(theDropDestination.dataHandle));
		theErr = ::ResolveAlias(	nil,
									(AliasHandle) theDropDestination.dataHandle,
									&theDestinationFSSpec,
									&aliasWasChanged);
		HUnlock(reinterpret_cast<Handle>(theDropDestination.dataHandle));
		if (theErr)
			return false;
		
		//
		// Next, find the FSSpec of the system's trash
		//
		theErr = ::FindFolder(	kOnSystemDisk, kTrashFolderType, kDontCreateFolder,
								&theTrashVRefNum, &theTrashDirID);
		if (theErr)
			return false;
		
		theErr = ::FSMakeFSSpec( theTrashVRefNum, theTrashDirID, nil, &theTrashFSSpec);
		if (theErr)
			return false;
		
		//
		// Compare the two FSSpecs.
		//
		if ((			theDestinationFSSpec.vRefNum ==	theTrashFSSpec.vRefNum)
		&& (			theDestinationFSSpec.parID	 ==	theTrashFSSpec.parID)
		&& (EqualString(theDestinationFSSpec.name,		theTrashFSSpec.name, false, true))) {
			//
			// Since the FSSpec of the destination of the drag is the same as the FSSpec of
			// the trash, the drag was to the trash.
			//
			// We get to this point _after_ the clipping file has been placed in the trash
			// so to complete the 'move', we simply delete this item.
			//
			return true;
		} else return false;
	}
	return false;
}


// ----------------------------------------------------------------------------
//	¥ CheckIfViewIsAlsoSender
// ----------------------------------------------------------------------------
//	This routine checks to see if the given drag came from the same view.
Boolean CDAVTable::CheckIfViewIsAlsoSender(DragReference inDragRef)
{	

	DragAttributes theDragAttributes;
	::GetDragAttributes(inDragRef, &theDragAttributes);
	
	return (theDragAttributes & kDragInsideSenderWindow);
}


// ----------------------------------------------------------------------------
//	¥ OnlyOneItemSelected
// ----------------------------------------------------------------------------
//	returns true if only 1 item is selected; false if 0 or more than 1
Boolean CDAVTable::OnlyOneItemSelected() {
	STableCell		cell;
	UInt32			numRows,numCols;
	
	GetTableSize(numRows,numCols);
	Boolean alreadyGotOne=false; //"I told them we already had one..."
	
	for (cell.row=1; cell.row<=numRows; cell.row++) {
		if (CellIsSelected(cell)) {
		   if (!alreadyGotOne)
		      alreadyGotOne=true;
		   else
		      return false;
		}
	}
	
	return alreadyGotOne;
}

// ----------------------------------------------------------------------------
//	¥ atLeastOneItemSelected
// ----------------------------------------------------------------------------
//	
Boolean CDAVTable::atLeastOneItemSelected() {
	STableCell		cell;
	UInt32			numRows,numCols;
	
	GetTableSize(numRows,numCols);
	
	for (cell.row=1; cell.row<=numRows; cell.row++) {
		if (CellIsSelected(cell)) {
		   return true;
		}
	}
	return false;
}


// ----------------------------------------------------------------------------
//	¥ atLeastOneItemSelectedIs
// ----------------------------------------------------------------------------
//	
Boolean CDAVTable::atLeastOneItemSelectedIs(CDAVTable::SelectionTypes type) {
	STableCell		cell;
	UInt32			numRows,numCols;
	
	GetTableSize(numRows,numCols);
	
	for (cell.row=1; cell.row<=numRows; cell.row++) {
		if (CellIsSelected(cell)) {
           CDAVTableItem *item=reinterpret_cast<CDAVTableItem*>(FindItemForRow(cell.row));
           CDAVItem& ditem = item->GetItem();
           if ((LOCKED==type)&&(ditem.GetIsLocked()) )
              return true;
           if ((UNLOCKED==type)&&(ditem.GetIsLocked()==false) )
              return true;
           if ((LOCKED_NOTLOCAL == type)&& ((ditem.GetIsLocked()==true && ditem.GetIsLocalLock()==false)))
              return true;
           if ((LOCKED_LOCAL == type)&& ((ditem.GetIsLocked()==true && ditem.GetIsLocalLock()==true)))
              return true;		
           if ((FILE == type) && (ditem.GetItemType()==CDAVItem::FILE))
              return true;
		}
	}
	return false;
}


// ----------------------------------------------------------------------------
//	¥ GetAllSelectedItems
// ----------------------------------------------------------------------------
//	
CDAVItemVector CDAVTable::GetAllSelectedItems() {
   CDAVItemVector items;

	STableCell		cell;
	UInt32			numRows,numCols;
	
	GetTableSize(numRows,numCols);
	
	for (cell.row=1; cell.row<=numRows; cell.row++) {
		if (CellIsSelected(cell)) {
		   CDAVTableItem *item=reinterpret_cast<CDAVTableItem*>(FindItemForRow(cell.row));
		   if (NULL != item) 
		      items.push_back(item->GetItem());
   		}
	}
   
   return items;
}

// ----------------------------------------------------------------------------
//	¥ GetTableItemForResource
// ----------------------------------------------------------------------------
//	
CDAVTableItem* CDAVTable::GetTableItemForResource (const std::string* resource) {
   CDAVItemVector items;

	STableCell		cell;
	UInt32			numRows,numCols;
	
	GetTableSize(numRows,numCols);
	
	for (cell.row=1; cell.row<=numRows; cell.row++) {
	   CDAVTableItem *item=reinterpret_cast<CDAVTableItem*>(FindItemForRow(cell.row));
	   if (NULL != item) {
	      if (item->GetItem().GetHREF().compare(*resource) == 0) {
	         return item;
	      }
	   }
   	  
	}
   
    return NULL;
}


// ----------------------------------------------------------------------------
//	¥ BuildDragRegionFromSelection
// ----------------------------------------------------------------------------
//	
void CDAVTable::BuildDragRegionFromSelection(LDragTask &inDragTask, ItemReference inItemRef, StRegion &mDragRegion) {
   Rect itemRect;

	STableCell		cell;
	UInt32			numRows,numCols;
	
	cell.col = 1;
	
	GetTableSize(numRows,numCols);
	
	for (cell.row=1; cell.row<=numRows; cell.row++) {
		if (CellIsSelected(cell)) {
		   CDAVTableItem *item=reinterpret_cast<CDAVTableItem*>(FindItemForRow(cell.row));
		   if (NULL != item) {
		      StRegion tmpRgn;
		      item->MakeDragRegion(cell, tmpRgn, itemRect);
	          StRegion	innerRgn = tmpRgn;	// Carve out interior of region so
	          ::InsetRgn(innerRgn, 2, 2);			//   that it's just a one-pixel thick
	          tmpRgn -= innerRgn;				//   outline of the item rectangle
	
	          mDragRegion += tmpRgn;			// Accumulate this item in our*/
		      ::SetDragItemBounds(inDragTask.GetDragReference(), inItemRef, &itemRect);
           }
   		}
	}
    
}


// ----------------------------------------------------------------------------
//	¥ RefreshItem
// ----------------------------------------------------------------------------
//	
void CDAVTable::RefreshItem(CDAVTableItem *inItem) {
   STableCell cell;
   cell.row=FindRowForItem(inItem);
   for (int i=0; i<6; i++) {
      cell.col=i;
      inItem->RefreshCell(cell);
   }
}




/* ==================================================================================================
 * CPropDisplayTable.cp															   
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

#include <LOutlineKeySelector.h>
#include <UAttachments.h>
#include <LTablePaneHostingGeometry.h>
#include <LOutlineRowSelector.h>
#include <LEditableOutlineItem.h>
#include <UNewTextDrawing.h>
#include <stdlib.h>
#include <UDrawingState.h>
#include <UDrawingUtils.h>


#include <CDAVProperty.h>
#include "CPropDisplayTable.h"
#include <CDAVTableAppConstants.h>
#include <string.h>
#include <LComparator.h>
#include <LowMem.h>
#include <LArrayIterator.h>
#include <LClipboard.h>

struct CPropTableItemDC  {
   std::string mCellData;
};

#ifdef PP_Target_Carbon
SInt32 min(SInt32 a, SInt32 b);
SInt32 min(SInt32 a, SInt32 b) {return a < b ? a : b;}
#endif

/**
   Subclass of LOutlineItem that exposes our data model and that
   also contains hooks to overcome the 256 character limit for cells
   imposed by LOutlineTable's use of LStr255 objects
**/
class CPropTableItem : public LOutlineItem  {
   public: 
      CPropTableItem(CDAVProperty &prop, std::string val);
      virtual ~CPropTableItem();

      void GetDrawContentsSelf( const STableCell& inCell, SOutlineDrawContents&	ioDrawContents);
      void GetDrawContentsSelf_Unlim( const STableCell& inCell, CPropTableItemDC& ioDrawContents);
      Boolean CanExpand() const {return false;};
      
      CDAVProperty& GetDAVProperty() {return mProperty;};
      std::string&  GetPropValue() { return mValue;};
      
   protected:
   	  virtual void UpdateRowSize();
   	  virtual void DrawCell(const STableCell&	inCell, const Rect& inLocalCellRect);
      virtual SInt16 CalcRowHeightForCell(const STableCell&	inCell);

      virtual void PrepareDrawContents(const STableCell& inCell , SOutlineDrawContents&	ioDrawContents);

      CDAVProperty mProperty;
      std::string  mValue;
      
      friend class CPropDisplayTable;
      
};

/*
   A Custom Comparator for this table class
*/
class CPropTableComparator : public LComparator
{
   protected:
      Boolean mIsAscending;
      int mSortColumn;
   public:
      CPropTableComparator():mIsAscending(true), mSortColumn(1) {;};
	  virtual ~CPropTableComparator() {;};
	  void SetSortColumn(int sortCol) {mSortColumn = sortCol;};
	  void SetIsAscending(Boolean asscend) {mIsAscending = asscend;};
	  
      virtual SInt32 Compare(const void* inItemOne, const void* inItemTwo,
								UInt32 inSizeOne, UInt32 inSizeTwo) const;
};


// ---------------------------------------------------------------------------
//		¥ CPropTableItem::CPropTableItem()
// ---------------------------------------------------------------------------
//
CPropTableItem::CPropTableItem(CDAVProperty &prop, std::string val): mProperty(prop), mValue(val) {
   mLeftEdge = 0;
   mIndentSize = 0;
}

// ---------------------------------------------------------------------------
//		¥ CPropTableItem::~CPropTableItem()
// ---------------------------------------------------------------------------
//
CPropTableItem::~CPropTableItem() {

}

// ---------------------------------------------------------------------------
//		¥ CalcRowHeightForCell()
// ---------------------------------------------------------------------------
//
SInt16 CPropTableItem::CalcRowHeightForCell(const STableCell&	inCell) {

	// Find out what we're supposed to draw.

	SOutlineDrawContents drawInfo;
	GetDrawContents(inCell, drawInfo);
    CPropTableItemDC drawInfo2;
    GetDrawContentsSelf_Unlim(inCell, drawInfo2);

	// Measure the font height and ensure that
	// there's space for at least one line of text.
	
	mOutlineTable->FocusDraw();
	UTextTraits::SetPortTextTraits(&drawInfo.outTextTraits);
	
	FontInfo fi;
	::GetFontInfo(&fi);
	
	SInt16 minRowHeight = (SInt16) (fi.ascent + fi.descent + 2);
			
	// If drawing multi-line text, measure the entire size of the string.

	if ((drawInfo.outMultiLineText) && (drawInfo.outTextString[0] > 0)) {
		
		Rect maxSize = { 0, 0, 32767, (SInt16) (drawInfo.ioCellBounds.right - drawInfo.ioCellBounds.left - 3) };
		Rect textSize;
		
			
		UNewTextDrawing::MeasureWithJustification(
			    (Ptr) drawInfo2.mCellData.c_str(),
				drawInfo2.mCellData.size(),
				maxSize,
				drawInfo.outTextTraits.justification,
				textSize);
		
		if (minRowHeight < textSize.bottom + 3)
			minRowHeight = (SInt16) (textSize.bottom + 3);
		
	}
	
	return minRowHeight;

}

// ---------------------------------------------------------------------------
//		¥ PrepareDrawContents()
// ---------------------------------------------------------------------------
//
void CPropTableItem::PrepareDrawContents(const STableCell& inCell, 
                                         SOutlineDrawContents& ioDrawContents) {
   LOutlineItem::PrepareDrawContents(inCell,ioDrawContents);	

	if (ioDrawContents.outMultiLineText) {
    	CPropTableItemDC drawInfo2;
    	GetDrawContentsSelf_Unlim(inCell, drawInfo2);
		
		// Multi-line text. Measure the text, but do not truncate it.
	
		Rect maxRect = ioDrawContents.ioCellBounds;
		::MacInsetRect(&maxRect, 3, 1);
		maxRect.top++;
		
		UNewTextDrawing::MeasureWithJustification(
				(Ptr) drawInfo2.mCellData.c_str(),
				drawInfo2.mCellData.size(),
				maxRect,
				ioDrawContents.outTextTraits.justification,
				ioDrawContents.prTextFrame);
			
		::MacInsetRect(&ioDrawContents.prTextFrame, -3, -1);

	}
 }


// ---------------------------------------------------------------------------
//		¥ CPropTableItem::GetDrawContentsSelf()
// ---------------------------------------------------------------------------
//
void CPropTableItem::GetDrawContentsSelf( const STableCell& inCell, SOutlineDrawContents& ioDrawContents) {
   ioDrawContents.outShowSelection = true;
   ioDrawContents.outHasIcon = false;
   ioDrawContents.outCanDoInPlaceEdit = false;
   ioDrawContents.outTextTraits.style = 0;
   ioDrawContents.outDoTruncation = true;
   ioDrawContents.outMultiLineText = true;
   
   if (1 == inCell.col) {
      LStr255 text = mProperty.GetNamespace().c_str();            
      text.Append(mProperty.GetPropertyName().c_str());
      LString::CopyPStr(text, ioDrawContents.outTextString);
   } else {
      LStr255 text = mValue.c_str();
      LString::CopyPStr(text, ioDrawContents.outTextString);   
   }
}

// ---------------------------------------------------------------------------
//		¥ CPropTableItem::GetDrawContentsSelf()
// ---------------------------------------------------------------------------
//
void CPropTableItem::GetDrawContentsSelf_Unlim( const STableCell& inCell, CPropTableItemDC& ioDrawContents) {   
   if (1 == inCell.col) {
      ioDrawContents.mCellData = mProperty.GetNamespace().c_str();
      ioDrawContents.mCellData+=mProperty.GetPropertyName();
   } else {
      for (int i=0; i<mValue.size(); i++)
         if ((mValue[i]=='\n')) {
            if (i!=0)
              ioDrawContents.mCellData += '\r';
         } else
            ioDrawContents.mCellData += mValue[i];
      //ioDrawContents.mCellData = mValue.c_str();
   }
}


// ---------------------------------------------------------------------------
//		¥ UpdateRowSize
// ---------------------------------------------------------------------------
//
void CPropTableItem::UpdateRowSize() {
	STableCell	theCell;
	theCell.row = FindRowForItem();
	theCell.col = 2;
	
	SOutlineDrawContents drawInfo;
	GetDrawContents(theCell, drawInfo);
    CPropTableItemDC drawInfo2;
    GetDrawContentsSelf_Unlim(theCell, drawInfo2);

	mOutlineTable->FocusDraw();
	UTextTraits::SetPortTextTraits(&drawInfo.outTextTraits);
	
	// If drawing multi-line text, measure the entire size of the string.
	if ( drawInfo.outMultiLineText )
	{	
		// If we are in the middle of the in place editing, the resizing should be done
		// to the text in the edit field.
		//if ( mEditField )
		//	mEditField->GetDescriptor( drawInfo.outTextString );
		
		if (drawInfo.outTextString[0] > 0)
		{
			Rect maxRect = { 0, 0, 32767, drawInfo.ioCellBounds.right - drawInfo.ioCellBounds.left - 6 };
			Rect textSize;
			if ( drawInfo.outHasIcon )
				maxRect.right -= 17;	// This is the corresponding amount in LOutlineTable.
			
			UNewTextDrawing::MeasureWithJustification(
					(Ptr) drawInfo2.mCellData.c_str(),
					drawInfo2.mCellData.size(),
					maxRect,
					drawInfo.outTextTraits.justification,
					textSize );					
			UInt16	newHeight = textSize.bottom + 2;
			if ( newHeight < 18 )
				newHeight = 18;
			mOutlineTable->SetRowHeight( newHeight, theCell.row, theCell.row );
		}
	}
}

// ---------------------------------------------------------------------------
//	¥ DrawCell													   [protected]
// ---------------------------------------------------------------------------
//
void CPropTableItem::DrawCell(const STableCell&	inCell, const Rect& inLocalCellRect) {
	// Find out what we're supposed to draw.

	SOutlineDrawContents drawInfo;
	GetDrawContents(inCell, drawInfo);

    CPropTableItemDC drawInfo2;
    GetDrawContentsSelf_Unlim(inCell, drawInfo2);

	// Draw disclosure triangle for first column.
	
	if ((inCell.col == 1) && CanExpand()) {
		DrawDisclosureTriangle();
	}

	// If there's an icon, draw it.
	
	if (drawInfo.outHasIcon && (drawInfo.outIconSuite != nil)) {
		if (drawInfo.outShowSelection && mOutlineTable->CellIsSelected(inCell)) {
			drawInfo.outIconTransform |= ttSelected;
		}
		
		if (drawInfo.prIconFrame.right < inLocalCellRect.right) { // don't draw if rect is outside cell.
			::PlotIconSuite(&drawInfo.prIconFrame, (SInt16) drawInfo.outIconAlign,
							(SInt16) drawInfo.outIconTransform, drawInfo.outIconSuite);
		}
	}
	
	// If there's text, draw it.

	if (drawInfo.prTextWidth > 0) {

		// If text is selected, erase selection rectangle first.
		// This covers the case where the selection is drawn on top of
		// an area that is shaded.
		Rect maxRect = drawInfo.ioCellBounds, newTextRect = drawInfo.ioCellBounds;
			
		::MacInsetRect(&maxRect, 3, 1);
		maxRect.top++;
			
		UNewTextDrawing::MeasureWithJustification(
				(Ptr) drawInfo2.mCellData.c_str(),
				drawInfo2.mCellData.size(),
				maxRect,
				drawInfo.outTextTraits.justification,
				newTextRect);
		
		newTextRect.right += 2;
		
		if (drawInfo.outShowSelection && mOutlineTable->CellIsSelected(inCell)) {
			RGBColor saveColor;
			::GetForeColor(&saveColor);
			::RGBForeColor(&Color_White);
			
			if (mOutlineTable->IsActive()) {
				::PaintRect(&drawInfo.prTextFrame);
			} else {
				::PenNormal();
				::MacFrameRect(&drawInfo.prTextFrame);
			}
			
			::RGBForeColor(&saveColor);
		}
		
		// Draw the text now.
		
		if (drawInfo.outMultiLineText) {
		
			// Multi-line text.
	
			Rect textRect = drawInfo.prTextFrame;
			::MacInsetRect(&textRect, 3, 1);
			textRect.right += 2;
			
			UTextDrawing::DrawWithJustification(
					(Ptr) drawInfo2.mCellData.c_str(),
					drawInfo2.mCellData.size(),
					//newTextRect,
					textRect,
					drawInfo.outTextTraits.justification,
					false);
		}
		else {
			
			// Single-line text. Just use QuickDraw.
		
			::MoveTo((SInt16) (drawInfo.prTextFrame.left + 3), drawInfo.outTextBaseline);
			::DrawString(drawInfo.prTruncatedString);
		}

		// If cell is selected, draw highlighting now.
	
		if (drawInfo.outShowSelection && mOutlineTable->CellIsSelected(inCell)) {
			StColorPenState::Normalize();
			::LMSetHiliteMode((UInt8) (::LMGetHiliteMode() & 0x7F));
			::MacInvertRect(&drawInfo.prTextFrame);
			if (!mOutlineTable->IsActive()) {
				Rect tempRect = drawInfo.prTextFrame;
				::LMSetHiliteMode((UInt8) (::LMGetHiliteMode() & 0x7F));
				::MacInsetRect(&tempRect, 1, 1);
				::MacInvertRect(&tempRect);
			}
		}
	}	
}



// ---------------------------------------------------------------------------
//		¥ CPropTableComparator::Compare()
// ---------------------------------------------------------------------------
//
SInt32 CPropTableComparator::Compare(const void* inItemOne, const void* inItemTwo,
								UInt32 , UInt32 ) const {
   CPropTableItem *item1, *item2;
   item1 = *(CPropTableItem**)inItemOne;
   item2 = *(CPropTableItem**)inItemTwo;

   if ((!item1) || (!item2))
      return 0;

   std::string val1, val2;
   if (1 == mSortColumn) {
      val1 = item1->GetDAVProperty().GetNamespace();
      val1 += item1->GetDAVProperty().GetPropertyName();
      val2 = item2->GetDAVProperty().GetNamespace();
      val2 += item2->GetDAVProperty().GetPropertyName();
   } else {
      val1= item1->GetPropValue();
      val2= item2->GetPropValue();
   }
   
   if (mIsAscending) {
      return strncmp(val1.c_str(), val2.c_str(), min(val1.size(), val2.size()));
   } else {
      return strncmp(val2.c_str(), val1.c_str(), min(val1.size(), val2.size()));
   }
}

// ---------------------------------------------------------------------------
//		¥ CPropDisplayTable::CPropDisplayTable()
// ---------------------------------------------------------------------------
//
CPropDisplayTable::CPropDisplayTable(LStream *inStream) : 
            CSortableOutlineTable(inStream, new CPropTableComparator(), true),mTableHeader(NULL) {
    
	AddAttachment(new LOutlineKeySelector(this, msg_AnyMessage));
	AddAttachment(new LKeyScrollAttachment(this));
	mFirstLevelIndent = 0;

	if (mSuperCommander != nil) {
		mSuperCommander->SetLatentSub(this);
	}
	
}

// ---------------------------------------------------------------------------
//		¥ CPropDisplayTable::~CPropDisplayTable()
// ---------------------------------------------------------------------------
//
CPropDisplayTable::~CPropDisplayTable() {
}


// ---------------------------------------------------------------------------
//		¥ CPropDisplayTable::Setup()
// ---------------------------------------------------------------------------
//
void CPropDisplayTable::Setup(LBevelButton* sortButton, LView* header) {
   mTableHeader = header;

   SetTableGeometry(new LTablePaneHostingGeometry(this, 50, 18));
   SetTableSelector(new LOutlineRowSelector( this ) );
   
   mSortButton = sortButton;
   mSortButton->AddListener(this);

   mRows = 0;	
}


// ---------------------------------------------------------------------------
//		¥ CPropDisplayTable::AddPropVal()
// ---------------------------------------------------------------------------
//
void CPropDisplayTable::AddPropVal(CDAVProperty& prop, std::string &val) {
	LOutlineItem *newItem;
    newItem = (LOutlineItem*)new CPropTableItem(prop, val);
    InsertSortedItem(newItem, NULL, false /*refresh*/, true);
}

// ---------------------------------------------------------------------------
//		¥ ScrollImageBy()
// ---------------------------------------------------------------------------
//
void CPropDisplayTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, Boolean inRefresh) {

   // call inherited method
   CSortableOutlineTable::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh); 
                             
   if (mTableHeader)
      mTableHeader->ScrollImageBy(inLeftDelta, 0, inRefresh);  
}

// ---------------------------------------------------------------------------
//		¥ CPropDisplayTable::ListenToMessage
// ---------------------------------------------------------------------------
//
void CPropDisplayTable::ListenToMessage(MessageT inMessage, void *) {
   if (kSortMessage == inMessage) {
      ControlButtonContentInfo controlInfo;
      mSortButton->GetContentInfo(controlInfo);
      if (controlInfo.u.resID == icn_SortUp)
         controlInfo.u.resID = icn_SortDown;
      else
         controlInfo.u.resID = icn_SortUp;
      
      SInt16 mSortIcon = controlInfo.u.resID;
      mSortButton->SetContentInfo(controlInfo);
      
      Boolean isAscending = true;
	  CPropTableComparator* comp = reinterpret_cast<CPropTableComparator*>(GetComparator());
	
      if (mSortIcon != icn_SortUp)
         isAscending = false;
			
      comp->SetIsAscending(isAscending);
      Sort(true/*refresh*/);
   } else if (kPropNameColMessage == inMessage){
	  CPropTableComparator* comp = reinterpret_cast<CPropTableComparator*>(GetComparator());
      comp->SetSortColumn(1);
      Sort(true/*refresh*/);
   } else if (kPropValColMessage == inMessage){
	  CPropTableComparator* comp = reinterpret_cast<CPropTableComparator*>(GetComparator());
      comp->SetSortColumn(2);
      Sort(true/*refresh*/);
   }
}

// ---------------------------------------------------------------------------
//		¥ CPropDisplayTable::ListenToMessage
// ---------------------------------------------------------------------------
//
void CPropDisplayTable::Sort(Boolean inRefresh) {
   CSortableOutlineTable::Sort(inRefresh);
   LArray tmpItems(GetItems());
	
   LArrayIterator indexer(tmpItems, LArrayIterator::from_End);
   LOutlineItem *item;
	
   while (indexer.Previous(&item)) {
      reinterpret_cast<CPropTableItem*>(item)->UpdateRowSize();
   }
}

// ---------------------------------------------------------------------------------
//		¥ ObeyCommand
// ---------------------------------------------------------------------------------
//	
Boolean CPropDisplayTable::ObeyCommand(CommandT inCommand, void *ioParam) {
   if (cmd_Copy == inCommand) {
      std::string clipContent;
      STableCell cell;
      UInt32 numRows,numCols;
	
      GetTableSize(numRows,numCols);
      CPropTableItemDC drawInfo;
	
      for (cell.row=1; cell.row<=numRows; cell.row++) {
         if (CellIsSelected(cell)) {
         	 CPropTableItem *item=reinterpret_cast<CPropTableItem*>(FindItemForRow(cell.row));

            cell.col=1;
            item->GetDrawContentsSelf_Unlim(cell,  drawInfo);
            clipContent += drawInfo.mCellData;
            cell.col=2;
            drawInfo.mCellData = "";
            item->GetDrawContentsSelf_Unlim(cell,  drawInfo);
            clipContent += "\t";
            clipContent += drawInfo.mCellData;
            clipContent += "\r";
         }
      }
      Boolean clearCB = true;
      ResType rt = FOUR_CHAR_CODE('TEXT');
      Ptr data = const_cast<Ptr>(clipContent.c_str());
      SInt32 dataLen = clipContent.length();
      LClipboard::GetClipboard()->SetData(rt, data, dataLen, clearCB);

   }
   return LCommander::ObeyCommand(inCommand, ioParam);
}

// ---------------------------------------------------------------------------------
//		¥ FindCommandStatus
// ---------------------------------------------------------------------------------
//	
void CPropDisplayTable::FindCommandStatus(CommandT inCommand, Boolean &outEnabled, Boolean &outUsesMark, UInt16 &outMark, Str255 outName) {
   if (cmd_Copy == inCommand) {
      STableCell cell;
      UInt32 numRows,numCols;
	
      GetTableSize(numRows,numCols);
	
      for (cell.row=1; cell.row<=numRows; cell.row++) {
         if (CellIsSelected(cell)) {
            outEnabled = true;
            return;
         }
      }
      outEnabled = false;
      return;
   };
   
   return LCommander::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);

}

// ---------------------------------------------------------------------------
//	¥ SetColWidth
// ---------------------------------------------------------------------------
//	Override so we can update the height of the rows 
void CPropDisplayTable::SetColWidth(UInt16 inWidth, TableIndexT	inFromCol,
	TableIndexT	inToCol) {
   LTableView::SetColWidth(inWidth, inFromCol, inToCol);

    LArray tmpItems(GetItems());
	
	LArrayIterator indexer(tmpItems, LArrayIterator::from_End);
    LOutlineItem				*item;
	
	while (indexer.Previous(&item))
	   reinterpret_cast<CPropTableItem*>(item)->UpdateRowSize();
	

}


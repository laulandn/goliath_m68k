/* ==================================================================================================
 * CDAVTable.h															   
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

#ifndef __CDAVTABLE_H__
#define __CDAVTABLE_H__

#pragma once

#include "CDAVTableApp.h"
#include <LArray.h>

#include <LMutexSemaphore.h>
#include <CSortableOutlineTable.h>
#include <LDragAndDrop.h>
#include <LBroadcaster.h>

#include<LDragTask.h>

class CDAVTable : public CSortableOutlineTable, public LDragAndDrop, public LBroadcaster, public LCommander
{
	public:
		enum {class_ID = 'CFLT'};
		
		enum SelectionTypes {
		   LOCKED=0,
		   UNLOCKED=1,
		   LOCKED_NOTLOCAL=2,
		   LOCKED_LOCAL=3,
		   FILE=4
		};
		
		CDAVTable();
		CDAVTable(LStream *inStream);
		~CDAVTable();
		
        void Setup(class CDAVTableWindow* wnd, UInt8 rowHeight, UInt8 columns, UInt16 columnWidths[]);
	    
	    void SetSortColor(const RGBColor &color);
	    void SetRowColor(const RGBColor &color);
	    RGBColor* GetRowColor(TableIndexT);

	    void DrawOverhang();
	    void DrawRowBackground(TableIndexT, const Rect &inLocalRowRect);
	    
	    void CellSingleClicked(class CDAVTableItem *);
	    void CellDoubleClicked(class CDAVTableItem *);
	
	    void SetSortColumn(TableIndexT sCol);
	    virtual void Sort(Boolean inRefresh);
	    virtual void RemoveItem(LOutlineItem*		inOutlineItem,
								Boolean				inRefresh = true,
								Boolean				inAdjustImageSize = true);
								
	    void AddItems(CDAVItemVector &items, class LOutlineItem *parent);
		virtual void AddItem(CDAVItem &item, class LOutlineItem *parent=NULL);
		virtual void _AddItem(CDAVItem &item, class LOutlineItem *parent=NULL);

        void RemoveAllOutlineItems();
        virtual void ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, Boolean inRefresh);
        Boolean OnlyOneItemSelected();
        Boolean atLeastOneItemSelected();
        Boolean atLeastOneItemSelectedIs(SelectionTypes type);
        CDAVItemVector GetAllSelectedItems();
        CDAVTableItem* GetTableItemForResource(const std::string* resource);
        void BuildDragRegionFromSelection(LDragTask &inDragTask, ItemReference inItemRef, StRegion &mDragRegion);

        virtual void DoDragReceive(DragReference inDragRef);

	    void RefreshItem(CDAVTableItem *inItem);
    protected:
        virtual Boolean		ItemIsAcceptable(DragReference inDragRef, ItemReference inItemRef);
        virtual void		CreateDragEvent(SMouseDownEvent	inMouseDown);
        virtual void		InsideDropArea(	DragReference	inDragRef);
        virtual void		LeaveDropArea(DragReference	inDragRef);
	
        virtual Boolean		CheckForTrash(LDragTask* inDragTask);
        virtual Boolean		CheckIfViewIsAlsoSender(DragReference inDragRef);
        
        virtual void HiliteDropArea(DragReference inDragRef);
	    virtual void		DoDragSendData(
								FlavorType			inFlavor,
								ItemReference		inItemRef,
								DragReference		inDragRef);
	    virtual void ClickSelf(const	SMouseDownEvent 	&inMouseDown);
	    
	    Boolean _onDragSendData(DragReference  inDragRef);
	    Boolean _IsDragOverFolder(DragReference inDragRef);
	    class CDAVTableItem* _getItemUnderDrag(DragReference inDragRef);

        CDAVItemVector mDragItems;

	private:
		
		class CDAVTableWindow* mWnd;
		RGBColor mSortColor;
		RGBColor			mRowColor;
		
		TableIndexT			mSortColumn;
       
        LMutexSemaphore fAccess; // access 

        STableCell mLastDragSelCell;
   friend class CDAVTableItem;
};

typedef struct {
   SInt32   leftDelta;
   SInt32   topDelta;
   Boolean refresh;
} ScrollImageArgsStruct;


#endif
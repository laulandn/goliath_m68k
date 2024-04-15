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

#ifndef __CDAVTABLEITEM_H__
#define __CDAVTABLEITEM_H__

#pragma once

#include <LEditableOutlineItem.h>
#include <CTableTypeAhead.h>
#include <LListener.h>


class CDAVTableItem : public LEditableOutlineItem, public LListener, public CTypeAheadOutlineItem  {
   public: 
      CDAVTableItem();
      virtual ~CDAVTableItem();
      
      void SetItem(CDAVItem &item);
      
//      void SetIcon(Handle icn) {mIcon = icn;};      
//      Handle GetIcon() {return mIcon;};
      CDAVItem &GetItem() {return mItem;};

      void GetDrawContentsSelf( const STableCell& inCell, SOutlineDrawContents&	ioDrawContents);
      Boolean CanExpand() const;
      void DrawRowAdornments( const Rect&		inLocalRowRect );
      void ExpandSelf();
      void SingleClick(const STableCell& inCell, const SMouseDownEvent& inMouseDown, const SOutlineDrawContents& inDrawContents, Boolean inHitText);
      void DoubleClick(const STableCell& inCell, const SMouseDownEvent& inMouseDown, const SOutlineDrawContents& inDrawContents, Boolean inHitText);

      virtual void StartInPlaceEdit(const STableCell &inCell );
    
      virtual void ListenToMessage(MessageT inMessage, void	*ioParam );
      virtual void Collapse();
      virtual void Expand();
      void AddItem(CDAVItem &item);
      void AddItems(CDAVItemVector &items);
	
      void ResetTransaction();
      void RefreshCell(const STableCell& inCell);

      virtual void GetDescriptor(Str255 outDescriptor);
							
    protected:
      CDAVItem mItem;
      
      Boolean mFolder;
#if PP_Target_Carbon
	  IconRef mIconRef;
#else
	  Handle mIcon;
#endif
	  Boolean mTransacting;

      LInPlaceEditField	*mEditField;
      LMutexSemaphore fAccess; // access 

      Boolean     expanded, initExpand;
      virtual void DrawCell(const STableCell& inCell, const Rect& inLocalCellRect);
      virtual void PrepareDrawContents(const STableCell& inCell , SOutlineDrawContents&	ioDrawContents);

      virtual void ClickCell(const STableCell& inCell, const SMouseDownEvent& inMouseDown);

      virtual Boolean CellHitByMarquee(const STableCell& inCell,
									   const Rect& inMarqueeLocalRect);
	private:
	   bool  mHandlingClick;
};

#endif

// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
// TNT Library
// CSortableOutlineTable.h
// ©ÊMark Tully and TNT Software 1998-1999
// Email : M.C.Tully@durham.ac.uk
// 14/9/98
// ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

#pragma once

#include						<LOutlineTable.h>

class							LComparator;
class							LStream;

typedef class CSortableOutlineTable : public LOutlineTable
{
	protected:
		LComparator				*mComparator;
		Boolean					mOwnsComparator;

		virtual void			SortOutlineItemsArray(
										LArray			*inItemsArrayToSort);
		virtual void			RefillItemsArray(
										ArrayIndexT		&ioCurrentIndex,
										LArray			*inItemsArray);

		// Disallowed function
		virtual void			InsertItem(
									LOutlineItem*		inOutlineItem,
									LOutlineItem*		inSuperItem,
									LOutlineItem*		inAfterItem,
									Boolean				inRefresh,
									Boolean				inAdjustImageSize)		{ LOutlineTable::InsertItem(inOutlineItem,inSuperItem,inAfterItem,inRefresh,inAdjustImageSize); }

	public:
		enum { class_ID = 'SRot' };

								CSortableOutlineTable(
										LStream*		inStream,
										LComparator		*inComparator=0L,
										Boolean			inAdoptComparator=true);
								~CSortableOutlineTable();
	
		virtual void			SetComparator(
										LComparator		*inComparator,
										Boolean			inAdoptComparator);
		virtual LComparator		*GetComparator()								{ return mComparator; }
		
		virtual void			Sort(
										Boolean			inRefesh);
		
		virtual void			SelectItems(
										LArray			&inArray);
		virtual void			SnapshotSelection(
										LArray			&inArray);

		virtual void			InsertSortedItem(
										LOutlineItem	*inOutlineItem,
										LOutlineItem	*inSuperItem,
										Boolean			inRefresh,
										Boolean			inAdjustImageSize);
} CSortableOutlineTable;
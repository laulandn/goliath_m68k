//======================================================================================
// Filename:	CHeadingTable.cp
// Copyright © 2000 Joseph Chan. All rights reserved.
//
// Description:	A table that is used as heading for an LTableView. In order for this
//				class to work properly, all operations involving adding, removing, and
//				resizing a table column must be done via this class, not the table that
//				uses this class as a heading.
//======================================================================================
// Revision History:
// Monday, January 17, 2000 - Original
//======================================================================================

#include "CHeadingTable.h"

#include <LAttachment.h>
#include <LBevelButton.h>
#include <LRadioGroup.h>
#include <LTablePaneHostingGeometry.h>
#include <LStream.h>
#include <UDrawingState.h>

CNoClickAttachment	*CNoClickAttachment::sAttachment = 0;

// -------------------------------------------------------------------------------------
// ¥ Constructor
// -------------------------------------------------------------------------------------
CHeadingTable::CHeadingTable(
	LStream					*inStream)
	: LTableView( inStream ), mTable( 0 ), mRadioGroup( 0 ), mDummyHeader( 0 ), 
	  mValueMessage( msg_Nothing), mTextTrait( 0 )
{
	// We simply use the space for the pointer to the table to hold the paneID
	SInt16	defaultWidth;
	Boolean	clickable;
	inStream->ReadData( &mTable, sizeof( PaneIDT ) );
	(*inStream) >> mValueMessage;
	(*inStream) >> defaultWidth;
	(*inStream) >> mTextTrait;
	(*inStream) >> clickable;
	// we may want to read in things first
	SetTableGeometry( new LTablePaneHostingGeometry( this, defaultWidth, mFrameSize.height ) );
	
	if ( clickable )
		mRadioGroup = new LRadioGroup;
}


// -------------------------------------------------------------------------------------
// ¥ Destructor
// -------------------------------------------------------------------------------------
CHeadingTable::~CHeadingTable()
{
	delete mRadioGroup;
}


// -------------------------------------------------------------------------------------
// ¥ SetColumnHeading
// -------------------------------------------------------------------------------------
void
CHeadingTable::SetColumnHeading(
	TableIndexT				inCol,
	ConstStringPtr			inHeading,
	ControlButtonTextAlignment inAlignment,
	SInt16					inOffset )
{
	if ( LBevelButton *bb = dynamic_cast<LBevelButton*>( FetchColumnPane( inCol ) ) )
	{
		bb->SetTextOffset( inOffset );
		bb->SetTextAlignment( inAlignment );
		bb->SetDescriptor( inHeading );
	}
}


// -------------------------------------------------------------------------------------
// ¥ GetColumnHeading
// -------------------------------------------------------------------------------------
StringPtr
CHeadingTable::GetColumnHeading(
	TableIndexT				inCol,
	Str255					outHeading ) const
{
	if ( LBevelButton *bb = dynamic_cast<LBevelButton*>( FetchColumnPane( inCol ) ) )
	{
		bb->GetDescriptor( outHeading );
	} else
	{
		outHeading[0] = 0;
	}
	return outHeading;
}
			

// -------------------------------------------------------------------------------------
// ¥ GetColumnHeadingTextOffset
// -------------------------------------------------------------------------------------
SInt16
CHeadingTable::GetColumnHeadingTextOffset(
	TableIndexT				inCol ) const
{
	SInt16	ret = 0;
	if ( LBevelButton *bb = dynamic_cast<LBevelButton*>( FetchColumnPane( inCol ) ) )
	{
		ret = bb->GetTextOffset();
	}
	return ret;
}


// -------------------------------------------------------------------------------------
// ¥ GetColumnHeadingTextAlignment
// -------------------------------------------------------------------------------------
ControlButtonTextAlignment
CHeadingTable::GetColumnHeadingTextAlignment(
	TableIndexT				inCol ) const
{
	ControlButtonTextAlignment	ret = 0;
	if ( LBevelButton *bb = dynamic_cast<LBevelButton*>( FetchColumnPane( inCol ) ) )
	{
		ret = bb->GetTextAlignment();
	}
	return ret;
}


// -------------------------------------------------------------------------------------
// ¥ SetValue
// -------------------------------------------------------------------------------------
void
CHeadingTable::SetValue(
	SInt32 					inValue )
{
	if ( !mRadioGroup ) return;
	
	if (inValue < GetMinValue() ) {		// Enforce min/max range
		inValue = GetMinValue();
	} else if (inValue > GetMaxValue() ) {
		inValue = GetMaxValue();
	}

	if (mValue != inValue) {		// If value is not the current value
		LTablePaneHostingGeometry	*theGeometry = 
				dynamic_cast<LTablePaneHostingGeometry*>( mTableGeometry );
		STableCell	theCell( 1, inValue );
		if ( LPane	*thePane = theGeometry->GetCellPane( theCell ) )
		{
			thePane->SetValue( Button_On );
			mValue = inValue;			//   Store new value
			BroadcastValueMessage();	//   Inform Listeners of value change
		}
	}
}


// -------------------------------------------------------------------------------------
// ¥ InsertCols
// -------------------------------------------------------------------------------------
SInt32
CHeadingTable::GetMaxValue() const
{
	return ( mRadioGroup ) ? mCols : 0;
}


// -------------------------------------------------------------------------------------
// ¥ InsertCols
// -------------------------------------------------------------------------------------
SInt32
CHeadingTable::GetMinValue() const
{
	return ( mRadioGroup ) ? ((mCols > 0 ) ? 1 : 0): 0;
}


// -------------------------------------------------------------------------------------
// ¥ InsertCols
// -------------------------------------------------------------------------------------
void
CHeadingTable::InsertCols(
	UInt32					inHowMany,
	TableIndexT				inAfterCol,
	const void				*inDataPtr,
	UInt32					inDataSize,
	Boolean					inRefresh )
{
	// Let the target table resize its image first.
	if ( mTable )
		mTable->InsertCols( inHowMany, inAfterCol, inDataPtr, inDataSize, inRefresh );

	LTableView::InsertCols( inHowMany, inAfterCol, inDataPtr, inDataSize, inRefresh );
	// Add the button to the cell.
	if (inAfterCol > mCols)				// Enforce upper bound
		inAfterCol= mCols;
	
	for ( SInt32 i = inAfterCol+1 ; i <= inAfterCol+inHowMany ; ++i )
	{
		Rect	cellRect;
		STableCell	theCell( 1, i );
		GetLocalCellRect( theCell, cellRect );
		SPaneInfo	theInfo = {	0,							// Pane ID
								GetColWidth( i ),
								mFrameSize.height,
								true,						// visible
								true,						// enabled
								{ true, true, true, true },	// Binding
								cellRect.left,				// left
								cellRect.top,				// top
								0,							// refCon
								this };						// superview
		
		LBevelButton	*bb = new LBevelButton( theInfo, 0,
												kControlBevelButtonSmallBevelProc,
												kControlBehaviorSticky,
												kControlContentTextOnly,
												0,				// Content ResID
												mTextTrait,
												"\p",			// title
												0,				// Initial value
												kControlBevelButtonPlaceNormally,
												kControlBevelButtonAlignTextFlushLeft,
												0,				// title offset
												kControlBevelButtonAlignSysDirection,
												Point_00 );		// graphic offset

		bb->FinishCreate();
		bb->SetValueMessage( (MessageT)bb );
		LTablePaneHostingGeometry	*theGeometry = 
			dynamic_cast<LTablePaneHostingGeometry*>( mTableGeometry );
	
		ThrowIfNil_( theGeometry );
		
		theGeometry->SetCellPane( theCell, bb );

		if ( mRadioGroup )
			mRadioGroup->AddRadio( bb );
		else
			bb->AddAttachment( CNoClickAttachment::GetAttachment(), 0, false );
			
		bb->AddListener( this );
	}
}


// -------------------------------------------------------------------------------------
// ¥ RemoveCols
// -------------------------------------------------------------------------------------
void
CHeadingTable::RemoveCols(
	UInt32					inHowMany,
	TableIndexT				inFromCol,
	Boolean					inRefresh )
{
	// Let the target table resize its image first.
	if ( mTable )
		mTable->RemoveCols( inHowMany, inFromCol, inRefresh );
		
	LTableView::RemoveCols( inHowMany, inFromCol, inRefresh );
}


// -------------------------------------------------------------------------------------
// ¥ RemoveAllCols
// -------------------------------------------------------------------------------------
void
CHeadingTable::RemoveAllCols(
	Boolean					inRefresh )
{
	if ( mTable )
		mTable->RemoveAllCols( inRefresh );
		
	LTableView::RemoveAllCols( inRefresh );
}


// -------------------------------------------------------------------------------------
//	¥ AdjustImageSize
// -------------------------------------------------------------------------------------
// Always follow the image width of the target table if possible. This will let us stay
// synchronized with the horizontal scroll bar no matter what. The only important thing
// here is remember to let mTable resize its image before we do it.
void
CHeadingTable::AdjustImageSize(
	Boolean	inRefresh)
{
	if (!mDeferAdjustment)
	{
		UInt32	width, height;
		mTableGeometry->GetTableDimensions(width, height);
		ResizeImageTo((SInt32) width, (SInt32) height, inRefresh);
		
		if ( mTable )
		{
			SDimension32	imageSize;
			mTable->GetImageSize( imageSize );
			ResizeImageBy( imageSize.width - mImageSize.width, height - mImageSize.height, 
							inRefresh );
		} else	
			ResizeImageTo((SInt32) width, (SInt32) height, inRefresh);
	}
}


// -------------------------------------------------------------------------------------
// ¥ SetRowHeight
// -------------------------------------------------------------------------------------
void
CHeadingTable::SetRowHeight(
	UInt16					inHeight,
	TableIndexT				inFromRow,
	TableIndexT				inToRow)
{
#pragma unused (inHeight, inFromRow, inToRow )
	SignalPStr_( "Should set the frame height instead" );
}


// -------------------------------------------------------------------------------------
// ¥ SetColWidth
// -------------------------------------------------------------------------------------
void
CHeadingTable::SetColWidth(
	UInt16					inWidth,
	TableIndexT				inFromCol,
	TableIndexT				inToCol)
{
	// Let the target table resize its image first.
	if ( mTable )
		mTable->SetColWidth( inWidth, inFromCol, inToCol );
		
	LTableView::SetColWidth( inWidth, inFromCol, inToCol );
}


// ---------------------------------------------------------------------------
//	¥ GetLocalCellRect
// ---------------------------------------------------------------------------
//	Pass back the bounding rectangle of the specified Cell and return
//	whether it intersects the Frame of the TableView
//
//	The bounding rectangle is in Local coordinates so it will always be
//	within QuickDraw space when its within the Frame. If the bounding
//	rectangle is outside the Frame, return false but do NOT set the rectangle
//	to (0,0,0,0) (this is opposite of what LTableView::GetLocalCellRect does)

Boolean
CHeadingTable::GetLocalCellRect(
	const STableCell	&inCell,
	Rect				&outCellRect) const
{
	SInt32	cellLeft, cellTop, cellRight, cellBottom;
	mTableGeometry->GetImageCellBounds(inCell, cellLeft, cellTop,
							cellRight, cellBottom);
	
	Boolean	insideFrame = 
		ImageRectIntersectsFrame(cellLeft, cellTop, cellRight, cellBottom);
		
	SPoint32	imagePt;
	imagePt.h = cellLeft;
	imagePt.v = cellTop;
	ImageToLocalPoint(imagePt, topLeft(outCellRect));
	outCellRect.right  = (SInt16) (outCellRect.left + (cellRight - cellLeft));
	outCellRect.bottom = (SInt16) (outCellRect.top + (cellBottom - cellTop));

	return insideFrame;
}


// -------------------------------------------------------------------------------------
// ¥ ResizeImageBy
// -------------------------------------------------------------------------------------
void
CHeadingTable::ResizeImageBy(
	SInt32				inWidthDelta,
	SInt32				inHeightDelta,
	Boolean				inRefresh)
{
	LTableView::ResizeImageBy(inWidthDelta, inHeightDelta, inRefresh);
	// Move the dummy header horizontally whenever we change the image size so
	// it remains attached to the right side of the table image.
	if ( mDummyHeader )
		mDummyHeader->MoveBy( inWidthDelta, 0, inRefresh );
}


// -------------------------------------------------------------------------------------
// ¥ ResizeFrameBy
// -------------------------------------------------------------------------------------
void
CHeadingTable::ResizeFrameBy(
	SInt16				inWidthDelta,
	SInt16				inHeightDelta,
	Boolean				inRefresh)
{
	// If reconcile overhang is enabled, resizing the frame will cause a redraw. If there
	// are any other things around the heading, they will get redrawn and cause flicker.
	// By disabling redrawn inside resize frame, we eliminate that problem.

	StVisibleRgn	curVis( GetMacPort() );

	LTableView::ResizeFrameBy(inWidthDelta, inHeightDelta, inRefresh);
	// Resize the first row to match the frame height, so only one row is ever exposed.
	LTableView::SetRowHeight( mFrameSize.height, 1, 1 );
	if ( mDummyHeader )
		mDummyHeader->ResizeFrameBy( inWidthDelta, inHeightDelta, inRefresh );
}


// -------------------------------------------------------------------------------------
// ¥ ListenToMessage
// -------------------------------------------------------------------------------------
void
CHeadingTable::ListenToMessage(
	MessageT				inMessage,
	void*					ioParam)
{
#pragma unused( ioParam )
	if ( *(SInt32*)ioParam == Button_Off ) return;	// Do not broadcast offs from the
													// radio group
	LTablePaneHostingGeometry	*theGeometry = 
			dynamic_cast<LTablePaneHostingGeometry*>( mTableGeometry );
	for ( STableCell theCell( 1, 1 ) ; theCell.col <= mCols ; ++theCell.col )
	{
		if ( (void*)inMessage == theGeometry->GetCellPane( theCell ) )
		{
			mValue = (SInt32)theCell.col;
			BroadcastValueMessage();
			break;
		}
	}
}


// -------------------------------------------------------------------------------------
// ¥ FinishCreateSelf
// -------------------------------------------------------------------------------------
void
CHeadingTable::FinishCreateSelf()
{
	PaneIDT	paneID = (PaneIDT) mTable;
	mTable = dynamic_cast<LTableView*>( mSuperView->FindPaneByID( paneID ) );
	if ( mTable && mTable->GetTableGeometry() )
	{
		TableIndexT	rows, cols;
		mTable->GetTableSize( rows, cols );
		for ( TableIndexT i = 1 ; i <= cols ; ++i )
		{
			InsertCols( 1, i-1, 0, 0, Refresh_No );
			// Don't want to end up setting the column width of the target table.
			LTableView::SetColWidth(  mTable->GetColWidth( i ), i, i );
		}
	}
	// Make sure that we only have 1 row.
	RemoveAllRows( Refresh_No );
	InsertRows( 1, 0, 0, 0, Refresh_No );
	// Make the dummy header at the end of the table. It is actually not a part of
	// the table, it simply sits at the right edge. It is used to simulate the end
	// cap bevel in the Finder view at the right edge when the frame size is larger
	// then the image size.
	SDimension16	tableFrameSize;
	GetFrameSize( tableFrameSize );
	Point	pt;
	UInt32	height, width;
	mTableGeometry->GetTableDimensions(width, height);
	SPoint32	imageLoc = { 0, (SInt32) width }; // left at the right edge of table
	ImageToLocalPoint( imageLoc, pt );

	SPaneInfo	theInfo = {	0,							// Pane ID
							tableFrameSize.width+10,	// the width is slightly bigger
							tableFrameSize.height,
							true,						// visible
							true,						// enabled
							{ false, true, false, true },// Binding
							pt.h,						// left
							pt.v,						// top
							0,							// refCon
							this };						// superview
	
	mDummyHeader = new LBevelButton( theInfo, 0,
									  kControlBevelButtonSmallBevelProc,
									  kControlBehaviorSticky,
									  kControlContentTextOnly,
									  0,				// Content ResID
									  mTextTrait,
									  "\p",			// title
									  0,				// Initial value
									  kControlBevelButtonPlaceNormally,
									  kControlBevelButtonAlignTextFlushLeft,
									  0,				// title offset
									  kControlBevelButtonAlignSysDirection,
									  Point_00 );		// graphic offset

	mDummyHeader->FinishCreate();
	// don't let users click on it if the other headings can be clicked
	mDummyHeader->AddAttachment( CNoClickAttachment::GetAttachment(), 0, false );
}


// -------------------------------------------------------------------------------------
//	¥ BroadcastValueMessage										   [protected]
// -------------------------------------------------------------------------------------
void
CHeadingTable::BroadcastValueMessage()
{
	if (mValueMessage != msg_Nothing) {
		SInt32	value = mValue;
		BroadcastMessage(mValueMessage, &value);
	}
}


// -------------------------------------------------------------------------------------
// ¥ FetchColumnPane
// -------------------------------------------------------------------------------------
LPane*
CHeadingTable::FetchColumnPane(
	TableIndexT				inCol ) const
{
	LPane	*thePane = 0;
	if ( inCol > 0 && inCol <= mCols )
	{
		LTablePaneHostingGeometry	*theGeometry = 
				dynamic_cast<LTablePaneHostingGeometry*>( mTableGeometry );
		STableCell	theCell( 1, inCol );
		thePane = theGeometry->GetCellPane( theCell );
	}
	return thePane;
}

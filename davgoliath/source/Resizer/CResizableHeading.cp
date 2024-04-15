//======================================================================================
// Filename:	CHeadingTable.cp
// Copyright © 2000 Joseph Chan. All rights reserved.
//
// Description:	A table header with resizable column width.
//
//======================================================================================
// Revision History:
// Monday, January 24, 2000 - Original
//======================================================================================

#include "CResizableHeading.h"
#include <LStream.h>
#include <UCursor.h>

const ResIDT		kColResizeCursor = 20128;

// ----------------------------------------------------------------------------------------
// ¥ Constructor
// ----------------------------------------------------------------------------------------
CResizableHeading::CResizableHeading(
	LStream				*inStream )
	: CHeadingTable( inStream ), mInitialPortPt( Point_00 ), mCursorID( 0 ),
	  mMinColWidth( 25 ), mCurResizedCol( 0 )
{
	(*inStream) >> mCursorID;
	(*inStream) >> mEnableResizing;
	(*inStream) >> mLiveResizer;
}


// ----------------------------------------------------------------------------------------
// ¥ Destructor
// ----------------------------------------------------------------------------------------
CResizableHeading::~CResizableHeading()
{
}


// ----------------------------------------------------------------------------------------
// ¥ Click
// ----------------------------------------------------------------------------------------
// We override Click instead of ClickSelf to prevent any dispatching to 
// subpanes if the cursor is inside a resizer region.
void
CResizableHeading::Click(
	SMouseDownEvent		&inMouseDown )
{
	CResizerTrackerView	*tracker = CResizerTrackerView::FindTracker( this );
	if ( mEnableResizing && tracker && 
		 (mCurResizedCol = FindResizer( inMouseDown.wherePort )) != 0 )
	{
		StRegion	theClipRegion;
		CalcResizerClipRegion( theClipRegion );
		short	width = mLiveResizer ? 0 : 2;
		tracker->TrackResizer( inMouseDown.wherePort, CResizerTrackerView::vertical, *this, 
								theClipRegion, width, mLiveResizer );
	} else
		CHeadingTable::Click( inMouseDown );
}


// ----------------------------------------------------------------------------------------
// ¥ AdjustMouse
// ----------------------------------------------------------------------------------------
// We override AdjustMouse instead of AdjustMouseSelf to prevent any dispatching to 
// subpanes if the cursor is inside a resizer region.
void
CResizableHeading::AdjustMouse(
								Point				inPortPt,
								const EventRecord&	inMacEvent,
								RgnHandle			outMouseRgn)
{
	if ( mEnableResizing && FindResizer( inPortPt ) )
		UCursor::SetTheCursor( mCursorID );
	else
		CHeadingTable::AdjustMouse( inPortPt, inMacEvent, outMouseRgn );
}


// ----------------------------------------------------------------------------------------
// ¥ FindResizer
// ----------------------------------------------------------------------------------------
TableIndexT
CResizableHeading::FindResizer( Point inPortPt )
{
	STableCell	hitCell;
	SPoint32	imagePt;
	Point		localPt = inPortPt;
	PortToLocalPoint( localPt );
	LocalToImagePoint( localPt, imagePt );
	
	TableIndexT	leftColumn = 0;
	Rect	cellRect;
	const short	kResizerWidth = 1;
	if ( GetCellHitBy( imagePt, hitCell ) )
	{
		GetLocalCellRect( hitCell, cellRect );
		if ( localPt.h <= cellRect.left + kResizerWidth )
		{
			leftColumn = hitCell.col - 1;
		} else if ( localPt.h >= cellRect.right - kResizerWidth )
		{
			leftColumn = hitCell.col;
		}
	} else if ( hitCell.col > mCols && hitCell.row > 0 && hitCell.row <= mRows )
	{
		--hitCell.col;
		GetLocalCellRect( hitCell, cellRect );
		if ( localPt.h >= cellRect.right - kResizerWidth &&
			 localPt.h <= cellRect.right + kResizerWidth )
		{
			leftColumn = hitCell.col;
		}
	}
	return leftColumn;
}


// ----------------------------------------------------------------------------------------
// ¥ StartTracking
// ----------------------------------------------------------------------------------------
void
CResizableHeading::StartTracking( Point inPortPt )
{
	mInitialPortPt = inPortPt;
}


// ----------------------------------------------------------------------------------------
// ¥ ResizerMoved
// ----------------------------------------------------------------------------------------
void
CResizableHeading::ResizerMoved( Point inOldPortPt, Point inNewPortPt, RgnHandle inClipRgn )
{
#pragma unused( inClipRgn )
	if ( mLiveResizer )
	{
		DoResize( inOldPortPt, inNewPortPt );
		if ( mSuperView )
			mSuperView->UpdatePort();
	}
}


// ----------------------------------------------------------------------------------------
// ¥ EndTracking
// ----------------------------------------------------------------------------------------
void
CResizableHeading::EndTracking( Point inPortPt )
{
	if ( !mLiveResizer )
	{
		DoResize( mInitialPortPt, inPortPt );
	}
	mInitialPortPt = Point_00;
}


// ----------------------------------------------------------------------------------------
// ¥ CalcResizerClipRegion
// ----------------------------------------------------------------------------------------
void
CResizableHeading::CalcResizerClipRegion( StRegion &ioRgn ) const
{

	Rect	frame, tableFrame = Rect_0000, cellRect;
	STableCell	theCell( 1, mCurResizedCol );
	GetLocalCellRect( theCell, cellRect );
	LocalToPortPoint( topLeft( cellRect ) );
	LocalToPortPoint( botRight( cellRect ) );
	
	CalcPortFrameRect( frame );
	if ( mTable )
		mTable->CalcPortFrameRect( tableFrame );
	
	::MacUnionRect( &frame, &tableFrame, &frame );
	frame.left = cellRect.left + 20;	// Minimum of 20 pixels for the column
	ioRgn = frame;

}


// ----------------------------------------------------------------------------------------
// ¥ DoResize
// ----------------------------------------------------------------------------------------
void
CResizableHeading::DoResize( Point inOldPortPt, Point inPortPt )
{
	if ( mCurResizedCol && mCurResizedCol <= mCols )
	{
		UInt16 width = GetColWidth( mCurResizedCol ) + inPortPt.h - inOldPortPt.h;
		SetColWidth( width, mCurResizedCol, mCurResizedCol );
	}
}

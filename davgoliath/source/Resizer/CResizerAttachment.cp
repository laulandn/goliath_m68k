//======================================================================================
// Filename:	CResizerAttachment.cp
// Copyright © 2000 Joseph Chan. All rights reserved.
//
// Description:	An attachment for a pane that transform it into a resizer. The attachment 
//				keeps track of the two panes that it is suppose to resize: Pane1 is the 
//				top/left pane and Pane2 is the bottom/right pane. Finally, the attachment
//				offers an option to do live drawing during resizing.
//======================================================================================
// Revision History:
// Friday, January 21, 2000 - Original
//======================================================================================
// CResizerAttachment.h <-- double click + Command-D for header

#include "CResizerAttachment.h"

#ifndef _H_LStream
#include <LStream.h>
#endif

#pragma mark == CResizerAttachment ==
// ----------------------------------------------------------------------------------------
// ¥ Constructor
// ----------------------------------------------------------------------------------------
CResizerAttachment::CResizerAttachment( LStream *inStream )
	: LAttachment( inStream ), mResizerPane( 0 )
{
	// We must intercept everything here because we need to intercept msg_FinishCreate
	mMessage = msg_AnyMessage;
	inStream->ReadData( &mPane1, sizeof(mPane1) );
	inStream->ReadData( &mPane2, sizeof(mPane2) );
	inStream->ReadData( &mPane1MinDim, sizeof(mPane1MinDim) );
	inStream->ReadData( &mPane2MinDim, sizeof(mPane2MinDim) );
	inStream->ReadData( &mHorizontal, sizeof(mHorizontal) );
	inStream->ReadData( &mLiveResizer, sizeof(mLiveResizer) );
	
	// Enfore positive minimum dimensions.
	if ( mPane1MinDim < 0 )
		mPane1MinDim = 0;
	if ( mPane2MinDim < 0 )
		mPane2MinDim = 0;
}


// ----------------------------------------------------------------------------------------
// ¥ Enable
// ----------------------------------------------------------------------------------------
void
CResizerAttachment::Enable()
{
	mEnabled = true;
}


// ----------------------------------------------------------------------------------------
// ¥ Disable
// ----------------------------------------------------------------------------------------
void
CResizerAttachment::Disable()
{
	mEnabled = false;
}


// ----------------------------------------------------------------------------------------
// ¥ ExecuteSelf
// ----------------------------------------------------------------------------------------
void
CResizerAttachment::ExecuteSelf( 
				MessageT		inMessage,
				void*			ioParam )
{
	// We must intercept everything here because we need to intercept msg_FinishCreate
	if ( inMessage == msg_FinishCreate )
	{
		mMessage = msg_Click;
		// If the host is not a pane or the host pane doesn't have a superview, just
		// delete self.
		mResizerPane = dynamic_cast<LPane*>( mOwnerHost );
		if ( !mResizerPane )
		{
			delete this;
			return;
		}
		// Search for the TrackerView
		mTracker = CResizerTrackerView::FindTracker( mResizerPane );
		if ( !mTracker )
		{
			delete this;
			return;
		}
		
		// Set up the two panes that we are resizing
		LView	*theView = mResizerPane->GetSuperView();
		mPane1 = theView->FindPaneByID( (PaneIDT) mPane1 );
		mPane2 = theView->FindPaneByID( (PaneIDT) mPane2 );
		
		
	} else if ( inMessage == msg_Click )
	{
		SMouseDownEvent	*theEvent = (SMouseDownEvent*) ioParam;
		StRegion	theClipRegion;
		CalcResizerClipRegion( theClipRegion );
		short	width = mLiveResizer ? 0 : 3;
		mTracker->TrackResizer( theEvent->wherePort, GetResizerMode(), *this, 
								theClipRegion, width, mLiveResizer );
		SetExecuteHost( false );
	}
}


// ----------------------------------------------------------------------------------------
// ¥ StartTracking
// ----------------------------------------------------------------------------------------
void
CResizerAttachment::StartTracking( Point inPortPt )
{
	mInitialPortPt = inPortPt;
}


// ----------------------------------------------------------------------------------------
// ¥ ResizerMoved
// ----------------------------------------------------------------------------------------
void
CResizerAttachment::ResizerMoved( Point inOldPortPt, Point inNewPortPt, RgnHandle inClipRgn )
{
#pragma unused( inClipRgn )
	if ( mLiveResizer )
	{
		DoResize( inOldPortPt, inNewPortPt );
		mResizerPane->GetSuperView()->UpdatePort();
	}
}


// ----------------------------------------------------------------------------------------
// ¥ EndTracking
// ----------------------------------------------------------------------------------------
void
CResizerAttachment::EndTracking( Point inPortPt )
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
CResizerAttachment::CalcResizerClipRegion( StRegion &ioRgn ) const
{
	Rect	frame;
	if ( mPane1 )
	{
		mPane1->CalcPortFrameRect( frame );
		// Enfore the minimum dimension for pane 1
		if ( mHorizontal )
			frame.top += mPane1MinDim;
		else
			frame.left += mPane1MinDim;
		ioRgn += frame;
	}

	if ( mPane2 )
	{
		mPane2->CalcPortFrameRect( frame );
		// Enfore the minimum dimension for pane 2
		if ( mHorizontal )
			frame.bottom -= mPane2MinDim;
		else
			frame.right -= mPane2MinDim;
		ioRgn += frame;
	}
	
	if ( mResizerPane )
	{
		mResizerPane->CalcPortFrameRect( frame );
		ioRgn += frame;
	}
	
	ioRgn.GetBounds( frame );
	ioRgn = frame;
}


// ----------------------------------------------------------------------------------------
// ¥ DoResize
// ----------------------------------------------------------------------------------------
void
CResizerAttachment::DoResize( Point inOldPortPt, Point inPortPt )
{
	if ( mPane1 && mPane2 && mResizerPane )
	{
		SPoint32	p1Loc, p2Loc, resizerLoc;
		mPane1->GetFrameLocation( p1Loc );
		mPane2->GetFrameLocation( p2Loc );
		mResizerPane->GetFrameLocation( resizerLoc );
		
		short	dv = 0, dh = 0;
		if ( mHorizontal )
		{
			Assert_( (p1Loc.v < resizerLoc.v) && (resizerLoc.v < p2Loc.v) );
			dv = inPortPt.v - inOldPortPt.v;
			mPane2->ResizeFrameBy( 0, -dv, false );
		} else
		{
			Assert_( (p1Loc.h < resizerLoc.h) && (resizerLoc.h < p2Loc.h) );
			dh = inPortPt.h - inOldPortPt.h;
			mPane2->ResizeFrameBy( -dh, 0, false );
		}
		
		mPane1->ResizeFrameBy( dh, dv, false );
		mPane2->MoveBy( dh, dv, false );
		mResizerPane->MoveBy( dh, dv, false );
		mResizerPane->GetSuperView()->Refresh();
	}
}

//======================================================================================
// Filename:	CResizerTrackerView.cp
// Copyright © 2000 Joseph Chan. All rights reserved.
//
// Description:	A view that tracks the dragging of resizers and does the appropriate
//				XOR animation. Any panes that responds to a click drag and wish to be
//				tracked must inherit from CResizerTarget.
// Note: The Erase on Update flag of the containing window will be turned OFF during
//		 tracking of resizers.
//======================================================================================
// Revision History:
// Friday, January 21, 2000 - Original
//======================================================================================
// CResizerTrackerView.h <-- double click + Command-D for header

#include "CResizerTrackerView.h"

#ifndef _H_LWindow
#include <LWindow.h>
#endif

#ifndef _H_UDrawingState
#include <UDrawingState.h>
#endif

#ifndef _H_UMemoryMgr
#include <UMemoryMgr.h>
#endif

namespace
{
	Boolean	MyTrackMouseLocation( Point *outPoint )
	{
		EventRecord	mouseUp;
		Boolean	down = !::EventAvail( mUpMask, &mouseUp );
		*outPoint = mouseUp.where;
		return down;
	}
}

// ----------------------------------------------------------------------------------------
// ¥ Destructor
// ----------------------------------------------------------------------------------------
CResizerTrackerView::CResizerTarget::~CResizerTarget()
{
	// empty
}


// ----------------------------------------------------------------------------------------
// ¥ FindTracker
// ----------------------------------------------------------------------------------------
// Find the most direct TrackerView that contains a pane. This is a static utility function
// that most ResizerTarget will use.
CResizerTrackerView*
CResizerTrackerView::FindTracker( LPane *inPane )
{
	CResizerTrackerView	*tracker = 0;
	if ( inPane )
	{
		LView	*super = inPane->GetSuperView();
		do
		{
			if ( (tracker = dynamic_cast<CResizerTrackerView*>( super )) != 0 )
				break;
		} while ( (super = super->GetSuperView()) != 0 );
	}
	return tracker;
}


// ----------------------------------------------------------------------------------------
// ¥ Constructor
// ----------------------------------------------------------------------------------------
CResizerTrackerView::CResizerTrackerView( LStream *inStream )
	: LOffscreenView( inStream ), mCurPortFrameRect( Rect_0000 ), mClipRgn( 0 ),
	  mInitialPortPt( Point_00 ), mCurPortPt( Point_00 ), mHalfLineWidth( 1 ), 
	  mIsTracking( false ), mMode( none )
{
}


// ----------------------------------------------------------------------------------------
// ¥ Draw
// ----------------------------------------------------------------------------------------
// Override to provide support for offscreen drawing if live drawing during resizing is 
// needed.
void
CResizerTrackerView::Draw(
				RgnHandle		inSuperDrawRgnH )
{
	if ( mIsTracking )
	{
		LOffscreenView::Draw( inSuperDrawRgnH );
	} else
	{
		LView::Draw( inSuperDrawRgnH );
	}
}


// ----------------------------------------------------------------------------------------
// ¥ TrackResizer
// ----------------------------------------------------------------------------------------
// This is the central function of the view. There are four steps that this method goes
// through for every tracking action.
// 1. Sets up the geometric limit for tracking. This method must be provided with a region
//	  in Port coordinates defines the area that the resizer can freely move within. The
//	  actual limits is the intersection of the current clipping region and the provided
//	  clipping region. Since FocusDraw() is called before the region calculation, the 
//	  current clipping region should be the frame rect of the TrackerView, and therefore
//	  should not modify the provided clipping region if it is entirely within the confine
//	  of the TrackerView. This arrangement permits multiple resizers be tracked by the 
//	  same Tracker.
// 2. Sets up some initial conditions for the tracking, like the initial port point,
//	  the tracking mode, and prime the XOR drawing if necessary. If a ResizerTarget
//	  wants to suppress the XOR drawing, pass in false for inIsLive. To anticipate
//	  that a ResizerTarget may actually want to doing live resize drawing, the Erase
//	  on Update flag of the containing window so subsequent call to UpdatePort will
//	  not cause any flicker.
// 3. Track the mouse location, apply any constrain on the location, and tell the
//	  ResizerTarget if the mouse has moved or not. Repeat this step until the mouse
//	  is let up.
// 4. After the mouse is let up, restore the Erase on Update flag if necessary, inform
//	  the target that tracking has ended, and reset all of the Tracker's states.
bool
CResizerTrackerView::TrackResizer(
				Point			inInitialPortPt,
				Mode			inMode,
				CResizerTarget	&inTarget,
				RgnHandle		inClipRgn,	// MUST be in port coordinates
				short			inLineWidth,
				Boolean			inIsLive )
{
	Assert_( ::PtInRgn( inInitialPortPt, inClipRgn ) );
	
	bool	 wasTracked = false;
	// Since we are going to do some XOR drawing, we must be focused first.
	FocusDraw();
	
	// Set up the clipping region for drawing.
	StClipRgnState	savedClipped;
	if ( inClipRgn )
	{
		::MacOffsetRgn( inClipRgn, mPortOrigin.h, mPortOrigin.v );
		savedClipped.ClipToIntersection( inClipRgn );
		::MacOffsetRgn( inClipRgn, -mPortOrigin.h, -mPortOrigin.v );
		mClipRgn = ::NewRgn();	// save up the resizing clip region before restoring.
		::GetClip( mClipRgn );
	}
	
	// Set up the necessary info for tracking and drawing.
	CalcPortFrameRect( mCurPortFrameRect );
	mInitialPortPt = inInitialPortPt;
	StValueChanger<Mode>	mode( mMode, inMode );
	StValueChanger<short>	width( mHalfLineWidth, (short)(inLineWidth / 2) );
	
	mCurPortPt = mInitialPortPt;

	// Do the tracking here
	inTarget.StartTracking( mInitialPortPt );
	// We must do the drawing after the tracking has start, and after the target finish
	// all its drawing in StartTracking.
	if ( !inIsLive )
		StartXOR();
	
	Point	globalPt = mInitialPortPt;
	bool	startTrack = false;
	{
		StValueChanger<Boolean>		track( mIsTracking, true );
		
		// We must turn off erase before drawing for the window
		LView	*super = GetSuperView();
		while ( super->GetSuperView() )
			super = super->GetSuperView();
		LWindow	*theWind = dynamic_cast<LWindow*>( super );
		Assert_( theWind );
		Boolean	hasErase = theWind->HasAttribute( windAttr_EraseOnUpdate );
		theWind->ClearAttribute( windAttr_EraseOnUpdate );
		
		// Use TrackMouseLocation/TrackMouseRegion under Carbon!
		while ( ::MyTrackMouseLocation( &globalPt ) )
		{
			Boolean	hasMoved = false;
			Point	oldPortPt = mCurPortPt;
			wasTracked = true;
			
			ConstrainMouse( globalPt, hasMoved );
			
			// Only do these things if the mouse has moved.
			if ( hasMoved )
			{
				if ( !inIsLive )
					UpdateXOR();
				inTarget.ResizerMoved( oldPortPt, mCurPortPt, mClipRgn );
			}
		}
		// We must restore the window attribute if it was set.
		if ( hasErase )
			theWind->SetAttribute( windAttr_EraseOnUpdate );
	}
	// We must end the XOR drawing sequence before telling the target about it because
	// the target may cause a refresh before we have a chance to restore the pixels.
	if ( !inIsLive )
		EndXOR();
	inTarget.EndTracking( mCurPortPt );
	
	// Cleanup after ourselves
	::DisposeRgn( mClipRgn );
	mClipRgn = 0;
	mCurPortPt = mInitialPortPt = Point_00;
	mCurPortFrameRect = Rect_0000;
	
	return wasTracked;
}


// ----------------------------------------------------------------------------------------
// ¥ DrawNewXOR
// ----------------------------------------------------------------------------------------
// Draw the appropriate XOR region, depending on whether the resizer is horizontal or
// vertical. Since the variable mMode is only set during TrackResizer, this method only
// draw during the duration of tracking.
void
CResizerTrackerView::DrawNewXOR()
{
	if ( mHalfLineWidth > 0 )
	{ 
		Rect	theRect( mCurPortFrameRect );
		PortToLocalPoint( topLeft( theRect ) );
		PortToLocalPoint( botRight( theRect ) );
		short	length = 0;
		switch ( mMode )
		{
			case horizontal:
				theRect.top = mCurPortPt.v - mHalfLineWidth;
				// One more pixel since the painting is within rect bounds
				theRect.bottom = mCurPortPt.v + mHalfLineWidth + 1;
				break;
				
			case vertical:
				theRect.left = mCurPortPt.h - mHalfLineWidth;
				// One more pixel since the painting is within rect bounds
				theRect.right = mCurPortPt.h + mHalfLineWidth + 1;
				break;
		}
		::RectRgn( mOldRgn, &theRect );
	}
}


// ----------------------------------------------------------------------------------------
// ¥ ConstrainMouse
// ----------------------------------------------------------------------------------------
void
CResizerTrackerView::ConstrainMouse( Point inGlobalPoint, Boolean &inMouseMoved )
{
	GlobalToPortPoint( inGlobalPoint );
	inMouseMoved = false;
	
	if ( !PtInRgn( inGlobalPoint, mClipRgn ) )
	{
		Rect	bbox;
#if PP_Target_Carbon
		GetRegionBounds(mClipRgn, &bbox);
#else
        bbox =  (**mClipRgn).rgnBBox;
#endif        
		if ( inGlobalPoint.v < bbox.top )
			inGlobalPoint.v = bbox.top;
		else if ( inGlobalPoint.v > bbox.bottom )
			inGlobalPoint.v = bbox.bottom;
		
		if ( inGlobalPoint.h < bbox.left )
			inGlobalPoint.h = bbox.left;
		else if ( inGlobalPoint.h > bbox.right )
			inGlobalPoint.h = bbox.right;
	}
	if ( (mCurPortPt.h != inGlobalPoint.h) || (mCurPortPt.v != inGlobalPoint.v) )
	{
		mCurPortPt = inGlobalPoint;
		inMouseMoved = true;
	}
}

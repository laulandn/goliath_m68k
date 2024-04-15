//======================================================================================
// Filename:	CMultiScrollerView.cp
// Copyright © 2000 Joseph Chan. All rights reserved.
//
// Description:	A derived class of LScrollerView that can scroll multiple subviews. The
//				scroll bars are always adjusted based on the image size of the primary
//				scrolling view.
//======================================================================================
// Revision History:
// Friday, January 21, 2000 - Original
//======================================================================================
// CMultiScrollerView.h <-- double click + Command-D for header

#include "CMultiScrollerView.h"
#include <LScrollBar.h>
#include <LStream.h>
#include <TArrayIterator.h>
#include <PP_Messages.h>
#include <UMemoryMgr.h>

// ---------------------------------------------------------------------------
//	¥ CMultiScrollerView				Parameterized Constructor [public]
// ---------------------------------------------------------------------------

CMultiScrollerView::CMultiScrollerView(
	const SPaneInfo&	inPaneInfo,
	const SViewInfo&	inViewInfo,
	SInt16				inHorizBarLeftIndent,
	SInt16				inHorizBarRightIndent,
	SInt16				inVertBarTopIndent,
	SInt16				inVertBarBottomIndent,
	SInt16				inBarThickness,
	LView*				inScrollingView,
	bool				inLiveScrolling,
	const TArray<LView*>&inScrollingViews )
	
	: LScrollerView(inPaneInfo, inViewInfo, inHorizBarLeftIndent, inHorizBarRightIndent,
				inVertBarTopIndent, inVertBarBottomIndent, inBarThickness, inScrollingView,
				inLiveScrolling)
{
	mScrollingViews		= inScrollingViews;
}


// ---------------------------------------------------------------------------
//	¥ CMultiScrollerView							Stream Constructor		  [public]
// ---------------------------------------------------------------------------

CMultiScrollerView::CMultiScrollerView(
	LStream*	inStream)
	
	: LScrollerView(inStream)
{
		// ScrollingView has not yet been created, since SuperViews are
		// created before their subviews when building Panes from a Stream.
		// Therefore, we store the ID of the ScrollingViews in place of the 
		// View*  so FinishCreateSelf() can set up the proper connections.
		
	UInt16	numViews;					// Install Panels
	*inStream >> numViews;
	
	for (SInt16 i = 0; i < numViews; i++) {
		PaneIDT		viewID;
		*inStream >> viewID;
		mScrollingViews.InsertItemsAt( 1, LArray::index_Last, (LView*) viewID );
	}
}


// ---------------------------------------------------------------------------
//	¥ ~CMultiScrollerView						Destructor				  [public]
// ---------------------------------------------------------------------------

CMultiScrollerView::~CMultiScrollerView()
{
}

#pragma mark -

// ---------------------------------------------------------------------------
//	¥ FinishCreateSelf											   [protected]
// ---------------------------------------------------------------------------
//	Finish creation of a Scroller by installing its ScrollingView

void
CMultiScrollerView::FinishCreateSelf()
{
	LScrollerView::FinishCreateSelf();
	SInt32	count = mScrollingViews.GetCount();
	for ( SInt32 i = 1 ; i <= count ; ++i )
	{
		// If we can't find the view, or if it is not a view, just put in nil.
		LView	*theView = dynamic_cast<LView*>
								(FindPaneByID( (PaneIDT) mScrollingViews[i] ));
		mScrollingViews[i] = theView;
	}
}

#pragma mark -

// ---------------------------------------------------------------------------
//	¥ VertScroll													  [public]
// ---------------------------------------------------------------------------
//	Function called to scroll vertically while clicking and holding inside
//	the vertical scroll bar

void
CMultiScrollerView::VertScroll(
	SInt16		inPart)
{
	LScrollerView::VertScroll( inPart );
	TArrayIterator<LView*> iterator( mScrollingViews, LArrayIterator::from_Start );
	LView	*theView;
	while (iterator.Next(theView))
	{
		if ( !theView ) continue;
		SPoint32		scrollUnit;
		SDimension16	scrollFrameSize;
		SInt32			vertUnits = 0;
		
		theView->GetScrollUnit(scrollUnit);
		theView->GetFrameSize(scrollFrameSize);
		
		switch (inPart) {				// Determine how much to scroll
		
			case kControlUpButtonPart:		// Scroll up one unit
				vertUnits = -1;
				break;
				
			case kControlDownButtonPart:	// Scroll down one unit
				vertUnits = 1;
				break;
				
			case kControlPageUpPart:		// Scroll up by Frame height
											//   less one unit of overlap
				vertUnits = 1 - (scrollFrameSize.height / scrollUnit.v);
				if (vertUnits >= 0) {
					vertUnits = -1;
				}
				break;
				
			case kControlPageDownPart:		// Scroll down by Frame height
											//   less one unit of overlap
				vertUnits = (scrollFrameSize.height / scrollUnit.v) - 1;
				if (vertUnits <= 0) {
					vertUnits = 1;
				}
				break;
		}
		
		if (vertUnits != 0) {			// Set ScrollBar value and scroll the view
			StValueChanger<bool>	tracking(mIsTrackingScroll, true);
			theView->ScrollPinnedImageBy(0, vertUnits * scrollUnit.v,
													Refresh_Yes);
		}
	}
}


// ---------------------------------------------------------------------------
//	¥ HorizScroll													  [public]
// ---------------------------------------------------------------------------
//	Function called to scroll horizontally while clicking and holding inside
//	the horizontal scroll bar

void
CMultiScrollerView::HorizScroll(
	SInt16		inPart)
{
	LScrollerView::HorizScroll( inPart );
	TArrayIterator<LView*> iterator( mScrollingViews, LArrayIterator::from_Start );
	LView	*theView;
	while (iterator.Next(theView))
	{
		if ( !theView ) continue;
		SPoint32		scrollUnit;
		SDimension16	scrollFrameSize;
		SInt32			horizUnits = 0;
		
		theView->GetScrollUnit(scrollUnit);
		theView->GetFrameSize(scrollFrameSize);
		
		switch (inPart) {				// Determine how much to scroll
		
			case kControlUpButtonPart:		// Scroll left one unit
				horizUnits = -1;
				break;
				
			case kControlDownButtonPart:	// Scroll right one unit
				horizUnits = 1;
				break;
				
			case kControlPageUpPart:		// Scroll left by Frame width
											//   less one unit of overlap
				horizUnits = 1 - (scrollFrameSize.width / scrollUnit.h);
				if (horizUnits >= 0) {
					horizUnits = -1;
				}
				break;
				
			case kControlPageDownPart:		// Scroll right by Frame width
											//   less one unit of overlap
				horizUnits = (scrollFrameSize.width / scrollUnit.h) - 1;
				if (horizUnits <= 0) {
					horizUnits = 1;
				}
				break;
		}
		
		if (horizUnits != 0) {			// Set ScrollBar value and scroll the view
			StValueChanger<bool>	tracking(mIsTrackingScroll, true);
			theView->ScrollPinnedImageBy(horizUnits * scrollUnit.h,
													0, Refresh_Yes);
		}
	}
}


// ---------------------------------------------------------------------------
//	¥ ThumbScroll													  [public]
// ---------------------------------------------------------------------------
//	Function called to scroll while dragging the thumb

void
CMultiScrollerView::ThumbScroll(
	LScrollBar*		inScrollBar,
	SInt32			inThumbValue)
{
	LScrollerView::ThumbScroll( inScrollBar, inThumbValue );
	TArrayIterator<LView*> iterator( mScrollingViews, LArrayIterator::from_Start );
	LView	*theView;
	while (iterator.Next(theView))
	{
		if ( !theView ) continue;
		SPoint32		scrollPosition;			// Current position
		theView->GetScrollPosition(scrollPosition);
		
			// Determine new position based on the scroll direction,
			// thumb value, and scroll unit
		
		SPoint32		scrollUnit;
		theView->GetScrollUnit(scrollUnit);
	
		if (inScrollBar == mHorizontalBar) {
			scrollPosition.h = inThumbValue * scrollUnit.h;
		
		} else if (inScrollBar == mVerticalBar) {
			scrollPosition.v = inThumbValue * scrollUnit.v;
		}
		
			// Scroll View to the new position. The scrolling view will
			// call back to us to adjust the scroll bars as a result of
			// calling ScrollPinnedImageTo(). Since we are tracking a live
			// thumb, the scroll bars are already in the right state. So we
			// set the mIsTrackingScroll flag so that the resulting call to
			// AdjustScrollBars() will do nothing.
		
		StValueChanger<bool>	tracking(mIsTrackingScroll, true);
		theView->ScrollPinnedImageTo(scrollPosition.h,
											scrollPosition.v, Refresh_Yes);
	}
}

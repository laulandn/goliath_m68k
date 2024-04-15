//======================================================================================
// Filename:	XORDrawing.cp
// Copyright © 2000 Joseph Chan. All rights reserved.
//
// Description:	An abstract mix-in that encapsulates the process of XOR animation. The
//				region that the class stores is the region that the previous XOR action
//				painted. Derived classes need to override DrawNewXOR() to build the new
//				region to be painted with XOR inside mOldRgn. A typical application of
//				this class would be to XOR animated a divider being dragged. In this
//				case, DrawNewXOR() will find out the mouse location, does any constraints
//				necessary, and possibly do a RectRgn with the divider's frame rect. Notice
//				that there is no code in this class that does any tracking. It is the
//				responsibility of client code to track the mouse and call UpdateXOR
//				repeatedly to animate.
//======================================================================================
// Revision History:
// Friday, January 21, 2000 - Original
//======================================================================================
// XORDrawing.h <-- double click + Command-D for header

#include "XORDrawing.h"

#ifndef _H_URegions
#include <URegions.h>
#endif

#ifndef _H_UDrawingState
#include <UDrawingState.h>
#endif


// -----------------------------------------------------------------------------
// ¥ StartXOR
// -----------------------------------------------------------------------------
void
XORDrawing::StartXOR()
{
	mOldRgn.Clear();
	DrawNewXOR();
	StColorPenState pen;
	pen.Normalize();
	
	// Paint the entire region with XOR to draw the first time.
#ifdef PP_Target_Carbon
	::PenMode(patXor);
	Pattern gray;
	UQDGlobals::GetGrayPat(&gray);
	::PenPat( &gray );
#else	
	::PenPat( &UQDGlobals::GetQDGlobals()->gray );
#endif
	::MacPaintRgn( mOldRgn );
}


// -----------------------------------------------------------------------------
// ¥ UpdateXOR
// -----------------------------------------------------------------------------
void
XORDrawing::UpdateXOR()
{
	// Save off the old region
	StRegion	xorRgn( mOldRgn );
	mOldRgn.Clear();
	DrawNewXOR();
	
	// Union the old region and the new region to avoid flicker in the drawing.
	xorRgn ^= mOldRgn;
	StColorPenState pen;
	pen.Normalize();
	
	// Paint the entire region with XOR.
	::PenMode(patXor);
#ifdef PP_Target_Carbon
	Pattern gray;
	UQDGlobals::GetGrayPat(&gray);
	::PenPat( &gray );
#else
	::PenPat( &UQDGlobals::GetQDGlobals()->gray );
#endif
	::MacPaintRgn( xorRgn );
}


// -----------------------------------------------------------------------------
// ¥ EndXOR
// -----------------------------------------------------------------------------
void
XORDrawing::EndXOR()
{
	StColorPenState pen;
	pen.Normalize();
	
	// Paint the entire region with XOR to erase the previous drawing so the
	// original pixels are restored
	::PenMode(patXor);
#ifdef PP_Target_Carbon
	Pattern gray;
	UQDGlobals::GetGrayPat(&gray);
	::PenPat( &gray );
#else
	::PenPat( &UQDGlobals::GetQDGlobals()->gray );
#endif
	::MacPaintRgn( mOldRgn );
	mOldRgn.Clear();
}


// -----------------------------------------------------------------------------
// ¥ Reset
// -----------------------------------------------------------------------------
void
XORDrawing::Reset()
{
	mOldRgn.Clear();
}



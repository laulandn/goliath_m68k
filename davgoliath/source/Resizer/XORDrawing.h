//======================================================================================
// Filename:	XORDrawing.h
// Copyright © 2000 Joseph Chan. All rights reserved.
//
// Description:	An abstract mix-in that encapsulates the process of XOR animation. The
//				region that the class stores is the region that the previous XOR action
//				painted. Derived classes need to override DrawNewXOR() to build the new
//				region to be painted with XOR inside mOldRgn. A typical application of
//				this class would be to XOR animated a divider being dragged. In this
//				case, DrawNewXOR() will find out the mouse location, does any constraints
//				necessary, and possibly do a RectRgn with the divider's frame rect. Notice
//				That there is no code in this class that does any tracking. It is the
//				responsibility of client code to track the mouse and call UpdateXOR
//				repeatedly to animate.
//======================================================================================
// Revision History:
// Friday, January 21, 2000 - Original
//======================================================================================
// XORDrawing.cp <-- double click + Command-D for implementation

#ifndef _H_XORDrawing
#define _H_XORDrawing
#pragma once

#ifndef _H_URegions
#include <URegions.h>
#endif



class XORDrawing
{
public:
	virtual	void		StartXOR();
	virtual	void		UpdateXOR();
	virtual	void		EndXOR();
	virtual	void		Reset();
	
protected:
			StRegion	mOldRgn;
	virtual	void		DrawNewXOR() = 0;
};

#endif


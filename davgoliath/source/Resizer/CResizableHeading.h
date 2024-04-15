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

#ifndef _H_CResizableHeading
#define _H_CResizableHeading
#pragma once

#include "CHeadingTable.h"
#include "CResizerTrackerView.h"


class CResizableHeading :	public CHeadingTable,
							public CResizerTrackerView::CResizerTarget
{
public:
	enum { class_ID = FOUR_CHAR_CODE('RzHd') };
						CResizableHeading(
								LStream					*inStream);
	virtual				~CResizableHeading();

	virtual	void		Click( SMouseDownEvent	&inMouseDown );

	virtual void		AdjustMouse(
								Point				inPortPt,
								const EventRecord&	inMacEvent,
								RgnHandle			outMouseRgn);
	
			bool		IsResizingEnabled() const { return mEnableResizing; }
	virtual	void		EnableResizing() { mEnableResizing = true; }
	virtual	void		DisableResizing() { mEnableResizing = false; }
			
			UInt16		GetMinColWidth() const { return mMinColWidth; }
	virtual	void		SetMinColWidth( UInt16 inMinWidth ) { mMinColWidth = inMinWidth; }
	
protected:
			Point		mInitialPortPt;
			ResIDT		mCursorID;
			UInt16		mMinColWidth;
			TableIndexT	mCurResizedCol;
			Boolean		mEnableResizing;
			Boolean		mLiveResizer;
			
	virtual	TableIndexT	FindResizer( Point inPortPt );
	
	virtual	void		StartTracking( Point inPortPt );
	virtual	void		ResizerMoved( Point inOldPortPt, Point inNewPortPt, RgnHandle inClipRgn );
	virtual	void		EndTracking( Point inPortPt );
	virtual	void		CalcResizerClipRegion( StRegion &ioRgn ) const;
	virtual	void		DoResize( Point inNewPortPt, Point inOldPortPt );
};


#endif
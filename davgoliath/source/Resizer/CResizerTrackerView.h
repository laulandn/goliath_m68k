//======================================================================================
// Filename:	CResizerTrackerView.h
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
// CResizerTrackerView.cp <-- double click + Command-D for implementation

#ifndef _H_CResizerTrackerView
#define _H_CResizerTrackerView
#pragma once

#ifndef _H_LOffscreenView
#include <LOffscreenView.h>
#endif

#ifndef _H_XORDrawing
#include "XORDrawing.h"
#endif


class CResizerTrackerView :	public LOffscreenView,
							public XORDrawing
{
public:
	enum { class_ID = FOUR_CHAR_CODE('RzTk') };
	enum Mode { none, horizontal, vertical };
	
	// Embedded mixin interface
	class CResizerTarget
	{
	public:
		virtual				~CResizerTarget() = 0;
		virtual	void		StartTracking( Point /*inPortPt*/ ) {}
		virtual	void		ResizerMoved(
									Point 		/*inOldPortPt*/,
									Point 		/*inNewPortPt*/,
									RgnHandle	/*inClipRgn*/ ) {}
		virtual	void		EndTracking( Point /*inPortPt*/ ) {}
	};
	
	static	CResizerTrackerView*	FindTracker( LPane *inPane );
							CResizerTrackerView( LStream *inStream );
									
	virtual void			Draw(	RgnHandle		inSuperDrawRgnH);
	
	virtual	bool			TrackResizer(
									Point			inInitialPortPt,
									Mode			inMode,
									CResizerTarget	&inTarget,
									RgnHandle		inClipRgn = 0,	// Must be in Port coordinates
									short			inLineWidth = 3,
									Boolean			inIsLive = false );
	
protected:
			Rect			mCurPortFrameRect;
			RgnHandle		mClipRgn;
			Point			mInitialPortPt, mCurPortPt;
			short			mHalfLineWidth;
			Boolean			mIsTracking;
			Mode			mMode;
			
	virtual	void			DrawNewXOR();
	virtual	void			ConstrainMouse( Point inGlobalPoint, Boolean &inMouseMoved );
	
private:
	// Disallowed
							CResizerTrackerView( const CResizerTrackerView & );
	CResizerTrackerView&	operator=( const CResizerTrackerView & );
};

#endif
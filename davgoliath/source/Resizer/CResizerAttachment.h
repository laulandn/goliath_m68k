//======================================================================================
// Filename:	CResizerAttachment.h
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
// CResizerAttachment.cp <-- double click + Command-D for implementation

#ifndef _H_CResizerAttachment
#define _H_CResizerAttachment
#pragma once

#ifndef _H_LAttachment
#include <LAttachment.h>
#endif

#ifndef _H_LView
#include <LView.h>
#endif

#ifndef _H_CResizerTrackerView
#include "CResizerTrackerView.h"
#endif

class CResizerAttachment :	public LAttachment,
							public CResizerTrackerView::CResizerTarget
{
public:
	enum { class_ID = FOUR_CHAR_CODE('RzAt') };
							CResizerAttachment( LStream *inStream );
	virtual					~CResizerAttachment() {}
	
	virtual	void			Enable();
	virtual	void			Disable();
			Boolean			IsEnabled() const { return mEnabled; }
	CResizerTrackerView::Mode	GetResizerMode() const
								{ return mHorizontal ?
									CResizerTrackerView::horizontal : 
									CResizerTrackerView::vertical; }
								
			LPane*			GetPane1() const { return mPane1; }
			LPane*			GetPane2() const { return mPane2; }
			LPane*			GetResizerPane() const { return mResizerPane; }

protected:
		CResizerTrackerView	*mTracker;
			LPane			*mPane1, *mPane2, *mResizerPane;
			Point			mInitialPortPt;
			short			mPane1MinDim, mPane2MinDim;
			Boolean			mHorizontal, mEnabled, mLiveResizer;
			
	virtual	void			ExecuteSelf( 
								MessageT		inMessage,
								void*			ioParam );
			
	virtual	void			StartTracking( Point inPortPt );
	virtual	void			ResizerMoved( Point inOldPortPt, Point inNewPortPt, RgnHandle inClipRgn );
	virtual	void			EndTracking( Point inPortPt );
	virtual	void			CalcResizerClipRegion( StRegion &ioRgn ) const;
	virtual	void			DoResize( Point inNewPortPt, Point inOldPortPt );
	
private:
	// Disallowed
							CResizerAttachment( const CResizerAttachment & );
		CResizerAttachment&	operator=( const CResizerAttachment & );
};



#endif



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

#ifndef _H_CMultiScrollerView
#define _H_CMultiScrollerView
#pragma once

#include <LScrollerView.h>

#if PP_Uses_Pragma_Import
	#pragma import on
#endif

#pragma warn_hidevirtual off
	template class TArray<LView*>;
#pragma warn_hidevirtual reset


// ---------------------------------------------------------------------------

class CMultiScrollerView :	public LScrollerView
{
public:
	enum { class_ID = FOUR_CHAR_CODE('Mslv') };
						CMultiScrollerView(
								const SPaneInfo&	inPaneInfo,
								const SViewInfo&	inViewInfo,
								SInt16				inHorizBarLeftIndent,
								SInt16				inHorizBarRightIndent,
								SInt16				inVertBarTopIndent,
								SInt16				inVertBarBottomIndent,
								SInt16				inBarThickness,
								LView*				inScrollingView,
								bool				inLiveScrolling,
								const TArray<LView*>&inScrollingViews );
								
						CMultiScrollerView(
								LStream*			inStream);
								
	virtual				~CMultiScrollerView();
	
	virtual void		VertScroll(
								SInt16				inPart);
								
	virtual void		HorizScroll(
								SInt16				inPart);
								
	virtual void		ThumbScroll(
								LScrollBar*			inScrollBar,
								SInt32				inThumbValue);

protected:
	TArray<LView*>	mScrollingViews;

	virtual void		FinishCreateSelf();	
										
private:
	// Disallowed
						CMultiScrollerView(
								const CMultiScrollerView&	inOriginal);
	CMultiScrollerView&	operator=(
								const CMultiScrollerView&	inOriginal);
};



#endif
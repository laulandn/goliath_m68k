//======================================================================================
// Filename:	CAdjustCursorAttachment.h
// Copyright © 2000 Joseph Chan. All rights reserved.
//
// Description:	An attachment for a pane that let it change the cursor when the mouse
//				is inside the pane. You can specify an optional cursor to display when
//				the pane is clicked.
//======================================================================================
// Revision History:
// Friday, January 21, 2000 - Original
//======================================================================================
// CAdjustCursorAttachment.cp <-- double click + Command-D for implementation

#ifndef _H_CAdjustCursorAttachment
#define _H_CAdjustCursorAttachment
#pragma once

#ifndef _H_PP_Messages
#include <PP_Messages.h>
#endif

#ifndef _H_LAttachment
#include <LAttachment.h>
#endif



class CAdjustCursorAttachment :	public LAttachment
{
public:
	enum { class_ID = FOUR_CHAR_CODE( 'CuAt' ) };
							CAdjustCursorAttachment( LStream *inStream );
	virtual					~CAdjustCursorAttachment();
	
protected:
			CCrsrHandle		mCurHandle, mClickCurHand;
			
	virtual	void			ExecuteSelf(
								MessageT		inMessage,
								void*			ioParam );

private:
	// Disallowed
							CAdjustCursorAttachment( const CAdjustCursorAttachment & );
	CAdjustCursorAttachment&	operator=( const CAdjustCursorAttachment & );
};


#endif

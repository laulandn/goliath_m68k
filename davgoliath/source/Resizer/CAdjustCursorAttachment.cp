//======================================================================================
// Filename:	CAdjustCursorAttachment.cp
// Copyright © 2000 Joseph Chan. All rights reserved.
//
// Description:	An attachment for a pane that let it change the cursor when the mouse
//				is inside the pane. You can specify an optional cursor to display when
//				the pane is clicked.
//======================================================================================
// Revision History:
// Friday, January 21, 2000 - Original
//======================================================================================
// CAdjustCursorAttachment.h <-- double click + Command-D for header

#include "CAdjustCursorAttachment.h"

#ifndef _H_LStream
#include <LStream.h>
#endif

// ----------------------------------------------------------------------------------------
//			¥ Constructor
// ----------------------------------------------------------------------------------------
CAdjustCursorAttachment::CAdjustCursorAttachment( LStream *inStream )
	: LAttachment( inStream ), mCurHandle( 0 ), mClickCurHand( 0 )
{
	mMessage = msg_AnyMessage;
	ResIDT	cursID, cursID2;
	inStream->ReadData( &cursID, sizeof( cursID ) );
	inStream->ReadData( &cursID2, sizeof( cursID2 ) );
	mCurHandle = ::GetCCursor( cursID );
	if ( cursID2 && cursID2 != cursID )
		mClickCurHand = ::GetCCursor( cursID2 );
	ThrowIfNil_( mCurHandle );
	// We allow the click cursor to be the same as the mouse inside cursor
}


// ----------------------------------------------------------------------------------------
//			¥ Destructor
// ----------------------------------------------------------------------------------------
CAdjustCursorAttachment::~CAdjustCursorAttachment()
{
	::DisposeCCursor( mCurHandle );
	if ( mClickCurHand )
		::DisposeCCursor( mClickCurHand );
}


// ----------------------------------------------------------------------------------------
//			¥ ExecuteSelf
// ----------------------------------------------------------------------------------------
void
CAdjustCursorAttachment::ExecuteSelf(
				MessageT		inMessage,
				void*			ioParam )
{
#pragma unused( inMessage, ioParam )
	SetExecuteHost( true );
	switch ( inMessage )
	{
		case msg_AdjustCursor:
			::SetCCursor( mCurHandle );
			SetExecuteHost( false );
			break;
		
		case msg_Click:
			if ( mClickCurHand )
				::SetCCursor( mClickCurHand );
			break;
	}
}
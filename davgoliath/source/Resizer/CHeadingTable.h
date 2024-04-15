//======================================================================================
// Filename:	CHeadingTable.h
// Copyright © 2000 Joseph Chan. All rights reserved.
//
// Description:	A table that is used as heading for an LTableView. In order for this
//				class to work properly, all operations involving adding, removing, and
//				resizing a table column must be done via this class, not the table that
//				uses this class as a heading.
//======================================================================================
// Revision History:
// Monday, January 17, 2000 - Original
//======================================================================================

#ifndef _H_CHeadingTable
#define _H_CHeadingTable
#pragma once
#include <Controls.h>
#include <LTableView.h>
#include <LBroadcaster.h>
#include <LListener.h>
#include <LAttachment.h>
#include <ControlDefinitions.h>

class LBevelButton;
class LRadioGroup;

class CHeadingTable :	public LTableView,
						public LBroadcaster,
						public LListener
{
public:
	enum { class_ID = FOUR_CHAR_CODE('HdTb') };
						CHeadingTable(
								LStream					*inStream);
	virtual				~CHeadingTable();

	virtual	void		SetColumnHeading(
								TableIndexT				inCol,
								ConstStringPtr			inHeading,
								ControlButtonTextAlignment inAlignment = teFlushDefault,
								SInt16					inOffset = 0 );
								
	virtual	StringPtr	GetColumnHeading(
								TableIndexT				inCol,
								Str255					outDescriptor ) const;
											
	virtual	SInt16		GetColumnHeadingTextOffset(
								TableIndexT				inCol ) const;
								
	virtual	ControlButtonTextAlignment GetColumnHeadingTextAlignment(
								TableIndexT				inCol ) const;
								
	virtual	SInt32		GetValue() const { return mValue; }
	virtual	void		SetValue(
								SInt32 					inValue );
			SInt32		GetMaxValue() const;
			SInt32		GetMinValue() const;
	
			MessageT	GetValueMessage() const	{ return mValueMessage; }
			void		SetValueMessage(
								MessageT				inValueMessage)
							{ mValueMessage = inValueMessage; }
							
	virtual void		InsertCols(
								UInt32					inHowMany,
								TableIndexT				inAfterCol,
								const void				*inDataPtr = nil,
								UInt32					inDataSize = 0,
								Boolean					inRefresh = false);

	virtual void		RemoveCols(
								UInt32					inHowMany,
								TableIndexT				inFromCol,
								Boolean					inRefresh);
	virtual void		RemoveAllCols(
								Boolean					inRefresh);

	virtual void		AdjustImageSize(
								Boolean					inRefresh);

	virtual void		SetRowHeight(
								UInt16					inHeight,
								TableIndexT				inFromRow,
								TableIndexT				inToRow);
								
	virtual void		SetColWidth(
								UInt16					inWidth,
								TableIndexT				inFromCol,
								TableIndexT				inToCol);
								
	virtual Boolean		GetLocalCellRect(
								const STableCell		&inCell,
								Rect					&outCellFrame) const;

	virtual void		ResizeImageBy(
								SInt32				inWidthDelta,
								SInt32				inHeightDelta,
								Boolean				inRefresh);

	virtual void		ResizeFrameBy(
								SInt16				inWidthDelta,
								SInt16				inHeightDelta,
								Boolean				inRefresh);

	virtual void		ListenToMessage(
								MessageT				inMessage,
								void*					ioParam);
								
protected:
			LTableView	*mTable;
			LRadioGroup	*mRadioGroup;
			LBevelButton *mDummyHeader;
			MessageT	mValueMessage;
			SInt32		mValue;
			ResIDT		mTextTrait;
			
	virtual	void		FinishCreateSelf();

	virtual void		BroadcastValueMessage();

private:
			LPane*		FetchColumnPane(
								TableIndexT				inCol ) const;
};


// Prevent anything from being clicked
class CNoClickAttachment :	public LAttachment
{
public:
	static CNoClickAttachment	*GetAttachment()
							{
								if ( !sAttachment ) sAttachment = new CNoClickAttachment;
								return sAttachment;
							}
protected:
	static	CNoClickAttachment	*sAttachment;
	
							CNoClickAttachment() : LAttachment( msg_Click )
							{ sAttachment = this; }
	virtual	void			ExecuteSelf( MessageT /*inMessage*/, void */*ioParam*/ )
							{ mExecuteHost = false; }
};

#endif



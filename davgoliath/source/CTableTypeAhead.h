/*******************************************************************************
* CTableTypeAhead.h
*
* © CopyRight 1998 Ascend Communications, Inc.
* Author: Thomas Inskip
*
* The CTableTypeAhead attachment handles type-ahead. You must attach this to
* your LOutlineTable object. If using Constructor, set the attachment message
* to msg_KeyPress, and  "Execute host" to true.
*
* The CTypeAheadTable is an abstract class you should mix into your table
* class and implement its GetCellDescriptor member function, or if you're using
* an LOutlineTable, you can instead mix CTypeAheadOutlineItem into your
* LOutlineItem class, and implement its GetDescriptor member function.
*
*
* History:
*	1998.02.12 TWI - Created.
*******************************************************************************/

#pragma once

#include <LAttachment.h>
#include <LPeriodical.h>
#include <UTables.h>


const short kTypeAheadStrLen = 64;

// Attach this to your table class
class CTableTypeAhead: public LAttachment, public LPeriodical
{
	public:
		enum { class_ID = 'CTTA' };
		
						CTableTypeAhead(
								MessageT		inMessage = msg_KeyPress,
								Boolean			inExecuteHost = true);
			
						CTableTypeAhead(
								LStream			*inStream);
							
	private:
		virtual void	ExecuteSelf(
								MessageT		inMessage,
								void			*ioParam);
							
		virtual	void	SpendTime(
								const EventRecord	&inMacEvent);
		
		void			SelectClosestMatch();
		
		void			Normalize(
								Str255 inStr1,
								Str255 inStr2,
								Str255 inStr3);
											
		unsigned char	mString[kTypeAheadStrLen];
		long			mLastKeyTicks;
};


// Mix into your table class, or use CTypeAheadOutlineItem
class CTypeAheadTable
{
	friend class CTableTypeAhead;
	
	protected:
		virtual void	GetCellDescriptor(
							const STableCell &inCell,
							Str255 outDescriptor) = 0;
};


// mix into your LOutlineItem class or use CTypeAheadTable
class CTypeAheadOutlineItem
{
	public:		
		virtual void GetDescriptor(
							Str255 outDescriptor) = 0;
		
};

/*******************************************************************************
* CTableTypeAhead.cp
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

#include "CTableTypeAhead.h"

#include <LTableView.h>
#include <LOutlineTable.h>
#include <LOutlineItem.h>
#include <LString.h>
#include <UKeyFilters.h>

const int kLookupDelay = 20;	// Number of ticks between key presses to do lookup


//------------------------------
// CTableTypeAhead
//------------------------------

// Constructor
CTableTypeAhead::CTableTypeAhead(
	MessageT		inMessage,
	Boolean			inExecuteHost)
		: LAttachment(inMessage, inExecuteHost)
{
	*mString = '\0';
	mLastKeyTicks = 0;
}


// Constructor
CTableTypeAhead::CTableTypeAhead(
	LStream			*inStream)
		: LAttachment(inStream)
{
	*mString = '\0';
	mLastKeyTicks = 0;
}


// Called with key presses
void CTableTypeAhead::ExecuteSelf(
	MessageT		inMessage,
	void			*ioParam)
{
	if (inMessage == msg_KeyPress)
	{
		EventRecord *eventPtr = (EventRecord *)ioParam;
		if (UKeyFilters::PrintingCharField(*eventPtr) == keyStatus_Input)
		{
			mString[++*mString] = eventPtr->message & charCodeMask;
		
			mLastKeyTicks = eventPtr->when;
			StartIdling();
		}
		else
		{
			StopIdling();
			*mString = '\0';
		}
	}
}


// Called at idle-time
void CTableTypeAhead::SpendTime(
	const EventRecord	&inMacEvent)
{
	if ((inMacEvent.when - mLastKeyTicks) >= kLookupDelay)
	{
		SelectClosestMatch();
		
		StopIdling();
		*mString = '\0';
	}
}


// Called after there's been a delay between keys-down events
void CTableTypeAhead::SelectClosestMatch()
{
	Str255 matchString, theString;
	STableCell theCell(0, 0), matchCell;
	LTableView *tableView = dynamic_cast<LTableView *>(mOwnerHost);
	CTypeAheadTable *taTable = dynamic_cast<CTypeAheadTable *>(mOwnerHost);
	LOutlineTable *outlineTable;
	CTypeAheadOutlineItem *taItem;
	
	unsigned char cp1, cp2, cp3;
	
	if (!taTable)
		outlineTable = dynamic_cast<LOutlineTable *>(mOwnerHost);
	if (tableView && (taTable || outlineTable))
	{
		if (tableView->GetNextCell(theCell))
		{
			tableView->UnselectCell(theCell);
			UpperString(mString, true);
			if (taTable)
				taTable->GetCellDescriptor(theCell, matchString);
			else
			{
				taItem = dynamic_cast<CTypeAheadOutlineItem *>
					(outlineTable->FindItemForRow(theCell.row));
				if (taItem)
					taItem->GetDescriptor(matchString);
				else
					*matchString = 0;
			}
			UpperString(matchString, true);
			matchCell = theCell;
			while (tableView->GetNextCell(theCell))
			{
				tableView->UnselectCell(theCell);
				if (taTable)
					taTable->GetCellDescriptor(theCell, theString);
				else
				{
					taItem = dynamic_cast<CTypeAheadOutlineItem *>
						(outlineTable->FindItemForRow(theCell.row));
					if (taItem)
						taItem->GetDescriptor(theString);
					else
						*matchString = 0;
				}
				UpperString(theString, true);
				Normalize(mString, matchString, theString);
				for (unsigned char i = 1; i <= *matchString; ++i)
				{
					cp1 = mString[i];
					cp2 = matchString[i];
					cp3 = theString[i];
					if (cp3 == cp1)
					{
						if (cp2 != cp1)
						{
							LString::CopyPStr(theString, matchString);
							matchCell = theCell;
							break;
						}
					}
					else
						if (cp3 > cp1)
						{
							if (cp2 < cp1)
							{
								LString::CopyPStr(theString, matchString);
								matchCell = theCell;
							}
							else
								if (cp2 > cp1)
								{
									if (cp3 < cp2)
									{
										LString::CopyPStr(theString, matchString);
										matchCell = theCell;
									}
								}
							break;
						}
						else
							if (cp3 < cp1)
							{
								if (cp2 < cp1)
								{
									if (cp3 > cp2)
									{
										LString::CopyPStr(theString, matchString);
										matchCell = theCell;
									}
								}
								break;
							}
				}
			}
			tableView->SelectCell(matchCell);
			tableView->ScrollCellIntoFrame(matchCell);
		}
	}
}

// Makes all three strings the same length, padding them if
// necessary with spaces
void CTableTypeAhead::Normalize(
	Str255 inStr1,
	Str255 inStr2,
	Str255 inStr3)
{
	unsigned char len = *inStr1;
	if (*inStr2 > len)
		len = *inStr2;
	if (*inStr3 > len)
		len = *inStr3;
	for (unsigned char i = *inStr1 + 1; i <= len; ++i)
		inStr1[i] = ' ';
	*inStr1 = len;
	for (unsigned char i = *inStr2 + 1; i <= len; ++i)
		inStr2[i] = ' ';
	*inStr2 = len;
	for (unsigned char i = *inStr3 + 1; i <= len; ++i)
		inStr3[i] = ' ';
	*inStr3 = len;
}

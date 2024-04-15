/* ==================================================================================================
 * CPropDisplayTable.h															   
 *    Goliath - a Finder like application that implements WebDAV
 *    Copyright (C) 1999-2001  Thomas Bednarz
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  For questions, suggestions, bug-reports, enhancement-requests etc.
 *  I may be contacted at:
 *
 *  tombednarz@hotmail.com
 * ===========================================================================
 */

#ifndef __CPROPDISPLAYTABLE_H_
#define __CPROPDISPLAYTABLE_H_

#pragma once

#include "CDAVTableApp.h"
#include <LArray.h>

#include <LMutexSemaphore.h>
#include <CSortableOutlineTable.h>
#include <LListener.h>
#include <LBevelButton.h>

class CPropDisplayTable : public CSortableOutlineTable, public LListener, public LCommander {
	public:
		enum {class_ID = 'PRPT'};
		
		
		CPropDisplayTable(LStream *inStream);
		~CPropDisplayTable();
		void Setup(LBevelButton *btn, LView* header);
        void AddPropVal(CDAVProperty& prop, std::string &val);
        virtual void ListenToMessage(MessageT inMessage, void *ioParam);
        virtual void ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, Boolean inRefresh);
        virtual void Sort(Boolean inRefesh);

	    virtual void SetColWidth(UInt16	inWidth, TableIndexT inFromCol,
								TableIndexT	inToCol);
								
        virtual void FindCommandStatus(CommandT inCommand, Boolean &outEnabled, Boolean &outUsesMark, UInt16 &outMark, Str255 outName);
        virtual Boolean ObeyCommand(CommandT inCommand, void *ioParam);
        

    protected:
       LBevelButton* mSortButton;
       LView*        mTableHeader;
       
};

#endif
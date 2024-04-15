/* ===========================================================================
 *	CDAVHeadingTable.cpp	   
 *
 *    Goliath - a Finder like application that implements WebDAV
 *    Copyright (C) 1999-2001  Thomas Bednarzär
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
 */
 

#include "CDAVHeadingTable.h"
#include <LTablePaneHostingGeometry.h>
#include <LBevelButton.h>
#include <LBroadcaster.h>

// ---------------------------------------------------------------------------
//		¥ CDAVHeadingTable()
// ---------------------------------------------------------------------------
//
CDAVHeadingTable::CDAVHeadingTable(LStream *inStream): CResizableHeading(inStream) {

}

// ---------------------------------------------------------------------------
//		¥ ~CDAVHeadingTable()
// ---------------------------------------------------------------------------
//
CDAVHeadingTable::~CDAVHeadingTable() {

}

// ---------------------------------------------------------------------------
//		¥ SetColumnHeadingValueMessage()
// ---------------------------------------------------------------------------
//
void CDAVHeadingTable::SetColumnHeadingValueMessage(TableIndexT inCol, MessageT msg) {
   LPane *thePane = _GetHeaderPane(inCol);
   
   if ( LBevelButton *bb = dynamic_cast<LBevelButton*>(  thePane  ) )
     bb->SetValueMessage(msg);
}

// ---------------------------------------------------------------------------
//		¥ SetColumnListener()
// ---------------------------------------------------------------------------
//
void CDAVHeadingTable::SetColumnListener(TableIndexT inCol, LListener *listener) {
   LPane *thePane = _GetHeaderPane(inCol);
   
   if ( LBroadcaster *bb = dynamic_cast<LBroadcaster*>(  thePane  ) )
     bb->AddListener(listener);
}

// ---------------------------------------------------------------------------
//		¥ _GetHeaderPane()
// ---------------------------------------------------------------------------
//
LPane* CDAVHeadingTable::_GetHeaderPane(TableIndexT inCol) {
	// ***teb; begin; this code is from the private method CResizableHeading::DoResize
	
	LPane	*thePane = NULL;
	if ( inCol > 0 && inCol <= mCols )
	{
		LTablePaneHostingGeometry	*theGeometry = 
				dynamic_cast<LTablePaneHostingGeometry*>( mTableGeometry );
		STableCell	theCell( 1, inCol );
		thePane = theGeometry->GetCellPane( theCell );
	}
   return thePane;
   //***teb; end duplicated code
   

}
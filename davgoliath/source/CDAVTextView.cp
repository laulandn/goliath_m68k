/* ==================================================================================================
 * CDAVTextView.cpp															   
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

#include "CDAVTextView.h"

#include <LStream.h>


// ---------------------------------------------------------------------------------
//	¥ CDAVTextView												[public]
// ---------------------------------------------------------------------------------
//	LStream constructor

CDAVTextView::CDAVTextView(
	LStream*	inStream)

	: LTextEditView(inStream)
{
	mIsDirty = false;
}


// ---------------------------------------------------------------------------------
//	¥ ~CDAVTextView											[public, virtual]
// ---------------------------------------------------------------------------------
//	Destructor

CDAVTextView::~CDAVTextView()
{
	// nothing
}


// ---------------------------------------------------------------------------------
//	¥ UserChangedText										[public, virtual]
// ---------------------------------------------------------------------------------
//	Called when a user action changes the text

void
CDAVTextView::UserChangedText()
{
	if (IsDirty() == false) {

			// Set the update menus flag.
		SetUpdateCommandStatus(true);
		
			// Set the dirty flag.
		SetDirty(true);
	}
}


// ---------------------------------------------------------------------------------
//	¥ SavePlace												[public, virtual]
// ---------------------------------------------------------------------------------

void
CDAVTextView::SavePlace(
	LStream*	outPlace )
{
		// Call inherited.
	LTextEditView::SavePlace(outPlace);
	
		// Save the image size.
	outPlace->WriteData(&mImageSize, sizeof(SDimension32));
}


// ---------------------------------------------------------------------------------
//	¥ RestorePlace											[public, virtual]
// ---------------------------------------------------------------------------------

void
CDAVTextView::RestorePlace(
	LStream*	inPlace )
{
		// Call inherited.
	LTextEditView::RestorePlace(inPlace);
	
		// Restore the image size.
	inPlace->ReadData(&mImageSize, sizeof(SDimension32));
	
		// Realign the text edit rects.
	AlignTextEditRects();
}

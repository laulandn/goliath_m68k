/* ==================================================================================================
 * CDAVTextView.h															   
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

#ifndef _H_CDAVTextView
#define _H_CDAVTextView
#pragma once

#include <LTextEditView.h>

class CDAVTextView : public LTextEditView {

public:
			enum { class_ID = FOUR_CHAR_CODE('TxtV') };
	
							CDAVTextView(
								LStream*		inStream);
	virtual					~CDAVTextView();
	
	virtual void			UserChangedText();
	
	virtual void			SavePlace(
								LStream*		outPlace);
	virtual void			RestorePlace(
								LStream*		inPlace);

			Boolean			IsDirty() const
								{
									return mIsDirty;
								}
			void			SetDirty(
								Boolean			inDirty)
								{
									mIsDirty = inDirty;
								}
								
protected:
			bool			mIsDirty;

private:
							CDAVTextView();
							CDAVTextView(const CDAVTextView& inOriginal);
			CDAVTextView&		operator=(const CDAVTextView& inRhs);
};

#endif // _H_CDAVTextView
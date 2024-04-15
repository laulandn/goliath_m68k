/* ==================================================================================================
 * AppleDoubleUtils.h															   
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

#ifndef __AppleDoubleUtils_h__
#define __AppleDoubleUtils_h__

#pragma once

#ifndef _LIST
#include <list>
#endif

/* format contants */
const long kAppleDoubleMagic = 0x00051607L;
const long kAppleDoubleVersion = 0x00020000;


/* applefile structures */
typedef struct af_header {
	UInt32 magic;
	UInt32 version;
	char fill[16];
	UInt16 entries;
} applfile_header;

typedef struct af_entry{
	UInt32 mId;
	UInt32 mOffset;
	UInt32 mLength;
} applfile_entry;

/* ID's for some AppleDefined entries */
const unsigned long kResourceForkEntry	= 2;
const unsigned long	kFinderInfoEntry	= 9;

void BuildAppleFileHeader(applfile_header &afhdr);

OSErr GetAppleFileHeader(FSSpec &inSpec, applfile_header &afhdr, std::list<applfile_entry>& outEntries);

#endif

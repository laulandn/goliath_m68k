/* ==================================================================================================
 * AppleDoubleUtils.cpp														   
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
#include "AppleDoubleUtils.h"

// ---------------------------------------------------------------------------
//		¥ BuildAppleFileHeader()
// ---------------------------------------------------------------------------
//		
void BuildAppleFileHeader(applfile_header &afhdr) {
	afhdr.magic = kAppleDoubleMagic;
	afhdr.version = kAppleDoubleVersion;
	for (int i=0; i<16; i++)
		afhdr.fill[i]=0;
	afhdr.entries =0;
}

// ---------------------------------------------------------------------------
//		¥ GetAppleFileHeader()
// ---------------------------------------------------------------------------
//		
OSErr GetAppleFileHeader(FSSpec &inSpec, applfile_header &afhdr, std::list<applfile_entry>& outEntries) {
	int foo = sizeof(short);
	OSErr err;
	SInt16 refNum;
	
	err = ::FSpOpenDF (&inSpec, fsRdPerm, &refNum);
	if (err != noErr)
		return err;
		
	SInt32 ioByteCount=0;
	
	ioByteCount = sizeof(afhdr.magic);	
	err = ::FSRead (refNum, &ioByteCount, &afhdr.magic);
	if (err != noErr)
		goto Exit;
				
	if (afhdr.magic != kAppleDoubleMagic)
		goto Exit;

	ioByteCount = sizeof(afhdr.version);	
	err = ::FSRead (refNum, &ioByteCount, &afhdr.version);
	if (err != noErr)
		goto Exit;
	if (afhdr.version != kAppleDoubleVersion)
		goto Exit;

	ioByteCount = sizeof(char[16]);	
	err = ::FSRead (refNum, &ioByteCount, &afhdr.fill);
	if (err != noErr)
		goto Exit;

	ioByteCount = sizeof(afhdr.entries);	
	err = ::FSRead (refNum, &ioByteCount, &afhdr.entries);
	if (err != noErr)
		goto Exit;

	applfile_entry	afEntry;
	ioByteCount = sizeof(UInt32);
	for (int i = 0; i < afhdr.entries; ++i) {
		err = ::FSRead(refNum, &ioByteCount, &afEntry.mId);
		if (err != noErr)
			break;	
		err = ::FSRead(refNum, &ioByteCount, &afEntry.mOffset);
		if (err != noErr)
			break;	
		err = ::FSRead(refNum, &ioByteCount, &afEntry.mLength);
		if (err != noErr)
			break;	
		outEntries.push_back(afEntry);
	}

Exit:
	::FSClose(refNum);
	return err;
}





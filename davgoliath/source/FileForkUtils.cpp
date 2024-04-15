/* ==================================================================================================
 * FileForkUtils.h															   
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

#ifndef __FileForkUtils_h__
#include "FileForkUtils.h"
#endif
#ifndef __AppleDoubleUtils_h__
#include "AppleDoubleUtils.h"
#endif
#ifndef __LSequentialFileStream_h__
#include "LSequentialFileStream.h"
#endif
#include <assert.h>

const UInt32 kBigCopyBuffSize  = 0x00004000;
const UInt32 kMinCopyBuffSize  = 0x00000200;

using namespace std;

// ---------------------------------------------------------------------------
//		¥ RestoreForks()
// ---------------------------------------------------------------------------
//		
static OSErr RestoreForks(	FSSpec& inRezFork, FSSpec& inRezInDF, UInt32 srcOffset, 
							void *copyBufferPtr, long copyBufferSize) {

   try {
      LWriteOnlyFileStream rezForkStream;
      rezForkStream.Open(inRezFork, true);
   
      LFileStream rezInDFStream(inRezInDF);
      rezInDFStream.OpenDataFork(fsRdPerm);
      rezInDFStream.SetMarker(srcOffset, streamFrom_Start);
   
      SInt32 bytesToGo = rezInDFStream.GetLength() - rezInDFStream.GetMarker();
   
      while (bytesToGo != 0) {
   	     SInt32 bytesWritten = rezInDFStream.ReadData(copyBufferPtr, std::min(bytesToGo, copyBufferSize));
		 rezForkStream.WriteData(copyBufferPtr, bytesWritten);
		 bytesToGo -= bytesWritten;
    }
   } catch (LException& e) {
   	  return e.GetErrorCode();
   }
   return noErr;
}

// ---------------------------------------------------------------------------
//		¥ RestoreFinderInfo()
// ---------------------------------------------------------------------------
//		
static OSErr RestoreFinderInfo( FSSpec& destFile, FSSpec& inRezInDF,  applfile_entry& inFinfoEntry) {
	FInfo outFinfo;
	FXInfo outFxInfo;

	assert(inFinfoEntry.mLength == sizeof(FInfo) + sizeof(FXInfo));
	try {
		LFileStream rezInDFStream(inRezInDF);
		rezInDFStream.OpenDataFork(fsRdPerm);
		rezInDFStream.SetMarker(inFinfoEntry.mOffset, streamFrom_Start);
		rezInDFStream.ReadData(&outFinfo, sizeof(FInfo));
		rezInDFStream.ReadData(&outFxInfo, sizeof(FXInfo));
		outFinfo.fdLocation.v = outFinfo.fdLocation.h = 0;
		
		return ::FSpSetFInfo(&destFile, &outFinfo);
 
	} catch (LException& e) {
		return e.GetErrorCode();
	}
	return noErr;
}

// ---------------------------------------------------------------------------
//		¥ MergeFlattenedForks()
// ---------------------------------------------------------------------------
//		
OSErr MergeFlattenedForks(FSSpec& inSourceDataFork, FSSpec& inSourceResDataFork) {
	OSErr err = noErr;
	Ptr copyBufferPtr;
	long copyBufferSize;

	applfile_header afhdr;
	std::list<applfile_entry> entries;
	err = GetAppleFileHeader(inSourceResDataFork, afhdr, entries);
	if (err != noErr)
		return err;

    UInt32 srcOffset = 0;
    std::list<applfile_entry>::iterator iter = entries.begin();
    while (iter != entries.end()) {
       if (kResourceForkEntry == (*iter).mId) {
          srcOffset = (*iter).mOffset;
          break;
       }
       ++iter;
    }
    
	copyBufferSize = kBigCopyBuffSize;
	copyBufferPtr = NewPtr(copyBufferSize);
	if ( copyBufferPtr == NULL ) {
		copyBufferSize = kMinCopyBuffSize;
		copyBufferPtr = NewPtr(copyBufferSize);
		if ( copyBufferPtr == NULL ){
			return ( memFullErr );
		}
	}
	
	StPointerBlock stPtrBlock(copyBufferPtr);

	::FSpCreateResFile(&inSourceDataFork, '????', '????', smSystemScript);
	err = ResError();
	if ( err != noErr )
		return err;

	err = RestoreForks(inSourceDataFork, inSourceResDataFork, srcOffset, copyBufferPtr, copyBufferSize);
	if (err != noErr)
		return err;
		
    iter = entries.begin();
    while (iter != entries.end()) {
       if (kFinderInfoEntry == (*iter).mId) {
		  return RestoreFinderInfo( inSourceDataFork, inSourceResDataFork,  *iter);
          break;
       }
       ++iter;
    }
    return noErr;
} 

// ---------------------------------------------------------------------------
//		¥ BuildAppleDoubleRezFile()
// ---------------------------------------------------------------------------
//		
OSErr BuildAppleDoubleRezFile(FSSpec& inDualForkedFile, FSSpec& outDblFile) {
	try {
	
	Ptr copyBufferPtr;
	long copyBufferSize;

	copyBufferSize = kBigCopyBuffSize;
	copyBufferPtr = NewPtr(copyBufferSize);
	if ( copyBufferPtr == NULL ) {
		copyBufferSize = kMinCopyBuffSize;
		copyBufferPtr = NewPtr(copyBufferSize);
		if ( copyBufferPtr == NULL ){
			return ( memFullErr );
		}
	}
	
	StPointerBlock stPtrBlock(copyBufferPtr);
	
	//see if we have res fork
   short tmpRefNum;
   long fileSize=0;
   OSErr err = ::FSpOpenRF (&inDualForkedFile, fsRdPerm, &tmpRefNum);
   if (!err)
      ::GetEOF (tmpRefNum, &fileSize);
   if (0 != tmpRefNum)
      ::FSClose(tmpRefNum);

	FInfo finfo;
	FXInfo fxInfo;
	memset(&fxInfo, 0, sizeof(FXInfo));
	err = ::FSpGetFInfo(&inDualForkedFile, &finfo);


	LWriteOnlyFileStream outStream;
    outStream.Open(outDblFile);
   
	applfile_header afhdr;
	BuildAppleFileHeader(afhdr);

    int numEntries = 1;
    if (tmpRefNum != 0)
       numEntries++;
       
	applfile_entry entry1, entry2;
	UInt32 initOffset = sizeof(afhdr.magic) + sizeof(afhdr.version) + sizeof(afhdr.fill) + sizeof(afhdr.entries)
						 + (numEntries*sizeof(applfile_entry));
	
	entry1.mId = kFinderInfoEntry;
	entry1.mOffset = initOffset;
	entry1.mLength = sizeof(FInfo) + sizeof(FXInfo);

	afhdr.entries = numEntries;
	
	initOffset += entry1.mLength;
	entry2.mId = kResourceForkEntry;
	entry2.mLength = fileSize;
	entry2.mOffset = entry1.mOffset + entry1.mLength;
	
	SInt32 byteCnt;

	byteCnt = sizeof(afhdr.magic);	
	outStream.PutBytes(&afhdr.magic, byteCnt);
	byteCnt = sizeof(afhdr.version);	
	outStream.PutBytes(&afhdr.version, byteCnt);
	byteCnt = sizeof(afhdr.fill);	
	outStream.PutBytes(&afhdr.fill, byteCnt);
	byteCnt = sizeof(afhdr.entries);	
	outStream.PutBytes(&afhdr.entries, byteCnt);


	byteCnt = sizeof(entry1.mId);	
	outStream.PutBytes(&entry1.mId, byteCnt);
	byteCnt = sizeof(entry1.mOffset);	
	outStream.PutBytes(&entry1.mOffset, byteCnt);
	byteCnt = sizeof(entry1.mLength);	
	outStream.PutBytes(&entry1.mLength, byteCnt);

	if (2 == numEntries) {
		byteCnt = sizeof(entry2.mId);	
		outStream.PutBytes(&entry2.mId, byteCnt);
		byteCnt = sizeof(entry2.mOffset);	
		outStream.PutBytes(&entry2.mOffset, byteCnt);
		byteCnt = sizeof(entry2.mLength);	
		outStream.PutBytes(&entry2.mLength, byteCnt);
	}

	assert(outStream.GetMarker() == entry1.mOffset);
	byteCnt = sizeof(FInfo);
	outStream.PutBytes(&finfo, byteCnt);
	byteCnt = sizeof(FXInfo);
	outStream.PutBytes(&fxInfo, byteCnt);
	assert(outStream.GetMarker() == entry1.mOffset + entry1.mLength);

	if (2 == numEntries) {
		LReadOnlyFileStream rezInStream;
		rezInStream.Open(inDualForkedFile, true);
		assert(outStream.GetMarker() == entry2.mOffset);
		SInt32 bytesToGo = fileSize;
		while (bytesToGo != 0) {
			SInt32  bytesRead  =  std::min(bytesToGo, copyBufferSize);
			rezInStream.GetBytes(copyBufferPtr, bytesRead);
			outStream.PutBytes(copyBufferPtr, bytesRead);
			bytesToGo -= bytesRead;
		}
		assert(outStream.GetMarker() == entry2.mOffset + entry2.mLength);
	}
	} catch (LException &e) {
		return e.GetErrorCode();
	}
	
	return noErr;
}


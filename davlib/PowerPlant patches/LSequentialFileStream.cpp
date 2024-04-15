/*
	LSequentialFileStream.cpp
	Copyright (C) 2000 i-drive.com (email: opensource@mail.idrive.com)

	This library is free software; you can redistribute it and/or
	modify it under the terms of version 2.1 of the GNU Lesser General
	Public License as published by the Free Software Foundation.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "LSequentialFileStream.h"
#include <Devices.h>
#include <algorithm>

using namespace std;


// File system works optimally with data blocks aligned to 512 at both ends

static inline char* RoundUpTo512 (char *p)
{
	return (char*) ((long)(p + 511) & (~511));
}

static long RoundDownTo512 (long s)
{
	return (s & (~511));
}

//============================================================
// LReadOnlyFileStream
//============================================================

// const SInt32 LReadOnlyFileStream::kFileBufferSize = 8192;

LReadOnlyFileStream::LReadOnlyFileStream (void)
{
#if 0
	mBufferRaw = new char [kFileBufferSize + 512];
	mBuffer = RoundUpTo512 (mBufferRaw);
	mBufferPos = 0;
	mBufferEnd = 0;
	mFileSize = 0;
#endif

	mRefNum = 0;
}

LReadOnlyFileStream::~LReadOnlyFileStream ()
{
	Close();
#if 0
	delete [] mBufferRaw;
#endif
}

void LReadOnlyFileStream::Open (const FSSpec &spec, bool resFork)
{
	OSErr	err;
	
	if (mRefNum)
		Throw_Err (fBsyErr);
		
	if (resFork)
		err = FSpOpenRF (&spec, fsRdPerm, &mRefNum);
	else
		err = FSpOpenDF (&spec, fsRdPerm, &mRefNum);
	ThrowIfOSErr_ (err);
#if 0
	GetEOF (mRefNum, &mFileSize);
#endif
}

void LReadOnlyFileStream::Close (void)
{
	if (mRefNum == 0)
		return;
	::FSClose (mRefNum);
	mRefNum = 0;
#if 0
	mFileSize = 0;
#endif
}

// Would be nice to set up a read-ahead buffer at some later point

ExceptionCode LReadOnlyFileStream::GetBytes (void *outBuffer, SInt32 &ioByteCount)
{
	return ::FSRead (mRefNum, &ioByteCount, outBuffer);
}


//============================================================
// LWriteOnlyFileStream
//============================================================

const SInt32 LWriteOnlyFileStream::kFileBufferSize = 8192;


LWriteOnlyFileStream::LWriteOnlyFileStream (void)
{
	mBufferRaw = 0;
	mBuffer = 0;
	mBufferPos = 0;
	mRefNum = 0;
}

LWriteOnlyFileStream::~LWriteOnlyFileStream ()
{
	Close();
}

void LWriteOnlyFileStream::Open (const FSSpec &spec, bool resFork)
{
	OSErr	err;
	
	if (mRefNum)
		Throw_Err (fBsyErr);
		
	mBufferRaw = new char [kFileBufferSize + 512];
	mBuffer = RoundUpTo512 (mBufferRaw);

	if (resFork)
		err = ::FSpOpenRF (&spec, fsWrPerm, &mRefNum);
	else
		err = ::FSpOpenDF (&spec, fsWrPerm, &mRefNum);
	ThrowIfOSErr_ (err);
}

void LWriteOnlyFileStream::Close (void)
{
	if (mRefNum == 0)
		return;
	FlushBuffer ();
	::FSClose (mRefNum);
	mRefNum = 0;
	delete [] mBufferRaw;
	mBufferRaw = 0;
	mBuffer = 0;
}

ExceptionCode LWriteOnlyFileStream::PutBytes (const void *inBuffer, SInt32 &ioByteCount)
{
	OSErr	err = noErr;
	SInt32	bytesLeft = ioByteCount;
	
	if (mBuffer == 0)
		return memAdrErr;
		
	while (bytesLeft > 0)
	{
		SInt32 bytesToWrite = min (bytesLeft, kFileBufferSize - mBufferPos);
		BlockMoveData (inBuffer, mBuffer + mBufferPos, bytesToWrite);
		
		mPosition += bytesToWrite;
		mBufferPos += bytesToWrite;
		
		(char*)inBuffer += bytesToWrite;
		bytesLeft -= bytesToWrite;
		
		if (mBufferPos >= kFileBufferSize)
		{		
			err = FlushBuffer();
			if (err)
				break;
		}
	}
	return err;
}

OSErr LWriteOnlyFileStream::FlushBuffer (void)
{
	SInt32 count = mBufferPos;
	ParamBlockRec pb;
	
	mBufferPos = 0;
                                          
	pb.ioParam.ioRefNum = mRefNum;
	pb.ioParam.ioBuffer = mBuffer;
	pb.ioParam.ioReqCount = count;
	pb.ioParam.ioPosMode = fsAtMark + noCacheMask;
	pb.ioParam.ioPosOffset = 0;
	return ::PBWriteSync(&pb);
}
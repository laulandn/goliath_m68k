/*
	LSequentialFileStream.h
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

#ifndef __LSequentialFileStream_h__
#define __LSequentialFileStream_h__

#include "LSequentialStream.h"

/*
	LReadOnlyFileStream and LWriteOnlyFileStream are sequential
	streams representing files. Bytes can be read/written in 
	sequential order only. I/O is buffered for efficiency. Only one
	fork can be open at a time.
*/

//--------------------- LReadOnlyFileStream ---------------------

class LReadOnlyFileStream : public LSequentialOutputStream
{
public:
	LReadOnlyFileStream (void);
	~LReadOnlyFileStream ();
	
	void					Open (const FSSpec &spec, bool resFork = false);
	void					Close (void);
	virtual ExceptionCode	GetBytes(
								void	*outBuffer,
								SInt32	&ioByteCount);

private:

//	Read ahead buffer
#if 0
	const static SInt32		kFileBufferSize;
	char*					mBufferRaw;
	char*					mBuffer;		// Aligned to 512 boundary
	SInt32					mBufferPos;
	SInt32					mBufferEnd;
	
	SInt32					mFileSize;
#endif

	SInt16					mRefNum;
};


//-------------------- LWriteOnlyFileStream ---------------------

class LWriteOnlyFileStream : public LSequentialInputStream
{
public:
	LWriteOnlyFileStream (void);
	~LWriteOnlyFileStream ();

	void					Open (const FSSpec &spec, bool resFork = false);
	void					Close (void);
	void Create				(OSType inCreator = '????', OSType inFileTyper = '????', ScriptCode	inScriptCode = smSystemScript);
	
	virtual ExceptionCode	PutBytes(
								const void	*inBuffer,
								SInt32		&ioByteCount);
								


private:
	OSErr					FlushBuffer (void);
	
	const static SInt32		kFileBufferSize;
	char*					mBufferRaw;
	char*					mBuffer;		// Aligned to 512 boundary
	SInt32					mBufferPos;
	SInt16					mRefNum;
};

#endif

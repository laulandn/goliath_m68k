/*
	LMacBinary.h
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

#include <string>
#include "LSequentialFileStream.h"

/*
	MacBinary header needs to be broken up into several parts
	to ensure that everything larger than a byte is even-aligned.
*/

#pragma options align=packed
							//			-- Offset --
							//	in struct	in MacBinary header
typedef struct
{
	UInt8	mOldVersion;			//	0		//   0
} SMacBinaryHeader1;

typedef struct
{
	Byte	mNameLength;			//	 0		//   1
	char	mName [63];				//	 1		//   2
	OSType	mType;					//	64		//  65
	OSType	mCreator;				//	68		//  69
	Byte	mFinderFlags;			//	72		//  73
	Byte	mFiller1;				//	73		//  74
	Point	mPosition;				//	74		//  75
	short	mFolderID;				//	78		//  79
	Byte	mProtected;				//	80		//  81
	Byte	mFiller2;				//	81		//  82
	UInt32	mDataForkLen;			//	82		//  83
	UInt32	mResForkLen;			//	86		//  87
	UInt32	mCreationDate;			//	90		//  91
	UInt32	mModificationDate;		//	94		//  95
	UInt16	mCommentLength;			//	98		//  99
	Byte	mFinderFlags2;			// 100		// 101
} SMacBinaryHeader2;

typedef struct
{
	OSType	mSignature;				//	 0		// 102
	Byte	mScript;				//	 4		// 106
	Byte	mXFinderFlags;			//	 5		// 107
	UInt32	mFiller3;				//	 6		// 108
	UInt32	mFiller4;				//	10		// 112
	UInt32	mTotalFileSize;			//	14		// 116
	UInt16	mSecondaryHeaderLength;	//	18		// 120
	Byte	mVersionNumber;			//	20		// 122
	Byte	mMinVersionNumber;		//	21		// 123
} SMacBinaryHeader3;

typedef struct
{
	UInt16	mCRC;					//	0		// 124
	UInt16	mFiller5;				//	2		// 126
} SMacBinaryHeader4;

#pragma options align=reset

typedef struct
{
	SMacBinaryHeader1	h1;
	SMacBinaryHeader2	h2;
	SMacBinaryHeader3	h3;
	SMacBinaryHeader4	h4;
	
	inline static unsigned short short_swap (unsigned short x);
	unsigned short CalculateCRC (void);
	unsigned short CrcCITTA (unsigned short crc, char v);
} SMacBinaryHeader;

#define kMBHeaderSize 128

//------------------------ LMacBinaryStream ---------------------
/*
	Abstract parent class for LMacBinaryEncoder and LMacBinaryDecoder.
*/
class LMacBinary
{
public:
	LMacBinary (void);
	
protected:
	
	SMacBinaryHeader		mMacBinaryHeader;
	std::string				mComment;
	
	// Offsets to various parts of file
	
	UInt32					mDataForkOffset;
	UInt32					mResourceForkOffset;
	UInt32					mCommentOffset;
	
	bool					mValidHeader;
	FSSpec					mFSSpec;
};

//------------------------ LMacBinaryEncoder ---------------------
/*
	A read-only file stream. Constructed from an existing Mac OS
	File. Output is a stream representing the file encoded in
	MacBinary III format (www.lazerware.com/formats/index.html).
*/

class LMacBinaryEncoder : public LSequentialOutputStream, LMacBinary
{
public:
	LMacBinaryEncoder	(const FSSpec& inSourceFile);
	virtual ~LMacBinaryEncoder	();
	
	virtual ExceptionCode	GetBytes(
									void*			outBuffer,
									SInt32&			ioByteCount);

	virtual SInt32			GetLength (void) const;

private:
	LReadOnlyFileStream		mDataForkStream;
	LReadOnlyFileStream		mResourceForkStream;
	bool					BuildHeader (void);
};

//------------------------ LMacBinaryDecoder ---------------------
/*
	A write-only stream. Creates a new file as specified. Accepts
	bytes representing a MacBinary-encoded file and writes the
	decoded result to the file.
*/

class LMacBinaryDecoder : public LSequentialInputStream, LMacBinary
{
public:	
	LMacBinaryDecoder  (void);
	LMacBinaryDecoder  (		const FSSpec& inTargetDirSpec,
								SInt32 totalSize,
								bool setFinderPosition = false);
						
	virtual ~LMacBinaryDecoder ();
	
	void					SetSpec (const FSSpec& inTargetDirSpec,
									SInt32 totalSize = 0,
									bool setFinderPosition = false);
	virtual SInt32			GetLength() const;
	
	virtual ExceptionCode	PutBytes(
									const void*		inBuffer,
									SInt32&			ioByteCount);
									
private:
	ExceptionCode 			HeaderReceived (void);
	ExceptionCode			FinishFile (void);
	
	LWriteOnlyFileStream	mDataForkStream;
	LWriteOnlyFileStream	mResourceForkStream;
	bool					mSetFinderPosition;
	SInt32					mTotalSize;
};



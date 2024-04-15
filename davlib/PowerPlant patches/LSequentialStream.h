/*
	LSequentialStream.h
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
#ifndef __LSequentialStream_h__
#define __LSequentialStream_h__

#pragma once
#ifndef _H_LStream
#include <LStream.h>
#endif

/*
	Sequential streams are one way streams: they are either read-only
	or write-only, and you can't seek or modify their lengths. They are
	useful for implementing encoders/decoders. Sequential streams can
	be chained (piped) to each other.
*/

// Abstract superclass implementing a read-only stream.
// Must be subclassed and GetBytes() implemented.

class LSequentialOutputStream : public LStream
{
public:
	LSequentialOutputStream (void);
	virtual ~LSequentialOutputStream ();
	
	virtual void			SetLength (SInt32	inLength);
	virtual SInt32			GetLength (void) const;
	virtual void			SetMarker (SInt32 inOffset, EStreamFrom inFromWhere);
	virtual SInt32			GetMarker (void) const;
	
protected:
	
	SInt32					mPosition;			// internal position
};

// Abstract superclass implementing a read-only stream.
// Must be subclassed and PutBytes() implemented.

class LSequentialInputStream : public LStream
{
public:
	LSequentialInputStream (void);
	virtual ~LSequentialInputStream ();
	
	virtual void			SetLength (SInt32	inLength);
	virtual SInt32			GetLength (void) const;
	virtual void			SetMarker (SInt32 inOffset, EStreamFrom inFromWhere);
	virtual SInt32			GetMarker (void) const;
	
protected:

	SInt32					mPosition;			// internal position
};

#endif
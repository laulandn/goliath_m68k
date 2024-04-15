/*
	LSequentialStream.cpp
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
#include <LSequentialStream.h>
#endif

// ===========================================================================
//	LSequentialInputStream
// ===========================================================================

LSequentialInputStream::LSequentialInputStream (void)
{
	mPosition = 0;
}

LSequentialInputStream::~LSequentialInputStream ()
{
}

void LSequentialInputStream::SetLength (SInt32 inLength)
{
	if (mPosition == 0 && inLength == 0)
		return;
	Throw_Err (writErr);
}

SInt32 LSequentialInputStream::GetLength (void) const
{
	return mPosition;
}

void LSequentialInputStream::SetMarker( SInt32		inOffset,
								EStreamFrom		inFromWhere)
{
	if (inFromWhere == streamFrom_End && inOffset == 0)
		return;

	if (inFromWhere == streamFrom_Start && inOffset == 0 && mPosition == 0)
		return;
		
	Throw_Err (writErr);
}

SInt32 LSequentialInputStream::GetMarker (void) const
{
	return mPosition;
}

// ===========================================================================
//	LSequentialOutputStream
// ===========================================================================

LSequentialOutputStream::LSequentialOutputStream (void)
{
	mPosition = 0;
}

LSequentialOutputStream::~LSequentialOutputStream ()
{
}

void LSequentialOutputStream::SetLength (SInt32 inLength)
{
	if (mPosition == 0 && inLength == 0)
		return;
	Throw_Err (writErr);
}

SInt32 LSequentialOutputStream::GetLength (void) const
{	
	return mPosition;
}

void LSequentialOutputStream::SetMarker( SInt32		inOffset,
								EStreamFrom		inFromWhere)
{
	if (inFromWhere == streamFrom_End && inOffset == 0)
		return;
		
	else if (inFromWhere == streamFrom_Start && inOffset == 0 && mPosition == 0)
		return;
		
	Throw_Err (writErr);
}

SInt32 LSequentialOutputStream::GetMarker (void) const
{
	return mPosition;
}

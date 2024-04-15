/*
	FileUtil.h
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
#include <Files.h>
#include <string>

bool	MakeNewFSSpec	(FSSpec& spec, short vRefNum, long parID, const std::string& name, bool saveSuffix);
OSErr	CheckFile		(const FSSpec &fs);
void	SplitName		(const std::string& name, std::string& head, std::string& suffix);
void	MakeNewName		(const std::string& rawName, const std::string& suffix, std::string& newName, UInt32 index, UInt32 maxLen);

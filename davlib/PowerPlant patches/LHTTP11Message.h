/*
	LHTTPMessage.h
	Copyright (C) 2000 i-drive.com (email: opensource@mail.idrive.com)

	This library is free software; you can redistribute it and/or
	modify it under the terms of version 2.1 of the the GNU Lesser General
	Public License as published by the Free Software Foundation.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

#include <LHTTPMessage.h>
#include <map>
#include <vector>

class LHTTP11Message : public LHTTPMessage
{
public:
					LHTTP11Message(void) : LHTTPMessage(), mPersistent (false) {}
					LHTTP11Message(const char * inMessage) : LHTTPMessage(inMessage), mPersistent (false) {}
	virtual 		~LHTTP11Message() {}
	std::string		*GetCookie (void) {return &mCookie;}
	void			SetCookie (std::string& cookie) {mCookie = cookie;}
	void			SetPersistent (bool persistent) {mPersistent = persistent;}

protected:	
	virtual void	BuildHeader (LDynamicBuffer * outHeader);
	
private:
	std::string											mCookie;
	bool												mPersistent;
};
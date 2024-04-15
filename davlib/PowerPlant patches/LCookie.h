/*
	LCookie.h
	Copyright (C) 2000 i-drive.com (email: opensource@mail.idrive.com)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

#include <string>
#include <list>
#include <time.h>

//-----------------------------------------------

class LCookie
{
public:

private:
	friend class LCookieList;
	
// Methods
	LCookie (const std::string& inCookie, const std::string& inDomain);
	LCookie (const char* inCookie , const char* inDomain);
	void Build (const std::string& inCookie, const std::string& inDomain);
	
	bool SetDate (const std::string& inStr);
	bool ValidateDomain (const std::string& cookieDomain, const std::string& hostDomain);
	
// Members
	std::string		mDomain;
	std::string		mName;
	std::string		mValue;
	std::string		mPath;
	time_t			mExpiration;		// Expiration time in seconds GMT since 1900
	bool			mValid;
	bool			mPermanent;
	bool			mSecure;
};


//-----------------------------------------------

class LCookieList
{
public:
	LCookieList (void);
	virtual ~LCookieList();
		
	void			Add (const char* inCookie, const char* inDomain);
	std::string		GetCookieString (const char* domain, const char* path, bool secure = false);

	virtual void	ReadCookies (void) {}	// Override to read cookies from disk
	virtual void	SaveCookies (void) {}	// Override to write cookies to disk
	
private:
	std::list<LCookie>	mCookieList;
};

/*
	LCookie.cpp
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

#include <LCookie.h>

#include <stdio.h>
#include <cerrno>
#include <cctype>

using namespace std;

// Spaces are allowed after the name and before the '=', but for now this will do
static const char kExpires[] = "expires=";
static const char kPath[] = "path=";
static const char kDomain[] = "domain=";
static const char kSecure[] = "secure=";

/*
	Partially follows the original cookie spec (www.netscape.com/newsref/std/cookie_spec.html).
	Does not implement RFC 2109 cookies.
*/

//------------------------------ LCookie ---------------------------------

LCookie::LCookie (const string& inCookie, const string& inDomain)
{
	Build (inCookie, inDomain);
}

LCookie::LCookie (const char* inCookie , const char* inDomain)
{
	Build ( string(inCookie), string (inDomain) );
}

void LCookie::Build (const string& inCookie, const string& hostDomain)
{
	long	fieldStart = 0;
	long	fieldEnd;
	bool	firstField = true;
	bool	lastField = false;
	long	valueStart;
	
	mValid = false;
	mPermanent = false;
	mExpiration = 0;
	mSecure = false;

	
	while (!lastField)
	{
		fieldEnd = inCookie.find (';', fieldStart);
		if (fieldEnd == string::npos)
		{
			fieldEnd = inCookie.size();
			lastField = true;
		}
		if (firstField)		// Cookie name and value
		{
			firstField = false;
			if ((valueStart = inCookie.find ('=', fieldStart)) != string::npos && valueStart < fieldEnd)
			{
				mName.assign (inCookie, fieldStart, valueStart - fieldStart);
				valueStart++;
				mValue.assign (inCookie, valueStart, fieldEnd - valueStart);
			}
			else
				return;		// Not valid
		}
		else
		{
							// "expires" tag
			if ((valueStart = inCookie.find (kExpires, fieldStart)) != string::npos && valueStart < fieldEnd)
			{
				valueStart += sizeof (kExpires) - 1;
				if ( SetDate (string (inCookie, valueStart, fieldEnd - valueStart) ) )
					mPermanent = true;	// Cookies with expiration dates are cached, and vice versa
			}
							// "path" tag
			else if ((valueStart = inCookie.find (kPath, fieldStart)) != string::npos && valueStart < fieldEnd)
			{
				valueStart += sizeof (kPath) - 1;
				mPath.assign (inCookie, valueStart, fieldEnd - valueStart );
				if (mPath == "/")
					mPath.clear();
			}
							// "domain" tag
			else if ((valueStart = inCookie.find (kDomain, fieldStart)) != string::npos && valueStart < fieldEnd)
			{
				valueStart += sizeof (kDomain) - 1;
				string cookieDomain (inCookie, valueStart, fieldEnd - valueStart);
				if (!ValidateDomain (cookieDomain, hostDomain))
					return;
				mDomain = cookieDomain;
			}
							// "secure" tag
			else if ((valueStart = inCookie.find (kSecure, fieldStart)) != string::npos && valueStart < fieldEnd)
			{
				mSecure = true;
			}
		}
		fieldStart = fieldEnd + 1;	// Go past next semicolon and spaces
		while (fieldStart < inCookie.size() && inCookie[fieldStart] == ' ')
			fieldStart++;
	}
	if (mDomain.size() == 0)
		mDomain = hostDomain;
	mValid = true;
}

bool LCookie::ValidateDomain (const string& cookieDomain, const string& hostDomain)
{
	int i, periodCount=0;
	static const char *kSpecialDomains[] = {".com",".edu",".net",".org",".gov",".mil",".int"};
	
// The cookie domain must match the end of the host domain
	if (hostDomain.size() < cookieDomain.size() || cookieDomain.size() <= 4)
		return false;
	if (hostDomain.compare (hostDomain.size() - cookieDomain.size(), cookieDomain.size(), cookieDomain) != 0)
		return false;
// Cookie domain must have at least 2 periods (for standard domains) or 3 (for others)
	for (long pos = 0, periodCount = 0; pos!=string::npos; pos = cookieDomain.find ('.', pos))
		periodCount++;
		
	if (periodCount >= 3)
		return true;
		
	else if (periodCount == 2)
	{
		for (i=0; i<7; i++)
			if (cookieDomain.find (kSpecialDomains[i], cookieDomain.size() - 4) != string::npos)
				return true;
	}
	return false;
}

bool LCookie::SetDate (const string& inStr)
{
	char				weekday [10],
						timezone [4],
						month [4];
	struct tm			time = {0};
	static const char*	kMonths[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	int					result;
	
	timezone [0] = 0;
	month [0] = 0;

	result = sscanf (inStr.c_str(), "%9s %d-%3s-%d %d:%d:%d %3s", weekday, &time.tm_mday, month, &time.tm_year,
																	&time.tm_hour, &time.tm_min, &time.tm_sec, timezone);
	if (errno || result != 8)
		return false;
		
	if (strncmp (timezone, "GMT", 3) != 0)		// Only GMT is allowed by Netscape spec
		return false;
		
	for (time.tm_mon = 0; time.tm_mon < 12; time.tm_mon ++)
		if (strcmp (month, kMonths[time.tm_mon]) == 0)
			break;
			
	if (time.tm_mon >= 12)
		return false;
		
	if (time.tm_year < 100)
		time.tm_year += 100;					// Two digit year, 2000-based
	else
		time.tm_year -= 1900;					// Four digit year
		
	mExpiration = mktime (&time);
	if (mExpiration == -1)
		return false;

	return true;
}

//------------------------- LCookieList ---------------------------
#ifdef macintosh
#pragma mark -
#endif

LCookieList::LCookieList (void)
{
	ReadCookies ();
}

LCookieList::~LCookieList ()
{
	SaveCookies ();
}

void LCookieList::Add (const char* inCookieStr, const char* inDomainStr)
{
	list<LCookie>::iterator start = mCookieList.begin(),
								end = mCookieList.end(),
								i;

	LCookie cookie (inCookieStr, inDomainStr);
	
	if (!cookie.mValid)
		return;

	for (i=start; i!= end; i++)
	{
		if (i->mDomain == cookie.mDomain && i->mName == cookie.mName)
		{
			*i = cookie;
			return;
		}
	}
	mCookieList.push_back (cookie);
}

string LCookieList::GetCookieString (const char* domain, const char* path, bool secure)
{
	list<LCookie>::iterator start = mCookieList.begin(),
								end = mCookieList.end(),
								i;
	time_t now = time (NULL);
	time_t gmt = mktime (gmtime (&now));
	string s;
	
	for (i=start; i!= end; i++)
	{
		if (!i->mValid)				// Bad cookie -- will be removed later
			continue;
		if (!secure && i->mSecure)	// Can't be sent on an insecure connection
			continue;
		if (strncmp (domain, i->mDomain.c_str(), i->mDomain.size() ) != 0)	// Does not match the current domain
			continue;
		if (i->mPath.size() && strncmp (path, i->mPath.c_str(), i->mPath.size() ) != 0)	// Check partial path
			continue;
		if (i->mExpiration && gmt > i->mExpiration)	// Expired -- mark for removal
		{
			i->mValid = false;
			continue;
		}
		if (s.size())
			s += "; ";
		s += i->mName;
		s += '=';
		s += i->mValue;
	}
	return s;
}

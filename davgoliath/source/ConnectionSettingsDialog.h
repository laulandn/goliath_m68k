/* ==================================================================================================
 * ConnectionSettingsDialog.h															   
 *    Goliath - a Finder like application that implements WebDAV
 *    Copyright (C) 1999-2002  Thomas Bednarz
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

#ifndef __ConnectionSettingsDialog_h__
#define __ConnectionSettingsDialog_h__

#pragma once

#include <string>

class DisplaySettingsData {

	public:
		std::string 	mHost;
		SInt16 			mPort;
		bool			mForceSecure;
		std::string     mPath;
		
		std::string 	mUserName;
		std::string 	mPassWord;
		bool 			mUseProxy;
		std::string 	mProxyServer;
		SInt16 			mProxyPort;
		bool 			mUseProxyAuth;
		std::string		mProxyUser;
		std::string		mProxyPass;
		
		bool			mDisableAppleDoubleEncoding;

};

extern const char* sHTTPProtocol;
extern const char* sHTTPSProtocol;
extern const SInt16 sHTTPPort;
extern const SInt16 sHTTPSPort;

bool DisplayConnectionSettingsDialog(DisplaySettingsData& inOutData, bool inEditing=false);

bool DisplayIDiskSettingsDialog(DisplaySettingsData& inOutData);

#endif
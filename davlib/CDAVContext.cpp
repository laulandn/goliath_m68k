/* ===========================================================================
 *	CDAVContext.cpp			   
 *
 *  This file is part of the DAVLib package
 *  Copyright (C) 1999-2000  Thomas Bednarzär
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
 */
 #include "CDAVContext.h"
	
// ---------------------------------------------------------------------------
//		¥ CDAVContext()
// ---------------------------------------------------------------------------
//	Constructor
CDAVContext::CDAVContext(const char *server, SInt32 port, Boolean forceSecure):
											m_server(server),
											m_port(port),
											m_useProxy(false),
											m_proxyServer(""),
											m_proxyPort(0),
											m_hasUserCred(false),
											mHasProxyCred(false),
											mForceSecure(forceSecure) {
}

// ---------------------------------------------------------------------------
//		¥ CDAVContext()
// ---------------------------------------------------------------------------
//	Constructor
CDAVContext::CDAVContext(const CDAVContext &rhs) {
   m_server=rhs.m_server;
   m_port=rhs.m_port;
   m_useProxy=rhs.m_useProxy;
   m_proxyServer=rhs.m_proxyServer;
   m_proxyPort=rhs.m_proxyPort;
   m_hasUserCred=rhs.m_hasUserCred;
   m_login=rhs.m_login;
   m_password=rhs.m_password;
   m_cookieList=rhs.m_cookieList;
   mHasProxyCred = rhs.mHasProxyCred;
   mProxyPassword = rhs.mProxyPassword;
   mProxyUserName = rhs.mProxyUserName;
   mForceSecure = rhs.mForceSecure;
}

// ---------------------------------------------------------------------------
//		¥ ~CDAVContext()
// ---------------------------------------------------------------------------
//	Destructor
CDAVContext::~CDAVContext() {
}


// ---------------------------------------------------------------------------
//		¥ SetProxyServer()
// ---------------------------------------------------------------------------
//
void CDAVContext::SetProxyServer(const char *proxyServer, SInt32 proxyPort) {
	m_useProxy = true;
	m_proxyServer = proxyServer;
	m_proxyPort = proxyPort;
}

// ---------------------------------------------------------------------------
//		¥ SetProxyServer()
// ---------------------------------------------------------------------------
//
void CDAVContext::SetProxyServer(const std::string &proxyServer, SInt32 proxyPort) {
	m_useProxy = true;
	m_proxyServer = proxyServer;
	m_proxyPort = proxyPort;
}

// ---------------------------------------------------------------------------
//		¥ SetUserCredentials()
// ---------------------------------------------------------------------------
//
void CDAVContext::SetUserCredentials(const char *login, const char *password) {
	m_hasUserCred = true;
	m_login = login;
	m_password = password;
}

// ---------------------------------------------------------------------------
//		¥ SetUserCredentials()
// ---------------------------------------------------------------------------
//
void CDAVContext::SetUserCredentials(ConstStr255Param login, ConstStr255Param password) {
   m_hasUserCred = true;
	m_login.assign ((const char*) login+1, login[0]);
	m_password.assign ((const char*) password+1, password[0]);
}

// ---------------------------------------------------------------------------
//		¥ SetUserCredentials()
// ---------------------------------------------------------------------------
//
void CDAVContext::SetUserCredentials(const std::string& cookie) {
	if (m_cookieList)
		m_cookieList->Add (cookie.c_str(), m_server.c_str());
}

// ---------------------------------------------------------------------------
//		¥ SetProxyCredentials()
// ---------------------------------------------------------------------------
//
void CDAVContext::SetProxyCredentials(const char *login, const char *password) {
   mHasProxyCred = true;
   mProxyUserName = login;
   mProxyPassword = password;
}

// ---------------------------------------------------------------------------
//		¥ equals()
// ---------------------------------------------------------------------------
//
Boolean CDAVContext::equals(CDAVContext &rhs) {
   return ((m_server == rhs.m_server) &&
          (m_port == rhs.m_port) &&
          (m_useProxy == rhs.m_useProxy) &&
          (m_proxyServer == rhs.m_proxyServer) &&
          (m_proxyPort == rhs.m_proxyPort) && 
          (m_hasUserCred == rhs.m_hasUserCred) &&
          (m_login == rhs.m_login) &&
          (m_password == rhs.m_password) &&
          (mForceSecure == rhs.mForceSecure));
}

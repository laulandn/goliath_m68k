/* ===========================================================================
 *	CDAVContext.h			   
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
 
/*!
   @header DAVLib
	This library implements HTTP 1.1 and the WebDAV (RFC 2518) extensions to HTTP.
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#ifndef __CDAVCONTEXT_H__
#define __CDAVCONTEXT_H__

#pragma once

#include <MacTypes.h>
#include <LCookie.h>
#include <string>
#include <LString.h>

/*!
  @class CDAVContext
  @discussion A class which contains all the information required to contact a DAV server.
   	Used in conjunction with CDAVRequest.
**/
class CDAVContext {

   public:
      /*!
	     @function CDAVContext
		  @discussion object constructor
	     @param srvr the host name or IP address of the web server
		  @param port the port to use in communicating with the server
      */
      CDAVContext(const char * srvr, SInt32 port, Boolean forceSecure = false);
      /*!
	     @function CDAVContect
	     @discussion copy constructor
		  @param rhs
      */
      CDAVContext(const CDAVContext &rhs);
      /*!
	     @function ~CDAVContext
	     @param object destructor
      */
      virtual ~CDAVContext();
      
      /*!
	     @function SetProxyServer
		  @discussion set a proxy server for use in this connection
	     @param srvr the host name or IP address of the proxy server
		  @param port the port to use in communicating with the proxy server
      */
		void SetProxyServer(const char *proxyServer, SInt32 proxyPort);
      /*!
	     @function SetProxyServer
		  @discussion set a proxy server for use in this connection
	     @param srvr the host name or IP address of the proxy server
		  @param port the port to use in communicating with the proxy server
      */
		void SetProxyServer(const std::string& proxyServer, SInt32 proxyPort);
      
      /*
         basic http authentication
      */
      /*!
	     @function SetUserCredentials
		  @param set an optional login and password for this connection
	     @param login plain text user login
		  @param password plain text user password
      */
		virtual void SetUserCredentials(const char * login, const char * password);
      /*!
	     @function SetUserCredentials
		  @param set an optional login and password for this connection
	     @param login plain text user login
		  @param password plain text user password
      */
      virtual void SetUserCredentials(ConstStr255Param login, ConstStr255Param password);
      /*!
	     @function SetUserCredentials
		  @param set an optional login and password for this connection
	     @param login plain text user login
		  @param password plain text user password
      */
		virtual void SetUserCredentials(const std::string& cookie);
   
      /*!
	     @function GetServerName
	     @discussion access the host name or IP address of the web server
      */
		const std::string& GetServerName() const {return m_server;}
      /*!
	     @function GetPort
		  @discussion access the port number to use in contacting the web server
      */
		SInt32 GetPort() {return m_port;}
      
      /*!
	     @function GetUsesProxy
	     @discussion get whether a proxy server needs to be used in this connection
      */
		Boolean GetUsesProxy() {return m_useProxy;}
      
      /*!
	     @function GetLogin
	     @discussion accesses the user login 
      */
		const char* GetLogin() {return m_login.c_str();}
      /*!
	     @function GetPassword
	     @discussion accesses the users password
      */
		const char* GetPassword() {return m_password.c_str();}
      /*!
	     @function GetCookieList
	     @discussion returns pointer to the list of currently active cookies 
      */
		LCookieList *GetCookieList() {return m_cookieList;}
      /*!
	     @function SetCookieList
	     @discussion sets the active cookie list
      */
		void SetCookieList (LCookieList *cookieList) {m_cookieList = cookieList;}
      
      /*!
	     @function GetHasUserCredentials
	     @discussion returns true if a login and password are set for this connection
      */
		Boolean GetHasUserCredentials() {return m_hasUserCred;}
      /*!
	     @function GetUsesProxy
	     @param returns true if a proxy server is set for this connection
      */
		Boolean GetUseProxy() {return m_useProxy;}
      
      /*!
	     @function GetProxyServer
		  @discussion get the host name or IP address of the proxy server
      */
		void GetProxyServer(PP_STD::string &proxySrvr) {proxySrvr = m_proxyServer;}
      /*!
	     @function GetProxyPort
	     @discussion get the port for communicating with the proxy server
      */
		SInt32 GetProxyPort() {return	 m_proxyPort;}
      /*!
	     @function SetProxyCredentials
	     @discussion set an optional login and password for the proxy server
	     @param login plain text user login
		  @param password plain text user password
      */
      void SetProxyCredentials(const char * login, const char * password);
      /*!
	     @function GetProxyLogin
	     @discussion access the clear text login for the proxy server
      */
      const char* GetProxyLogin() {return mProxyUserName.c_str();};
      /*!
	     @function GetProxyPassword
	     @discussion access the clear text password for the proxy server.
      */
      const char* GetProxyPassword() {return mProxyPassword.c_str();};
      /*!
	     @function GetHasProxyCredentials
	     @discussion get whether a credentials are required for the proxy server 
      */
      Boolean GetHasProxyCredentials() {return mHasProxyCred;};

      /*!
	     @function equals		  
		  @result returns true if the 2 objects are equal
	     @param rhs the object to compare with this object
      */
      Boolean equals(CDAVContext &rhs);

      /*!
	     @function GetForceSecure		  
		  @result returns true if the server connection is set to always
		     be secure (regardless of port)
      */
      Boolean GetForceSecure() {return mForceSecure;};
   protected:
      //the WebDAV server
		std::string		m_server;
      //port to use in contacting WebDAV server
		SInt32			m_port;
      //if true, optionally use a proxy server
		Boolean		  	m_useProxy;
      //if using a proxy server, the name or IP address of the proxy server
		PP_STD::string	m_proxyServer;
      //if using a proxy server, the port of the proxy server      
		SInt32			m_proxyPort;
      //if user and password have been set
		Boolean			m_hasUserCred;
      // set username
		std::string		m_login;
      // set password
		std::string		m_password;	
		// active cookie list
		LCookieList		*m_cookieList;
      
		Boolean         mHasProxyCred;
		std::string     mProxyPassword;
		std::string     mProxyUserName;
      
		Boolean         mForceSecure;
 };

#endif

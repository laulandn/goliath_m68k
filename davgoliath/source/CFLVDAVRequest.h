/* ==================================================================================================
 * CFLVDAVRequest.cp															   
 *    Goliath - a Finder like application that implements WebDAV
 *    Copyright (C) 1999-2001  Thomas Bednarz
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
#pragma once
#ifndef __CFLVDAVREQUEST_H__
#define __CFLVDAVREQUEST_H__
#include <CDAVRequest.h>


class CFLVDAVRequest : public CDAVRequest {
   public:
      //ctors & dtor
      /*
         Constructs a CDAVRequest to operate against a particular DAV server.
         srvr - the server to connect; either a host name or IP address
         port - the port which the DAV server is bound.
      */
      CFLVDAVRequest(CDAVContext *ctx);
	  CFLVDAVRequest(LThread & inThread, CDAVContext *ctx);
      
      CFLVDAVRequest(CFLVDAVRequest &rhs);
      
      /*
         Destructor
      */
      virtual ~CFLVDAVRequest();
   
      void SetSuppressErrorDialog(Boolean supp) {mSuppressDlog = supp;};
   protected:
      // a virtual function to handle any error reporting or logging
	  virtual void _OnDavRequestError(LHTTPResponse &theResponse, const char* displayString=NULL);   
      virtual void _HandleSSLexception(LHTTPResponse &theResponse, LSSLException& e);
      
      class CDAVTableApp  *mApp;
      
      virtual void _OnDavItemCreated(CDAVItem& theItem);
      virtual void _OnDavItemDataChange(CDAVItem& theItem);
      
      Boolean mSuppressDlog;
      
};
#endif


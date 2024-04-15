/* ===========================================================================
 *	CDAVInfo.h		   
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
 * ===========================================================================
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

#pragma once
#ifndef __CDAVINFO_H__
#define __CDAVINFO_H__

#include <string>
#include <MacTypes.h>

/*!
  @class CDAVInfo
  This class is used to examine information returned from a web server
  and ascertain what functionality is available.
 */
class CDAVInfo {
   public:

  /*!
	 @enum ProtocolOperations
	 @constant noOperations no recognized operations returned from the server.  These values 
	 are ANDed together in a bitfield.
	 @constant HTTP_Get the server implements GET
	 @constant HTTP_Head the server implements HEAD
	 @constant HTTP_Post the server implements POST
	 @constant HTTP_Put  the server implements PUT
	 @constant HTTP_Options the server implements OPTIONS
	 @constant DAV_PropFind the server implements PROPFIND
	 @constant DAV_Mkcol the server implements MKCOL
	 @constant DAV_Delete the server implements DELETE
	 @constant DAV_Copy the server implements COPY
	 @constant DAV_Move the server implements MOVE
	 @constant DAV_PropPatch the server implements PROPPATCH
	 @constant DAV_Lock the server implements LOCK
	 @constant DAV_Unlock the server implements UNLOCK

  */
      enum ProtocolOperations { 
         noOperations        = 0,
         HTTP_Get            = 1<<0,
         HTTP_Head           = 1<<1,
         HTTP_Post           = 1<<2,
         HTTP_Put            = 1<<3,
         HTTP_Options        = 1<<4,
         DAV_PropFind        = 1<<5,
         DAV_Mkcol           = 1<<6,
         DAV_Delete          = 1<<7,
         DAV_Copy            = 1<<8,
         DAV_Move            = 1<<9,
         DAV_PropPatch       = 1<<10,
         DAV_Lock            = 1<<11,
         DAV_Unlock          = 1<<12
      };
	
      /*!
	     @function CDAVInfo
	     @discussion object constructor
      */
      CDAVInfo();
      /*!
	     @function CDAVInfo
	     @discussion object constructor
      */
      CDAVInfo(CDAVInfo &rhs);
      /*!
	      @function ~CDAVInfo
	      @discussion object destructor
      */
      virtual ~CDAVInfo();
      
      /*!
		  @function getSupportedOperations
		  @discussion returns all supported operations as specified by 
		  ProtocolOperations ANDed together
		 */
      SInt32 getSupportedOperations();
      /*!
		  @function setSupportedOperations
		  @discussion set all supported operations as specified by 
		  ProtocolOperations ANDed together
		 */
      void   setSupportedOperations(SInt32 ops);
      
      /*!
		  @function setMSHeaderFields
		  @discussion sets the contents of any Microsoft Authoring HTTP headers
		 */
      void setMSHeaderFields(std::string &msHdrs);
      /*!
		  @function getHadMSHeaderFields
		  @discussion returns whether the server returned any Microsoft specific
		  HTTP headers
		 */
      Boolean getHasMSHeaderFields();
      /*!
		  @function getMSHeaderFields
		  @discussion returns the content of any server returned any Microsoft specific
		  HTTP headers
		 */
      std::string& getMSHeaderFields();
      
      /*!
		  @function getDavClassSupport
		  @discussion returns the DAV server class 
		 */
      std::string& getDavClassSupport();
      /*!
		  @function setDavClassSupport
		  @discussion sets the DAV server class
		 */
      void setDavClassSupport(std::string& vl);
      
      /*!
		  @function getServerType
		  @discussion returns the server type (ie, Apache 1.3.9/mod_dav)
		 */
      std::string& getServerType();
      /*!
		  @function setServerType 
		  @discussion sets the server type
		 */
      void setServerType(std::string &st);
      
      /*!
         @function getHasExecutableSupport
         @discussion mod_dav 1.0 introduced a custom property to allow users to
             set the executable bit on a Unix filesystem.  The presence of this
             property is signaled with in the HTTP header DAV: by an opaque string.
             This function returns true if this function is available.
      */
      Boolean getHasExecutableSupport();
      
      /*!
		  @function =
		  @discussion assignment operator
		 */
      CDAVInfo&  operator = (CDAVInfo& rhs);
				

   protected:
      SInt32       mProtocolOps;
      
      std::string  mDAVLevel;
      
      std::string  mServerType;
      
      Boolean      mHasMSAuthorHdr;
      std::string  mMSAuthorVia;
      
      Boolean      mModDavExecutable;
      
};


#endif

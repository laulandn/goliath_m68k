/* ==================================================================================================
 * DataTransfer.h															   
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
 
 #ifndef __DATATRANSFER_H__
 #define __DATATRANSFER_H__

#pragma once

#include <LDragAndDrop.h>
#include <vector>
#include <CDAVItem.h>
#include <xmlparse.h>
#include <list>

class CDAVContext;
const UInt32 cDAVItemType = 'DAV1';	// Usable as a FlavorType or an OSType

class  DAVItemData {
public:

   std::string  server;
   SInt32  port;
   
   Boolean hasProxy;
   std::string  proxyserver;
   SInt32  proxyport;

   Boolean hasAuth;
   std::string  login;
   std::string  password;

   Boolean hasProxyAuth;
   std::string  proxylogin;
   std::string  proxypassword;

   Boolean isCut;
   
   std::list<std::string>  sourceURIs;

   DAVItemData();

   static XML_Error ParseDAVItemClip(Ptr inClipData, SInt32 clipSize, DAVItemData *outTheData);

   static void DAVItemsToClip(CDAVItemVector &inVector, CDAVContext *inContext, bool inIsCut, std::string& outClip);
   
};




#endif

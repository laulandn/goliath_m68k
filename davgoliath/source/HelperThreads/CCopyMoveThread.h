/* ==================================================================================================
 * CCopyMoveThread.h															   
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

#ifndef __CCOPYMOVETHREAD_H__
#define __CCOPYMOVETHREAD_H__

#pragma once
#include <CFLVThread.h>
#include "DataTransfer.h"
#include <string>

class CCopyMoveThread :  public CFLVThread {
   
   public:

      
      CCopyMoveThread(CDAVContext *ctx, CDAVTableWindow *wnd,  
                      std::string& baseResource, DAVItemData*  cbData);
			
	  virtual void cancelOperation(); 
	  
   protected:
     //***teb - fix this to use new std::string members
     
	 virtual void*		_executeDAVTransaction();
	         Boolean    _executeLocalResource(std::string *resource);

	 Boolean            mCancel;
	 std::string        mBaseResource;	 
	 
	 std::string  mServer;
     SInt32   mPort;
   
     Boolean  mHasProxy;
     std::string  mProxyServer;
     SInt32   mProxyPort;

     Boolean  mHasAuth;
     std::string  mLogin;
     std::string  mPassword;

     Boolean  mHasProxyAuth;
     std::string  mProxyLogin;
     std::string  mProxyPassword;

     Boolean  mIsCut;
     std::list<std::string> mResources;
     
};

#endif


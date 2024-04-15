/* ==================================================================================================
 * CUploadFileThread.h															   
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

#ifndef __CUPLOADFILETHREAD_H__
#define __CUPLOADFILETHREAD_H__

#pragma once
#include <CFLVThread.h>
#include <vector.h>

typedef vector<FSSpec> FSSpecVector;

class CUploadFileThread :  public CFLVThread {
   
   public:

      
      CUploadFileThread(CDAVContext *ctx, CDAVTableWindow *wnd, std::string &resource, FSSpec &theSpec);
      CUploadFileThread(CDAVContext *ctx, CDAVTableWindow *wnd, std::string &resource, FSSpecVector &theSpecs);
			
	  virtual void cancelOperation(); 
	  
	  void SetParentOutlineItem(CDAVTableItem* item) {mParentItem = item;};
	  void SetIsResourceFullyQualified(Boolean fq);
	  void SetLockToken(std::string& locktoken, Boolean isLocal=false, Boolean unlockItem=false);
   protected:
     std::string    mResource;
     FSSpec         mSpec;
     FSSpecVector   mSpecVector;
     Boolean        mCancel;
     
     Boolean        mResourceFullyQualfied;
     Boolean        mHasLockToken;
     std::string    mLockToken;
     Boolean        mHasLocalLock;
     Boolean        mUnlockItem;
     
     CDAVTableItem* mParentItem;
     UInt32         mTotalItemsToUpload;

     bool           mResourceIsBase;
     
	 virtual void*		_executeDAVTransaction();
	 
	 class CClientLockManager *mLockMgr;
	 
	 virtual Boolean    _uploadFSSpec(FSSpec *theSpec, const std::string& inResource, Boolean message,
	                                  class CProgressDialog* progwin);
     void _itemAdded(CDAVItem &item,  Boolean newResource);
     Boolean _doAlert();

     Boolean _ProcessFolder(FSSpec* theSpec, const std::string& inResource, Boolean message, CProgressDialog* progwin);
     Boolean _ProcessFile(FSSpec* theSpec, const std::string& inResource, Boolean message, CProgressDialog* progwin);

};


#endif


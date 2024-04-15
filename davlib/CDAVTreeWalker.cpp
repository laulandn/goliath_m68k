/* ===========================================================================
 *	CDAVTreeWalker.cpp		   
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
 
 #include "CDAVTreeWalker.h"
// #include "CDAVItemUtils.h"
  
// ---------------------------------------------------------------------------
//		¥ CDAVTreeWalker()
// ---------------------------------------------------------------------------
//	
 CDAVTreeWalker::CDAVTreeWalker(LThread& thread, CDAVRequest *req, bool dotFilesAreHidden):
    mThread(thread), mRequest(req), mTotalItems(0), mHideDotFiles(dotFilesAreHidden) {
    mProperties.push_back(ResourceType);
    
 }
 
 
// ---------------------------------------------------------------------------
//		¥ ~CDAVTreeWalker()
// ---------------------------------------------------------------------------
//	
CDAVTreeWalker::~CDAVTreeWalker() {
 
}

 
// ---------------------------------------------------------------------------
//		¥ walkTree()
// ---------------------------------------------------------------------------
//	
Boolean CDAVTreeWalker::WalkTree(const char* uri) {
   return _walkTree(uri);
   
}

// ---------------------------------------------------------------------------
//		¥ _walkTree()
// ---------------------------------------------------------------------------
//	
Boolean CDAVTreeWalker::_walkTree(const char* theResource) {
   CDAVItemVector tmp, children;
   CDAVItemVector::iterator iter;
   
   CDAVRequest::ReqStatus stat = mRequest->ListDirectory(mThread, theResource, tmp, mProperties);
   for (CDAVItemVector::iterator iter = tmp.begin(); iter != tmp.end(); ++iter) {
      if (mHideDotFiles) {
         std::string fname;
         iter->GetFileName(fname);
      	 if (fname.size() > 0 && fname[0]!= '.')
      	    children.push_back(*iter);
      } else {
         children.push_back(*iter);
      }
   }

   if ((CDAVRequest::SUCCESS != stat) && (CDAVRequest::WARNING != stat))
      return false;
   
   for (iter = children.begin(); iter!= children.end(); iter++) {
      CDAVItem *theItem = &*iter;
      _onItemAccess(theItem);
      mTotalItems++;
      if (iter->GetItemType() == CDAVItem::COLLECTION) {
         if (!_walkTree(iter->GetHREF().c_str()))
            return false;
      }
   }
   
   return true;
}


// ---------------------------------------------------------------------------
//		¥ walkTree()
// ---------------------------------------------------------------------------
//	
void CDAVTreeWalker::_onItemAccess(CDAVItem* theItem) {
   #pragma unused(theItem)
}

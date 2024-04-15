/* ===========================================================================
 *	CDAVTreeWalker.h			   
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
#ifndef __CDAVTREEWALKER_H__
#define __CDAVTREEWALKER_H__

#include "CDAVRequest.h"
#include "CDAVProperty.h"

class CDAVTreeWalker {

   public:
      CDAVTreeWalker(LThread &thread, CDAVRequest *req, bool dotFilesAreHidden);
      virtual ~CDAVTreeWalker();
      
      Boolean WalkTree(const char* uri);
      
      UInt32 GetTotalItems() {return mTotalItems;};
   protected:
      Boolean _walkTree(const char* theResource);

      void _onItemAccess(CDAVItem* theItem);
      
      
      LThread            &mThread;
      UInt32              mTotalItems;
      CDAVRequest        *mRequest;
      CDAVPropertyVector  mProperties;
      bool                mHideDotFiles;
};

#endif
/* ==================================================================================================
 * CUnlockItemsThread.h															   
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

#ifndef __CUNLOCKITEMSTHREAD_H__
#define __CUNLOCKITEMSTHREAD_H__

#pragma once
#include <CFLVThread.h>

class CUnlockItemsThread :  public CFLVThread {
   
   public:

      
      CUnlockItemsThread(CDAVContext *ctx, CDAVTableWindow *wnd, CDAVItemVector& items);
			
   protected:	 
     CDAVItemVector   mItemVector;

	 virtual void*		_executeDAVTransaction();
	 void _unlockResource(CDAVItem* theItem);
	 class CClientLockManager* mLockMgr;
};

#endif

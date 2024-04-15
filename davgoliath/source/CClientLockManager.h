/* ==================================================================================================
 * CClientLockManager.h															   
 *    Goliath - a Finder like application that implements WebDAV
 *    Copyright (C) 1999-2002  Thomas Bednarz
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
 
#ifndef __CCLIENTLOCKMANAGER_H__
#define __CCLIENTLOCKMANAGER_H__

#include <CGoliathPreferences.h>
#include <LMutexSemaphore.h>

/**
   Encapsulates the CGoliathPreferences for persistent storage
**/
class  CClientLockManager: protected CGoliathPreferences {
   public:
      CClientLockManager();
      virtual ~CClientLockManager();  
      virtual Boolean Init();
      
      void ItemLocked(class CDAVContext *ctx, class CDAVItem *item);
      void ItemUnlocked(class CDAVContext *ctx, class CDAVItem *item);
      void ItemUnlocked(class CDAVContext *ctx, const char* uri);
      Boolean GetLockToken(class CDAVContext *ctx, class CDAVItem *item,
                           std::string &outLockToken);
      Boolean GetLockToken(class CDAVContext *ctx, const char* uri, std::string &outLockToken);
      
   protected:
      virtual Boolean _populatePrefs(Boolean useDefaults);
      std::string _GenerateKey(CDAVContext *ctx, const char* uri);

      LMutexSemaphore fAccess;

      
};

#endif


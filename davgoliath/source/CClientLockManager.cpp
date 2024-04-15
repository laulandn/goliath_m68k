/* ==================================================================================================
 * CClientLockManager.cpp									   
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

#include "CClientLockManager.h"
#include <CDAVTableAppConstants.h>
#include <CDAVContext.h>
#include <CDAVItem.h>
#include <UThread.h>
#include <string.h>

// ---------------------------------------------------------------------------------
//		¥ CClientLockManager()
// ---------------------------------------------------------------------------------
//
CClientLockManager::CClientLockManager(): CGoliathPreferences(LStr255(str_FileNameStrings, str_GoliathLockDatabase)) {

}


// ---------------------------------------------------------------------------------
//		¥ ~CClientLockManager()
// ---------------------------------------------------------------------------------
//
CClientLockManager::~CClientLockManager() {

}

// ---------------------------------------------------------------------------------
//		¥ Init()
// ---------------------------------------------------------------------------------
//
Boolean CClientLockManager::Init() {
   return CGoliathPreferences::Init();
}

// ---------------------------------------------------------------------------------
//		¥ ItemLocked()
// ---------------------------------------------------------------------------------
//
void CClientLockManager::ItemLocked(class CDAVContext *ctx, class CDAVItem *item) {
   StMutex mutex(fAccess);
   std::string key = _GenerateKey(ctx, item->GetURI().c_str());
   /*std::string key = ctx->GetServerName();
   key.append(item->GetURI());*/
   SetPrefValue(key.c_str(), item->GetLockToken().c_str());
   CommitPreferences();
}



// ---------------------------------------------------------------------------------
//		¥ ItemUnlocked()
// ---------------------------------------------------------------------------------
//
void CClientLockManager::ItemUnlocked(class CDAVContext *ctx, class CDAVItem *item) {
   ItemUnlocked(ctx, item->GetURI().c_str());
}

// ---------------------------------------------------------------------------------
//		¥ ItemUnlocked()
// ---------------------------------------------------------------------------------
//
void CClientLockManager::ItemUnlocked(class CDAVContext *ctx, const char* uri) {
   StMutex mutex(fAccess);
   std::string key = _GenerateKey(ctx, uri);
   RemovePref(key.c_str());
   CommitPreferences();
}

// ---------------------------------------------------------------------------------
//		¥ CPreferencesManager::GetLockToken()
// ---------------------------------------------------------------------------------
//
Boolean CClientLockManager::GetLockToken(class CDAVContext *ctx, class CDAVItem *item, std::string &outLockToken) {
   return GetLockToken(ctx, item->GetURI().c_str(), outLockToken);
}

// ---------------------------------------------------------------------------------
//		¥ _populatePrefs()
// ---------------------------------------------------------------------------------
//
Boolean CClientLockManager::GetLockToken(CDAVContext *ctx, const char* uri, std::string &outLockToken) {
   StMutex mutex(fAccess);
   /*std::string key = ctx->GetServerName();
   key.append(uri);*/
   std::string key = _GenerateKey(ctx, uri);
   PreferencesMap::iterator iter = mPrefVals.begin();
   while (iter != mPrefVals.end()) {
      if (strcmp((*iter).first.c_str(), key.c_str())==0) {
         outLockToken = (*iter).second;
         return true;
      }
      iter++;
   }

   return false;
}

      
// ---------------------------------------------------------------------------------
//		¥ _populatePrefs()
// ---------------------------------------------------------------------------------
//
Boolean CClientLockManager::_populatePrefs(Boolean useDefaults) {
   if (!useDefaults)
      return _loadFromPrefFile();
   return true;
}

// ---------------------------------------------------------------------------------
//		¥ _GenerateKey()
// ---------------------------------------------------------------------------------
//      
std::string CClientLockManager::_GenerateKey(CDAVContext *ctx, const char* uri) {
   std::string key = ctx->GetServerName();
   key.append(uri);
   //if a collection and has a trailing delimiter, strip it to normalize URI
   if (key[key.size()-1]=='/') {
      key.erase(key.size()-1);
   }
   return key;
}

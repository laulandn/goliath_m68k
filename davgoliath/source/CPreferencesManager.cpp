/* ==================================================================================================
 * CPreferencesManager.cpp															   
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
 

#include <CPreferencesManager.h>
#include <string.h>
#include <Folders.h>
#include <string>

// ---------------------------------------------------------------------------------
//		¥ CPreferencesManager()
// ---------------------------------------------------------------------------------
//
CPreferencesManager::CPreferencesManager(LStr255& prefsFileName): mInit(false),
mUseDefaults(true) {
   OSErr  err;
   short  vRefNum;
   long   dirID;
   Boolean useDefaults=true;
   
   //locate our preferences file; if it doesn't exist, create it
   err = ::FindFolder( kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
                       &vRefNum, &dirID);
                       
   if (err != noErr) 
	   return;   
   
   mPrefsFile.vRefNum = vRefNum;
   mPrefsFile.parID   = dirID;
   LString::CopyPStr(prefsFileName, mPrefsFile.name, sizeof(StrFileName));

   ::FSpCreateResFile(&mPrefsFile, '????', 'pref', (ScriptCode)nil );
   err = ResError();
   if ( err == dupFNErr ) {
      mUseDefaults=false;
      mInit = true;
   } else {
      mInit = true;
   }   
}

// ---------------------------------------------------------------------------------
//		¥ Init()
// ---------------------------------------------------------------------------------
//
Boolean CPreferencesManager::Init() {
   if (mInit) {
      mInit = _populatePrefs(mUseDefaults);
   } 
   return mInit;
}

// ---------------------------------------------------------------------------------
//		¥ ~CPreferencesManager()
// ---------------------------------------------------------------------------------
//
CPreferencesManager::~CPreferencesManager() {
}

// ---------------------------------------------------------------------------------
//		¥ GetPrefValue()
// ---------------------------------------------------------------------------------
//
const char* CPreferencesManager::GetPrefValue(const char* key) {
   return mPrefVals[std::string(key)].c_str();
}

// ---------------------------------------------------------------------------------
//		¥ GetPrefValue()
// ---------------------------------------------------------------------------------
//
const char* CPreferencesManager::GetPrefValue(std::string &key) {
   return mPrefVals[key].c_str();
}

// ---------------------------------------------------------------------------------
//		¥ SetPrefValue()
// ---------------------------------------------------------------------------------
//
void CPreferencesManager::SetPrefValue(const char* key, const char* value) {
   mPrefVals[std::string(key)] = std::string(value);
}

// ---------------------------------------------------------------------------------
//		¥ SetPrefValue()
// ---------------------------------------------------------------------------------
//
void CPreferencesManager::SetPrefValue(std::string &key, const char* value) {
   mPrefVals[key] = std::string(value);
}


// ---------------------------------------------------------------------------------
//		¥ CPreferencesManager::RemovePref()
// ---------------------------------------------------------------------------------
//
void CPreferencesManager::RemovePref(const char* key) {
    PreferencesMap::iterator iter = mPrefVals.begin();
    while (iter != mPrefVals.end()) {
       if (strcmp((*iter).first.c_str(), key)==0) {
          mPrefVals.erase(key);
          return;
       }
       iter++;
    }
    
}


// ---------------------------------------------------------------------------------
//		¥ ClearPreferences()
// ---------------------------------------------------------------------------------
//
Boolean CPreferencesManager::ClearPreferences() {
   mPrefVals.clear();
   return CommitPreferences();
}

// ---------------------------------------------------------------------------------
//		¥ _populatePrefs()
// ---------------------------------------------------------------------------------
//
Boolean CPreferencesManager::_populatePrefs(Boolean) {
   return false;
}


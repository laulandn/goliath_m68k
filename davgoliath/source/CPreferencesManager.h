/* ==================================================================================================
 * CPreferencesManager.h															   
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
 
#ifndef __CPREFERENCESMANAGER_H__
#define __CPREFERENCESMANAGER_H__

#include <map>
#include <LBroadcaster.h>
#include <LString.h>

typedef std::map<std::string, std::string> PreferencesMap;
/**
   Generic key-value associative references storage mechanism.  Does not implement 
   persistence; hence the pure virtual functions.   
**/
class CPreferencesManager : public LBroadcaster {
   public:
      CPreferencesManager(LStr255& prefsFileName);
      virtual ~CPreferencesManager();  
      virtual Boolean Init();
      
      const char* GetPrefValue(const char* key);
      const char* GetPrefValue(std::string &key);
      
      void SetPrefValue(const char* key, const char* value);
      void SetPrefValue(std::string &key, const char* value);
      
      void RemovePref(const char* key);
      
      Boolean ClearPreferences();
      virtual Boolean CommitPreferences()=0;
      
   protected:
      virtual Boolean _loadFromPrefFile()=0;
      virtual Boolean _saveToPrefFile()=0;
      
      virtual Boolean _populatePrefs(Boolean useDefaults);
      
   protected:
      Boolean        mInit;        //class has been initialized and ready for use
      FSSpec         mPrefsFile;   //FSSpec for persistent storage
      PreferencesMap mPrefVals;    //actual name value pairs
      Boolean        mUseDefaults;
};

#endif


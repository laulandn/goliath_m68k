/* ==================================================================================================
 * CGoliathPreferences.h															   
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
 
#ifndef __CGOLIATHPREFERENCES_H__
#define __CGOLIATHPREFERENCES_H__

#include <CPreferencesManager.h>


/**
   Generic key-value associative references storage mechanism.  Does not implement 
   persistence; hence the pure virtual functions.   
**/
class CGoliathPreferences : public CPreferencesManager {
   public:
      CGoliathPreferences(LStr255 prefsFileName);
      virtual ~CGoliathPreferences();  
      
      virtual Boolean CommitPreferences();
      
      // Prefrence keys and values
      static const char* STARTUP_ACTION;
      static const char* STARTUP_ACTION_NEW;
      static const char* STARTUP_ACTION_OPEN;
      static const char* STARTUP_ACTION_IDISK;
      static const char* STARTUP_ACTION_NONE;
      static const char* STARTUP_ACTION_COOKIE;
      static const char* LOCK_OWNER;
      static const char* SET_LOCK;
      static const char* SHOWDOTFILES;
            
      static const char* PROXY_SERVER;
      static const char* PROXY_SRVRPORT;
      static const char* PROXY_USER;
      static const char* PROXY_PASSWORD;
      
      static const char* SORT_CASEINSENSITIVE;
      
      static const char* TRUE_VALUE;
      static const char* FALSE_VALUE;
      
      
   protected:
      virtual Boolean _loadFromPrefFile();
      virtual Boolean _saveToPrefFile();
      virtual Boolean _populatePrefs(Boolean useDefaults);
      
};

#endif


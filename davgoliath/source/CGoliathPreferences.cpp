/* ==================================================================================================
 * CGoliathPreferences.cpp															   
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
 

#include <CGoliathPreferences.h>
#include <xmlparse.h>
#include <LDynamicBuffer.h>
#include <CDAVLibUtils.h>
#include <CDAVRequest.h>
#include <CXMLParser.h>
#include <string>
#include <string.h>
#include "CDAVTableApp.h"
#include "CDAVTableAppConstants.h"
#include <stdio.h>

// defined preference keys
const char* CGoliathPreferences::STARTUP_ACTION = "startup.action";
const char* CGoliathPreferences::STARTUP_ACTION_NEW ="new";
const char* CGoliathPreferences::STARTUP_ACTION_OPEN = "load";
const char* CGoliathPreferences::STARTUP_ACTION_IDISK = "idisk";
const char* CGoliathPreferences::STARTUP_ACTION_COOKIE = "start.cookie";
const char* CGoliathPreferences::STARTUP_ACTION_NONE = "none";
const char* CGoliathPreferences::LOCK_OWNER     = "owner";
const char* CGoliathPreferences::SET_LOCK   = "set.lock";
const char* CGoliathPreferences::SHOWDOTFILES = "showdotfiles";
const char* CGoliathPreferences::PROXY_SERVER = "proxy.server";
const char* CGoliathPreferences::PROXY_SRVRPORT = "proxy.port";
const char* CGoliathPreferences::PROXY_USER = "proxy.user";
const char* CGoliathPreferences::PROXY_PASSWORD  = "proxy.pass";
const char* CGoliathPreferences::SORT_CASEINSENSITIVE = "sort.casesensitive";

const char* CGoliathPreferences::TRUE_VALUE  = "1";
const char* CGoliathPreferences::FALSE_VALUE = "0";

const char* PREFLIST = "preflist";
const char* PREF     = "pref";
const char* NAME     = "name";
const char* VALUE    = "value";


/*****************************************************************

   <!-- DTD for Preferences  -->
   <!ELEMENT preflist (pref+) >
             
   <!ELEMENT pref        (name, value)>
   <!ELEMENT name        (#PCDATA)>
   <!ELEMENT value       (#PCDATA)>   

*****************************************************************/

class CGoliathPrefsParser : public CXMLParser {
   public:
      CGoliathPrefsParser(CPreferencesManager *prefMgr);
      virtual void StartElementHandler(const XML_Char *name, const XML_Char **atts);
      virtual void EndElementHandler(const XML_Char *name);
      virtual void CharacterDataHandler(const XML_Char *s, int len);
            
   protected:
      enum elemtype {
         none=0,
         name=1,
         value=2
      };
      
      CPreferencesManager *mPrefMgr;
      std::string          mName;
      std::string          mValue;
      elemtype             mCurrType;
};

// ---------------------------------------------------------------------------------
//		¥ CGoliathPreferences()
// ---------------------------------------------------------------------------------
//
CGoliathPreferences::CGoliathPreferences(LStr255 prefsFileName): CPreferencesManager(prefsFileName) {

}

// ---------------------------------------------------------------------------------
//		¥ ~CGoliathPreferences()
// ---------------------------------------------------------------------------------
//
CGoliathPreferences::~CGoliathPreferences() {
}

// ---------------------------------------------------------------------------------
//		¥ CommitPreferences()
// ---------------------------------------------------------------------------------
//
Boolean CGoliathPreferences::CommitPreferences() {
   return _saveToPrefFile();
}
 
// ---------------------------------------------------------------------------------
//		¥ ~CGoliathPreferences()
// ---------------------------------------------------------------------------------
//
Boolean CGoliathPreferences::_loadFromPrefFile() {
printf("loadfrompreffile\n");
   CGoliathPrefsParser prefParser(this);
   LFileStream filestream(mPrefsFile);
   filestream.OpenDataFork(fsRdPerm);
   XML_Error err = prefParser.ParseXMLDocument(filestream);
   filestream.CloseDataFork();
   if (err != XML_ERROR_NONE) 
      return false;
      
   return true;
}

// ---------------------------------------------------------------------------------
//		¥ ~CGoliathPreferences()
// ---------------------------------------------------------------------------------
//
Boolean CGoliathPreferences::_saveToPrefFile() { 
printf("savetopreffile\n");     
   LDynamicBuffer dynBuf;
   LStr255 tmpStr;

   CXMLStringUtils::beginXMLDAVBody(dynBuf);
   CXMLStringUtils::startElement(dynBuf, PREFLIST);
   
   std::map<std::string, std::string>::iterator iter = mPrefVals.begin();
   while (iter!= mPrefVals.end()) {
      std::string prefname = (*iter).first, prefval = (*iter).second;
      CXMLStringUtils::startElement(dynBuf, PREF);
      CXMLStringUtils::startElement(dynBuf, NAME);
      dynBuf.ConcatenateBuffer(prefname.c_str());   
      CXMLStringUtils::endElement(dynBuf, NAME);
      CXMLStringUtils::startElement(dynBuf, VALUE);      
      dynBuf.ConcatenateBuffer(prefval.c_str());   
      CXMLStringUtils::endElement(dynBuf, VALUE);
      CXMLStringUtils::endElement(dynBuf, PREF);      
      iter++;
   }
   CXMLStringUtils::endElement(dynBuf, PREFLIST);

   LFile outFile(mPrefsFile);
   
   return CDAVRequest::DynBufferToFile(&dynBuf, outFile);
}


// ---------------------------------------------------------------------------------
//		¥ _populatePrefs()
// ---------------------------------------------------------------------------------
//
Boolean CGoliathPreferences::_populatePrefs(Boolean useDefaults) {
printf("populateprefs\n");
   if (useDefaults) {
   printf("usedefaults\n");
      SetPrefValue(STARTUP_ACTION, STARTUP_ACTION_NEW);
      //***teb - iBackup customization
      LStr255 rezHost(str_FileNameStrings, str_iDiskHostName);
      LStr255 findStr("ibackup.com");
      if (rezHost.Find(findStr) != 0)
         SetPrefValue(STARTUP_ACTION, STARTUP_ACTION_IDISK);
     
     
      LStr255 tmpStr(str_UIStrings, str_DefaultLockUserName);
	  std::string tmpCstr;
	  tmpCstr.assign(tmpStr.TextPtr(), tmpStr.Length());
      SetPrefValue(LOCK_OWNER, tmpCstr.c_str());
      SetPrefValue(SET_LOCK, TRUE_VALUE);
      SetPrefValue(SORT_CASEINSENSITIVE, FALSE_VALUE);
      return CommitPreferences();
   } else {
   printf("!usedefaults\n");
      bool ret = _loadFromPrefFile();
      if (ret) {
           //***teb - iBackup customization
	      LStr255 rezHost(str_FileNameStrings, str_iDiskHostName);
    	  LStr255 findStr("ibackup.com");
      	  if (rezHost.Find(findStr) != 0) {
             std::string cookieVal = GetPrefValue(STARTUP_ACTION_COOKIE);
             if (cookieVal.empty()) {
                SetPrefValue(STARTUP_ACTION, STARTUP_ACTION_IDISK);
                SetPrefValue(STARTUP_ACTION_COOKIE, "setonce");
                CommitPreferences();
             }
          }
      }
      return ret;
   }
}

// ---------------------------------------------------------------------------------
//		¥ CGoliathPrefsParser()
// ---------------------------------------------------------------------------------
//
CGoliathPrefsParser::CGoliathPrefsParser(CPreferencesManager *prefMgr): mCurrType(none), mPrefMgr(prefMgr) {

}

// ---------------------------------------------------------------------------------
//		¥ CGoliathPrefsParser::StartElementHandler()
// ---------------------------------------------------------------------------------
//
void CGoliathPrefsParser::StartElementHandler(const XML_Char *elemname, const XML_Char **) {
   mCurrType = none;
   if (strcmp(elemname, NAME) ==0) {
      mCurrType = name;
   } else if (strcmp(elemname, VALUE) ==0) {
      mCurrType = value;
   } else {
      mCurrType = none;
   }
}


// ---------------------------------------------------------------------------------
//		¥ CGoliathPrefsParser::EndElementHandler()
// ---------------------------------------------------------------------------------
//
void CGoliathPrefsParser::EndElementHandler(const XML_Char* name) {
   if (strcmp(name, PREF) ==0) {
      mPrefMgr->SetPrefValue(mName.c_str(), mValue.c_str());
      mName = "";
      mValue = "";
   }
}

// ---------------------------------------------------------------------------------
//		¥ CGoliathPrefsParser::CharacterDataHandler()
// ---------------------------------------------------------------------------------
//
void CGoliathPrefsParser::CharacterDataHandler(const XML_Char *s, int len) {
   if (name == mCurrType) {
      mName.append(s, len);
   } else if (value == mCurrType) {
      mValue.append(s, len);
   }
} 


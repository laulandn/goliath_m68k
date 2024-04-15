/* ===========================================================================
 *	CDAVInfo.cpp			   
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
 * ===========================================================================
 */
 
#include "CDAVInfo.h"
#include <string.h>

const char* kModDavExecutableHdr = "<http://apache.org/dav/propset/fs/1>";



// ---------------------------------------------------------------------------
//		¥ CDAVInfo()
// ---------------------------------------------------------------------------
//	
CDAVInfo::CDAVInfo():mProtocolOps(noOperations),  mHasMSAuthorHdr(false), mModDavExecutable(false) {
   
}

// ---------------------------------------------------------------------------
//		¥ CDAVInfo()
// ---------------------------------------------------------------------------
//	
CDAVInfo::CDAVInfo(CDAVInfo &rhs) {
   mProtocolOps = rhs.mProtocolOps;
   mDAVLevel = rhs.mDAVLevel;
   mServerType = rhs.mServerType;
   mHasMSAuthorHdr = rhs.mHasMSAuthorHdr;
   mMSAuthorVia = rhs.mMSAuthorVia;
   mModDavExecutable = rhs.mModDavExecutable;
}


// ---------------------------------------------------------------------------
//		¥ ~CDAVInfo()
// ---------------------------------------------------------------------------
//	
CDAVInfo::~CDAVInfo() {

}

// ---------------------------------------------------------------------------
//		¥ = ()
// ---------------------------------------------------------------------------
//	
CDAVInfo&  CDAVInfo::operator = (CDAVInfo& rhs) {
   mProtocolOps = rhs.mProtocolOps;
   mDAVLevel = rhs.mDAVLevel;
   mServerType = rhs.mServerType;
   mHasMSAuthorHdr = rhs.mHasMSAuthorHdr;
   mMSAuthorVia = rhs.mMSAuthorVia;
   mModDavExecutable = rhs.mModDavExecutable;
   return *this;
}


// ---------------------------------------------------------------------------
//		¥ getSupportedOperations()
// ---------------------------------------------------------------------------
//	
SInt32 CDAVInfo::getSupportedOperations() {
   return mProtocolOps;
}
      
// ---------------------------------------------------------------------------
//		¥ setSupportedOperations()
// ---------------------------------------------------------------------------
//	
void CDAVInfo::setSupportedOperations(SInt32 ops) {
   mProtocolOps = ops;
}
      
// ---------------------------------------------------------------------------
//		¥ setMSHeaderFields()
// ---------------------------------------------------------------------------
//	
void CDAVInfo::setMSHeaderFields(std::string &msHdrs) {
   mHasMSAuthorHdr = true;
   mMSAuthorVia = msHdrs;
}

// ---------------------------------------------------------------------------
//		¥ getHasMSHeaderFields()
// ---------------------------------------------------------------------------
//	
Boolean CDAVInfo::getHasMSHeaderFields() {
   return mHasMSAuthorHdr;
}


// ---------------------------------------------------------------------------
//		¥ getMSHeaderFields()
// ---------------------------------------------------------------------------
//	
std::string& CDAVInfo::getMSHeaderFields() {
   return mMSAuthorVia;
}
      
// ---------------------------------------------------------------------------
//		¥ getDavVersionLevel()
// ---------------------------------------------------------------------------
//	
std::string& CDAVInfo::getDavClassSupport() {
   return mDAVLevel;
}

// ---------------------------------------------------------------------------
//		¥ setDavVersionLevel()
// ---------------------------------------------------------------------------
//	
void CDAVInfo::setDavClassSupport(std::string& vl) {
   mDAVLevel = vl;
   if (NULL != strstr(vl.c_str(), kModDavExecutableHdr))
      mModDavExecutable = true;
   
}
      
// ---------------------------------------------------------------------------
//		¥ getServerType()
// ---------------------------------------------------------------------------
//	
std::string& CDAVInfo::getServerType() {
   return mServerType;
}

// ---------------------------------------------------------------------------
//		¥ setServerType()
// ---------------------------------------------------------------------------
//
void CDAVInfo::setServerType(std::string &st) {
   mServerType = st;
}


// ---------------------------------------------------------------------------
//		¥ getHasExecutableSupport()
// ---------------------------------------------------------------------------
//
Boolean CDAVInfo::getHasExecutableSupport() {
   return mModDavExecutable;
}

      

/* ===========================================================================
 *	CDAVItem.cpp			   
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
 
#include "CDAVItem.h"
#include "CDAVLibUtils.h"
#include <string.h>

std::string emptyPropValStr("");

// ---------------------------------------------------------------------------
//		¥ CDAVItem()
// ---------------------------------------------------------------------------
//	Constructor
CDAVItem::CDAVItem(): mFileNameIndex(-1), mFileNameLength(-1), mValid(false),
mIsLocked(false), mIsLocallyOwnedLock(false) {

}

// ---------------------------------------------------------------------------
//		¥ CDAVItem()
// ---------------------------------------------------------------------------
//	
CDAVItem::CDAVItem(std::string &href): mValid(true), mIsLocked(false),
                   mIsLocallyOwnedLock(false) {
   if (href[href.length()-1]=='/')
      SetItemType(CDAVItem::COLLECTION);
   else
     SetItemType(CDAVItem::FILE);
     
   SetHREF(href);
}


// ---------------------------------------------------------------------------
//		¥ CDAVItem()
// ---------------------------------------------------------------------------
//	
CDAVItem::CDAVItem(const CDAVItem &rhs) {
   m_href = rhs.m_href;
   m_href_p = rhs.m_href_p;
   m_itemType = rhs.m_itemType;
   m_props = rhs.m_props;
   mFileNameIndex = rhs.mFileNameIndex;
   mFileNameLength = rhs.mFileNameLength;
   mIsLocked = rhs.mIsLocked;
   mLockToken = rhs.mLockToken;
   mTimeout = rhs.mTimeout;
   mLockScope = rhs.mLockScope;
   mLockType = rhs.mLockType;
   mLockDepth = rhs.mLockDepth;
   mOwner = rhs.mOwner;
   mIsLocallyOwnedLock = rhs.mIsLocallyOwnedLock;
}


// ---------------------------------------------------------------------------
//		¥ ~CDAVItem()
// ---------------------------------------------------------------------------
//	
CDAVItem::~CDAVItem() {

}



// ---------------------------------------------------------------------------
//		¥ GetHREF()
// ---------------------------------------------------------------------------
//	
const PP_STD::string& CDAVItem::GetHREF() const{
	return m_href; 
}

// ---------------------------------------------------------------------------
//		¥ GetHREF()
// ---------------------------------------------------------------------------
//	
const LStr255& CDAVItem::GetPHREF(){
	m_href_p = LStr255 (m_href.c_str(), m_href.size());
   return m_href_p; 
}

// ---------------------------------------------------------------------------
//		¥ GetURI()
// ---------------------------------------------------------------------------
//	
const std::string& CDAVItem::GetURI() const{
   return m_href;
}

// ---------------------------------------------------------------------------
//		¥ GetFileName()
// ---------------------------------------------------------------------------
//	
void CDAVItem::GetFileName(PP_STD::string &name) const{
	name = GetHREF().substr(mFileNameIndex, mFileNameLength);
}

// ---------------------------------------------------------------------------
//		¥ GetParentPath()
// ---------------------------------------------------------------------------
//	
void CDAVItem::GetParentPath(PP_STD::string &path) const{
	path = GetHREF().substr(0, mFileNameIndex-1);
}

// ---------------------------------------------------------------------------
//		¥ SetHREF()
// ---------------------------------------------------------------------------
//	
void CDAVItem::SetHREF(std::string &href) {
   m_href = href;
   m_href_p = href.c_str();
   
   UInt16 nameStart;
	if (GetItemType() == CDAVItem::COLLECTION)
	{
		nameStart = m_href.rfind('/', m_href.size() - 2);
		mFileNameIndex = nameStart + 1;
		mFileNameLength = m_href.size() - nameStart - 2;
	}
	else
	{
		nameStart = m_href.rfind('/');
		mFileNameIndex = nameStart + 1;
		mFileNameLength = m_href.size()-nameStart;
	}
}
	
// ---------------------------------------------------------------------------
//		¥ GetItemType()
// ---------------------------------------------------------------------------
//	
CDAVItem::ItemType CDAVItem::GetItemType() const{
   return m_itemType;
}

// ---------------------------------------------------------------------------
//		¥ GetItemType()
// ---------------------------------------------------------------------------
//	
Boolean CDAVItem::GetParentHREF(PP_STD::string &pntHRef) const{
   return CURLStringUtils::GetURIParent(m_href, pntHRef);
}

// ---------------------------------------------------------------------------
//		¥ SetItemType()
// ---------------------------------------------------------------------------
//	
void CDAVItem::SetItemType(CDAVItem::ItemType type) {
   m_itemType = type;
}



// ---------------------------------------------------------------------------
//		¥ GetPropertyValue()
// ---------------------------------------------------------------------------
//	
std::string& CDAVItem::GetPropertyValue(CDAVProperty &propType) {
	if (m_props.find (propType) != m_props.end())
		return m_props [propType];
	else
		return emptyPropValStr;
}

// ---------------------------------------------------------------------------
//		¥ SetPropertyValue()
// ---------------------------------------------------------------------------
//	
void CDAVItem::SetPropertyValue(CDAVProperty &propType, std::string& pvalue) {
	m_props[propType] = pvalue;
}

// ---------------------------------------------------------------------------
//		¥ GetRawPropertyValue()
// ---------------------------------------------------------------------------
//	
std::string& CDAVItem::GetRawPropertyValue(CDAVProperty& propType) {
   return mRawPropertyMap[propType];
}

// ---------------------------------------------------------------------------
//		¥ RemoveProperty()
// ---------------------------------------------------------------------------
//	
void CDAVItem::RemoveProperty(CDAVProperty *prop) {
   DAVPropertyMap::iterator iter;
   iter = m_props.find(*prop);
   if (iter != m_props.end())
      m_props.erase(iter);

   iter = mRawPropertyMap.find(*prop);
   if (iter != mRawPropertyMap.end())
      mRawPropertyMap.erase(iter);
}


// ---------------------------------------------------------------------------
//		¥ SetRawPropertyValue()
// ---------------------------------------------------------------------------
//	
void CDAVItem::SetRawPropertyValue(CDAVProperty& propType, std::string& pvalue) {
   mRawPropertyMap[propType] = pvalue;
}

// ---------------------------------------------------------------------------
//		¥ GetIsLocked()
// ---------------------------------------------------------------------------
//	
Boolean CDAVItem::GetIsLocked() {
   return mIsLocked;
}

// ---------------------------------------------------------------------------
//		¥ SetLockInformation()
// ---------------------------------------------------------------------------
//
void CDAVItem::SetLockInformation(LockType lockType, LockScope lockScope, const char* timeout,
                              const char* lockToken, DAVTypes::PropertyDepth lockDepth,
                              const char* owner) {
   mIsLocked = true;
   mLockToken = lockToken;
   mTimeout = timeout;
   mLockScope = lockScope;
   mLockType = lockType;
   mLockDepth = lockDepth;                     
   mOwner = owner;
}

// ---------------------------------------------------------------------------
//		¥ GetLockToken()
// ---------------------------------------------------------------------------
//
std::string CDAVItem::GetLockToken() {
   return mLockToken;
}

// ---------------------------------------------------------------------------
//		¥ GetLockTimeout()
// ---------------------------------------------------------------------------
//
std::string CDAVItem::GetLockTimeout() {
   return mTimeout;
}

// ---------------------------------------------------------------------------
//		¥ GetLockOwner()
// ---------------------------------------------------------------------------
//
std::string CDAVItem::GetLockOwner() {
   return mOwner;
}

// ---------------------------------------------------------------------------
//		¥ GetLockScope()
// ---------------------------------------------------------------------------
//
CDAVItem::LockScope CDAVItem::GetLockScope() {
   return mLockScope;
}

// ---------------------------------------------------------------------------
//		¥ GetLockType()
// ---------------------------------------------------------------------------
//
CDAVItem::LockType CDAVItem::GetLockType() {
   return mLockType;
}

// ---------------------------------------------------------------------------
//		¥ GetLockDepth()
// ---------------------------------------------------------------------------
//
DAVTypes::PropertyDepth CDAVItem::GetLockDepth() {
   return mLockDepth;
}

// ---------------------------------------------------------------------------
//		¥ SetIsLocalLock()
// ---------------------------------------------------------------------------
//
void CDAVItem::SetIsLocalLock(Boolean isLocalLock) {
   mIsLocallyOwnedLock = isLocalLock;
}

// ---------------------------------------------------------------------------
//		¥ GetIsLocalLock()
// ---------------------------------------------------------------------------
//
Boolean CDAVItem::GetIsLocalLock() {
   return mIsLocallyOwnedLock;
}

// ---------------------------------------------------------------------------
//		¥ ResetLockStatus()
// ---------------------------------------------------------------------------
//
void CDAVItem::ResetLockStatus() {
   mIsLocked = false;
   mLockToken = "";
   mTimeout = "";
   mOwner = "";
   mIsLocallyOwnedLock = false;
}

// ---------------------------------------------------------------------------
//		¥ GetIsExecutable()
// ---------------------------------------------------------------------------
//
Boolean CDAVItem::GetIsExecutable() {
   std::string execVal = GetPropertyValue(mod_davExecutable);
   if (strcmp(execVal.c_str(), "T")==0)
      return true;
   return false;
}

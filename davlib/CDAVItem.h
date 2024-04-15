/* ===========================================================================
 *	CDAVItem.h			   
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

/*!
   @header DAVLib
	This library implements HTTP 1.1 and the WebDAV (RFC 2518) extensions to HTTP.
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/
 
#pragma once

#ifndef __CDAVITEM_H__
#define __CDAVITEM_H__

#include <string>
#include <vector>
#include <map>
#include <LString.h>
#include <DAVTypes.h>
#include <CDAVProperty.h>

typedef std::map<const CDAVProperty, std::string> DAVPropertyMap;

/*!
  @class CDAVItem
  @abstract Abstraction of a resource on a DAV server
**/
class CDAVItem {
   public:
     /*!
		 @enum ItemType
		 @discussion this data type enumerates the various types of resources
		 @constant COLLECTION denotes a collection resource
		 @constant FILE denotes a file resource
	   */
      enum ItemType {
         COLLECTION=0,
         FILE=1
      };
      /*!
		  @enum LockType
		  @discussion enumerates the various types of WebDAV locks.
		  @constant WRITE a write lock
		 */
      enum LockType {
         WRITE=0
      };
      /*! 
		  @enum LockScope
		  @constant EXCLUSIVE defines an exclusive lock
		  @constant SHARED defines a lock that is shared
		*/
      enum LockScope {
         EXCLUSIVE = 0,
         SHARED    = 1
      };
      
      
      //constructors and destructor
      /*!
		  @function CDAVItem
		  @discussion object constructor		  
		 */
      CDAVItem();
      /*!
		  @function CDAVItem
		  @discussion object constructor		  
		  @param the URI that this object represents
		 */
      CDAVItem(std::string &href);
      /*!
		  @function CDAVItem
		  @discussion object copy constructor		  
		  @param rhs copy state from this object
		 */
      CDAVItem(const CDAVItem &rhs);
      /*! 
		  @function ~CDAVItem
		  @discussion object destructor
		*/
      virtual ~CDAVItem();
      
      /*!
		  @function SetHREF
		  @discussion set the URI that this item represents
		  @param href the URI referenced by this item
		*/
      void SetHREF(std::string &href);
	   /*! 
		  @function GetHREF
		  @result returns the URI that this item represents as a string
		*/
	  const std::string& GetHREF() const;
	   /*! 
		  @function GetPHREF
		  @result returns the URI that this item represents as an LStr255
		*/
	  const LStr255& GetPHREF();
	   /*! 
		  @function GetURI
		  @result returns the URI that this item represents as a reference to a std::string
		*/	 
      const std::string& GetURI() const;
	   /*!
		  @function GetFileName
		  @discussion calculate the file name of the URI that this item refers to
		  @param outName output parameter that will receive the file name of the item
		 */
      void GetFileName(PP_STD::string &name) const;
	   /*!
		  @function GetParentPath
		  @discussion calculate the path of this item's parent
		  @param outName output parameter that will receive the path of this item's parent
		 */
      void GetParentPath(PP_STD::string &path) const;
      
      // file or collection
	   /*!
		  @function GetItemType
		  @result an ItemType
		*/
      ItemType GetItemType() const;
	   /*!
		  @function SetItemType
		  @param type
		*/
      void SetItemType(ItemType type);
	   /*!
		  @function GetParentHREF
		  @param outPntHRef
		 */
      Boolean GetParentHREF(std::string &pntHRef) const;
      
      // property APIs
	   /*!
		  @function GetPropertyValue
		  @discussion retrieves the value of the specified property from this object
		  @param propType the proerty to retrieve
		 */
      std::string& GetPropertyValue(CDAVProperty& propType);
	   /*!
		  @function SetPropertyValue
		  @discussion sets a specified property's value
		  @param propType the property to set
		  @param pvalue the property value
		 */
      void SetPropertyValue(CDAVProperty& propType, std::string& pvalue);
	   /*!
		  @function GetRawPropertyValue
		  @discussion for a specifiec property, return the raw XML returned from the 
		  server containing the property's valye
		  @param propType the property to retrieve
		*/
      std::string& GetRawPropertyValue(CDAVProperty& propType);
	   /*!
		  @function SetRawPropertyValue
		  @discussion for a specifiec property, set the raw XML returned from the 
		  server containing the property's valye
		  @param propType the property to retrieve
		  @param pvalue the raw property value
		*/
      void SetRawPropertyValue(CDAVProperty& propType, std::string& pvalue);
	   /*!
		  @function RemoveProperty
		  @discussion remove a specifiec property from this item
		  @param prop the property to remove
		*/
      void RemoveProperty(CDAVProperty *prop);
      
      // Locking APIs
	   /*!
		  @function GetIsLocked
		  @discussion returns true if this item is locked
		 */
      Boolean GetIsLocked();
	   /*!
		  @function GetLockToken
		  @discussion returns the lock token for this item
		 */
      std::string            GetLockToken();
	   /*!
		  @function GetLockTimeout
		  @discussion gets the timeout of the lock associated with this item
		 */
      std::string            GetLockTimeout();
	   /*!
		  @function GetLockOwner
		  @discussion returns the lock owner
		 */
      std::string            GetLockOwner();
	   /*!
		  @function GetLockScope
		 */
      LockScope               GetLockScope();
	   /*!
		  @function GetLockType
		 */
      LockType                GetLockType();
	   /*!
		  @function GetLockDepth
		 */
      DAVTypes::PropertyDepth GetLockDepth();
      
	   /*!
		  @function SetLockInformation
		 */
      void SetLockInformation(LockType lockType, LockScope lockScope, const char* timeout,
                              const char* lockToken, DAVTypes::PropertyDepth lockDepth,
                              const char* owner);
                              
	   /*!
		  @function SetIsLocalLock
		  @discussion sets a state whether the current lock on this item is bound to the 
		  current user/computer
		 */
      void SetIsLocalLock(Boolean isLocalLock);
	   /*!
		  @function GetIsLocalLock
		  @discussion returns true if the current lock on this item is bound to the 
		  current user/computer

		 */
      Boolean GetIsLocalLock();
      
	   /*!
		  @function ResetLockStatus
		  @discussion set lock states to unlocked and empty                       
		 */
      void ResetLockStatus();
      
      /*!
         @function GetIsExecutable
         @discussion mod_dav 1.0 introduced a special property to designate that
            a resource was executable; this is primarily intended to be used with CGI
            scripts on Unix.  This will return the executable state of the given item.
      */
      Boolean GetIsExecutable();
      
   protected:
      std::string     m_href;
      LStr255         m_href_p;
      ItemType        m_itemType;
      DAVPropertyMap  m_props;
      UInt16          mFileNameIndex, mFileNameLength;
      Boolean         mValid;
      
      Boolean         mIsLocked;
      std::string     mLockToken;
      std::string     mTimeout;
      LockScope       mLockScope;
      LockType        mLockType;
      std::string     mOwner;
      DAVTypes::PropertyDepth   mLockDepth;
      
      Boolean         mIsLocallyOwnedLock;
      
      DAVPropertyMap  mRawPropertyMap;
};

typedef std::vector<CDAVItem> CDAVItemVector;

#endif

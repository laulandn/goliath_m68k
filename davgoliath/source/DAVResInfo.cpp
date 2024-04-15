/* ==================================================================================================
 * DAVResInfo.cpp															   
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
#include "DAVResInfo.h"

#include <MacTypes.h>

/*
const short kServerId  = 128;
const short kPort      = 129;
const short kResource  = 129;
const short kLockToken = 130;
const short kLogin     = 131;
const short kUserPassword  = 132;
*/

#define kDAVInfoResType 'WDAV'

typedef struct {
   Byte* data;
} DAVResInfoItem, **DavResInfoItemHandle;

DavResInfoItemHandle _GetDAVInfoItem(short resId, OSErr *err);
DavResInfoItemHandle _NewDAVInfoItem(short resId);


//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ _GetDAVInfoItem
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//  
DavResInfoItemHandle _GetDAVInfoItem(short resId, OSErr *err) {
   DavResInfoItemHandle davinfo;
   
   davinfo = reinterpret_cast<DavResInfoItemHandle>(Get1Resource(kDAVInfoResType, resId));
   *err = ResError();
   if ((nil == davinfo) && (*err == noErr)) 
      *err = resNotFound;
   
   if (davinfo != nil) {
      if (*err != noErr || (nil == *davinfo))
         davinfo = nil; 
   }
   return davinfo;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ NewDAVInfoItem
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//  
DavResInfoItemHandle _NewDAVInfoItem(short resId) {
   DavResInfoItemHandle davinfo;
   OSErr err;
   
   davinfo = _GetDAVInfoItem(resId, &err);
   if (nil == davinfo) {
      davinfo = reinterpret_cast<DavResInfoItemHandle>(NewHandleClear(sizeof(DAVResInfoItem)));
      if (davinfo != nil) {
         AddResource(reinterpret_cast<Handle>(davinfo), kDAVInfoResType, resId, "\p");
         if (ResError() != noErr) {
            DisposeHandle(reinterpret_cast<Handle>(davinfo));
            davinfo = nil;
         }
      }
   } else {
      SetHandleSize(reinterpret_cast<Handle>(davinfo), sizeof(DAVResInfoItem));
      ChangedResource(reinterpret_cast<Handle>(davinfo));
   }
   return davinfo;
}


//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ ImprintDAVInfo
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//  
Boolean ImprintDAVInfo(FSSpec *theSpec, const char* server, const char* port, 
                       const char* resource, const char* locktoken,
                       const char* login, const char* password) {
                       
   SInt16 origResFile = CurResFile();
   SInt16 refNum = FSpOpenResFile(theSpec, fsCurPerm);
   UseResFile(refNum);
 
   CloseResFile(refNum);
   UseResFile(origResFile);
   
   return false;                      
}


//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ HasDAVResInfo
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//  
Boolean HasDAVResInfo(FSSpec *theSpec) {
   SInt16 origResFile = CurResFile();
   SInt16 refNum = FSpOpenResFile(theSpec, fsCurPerm);
   OSErr err;
   UseResFile(refNum);

   DavResInfoItemHandle foo = _GetDAVInfoItem(kServerId, &err);
 
   CloseResFile(refNum);
   UseResFile(origResFile);

   return (err != noErr);
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ GetDAVInfoFromResource
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//  
Boolean GetDAVInfoFromResource(FSSpec *theSpec, const short fieldId, std::string& outVal) {
   SInt16 origResFile = CurResFile();
   SInt16 refNum = FSpOpenResFile(theSpec, fsCurPerm);
   UseResFile(refNum);
   Boolean retVal = false;
   OSErr err;
   
   DavResInfoItemHandle foo = _GetDAVInfoItem(fieldId, &err);
   
   if (err != noErr) {
      StHandleLocker(reinterpret_cast<Handle>(foo));
      outVal.assign((char*)((DAVResInfoItem*)(*foo))->data);
      retVal = true;
   }
   
   CloseResFile(refNum);
   UseResFile(origResFile);
   
   return retVal;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ GetDAVInfoFromResource
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//  
Boolean GetDAVInfoFromResource(FSSpec *theSpec, std::string& outServer,
                                std::string& outPort,  std::string& outResource,
                                std::string& outLockToken, Boolean &mHasAuthentication,
                                std::string& outLogin,  std::string& outPassword) {
                                
   SInt16 origResFile = CurResFile();
   SInt16 refNum = FSpOpenResFile(theSpec, fsCurPerm);
   UseResFile(refNum);
 
   CloseResFile(refNum);
   UseResFile(origResFile);
   
   return false;                               
}
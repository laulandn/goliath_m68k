/* ==================================================================================================
 * CNavServicesUtils.h															   
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
 
 #ifndef __CNAVSERVICESUTILS_H__
 #define __CNAVSERVICESUTILS_H__
 
 #include <MacTypes.h>
 #include <Files.h>
 #include <PP_Types.h>
 #include <AEDataModel.h>
 
 
 class CNavServicesUtils {
 
    public:
       static Boolean getfileNavSrv(FSSpec *theSpec, ResIDT strListId, ResIDT strId, OSType *openType, OSType *openComp, OSType* secondOpenType=NULL);
       static Boolean savefileNavSrv(FSSpec *outSpec, ResIDT strList, ResIDT strId, OSType fileType, OSType creator);
       static Boolean getfolderNavSrv(FSSpec *outSpec, ResIDT strList, ResIDT strId);
       
       static Boolean getApplicationNavSrv(FSSpec *outSpec, ResIDT strList, ResIDT strId);
   protected:
       static OSErr _AEGetDescData(const AEDesc *desc, DescType *typeCode, void *dataBuffer, ByteCount maximumSize, ByteCount *actualSize);

 };
 
 
 #endif
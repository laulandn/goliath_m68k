/* ==================================================================================================
 * CIconSuiteManager.h															   
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
 
 #ifndef __ICONSUITEMANAGER_H__
 #define __ICONSUITEMANAGER_H__
 
 #include <map>
 #include <string>
 #include <LString.h>
 #include <Icons.h> 
 #include <CDAVItem.h>
 
 class CIconSuiteManager {
   public:
#if PP_Target_Carbon
      static OSErr GetIconForResource(const std::string &resource, IconRef* icnRef);
      static OSErr GetIconForItem(CDAVItem &resItem, IconRef* icnRef);
#else
      static Handle GetIconForResource(const LStr255 &resource);
      static Handle GetIconForItem(CDAVItem &resItem);

   protected:
      static OSErr _GetNormalFileIcon(OSType fileType, OSType fileCreator, IconSelectorValue iconSelector,
                               Handle *theSuite);

      static short _FindGenericIconID(OSType theType, Boolean	*inFinder);
      static void _GetFinderFilename(StringPtr _finderFilename);
      static OSErr _GetResourceIcons(Handle *theSuite, short theID, long theSelector);
      static OSErr _Get1IconSuite(Handle	*theSuite, short theID, long theSelector);
      static OSErr _CopyEachIcon(Handle theSuite);
      static Boolean _IsSuiteEmpty(Handle theSuite);
      static Boolean _InOneDesktop(short vRefNum, OSType fileCreator, short *dtRefNum);
      static short _FindDesktopDatabase(short firstVRefNum, OSType fileCreator);


#endif
};
 
 
 #endif
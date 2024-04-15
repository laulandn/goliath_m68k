/* ==================================================================================================
 * CIconSuiteManager.cpp															   
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
 
#include "CIconSuiteManager.h"
 
#include <Icons.h>
#include <Finder.h>
#include <LString.h>
#include <LFile.h>
#include <UInternetConfig.h>
#include <CDAVTableApp.h>
#include <Folders.h>
#include <MixedMode.h>
#include <LowMem.h>



 
#if PP_Target_Carbon

// ---------------------------------------------------------------------------
//		¥ GetIconForResource()
// ---------------------------------------------------------------------------
//
OSErr CIconSuiteManager::GetIconForResource(const std::string &resource, IconRef* icnRef) {
   OSType creator='????', fileType = '????';
   ICMapEntry entry;
 
   LStr255 rezStr;
   std::string::size_type lastDelim = resource.rfind('/');
   if (lastDelim == std::string::npos) {
      if (resource.size() > 255) {
         lastDelim = resource.size() - (resource.size() - 255);
      } else {
         lastDelim = 0;
      }
   }
    
   std::string tmp = resource.substr(lastDelim);
   rezStr.Assign(tmp.c_str(), tmp.size());
   
   OSStatus err =  UInternetConfig::PP_ICMapFilename(rezStr,  entry);
   bool getDefaultIcon = true;
   if (noErr == err) {
      creator = entry.fileCreator;
      fileType = entry.fileType;
      err = GetIconRef(kOnSystemDisk, creator, fileType, icnRef);
   }
   return err;  
}
 
 
/// ---------------------------------------------------------------------------
//		¥ GetIconForItem()
// ---------------------------------------------------------------------------
//
OSErr CIconSuiteManager::GetIconForItem(CDAVItem &resItem, IconRef* icnRef) {
    return GetIconForResource(resItem.GetHREF(), icnRef);
}

#else
// ===========================================================================

typedef struct genericIconInfo { 
   OSType type;
   short id;
} GenericIconInfo;

typedef struct getIconData {
   OSType fileCreator;
   OSType fileType;
   short DTRefNum;
} GetIconData;

// ===========================================================================


// ===========================================================================

static GenericIconInfo gGenericFinderIcons[]={ 
   {'ifil',12500},
   {'ifil',12500},
   {'sfil',14000},
   {'ffil',14500},
   {'tfil',14501},
   {'kfil',14750},
   {'FFIL',15500},
   {'DFIL',15750}
};
                                              
static GenericIconInfo gGenericSysIcons[]={ 
   {kContainerFolderAliasType,genericFolderIconResource},
   {kContainerTrashAliasType,trashIconResource},
   {kSystemFolderAliasType,systemFolderIconResource},
   {'INIT',genericExtensionIconResource},
   {'APPL',genericApplicationIconResource},
   {'dfil',genericDeskAccessoryIconResource},
   {'pref',genericPreferencesIconResource},
   {kAppleMenuFolderAliasType,appleMenuFolderIconResource},
   {kControlPanelFolderAliasType,controlPanelFolderIconResource},
   {kExtensionFolderAliasType,extensionsFolderIconResource},
   {kPreferencesFolderAliasType,preferencesFolderIconResource},
   {kStartupFolderAliasType,startupFolderIconResource},
   {kApplicationAliasType,genericApplicationIconResource},
   {kExportedFolderAliasType,ownedFolderIconResource},
   {kDropFolderAliasType,dropFolderIconResource},
   {kSharedFolderAliasType,sharedFolderIconResource},
   {kMountedFolderAliasType,mountedFolderIconResource}
};

// ===========================================================================



// ===========================================================================

static pascal OSErr CopyOneIcon(ResType theType, Handle	*theIcon, void	*yourDataPtr);
static pascal OSErr GetIconProc(ResType theType, Handle *theIcon, void *yourDataPtr);
static pascal OSErr TestHandle(ResType theType, Handle *theIcon, void *yourDataPtr);
static pascal OSErr Get1Icon(ResType theType, Handle *theIcon, void *resID);

// ===========================================================================



// ---------------------------------------------------------------------------
//		¥ GetIconForResource()
// ---------------------------------------------------------------------------
//
Handle CIconSuiteManager::GetIconForResource(const LStr255 &resource) {
   OSType creator='????', fileType = '????';
   ICMapEntry entry;
   Handle theSuite;

   OSStatus icErr =  UInternetConfig::PP_ICMapFilename(resource,  entry);
   if (noErr == icErr) {
      creator = entry.fileCreator;
      fileType = entry.fileType;
      OSErr err = _GetNormalFileIcon(fileType, creator, kSelectorAllAvailableData,
                                             &theSuite);
   }  else {
      ::GetIconSuite(&theSuite, icn_file, kSelectorAllAvailableData);
   }  

   return theSuite;
}
 
 
// ---------------------------------------------------------------------------
//		¥ GetIconForItem()
// ---------------------------------------------------------------------------
//
Handle CIconSuiteManager::GetIconForItem(CDAVItem &resItem) {
    return GetIconForResource(resItem.GetPHREF());
}
 
// ---------------------------------------------------------------------------
//		¥ _IsSuiteEmpty()
// ---------------------------------------------------------------------------
//
Boolean CIconSuiteManager::_IsSuiteEmpty( Handle theSuite ) {
   Boolean			retVal;
   IconActionUPP	testHandleProc;

   testHandleProc = NewIconActionUPP( TestHandle );
	
   retVal = true;
   ::ForEachIconDo(theSuite, svAllAvailableData, testHandleProc, &retVal);
   DisposeRoutineDescriptor( (UniversalProcPtr)testHandleProc );
   return retVal;
}


// ---------------------------------------------------------------------------
//		¥ _InOneDesktop()
// ---------------------------------------------------------------------------
// Determine whether the desktop database for one particular volume contains icons for
// a given creator code, and if so, return its reference number.
Boolean	CIconSuiteManager::_InOneDesktop(short vRefNum, OSType fileCreator, short *dtRefNum) {
   OSErr		err;
   DTPBRec		deskRec;
   Boolean		retVal;
	
   HParamBlockRec         _myHPB;
   GetVolParmsInfoBuffer  _infoBuffer;
	
   retVal = false;	// default to failure
   deskRec.ioNamePtr = NULL;
   deskRec.ioVRefNum = vRefNum;
	
   // check to make sure we've got a database first:
   _myHPB.ioParam.ioNamePtr  = (StringPtr)nil;
   _myHPB.ioParam.ioVRefNum  = vRefNum;
   _myHPB.ioParam.ioBuffer   = (Ptr)&_infoBuffer;
   _myHPB.ioParam.ioReqCount = sizeof(_infoBuffer);
   if ( ((err=PBHGetVolParms(&_myHPB,false/*async*/))!=noErr) ||
	    ((_infoBuffer.vMAttrib&(1L<<bHasDesktopMgr))==0) )
      return retVal;

   err = PBDTGetPath( &deskRec );
   if( !err ) {
      /*	We want to ignore any non-icon data, such as the 'paul'
			item that is used for drag-and-drop. */
      deskRec.ioFileCreator = fileCreator;
      deskRec.ioIndex = 1;
      do {
         deskRec.ioTagInfo = 0;
         err = ::PBDTGetIconInfoSync( &deskRec );
         deskRec.ioIndex += 1;
      } while( (err == noErr) && (deskRec.ioIconType <= 0) );
	
      if(err == noErr) {
         retVal = true;
         *dtRefNum = deskRec.ioDTRefNum;
      }
   }
   return retVal;
}


// ---------------------------------------------------------------------------
//		¥ _FindDesktopDatabase()
// ---------------------------------------------------------------------------
//  Find the reference number of a desktop database containing icons
//  for a specified creator code. The search begins on a specified volume,
//  but covers all volumes.
short CIconSuiteManager::_FindDesktopDatabase(short firstVRefNum, OSType fileCreator) {
   VolumeParam vpb;
   short DTRefNum = 0;

   if( !_InOneDesktop(firstVRefNum, fileCreator, &DTRefNum) ) {
      vpb.ioNamePtr = NULL;
      for (vpb.ioVolIndex = 1; PBGetVInfoSync((ParmBlkPtr )&vpb) == noErr; ++vpb.ioVolIndex) {
         if(vpb.ioVRefNum == firstVRefNum)
            continue;
         if( _InOneDesktop(vpb.ioVRefNum, fileCreator, &DTRefNum) )
            break;
      }
   }
   return DTRefNum;
}


// ---------------------------------------------------------------------------
//		¥ _GetNormalFileIcon()
// ---------------------------------------------------------------------------
//
OSErr CIconSuiteManager::_GetNormalFileIcon(OSType fileType, OSType fileCreator, IconSelectorValue iconSelector,
                                             Handle *theSuite) {
   OSErr err;
   short saveResFile, FinderResFile, sysVRefNum;
   long sysDirID;
   Boolean inFinder;
   short iconID;
   LStr255 finderName; 
   IconActionUPP getIconProcPtr;
   GetIconData getData;

   iconID = _FindGenericIconID(fileType, &inFinder );
   saveResFile = ::CurResFile();

   if( inFinder ) {
      ::FindFolder(kOnSystemDisk, kSystemFolderType, kDontCreateFolder, &sysVRefNum, &sysDirID);
      ::SetResLoad( false );
      _GetFinderFilename( finderName );
      FinderResFile = ::HOpenResFile(sysVRefNum, sysDirID, finderName, fsRdPerm);
      ::SetResLoad( true );

      if(FinderResFile == -1)
         err = ::ResError();
      else {
         err = _GetResourceIcons(theSuite, iconID, iconSelector);
         ::CloseResFile( FinderResFile );
      }
	} else	{
      getData.DTRefNum = _FindDesktopDatabase(-1, fileCreator);

      if(getData.DTRefNum != 0)	{
         err = ::NewIconSuite( theSuite );
         if( !err ) {
            getData.fileCreator	= fileCreator;
            getData.fileType	= fileType;
            if(getData.fileType == kApplicationAliasType) {
               getData.fileType = 'APPL';
            }
            getIconProcPtr = NewIconActionUPP(GetIconProc);
            err = ::ForEachIconDo(*theSuite, iconSelector, getIconProcPtr, &getData);
   #if TARGET_CPU_68K
   #else
            ::DisposeRoutineDescriptor( getIconProcPtr );
   #endif
         }
      }
      if( (getData.DTRefNum == 0) || _IsSuiteEmpty( *theSuite )) {
         ::UseResFile( 0 );
         err = _GetResourceIcons(theSuite, iconID, iconSelector);
      }
   }

   ::UseResFile( saveResFile );
   return err;
 }
 


// ---------------------------------------------------------------------------
//		¥ _FindGenericIconID()
// ---------------------------------------------------------------------------
//
short CIconSuiteManager::_FindGenericIconID(OSType theType, Boolean	*inFinder) {
   short id=genericDocumentIconResource; // default
   GenericIconInfo *_icon, *_endIcon;
   
   for (_icon=gGenericFinderIcons,_endIcon=_icon+sizeof(gGenericFinderIcons)/sizeof(GenericIconInfo);
      (_icon<_endIcon)&&(_icon->type!=theType); _icon++) ;
      if (!(*inFinder=(_icon<_endIcon)))
      for (_icon=gGenericSysIcons,_endIcon=_icon+sizeof(gGenericSysIcons)/sizeof(GenericIconInfo);
         (_icon<_endIcon)&&(_icon->type!=theType); _icon++) ;
      if (_icon<_endIcon)
         id = _icon->id;

	return id;
}



// ---------------------------------------------------------------------------
//		¥ _GetFinderFilename()
// ---------------------------------------------------------------------------
//
void CIconSuiteManager::_GetFinderFilename(StringPtr _finderFilename) {
   Str255 _defaultFinderFilename="\pFinder";
   StringPtr _lowMemFinderName;

   _lowMemFinderName = LMGetFinderName();
   if( (_lowMemFinderName != (StringPtr )nil) && (_lowMemFinderName[0] > 0))
      ::BlockMove(_lowMemFinderName, _finderFilename, _lowMemFinderName[0]+1);
   else
      ::BlockMove(_defaultFinderFilename, _finderFilename, _defaultFinderFilename[0]+1);
}



// ---------------------------------------------------------------------------
//		¥ _GetResourceIcons()
// ---------------------------------------------------------------------------
//
OSErr CIconSuiteManager::_GetResourceIcons(Handle *theSuite, short theID, long theSelector) {
   OSErr err;
	
   err = _Get1IconSuite(theSuite, theID, theSelector);
   if(err == noErr) {
      err = _CopyEachIcon( *theSuite );
   }
   return err;
}



// ---------------------------------------------------------------------------
//		¥ _Get1IconSuite()
// ---------------------------------------------------------------------------
//
OSErr CIconSuiteManager::_Get1IconSuite(Handle *theSuite, short theID, long theSelector) {
   OSErr err;
   IconActionUPP get1IconProc;

   err = ::NewIconSuite( theSuite );
   if( !err ) {
      get1IconProc = NewIconActionProc( Get1Icon );
      err = ::ForEachIconDo(*theSuite, theSelector, get1IconProc, &theID);
      #if TARGET_CPU_68K
      #else
      ::DisposeRoutineDescriptor( get1IconProc );
      #endif
   }
   return err;
}

// ---------------------------------------------------------------------------
//		¥ Get1Icon()
// ---------------------------------------------------------------------------
//
static pascal OSErr Get1Icon(ResType theType, Handle *theIcon, void	*resID) {
    short id = *((short*)resID);
	*theIcon = Get1Resource(theType, id);
	return noErr;
}

// ---------------------------------------------------------------------------
//		¥ CopyOneIcon()
// ---------------------------------------------------------------------------
//
static pascal OSErr CopyOneIcon(ResType /*theType*/, Handle	*theIcon, void* /*yourDataPtr*/) {
   OSErr err;
	
   if(*theIcon != NULL) {
      ::LoadResource( *theIcon );
      err = ::HandToHand( theIcon );
      if(err != noErr)
         *theIcon = NULL;
   }
   return noErr;
}



// ---------------------------------------------------------------------------
//		¥ _CopyEachIcon()
// ---------------------------------------------------------------------------
//
OSErr CIconSuiteManager::_CopyEachIcon(Handle theSuite) {
   IconActionUPP copyOneIconProc;
   OSErr err;
   copyOneIconProc = NewIconActionUPP( CopyOneIcon );
   err = ::ForEachIconDo(theSuite, svAllAvailableData, copyOneIconProc, NULL);
   #if TARGET_CPU_68K
   #else
   ::DisposeRoutineDescriptor( copyOneIconProc );
   #endif
   return err;
}


// ---------------------------------------------------------------------------
//		¥ GetIconProc()
// ---------------------------------------------------------------------------
//  This is an IconAction procedure to fill in one slot of an icon suite, 
//  given a file type, creator, and desktop database.
static	pascal OSErr GetIconProc(ResType theType, Handle *theIcon, void *yourDataPtr) {
   OSErr err;
   GetIconData *data;
   DTPBRec deskRec;

   err = noErr;
   data = reinterpret_cast<GetIconData*>(yourDataPtr);
   *theIcon = ::NewHandle( kLarge8BitIconSize );

   if( !(*theIcon) ) {
      err = memFullErr;
   } else {
      ::HLock( *theIcon );
	
      deskRec.ioDTRefNum = data->DTRefNum;
      deskRec.ioDTBuffer = **theIcon;
      deskRec.ioDTReqCount = kLarge8BitIconSize;
      deskRec.ioFileCreator = data->fileCreator;
      deskRec.ioFileType = data->fileType;
	
      switch( theType ) {
         case large1BitMask:
            deskRec.ioIconType = kLargeIcon;
         break;
	     case large4BitData:
            deskRec.ioIconType = kLarge4BitIcon;
         break;
         case large8BitData:
            deskRec.ioIconType = kLarge8BitIcon;
         break;
         case small1BitMask:
            deskRec.ioIconType = kSmallIcon;
         break;
         case small4BitData:
            deskRec.ioIconType = kSmall4BitIcon;
         break;
         case small8BitData:
            deskRec.ioIconType = kSmall8BitIcon;
         break;
         default:
            // The desktop database does not have "mini" icons
            deskRec.ioIconType = 1000;
         break;
      }

      err = ::PBDTGetIconSync( &deskRec );
      if(err == noErr) {
         ::HUnlock( *theIcon );
         ::SetHandleSize(*theIcon, deskRec.ioDTActCount);
      } else {
         ::DisposeHandle( *theIcon );
         *theIcon = NULL;
         err = noErr;
      }
   }
   
   return err;
}

// ---------------------------------------------------------------------------
//		¥ TestHandle()
// ---------------------------------------------------------------------------
//  
static pascal OSErr TestHandle(ResType /*theType*/, Handle *theIcon, void *yourDataPtr) {
   if(*theIcon != NULL)
      *(Boolean *)yourDataPtr = false;	// not empty!

	return noErr;
}

#endif

 
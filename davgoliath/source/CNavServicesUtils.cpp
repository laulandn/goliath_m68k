/* ==================================================================================================
 * CNavServicesUtils.cpp															   
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
 
 #include <CNavServicesUtils.h>
 #include <Navigation.h>
 #include <TextUtils.h>
 #include <UConditionalDialogs.h>
 
 #include "UClassicDialogs.h"

#if !TARGET_API_MAC_CARBON
static bool sCheck8_1 = false;
static bool sIs8_1 = false;

static bool Is8_1() {
	SInt32 sysVersion;
	OSErr err = ::Gestalt(gestaltSystemVersion, &sysVersion);
	sCheck8_1 = err != noErr;
	sIs8_1 = sysVersion <= 0x0810;
	return sIs8_1;
}

#endif

// ---------------------------------------------------------------------------------
//		¥ getfileNavSrv()
// ---------------------------------------------------------------------------------
//
Boolean CNavServicesUtils::getfileNavSrv(FSSpec *theSpec, ResIDT strListId, ResIDT strId, OSType *openType, OSType *openComp, OSType* secondOpenType) {
   Boolean retVal = false;

#if !TARGET_API_MAC_CARBON
   if (Is8_1()) {
      return UClassicDialogs::AskChooseOneFile(*openType, *theSpec, kNavDefaultNavDlogOptions);
   }
#endif

   NavReplyRecord       theReply;
   NavDialogOptions     dialogOptions;
   OSErr                theErr = noErr;
   NavTypeListHandle    openList = NULL;

   theErr = NavGetDefaultDialogOptions(&dialogOptions);
   dialogOptions.dialogOptionFlags -= kNavAllowMultipleFiles;	//	only a single file
   GetIndString(dialogOptions.message, strListId, strId);
   int numTypes=1;
   if (secondOpenType)
      numTypes++;
      
   if ((NULL != openComp) && (NULL != openType)) {
     openList = (NavTypeListHandle) NewHandleClear ( sizeof (NavTypeList) + 
																(sizeof (OSType) * numTypes ));
     if (nil != openList) {
        (*openList)->componentSignature = *openComp;
	    (*openList)->reserved = 0;
        (*openList)->osTypeCount = 1;
        (*openList)->osType[0] = *openType;
        if (secondOpenType) {
           (*openList)->osTypeCount = 2;
           (*openList)->osType[1] = *secondOpenType;
        }
     }
   }
   theErr = NavChooseFile(NULL, &theReply, &dialogOptions, NULL, NULL, NULL,
							openList, NULL);	
   if (theReply.validRecord && (theErr == noErr)) {
      AEKeyword keyword;
      DescType actualType;
      Size actualSize;
      if (noErr == AEGetNthPtr ( & theReply.selection, 1, typeFSS, & keyword, & actualType, theSpec, 
           sizeof ( FSSpec ), & actualSize))
           retVal = true;
      NavDisposeReply(&theReply);
   }
   if (nil != openList)
      DisposeHandle((Handle)openList);
   
   return retVal;
}

// ---------------------------------------------------------------------------------
//		¥ savefileNavSrv()
// ---------------------------------------------------------------------------------
//
Boolean CNavServicesUtils::savefileNavSrv(FSSpec *outSpec, ResIDT strList, ResIDT strId,
                                          OSType fileType, OSType creator) {
   Boolean retVal = false;

#if !TARGET_API_MAC_CARBON
   if (Is8_1()) {
    	bool replacing;
		return UClassicDialogs::AskSaveFile("\p", fileType, *outSpec, replacing, kNavDefaultNavDlogOptions);
	 }
#endif
   
   NavReplyRecord       theReply;
   NavDialogOptions     dialogOptions;
   OSErr                theErr = noErr;
   NavTypeListHandle    openList = NULL;

   theErr = NavGetDefaultDialogOptions(&dialogOptions);
   dialogOptions.dialogOptionFlags -= kNavAllowMultipleFiles;	//	only a single file
   dialogOptions.dialogOptionFlags += kNavNoTypePopup;
   dialogOptions.dialogOptionFlags += kNavDontAddTranslateItems;
   dialogOptions.dialogOptionFlags -= kNavAllowStationery;
   GetIndString(dialogOptions.message, strList, strId);//str_UIStrings, str_SelUploadFile);
   //openList = (NavTypeListHandle) NewHandleClear ( sizeof (NavTypeList) + 
	/*															(sizeof (OSType) ));
   if (nil != openList) {
      (*openList)->componentSignature = 0;
	  (*openList)->reserved = 0;
      (*openList)->osTypeCount = 1;
      (*openList)->osType[0] = 'APPL';
   }*/

   theErr = NavPutFile (NULL, &theReply, &dialogOptions, NULL, fileType, creator,/*DocTypeCode, AppCreatorCode,*/ NULL);

   if (theReply.validRecord && (theErr == noErr)) {
      AEKeyword keyword;
      DescType actualType;
      Size actualSize;
      if (noErr == AEGetNthPtr ( & theReply.selection, 1, typeFSS, & keyword, & actualType, outSpec, 
           sizeof ( FSSpec ), & actualSize))
           retVal = true;
      NavDisposeReply(&theReply);

   }
   if (nil != openList)
      DisposeHandle((Handle)openList);
   
   return retVal;
}

// ---------------------------------------------------------------------------------
//		¥ getfolderNavSrv()
// ---------------------------------------------------------------------------------
//
Boolean CNavServicesUtils::getfolderNavSrv(FSSpec *outSpec, ResIDT strList, ResIDT strId) {
   Boolean retVal = false;
   
#if !TARGET_API_MAC_CARBON
   if (Is8_1()) {
      SInt32 dum;
      return UClassicDialogs::AskChooseFolder(*outSpec, dum);
   }
#endif
   
   NavReplyRecord       theReply;
   NavDialogOptions     dialogOptions;
   OSErr                theErr = noErr;
   NavTypeListHandle    openList = NULL;

   theErr = NavGetDefaultDialogOptions(&dialogOptions);
   dialogOptions.dialogOptionFlags -= kNavAllowMultipleFiles;	//	only a single file
   GetIndString(dialogOptions.message, strList, strId);//str_UIStrings, str_DownloadSelFolder);
   //openList = (NavTypeListHandle) NewHandleClear ( sizeof (NavTypeList) + 
	/*															(sizeof (OSType) ));
   if (nil != openList) {
      (*openList)->componentSignature = 0;
	  (*openList)->reserved = 0;
      (*openList)->osTypeCount = 1;
      (*openList)->osType[0] = 'APPL';
   }*/

   theErr = NavChooseFolder(NULL, &theReply, &dialogOptions, NULL, NULL, NULL);
							
   if (theReply.validRecord && (theErr == noErr)) {
      AEKeyword keyword;
      DescType actualType;
      Size actualSize;
      if (noErr == AEGetNthPtr ( & theReply.selection, 1, typeFSS, & keyword, & actualType, outSpec, 
           sizeof ( FSSpec ), & actualSize))
           retVal = true;
      NavDisposeReply(&theReply);
      
   }
   
   if (nil != openList)
      DisposeHandle((Handle)openList);
   
   return retVal;
}
 
// ---------------------------------------------------------------------------------
//		¥ getApplicationNavSrv
// ---------------------------------------------------------------------------------
//
Boolean CNavServicesUtils::getApplicationNavSrv(FSSpec *outSpec, ResIDT strList, ResIDT strId) {
   Boolean retVal = false;
   
#if !TARGET_API_MAC_CARBON
   if (Is8_1()) {
      return UClassicDialogs::AskChooseOneFile('APPL', *outSpec, kNavDefaultNavDlogOptions);
   }
#endif

   NavReplyRecord       theReply;
   NavDialogOptions     dialogOptions;
   OSErr                theErr = noErr;
   NavTypeListHandle    openList = NULL;

   theErr = NavGetDefaultDialogOptions(&dialogOptions);
   dialogOptions.dialogOptionFlags -= kNavAllowMultipleFiles;	//	only a single file
   dialogOptions.dialogOptionFlags += kNavSupportPackages;
   
   GetIndString(dialogOptions.message, strList, strId);

   openList = (NavTypeListHandle) NewHandleClear ( sizeof (NavTypeList) + 
																(sizeof (OSType) * 1 ));
   if (nil != openList) {
	 (*openList)->reserved = 0;
     (*openList)->osTypeCount = 1;
     (*openList)->osType[0] = 'APPL';
   }
   
   theErr = NavChooseFile(NULL, &theReply, &dialogOptions, NULL, NULL, NULL,
							openList, NULL);	
   if (theReply.validRecord && (theErr == noErr)) {
      AEKeyword keyword;
      DescType actualType;
      Size actualSize;
      if (noErr == AEGetNthPtr ( & theReply.selection, 1, typeFSS, & keyword, & actualType, outSpec, 
           sizeof ( FSSpec ), & actualSize))
           retVal = true;
      NavDisposeReply(&theReply);
   }
   if (nil != openList)
      DisposeHandle((Handle)openList);
   
   return retVal;

}
      
// ---------------------------------------------------------------------------------
//		¥ _AEGetDescData
// ---------------------------------------------------------------------------------
//
OSErr CNavServicesUtils::_AEGetDescData(const AEDesc *desc, DescType *typeCode, void *dataBuffer,
                                       ByteCount maximumSize, ByteCount *actualSize) {

	Handle h = (Handle)desc->dataHandle;
	ByteCount dataSize = GetHandleSize(h);
	ByteCount theSize;
		
	theSize = (dataSize < maximumSize) ? dataSize : maximumSize;

	if (typeCode)
		*typeCode = desc->descriptorType;
			
	if (actualSize)
		*actualSize = theSize;
		
	if (h && *h && dataBuffer)
		BlockMoveData(*h, dataBuffer, theSize);
		
	return noErr;                                       
}                                       

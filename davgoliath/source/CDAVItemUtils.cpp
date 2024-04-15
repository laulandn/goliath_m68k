/* ==================================================================================================
 * CDAVItemUtils.cpp														   
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

#include "CDAVItemUtils.h"
#include <CDAVItem.h>
#include <algorithm>
#include "CDAVTableApp.h"
#include "CGoliathPreferences.h"
#include "string.h"
#include <stdio.h>

bool greater_than(CDAVItem& first, CDAVItem &second);
UInt32 _getNumberOfChildren(FSSpec *theSpec);

// ---------------------------------------------------------------------------
//		¥ greater_than()
// ---------------------------------------------------------------------------
//	
bool greater_than(CDAVItem& first, CDAVItem &second) {
   const LStr255 &firstHref=first.GetPHREF(), &secHref=second.GetPHREF();
   SInt16 val = firstHref.CompareTo(secHref);
   if (0 == val)
      return false;
   else if (0<val)
      return false;
   else
      return true;
}

// ---------------------------------------------------------------------------
//		¥ PruneItemVector()
// ---------------------------------------------------------------------------
//
CDAVItemVector CDAVItemUtils::PruneItemVector(CDAVItemVector &items) {
   LStr255 prevStr;
   Boolean hasPrevStr = false;

   CDAVItemVector tmpItems(items), retVector;   
   std::sort(tmpItems.begin(), tmpItems.end(), greater_than);
   CDAVItemVector::iterator iiter;
   for (iiter = tmpItems.begin(); iiter != tmpItems.end(); iiter++) {
      if (false == hasPrevStr) {
         hasPrevStr = true;
         prevStr = iiter->GetPHREF();
         retVector.push_back(*iiter);
      } else {
         if (iiter->GetPHREF().BeginsWith(prevStr)== false) {
            prevStr = iiter->GetPHREF();
            retVector.push_back(*iiter);
         } 
      }
   }
   return retVector;
}


// ---------------------------------------------------------------------------
//		¥ _getNumberOfChildren()
// ---------------------------------------------------------------------------
//	
UInt32 _getNumberOfChildren(FSSpec *theSpec) {
   UInt32 retval = 0;
   CInfoPBRec cpb;
   cpb.hFileInfo.ioNamePtr = theSpec->name;
   cpb.hFileInfo.ioVRefNum = theSpec->vRefNum;
   cpb.hFileInfo.ioFDirIndex = 0;
   cpb.hFileInfo.ioDirID = theSpec->parID;
      
   OSErr result = noErr, ioResult = PBGetCatInfoSync((CInfoPBPtr) &cpb);
   if (!ioResult)	{
      if (cpb.hFileInfo.ioFlAttrib & ioDirMask)/* see if we got a directory */ {
         short index = 1;
         retval+=1;
         long dirID =  cpb.dirInfo.ioDrDirID;
         do {
            cpb.dirInfo.ioFDirIndex = index;
	        cpb.dirInfo.ioDrDirID = dirID;
	        result = PBGetCatInfoSync(&cpb);
	           
	        if (result == noErr) {
	           FSSpec nestSpec;
	           LString::CopyPStr(cpb.dirInfo.ioNamePtr, nestSpec.name, sizeof(StrFileName));
	           nestSpec.vRefNum = cpb.dirInfo.ioVRefNum;
	           nestSpec.parID = cpb.dirInfo.ioDrParID;
	           retval+=_getNumberOfChildren(&nestSpec);
	         }
	         ++index;	
	      } while (noErr == result);
	      return retval;
      } else {
         return 1;
      }
   }
   return 0;
}

// ---------------------------------------------------------------------------
//		¥ CalculateTotalTransactions()
// ---------------------------------------------------------------------------
//	
UInt32 CDAVItemUtils::CalculateTotalTransactions(FSSpec *theSpec) {
   return _getNumberOfChildren(theSpec);
}

// ---------------------------------------------------------------------------
//		¥ GetURIForAppleDoubleResourceFork()
// ---------------------------------------------------------------------------
//	
void CDAVItemUtils::GetURIForAppleDoubleResourceFork( const CDAVItem& inItem, 
                                                      std::string& outRezForkURI) {
	outRezForkURI.clear();
    inItem.GetParentPath(outRezForkURI);
    if (outRezForkURI[outRezForkURI.size()-1] != '/')
       outRezForkURI+='/';
    outRezForkURI+="._";
    std::string fName;
    inItem.GetFileName(fName);
    outRezForkURI+=fName;
}

// ---------------------------------------------------------------------------
//		¥ ProcessDotFiles()
// ---------------------------------------------------------------------------
//	
void CDAVItemUtils::ProcessDotFiles(CDAVItemVector& inVector, CDAVItemVector& outVector) {
     bool showHidden = false;
      std::string showDotFiles = GetApplicationInstance()->GetPreferencesManager()->GetPrefValue(CGoliathPreferences::SHOWDOTFILES);
     if (strcmp(showDotFiles.c_str(), CGoliathPreferences::TRUE_VALUE)==0)
        showHidden = true;

      for (CDAVItemVector::iterator iter = inVector.begin(); iter != inVector.end(); ++iter) {
      	 if (!showHidden) {
         	 CDAVItem* theItem = &*iter;
         	 std::string fname;
        	 theItem->GetFileName(fname);
      	    if (fname.size() > 0 && fname[0]!= '.')
      	    	outVector.push_back(*iter);
      	 } else {
         	outVector.push_back(*iter);
         }
      }

}

// ---------------------------------------------------------------------------
//		¥ GetATempFile()
// ---------------------------------------------------------------------------
//	
void CDAVItemUtils::GetATempFile(FSSpec& tempFileSpec) {
	SInt16	theVRef;
	SInt32	theDirID;
	OSErr	theErr;

	ThrowIfOSErr_(::FindFolder(kOnSystemDisk, kTemporaryFolderType,
					kCreateFolder, &theVRef, &theDirID));
	
	//create a new file name from the tickcount
	//	we loop until we hit a name not in use
	do {
		LStr255 tempFileName;
		tempFileName = (SInt32)TickCount();
		theErr = ::FSMakeFSSpec(theVRef, theDirID, tempFileName, &tempFileSpec);
	} while (theErr != fnfErr);
}

// ---------------------------------------------------------------------------
//		¥ FileExists()
// ---------------------------------------------------------------------------
//	
bool CDAVItemUtils::FileExists(const FSSpec& inFileSpec) {
	FInfo	finderInfo;			// File exists if we can get its Finder info
	
	return (::FSpGetFInfo(&inFileSpec, &finderInfo) == noErr);
}

// ---------------------------------------------------------------------------
//		¥ GetUniqueFileName()
// ---------------------------------------------------------------------------
//	
LStr255 CDAVItemUtils::GetUniqueFileName(const LStr255& inBaseName, const FSSpec& parentSpec) {
   FSSpec newSpec(parentSpec);
   LString::CopyPStr(inBaseName, newSpec.name, sizeof(StrFileName));
   if (!FileExists(newSpec))
      return inBaseName;

   int i=1;
   char buf[6];
   while (1) {
      LStr255 tmpName(inBaseName);
      sprintf(buf, " %d", i);
      tmpName.Append(buf);
      LString::CopyPStr(tmpName, newSpec.name, sizeof(StrFileName));
      if (!FileExists(newSpec))
         return tmpName;
      
      i++;
   }
}


// ---------------------------------------------------------------------------
//		¥ GetDirName()
// ---------------------------------------------------------------------------
//	
OSErr CDAVItemUtils::GetDirName(short vRefNum, long dirID, Str31 name) {
	CInfoPBRec pb;
	OSErr error;

	if ( name != NULL ) {
		pb.dirInfo.ioNamePtr = name;
		pb.dirInfo.ioVRefNum = vRefNum;
		pb.dirInfo.ioDrDirID = dirID;
		pb.dirInfo.ioFDirIndex = -1;	/* get information about ioDirID */
		error = PBGetCatInfoSync(&pb);
	} else {
		error = paramErr;
	}
	
	return error;
}

// ---------------------------------------------------------------------------
//		¥ IsDirectory()
// ---------------------------------------------------------------------------
//	
bool CDAVItemUtils::IsDirectory(FSSpec* theSpec) {
   CInfoPBRec cpb;
   cpb.hFileInfo.ioNamePtr = theSpec->name;
   cpb.hFileInfo.ioVRefNum = theSpec->vRefNum;
   cpb.hFileInfo.ioFDirIndex = 0;
   cpb.hFileInfo.ioDirID = theSpec->parID;

   if (noErr != PBGetCatInfoSync((CInfoPBPtr) &cpb))
      return false;

   return (cpb.hFileInfo.ioFlAttrib & ioDirMask);
}

// ---------------------------------------------------------------------------------
//		¥ GetDirID()
// ---------------------------------------------------------------------------------
//		
bool CDAVItemUtils::GetDirID(FSSpec* theSpec, long& outDirId) {
	CInfoPBRec cpb;
	cpb.hFileInfo.ioNamePtr = theSpec->name;
	cpb.hFileInfo.ioVRefNum = theSpec->vRefNum;
	cpb.hFileInfo.ioFDirIndex = 0;
	cpb.hFileInfo.ioDirID = theSpec->parID;

	if (noErr != PBGetCatInfoSync((CInfoPBPtr) &cpb))
		return false;
	
	outDirId = cpb.dirInfo.ioDrDirID;
	
	return true;
}

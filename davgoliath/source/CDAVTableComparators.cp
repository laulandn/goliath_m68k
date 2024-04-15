/* ==================================================================================================
 * CDAVTableComparators.cp															   
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
 
#include "CDAVTableComparators.h"
#include "CDAVTable.h"
#include "CDAVTableItem.h"
#include <CDAVItem.h>
#include <CDAVProperty.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "CGoliathPreferences.h"

#include <LInternetMessage.h>

using namespace std;

SInt32 cmin(SInt32 a, SInt32 b);

SInt32 cmin(SInt32 a, SInt32 b) {return a < b ? a : b;}


// ---------------------------------------------------------------------------------
//		¥ ParseDateString
// ---------------------------------------------------------------------------------
// This version takes a char * assumed to be a RFC822 style formatted date/time
//	string and sets-up a DateTimeRec with the info.
void ParseDateString(const char * inDateTime, DateTimeRec* outDateTimeRec) {
	StPointerBlock tempDateTime((SInt32) (strlen(inDateTime) + 1));
	strcpy(tempDateTime, inDateTime);

	//Day of Week
	char * p = strtok(tempDateTime, ", ");
	if (p) {
		SInt16 pLen = (SInt16) strlen(p);
		for (UInt16 i = 0; i < 7; i++) {
			if (CompareText(p,
							kRFC822Days[i],
							pLen,
							(SInt16) strlen((char*)kRFC822Days[i]),
							nil) == 0) {
				outDateTimeRec->dayOfWeek = (SInt16) ++i;
				break;
			}
		}
	}

	//Day of month
	p = strtok(nil, " ");
	if (p)
		outDateTimeRec->day  = (SInt16) atoi(p);

	//Month
	p = strtok(nil, " ");
	if (p) {
		SInt16 pLen = (SInt16) strlen(p);
		for (UInt16 i = 0; i < 12; i++) {
			if (CompareText(p,
							kRFC822Months[i],
							pLen,
							(SInt16) strlen((char *)kRFC822Months[i]),
							nil) == 0) {
				outDateTimeRec->month = (SInt16) ++i;
				break;
			}
		}
	}

	//Year
	p = strtok(nil, " ");
	if (p)
		outDateTimeRec->year  = (SInt16) atoi(p);

	//Hour
	p = strtok(nil, ":");
	if (p)
		outDateTimeRec->hour  = (SInt16) atoi(p);

	//Minute
	p = strtok(nil, ":");
	if (p)
		outDateTimeRec->minute  = (SInt16) atoi(p);

	//Second
	p = strtok(nil, " ");
	if (p)
		outDateTimeRec->second  = (SInt16) atoi(p);
}



// ---------------------------------------------------------------------------------
//		¥ CDAVTableComparator
// ---------------------------------------------------------------------------------
//	
CDAVTableComparator::CDAVTableComparator(): m_direction(CDAVTableComparator::Ascending),
m_column(CDAVTableComparator::Name) {
	mPreferences = GetApplicationInstance()->GetPreferencesManager();
}


// ---------------------------------------------------------------------------------
//		¥ ~CDAVTableComparator
// ---------------------------------------------------------------------------------
//	
CDAVTableComparator::~CDAVTableComparator() {

}

// ---------------------------------------------------------------------------------
//		¥ Compare
// ---------------------------------------------------------------------------------
//	
SInt32	CDAVTableComparator::Compare(const void* inItemOne, const void* inItemTwo,
								UInt32 inSizeOne, UInt32 inSizeTwo) const {

#pragma unused(inSizeOne, inSizeTwo)

   CDAVTableItem *item1, *item2;
   item1 = *(CDAVTableItem**)inItemOne;
   item2 = *(CDAVTableItem**)inItemTwo;
	
//   Assert_(item1 != nil);
//   Assert_(item2 != nil);   
   
   if ((!item1) || (!item2))
      return 0;
      
   if (CDAVTableComparator::Ascending == m_direction) {
      switch (m_column) {     
         case Name:
            return CompareNameAscending(item1, item2);
         break;
         case Date:
            return CompareDateAscending(item1, item2);
         break;
         case Size:
            return CompareSizeAscending(item1, item2);
         break;
         case Kind:
            return CompareKindAscending(item1, item2);
         break;
         case LockOwner:
            return CompareLockOwnerAscending(item1, item2);
         break;
      }
   } else {
      switch (m_column) {     
         case Name:
            return CompareNameDescending(item1, item2);
         break;
         case Date:
            return CompareDateDescending(item1, item2);
         break;
         case Size:
            return CompareSizeDescending(item1, item2);
         break;
         case Kind:
            return CompareKindDescending(item1, item2);
         break;
         case LockOwner:
            return CompareLockOwnerDescending(item1, item2);
         break;
      }   
   }	
   Assert_(0);
   return 0;							
}
								
// ---------------------------------------------------------------------------------
//		¥ setSortDirection
// ---------------------------------------------------------------------------------
//	
void CDAVTableComparator::setSortDirection(SortDirection type) {
   m_direction = type;
}

// ---------------------------------------------------------------------------------
//		¥ setSortColumn
// ---------------------------------------------------------------------------------
//	
void CDAVTableComparator::setSortColumn(Column column) {
   m_column = column;
}

// ---------------------------------------------------------------------------------
//		¥ CompareNameAscending
// ---------------------------------------------------------------------------------
//	
SInt32 CDAVTableComparator::CompareNameAscending(class CDAVTableItem *item1, class CDAVTableItem *item2) const {
	std::string href1 = item1->GetItem().GetHREF();
	std::string href2 = item2->GetItem().GetHREF();

    std::string sortCaseInsensitive = mPreferences->GetPrefValue(CGoliathPreferences::SORT_CASEINSENSITIVE);
    if (strcmp(sortCaseInsensitive.c_str(), CGoliathPreferences::TRUE_VALUE)==0) {
		for (int i=0; i<href1.size(); i++) {
			if (isupper(href1[i]))
				href1[i] = tolower(href1[i]);
		}
		for (int i=0; i<href2.size(); i++) {
			if (isupper(href2[i]))
				href2[i] = tolower(href2[i]);
		}
	}

	return href1.compare (href2);
}

// ---------------------------------------------------------------------------------
//		¥ CompareNameDescending
// ---------------------------------------------------------------------------------
//	
SInt32 CDAVTableComparator::CompareNameDescending(class CDAVTableItem *item1, class CDAVTableItem *item2) const {
	std::string href1 = item1->GetItem().GetHREF();
	std::string href2 = item2->GetItem().GetHREF();

    std::string sortCaseInsensitive = mPreferences->GetPrefValue(CGoliathPreferences::SORT_CASEINSENSITIVE);
    if (strcmp(sortCaseInsensitive.c_str(), CGoliathPreferences::TRUE_VALUE)==0) {
		for (int i=0; i<href1.size(); i++) {
			if (isupper(href1[i]))
				href1[i] = tolower(href1[i]);
		}
		for (int i=0; i<href2.size(); i++) {
			if (isupper(href2[i]))
				href2[i] = tolower(href2[i]);
		}
	}

	return href2.compare (href1);
}

// ---------------------------------------------------------------------------------
//		¥ CompareDateAscending
// ---------------------------------------------------------------------------------
//	
SInt32 CDAVTableComparator::CompareDateAscending(class CDAVTableItem *item1, class CDAVTableItem *item2) const {
   std::string &date1 = item1->GetItem().GetPropertyValue(LastModified), &date2 = item2->GetItem().GetPropertyValue(LastModified);   
   
   DateTimeRec dtrec1, dtrec2;
   unsigned long dtsecs1, dtsecs2;
   
   memset(&dtrec1, 0, sizeof(DateTimeRec));
   memset(&dtrec2, 0, sizeof(DateTimeRec));
   
   ParseDateString(date1.c_str(), &dtrec1);
   ParseDateString(date2.c_str(), &dtrec2);
   DateToSeconds(&dtrec1, &dtsecs1);
   DateToSeconds(&dtrec2, &dtsecs2);
   
   if (dtsecs1 == dtsecs2)
      return 0;
   if (dtsecs1 > dtsecs2)
      return 1;
   return -1;
   
}

// ---------------------------------------------------------------------------------
//		¥ CompareDateDescending
// ---------------------------------------------------------------------------------
//	
SInt32 CDAVTableComparator::CompareDateDescending(class CDAVTableItem *item1, class CDAVTableItem *item2) const {
   std::string &date1 = item1->GetItem().GetPropertyValue(LastModified), &date2 = item2->GetItem().GetPropertyValue(LastModified);   

   DateTimeRec dtrec1, dtrec2;
   unsigned long dtsecs1, dtsecs2;
   
   memset(&dtrec1, 0, sizeof(DateTimeRec));
   memset(&dtrec2, 0, sizeof(DateTimeRec));
   
   ParseDateString(date1.c_str(), &dtrec1);
   ParseDateString(date2.c_str(), &dtrec2);
   DateToSeconds(&dtrec1, &dtsecs1);
   DateToSeconds(&dtrec2, &dtsecs2);

   if (dtsecs1 == dtsecs2)
      return 0;
   if (dtsecs1 > dtsecs2)
      return -1;
   return 1;
}

// ---------------------------------------------------------------------------------
//		¥ CompareSizeAscending
// ---------------------------------------------------------------------------------
//	
SInt32 CDAVTableComparator::CompareSizeAscending(class CDAVTableItem *item1, class CDAVTableItem *item2) const {
   std::string &size1 = item1->GetItem().GetPropertyValue(ContentLength), &size2 = item2->GetItem().GetPropertyValue(ContentLength);   
   int s1 = atoi(size1.c_str()), s2 = atoi(size2.c_str());
   if (s1 == s2)
      return 0;
   if (s1 > s2)
      return 1;
   return -1;
}

// ---------------------------------------------------------------------------------
//		¥ CompareSizeDescending
// ---------------------------------------------------------------------------------
//	
SInt32 CDAVTableComparator::CompareSizeDescending(class CDAVTableItem *item1, class CDAVTableItem *item2) const {
   std::string &size1 = item1->GetItem().GetPropertyValue(ContentLength), &size2 = item2->GetItem().GetPropertyValue(ContentLength);   
   int s1 = atoi(size1.c_str()), s2 = atoi(size2.c_str());
   if (s1 == s2)
      return 0;
   if (s1 > s2)
      return -1;
   return 1;
}

// ---------------------------------------------------------------------------------
//		¥ CompareKindAscending
// ---------------------------------------------------------------------------------
//	
SInt32 CDAVTableComparator::CompareKindAscending(class CDAVTableItem *item1, class CDAVTableItem *item2) const {
   CDAVItem::ItemType t1 = item1->GetItem().GetItemType(), t2 = item2->GetItem().GetItemType();
   if (t1 == t2)
      return 0;
   if (t1 > t2)
      return 1;
   return -1;
}

// ---------------------------------------------------------------------------------
//		¥ CompareKindDescending
// ---------------------------------------------------------------------------------
//	
SInt32 CDAVTableComparator::CompareKindDescending(class CDAVTableItem *item1, class CDAVTableItem *item2) const {
   CDAVItem::ItemType t1 = item1->GetItem().GetItemType(), t2 = item2->GetItem().GetItemType();
   if (t1 == t2)
      return 0;
   if (t1 > t2)
      return -1;
   return 1;

}

// ---------------------------------------------------------------------------------
//		¥ CompareLockOwnerAscending
// ---------------------------------------------------------------------------------
//	
SInt32 CDAVTableComparator::CompareLockOwnerAscending(class CDAVTableItem *item1, class CDAVTableItem *item2) const {
   std::string t1 =  item1->GetItem().GetLockOwner(), t2 =  item2->GetItem().GetLockOwner();

    std::string sortCaseInsensitive = mPreferences->GetPrefValue(CGoliathPreferences::SORT_CASEINSENSITIVE);
    if (strcmp(sortCaseInsensitive.c_str(), CGoliathPreferences::TRUE_VALUE)==0) {
		for (int i=0; i<t1.size(); i++) {
			if (isupper(t1[i]))
				t1[i] = tolower(t1[i]);
		}
		for (int i=0; i<t2.size(); i++) {
			if (isupper(t2[i]))
				t2[i] = tolower(t2[i]);
		}
	}

   if (t1 == t2)
      return 0;
   if (t1 > t2)
      return 1;
   return -1;
}

// ---------------------------------------------------------------------------------
//		¥ CompareLockOwnerDescending
// ---------------------------------------------------------------------------------
//	
SInt32 CDAVTableComparator::CompareLockOwnerDescending(class CDAVTableItem *item1, class CDAVTableItem *item2) const {
   std::string t1 =  item1->GetItem().GetLockOwner(), t2 =  item2->GetItem().GetLockOwner();
   std::string sortCaseInsensitive = mPreferences->GetPrefValue(CGoliathPreferences::SORT_CASEINSENSITIVE);
    if (strcmp(sortCaseInsensitive.c_str(), CGoliathPreferences::TRUE_VALUE)==0) {
		for (int i=0; i<t1.size(); i++) {
			if (isupper(t1[i]))
				t1[i] = tolower(t1[i]);
		}
		for (int i=0; i<t2.size(); i++) {
			if (isupper(t2[i]))
				t2[i] = tolower(t2[i]);
		}
	}

   if (t1 == t2)
      return 0;
   if (t1 > t2)
      return -1;
   return 1;

}


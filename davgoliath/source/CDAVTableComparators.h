/* ==================================================================================================
 * CDAVTableComparators.h													   
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
#ifndef __CDAVTABLECOMPARATOR_H__
#define __CDAVTABLECOMPARATOR_H__

#include <LComparator.h>
#include <LString.h>

class CDAVTableComparator : public LComparator
{
	public:
	   enum SortDirection {
	      Ascending  = 0,
	      Descending = 1
	   };
	   
	   enum Column {
	      Name = 0,
	      Date = 1,
	      Size = 2,
	      Kind = 3,
	      LockOwner=4
	   };
	   
						CDAVTableComparator();
		virtual			~CDAVTableComparator();
		virtual SInt32	Compare(const void* inItemOne, const void* inItemTwo,
								UInt32 inSizeOne, UInt32 inSizeTwo) const;
								
		void setSortDirection(SortDirection type);
	    void setSortColumn(Column column);
		
     protected:
        SInt32 CompareNameAscending(class CDAVTableItem *item1, class CDAVTableItem *item2) const;
        SInt32 CompareNameDescending(class CDAVTableItem *item1, class CDAVTableItem *item2) const;
        SInt32 CompareDateAscending(class CDAVTableItem *item1, class CDAVTableItem *item2) const;
        SInt32 CompareDateDescending(class CDAVTableItem *item1, class CDAVTableItem *item2) const;
        SInt32 CompareSizeAscending(class CDAVTableItem *item1, class CDAVTableItem *item2) const;
        SInt32 CompareSizeDescending(class CDAVTableItem *item1, class CDAVTableItem *item2) const;
        SInt32 CompareKindAscending(class CDAVTableItem *item1, class CDAVTableItem *item2) const;
        SInt32 CompareKindDescending(class CDAVTableItem *item1, class CDAVTableItem *item2) const;
        SInt32 CompareLockOwnerAscending(class CDAVTableItem *item1, class CDAVTableItem *item2) const;
        SInt32 CompareLockOwnerDescending(class CDAVTableItem *item1, class CDAVTableItem *item2) const;
								
   	    SortDirection m_direction;
   	    Column        m_column;
   	    LStr255 mCollectionDisplay, mFileDisplay;
   	    class CGoliathPreferences*	mPreferences;
};

void ParseDateString(const char * inDateTime, DateTimeRec* outDateTimeRec);

#endif



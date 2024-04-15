/* ==================================================================================================
 * DAVMessages.h															   
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
 
 #ifndef __DAVMESSAGES_H__
 #define __DAVMESSAGES_H__
 
 #include <LWindow.h>
 #include <string>
 
 
/*
   Central specification of the particular LBroadcaster/LListener
   messages and data types that are braodcasted within this application
   as DAV operations succeed and or fail.

   In particular, each MessageT defined here has a corresponding 
   struct defined that is the ioParam for the broadcast.
   
   msg_<message-id> has a ioParam type of msg_<message-id>Struct
   
*/

// ---------------------------------------------------------------------------
//		¥ msg_DAVItemAdded
const MessageT  msg_DAVItemAdded        = 2000;
typedef struct {
   class CDAVItem* item;
   class LOutlineItem*   parent;
   LWindow*  originatingWindow;
} msg_DAVItemAddedStruct;

// ---------------------------------------------------------------------------
//		¥ msg_DAVItemAdded
const MessageT  msg_DAVItemsAdded        = 2001;
typedef struct {
   class CDAVItemVector* items;
   class LOutlineItem*   parent;
   LWindow*              originatingWindow;
} msg_DAVItemsAddedStruct;


// ---------------------------------------------------------------------------
//		¥ msg_DAVItemChanged
const MessageT  msg_DAVItemChanged      = 2002;
typedef struct {
   class CDAVItem* item;
   LWindow*  originatingWindow;
} msg_DAVItemChangedStruct;

// ---------------------------------------------------------------------------
//		¥ msg_DAVItemLocked
const MessageT  msg_DAVItemLocked       = 2003;
typedef struct {
   class CDAVItem* item;
   LWindow*  originatingWindow;
} msg_DAVItemLockedStruct;

// ---------------------------------------------------------------------------
//		¥ msg_DAVItemUnlocked
const MessageT  msg_DAVItemUnlocked     = 2004;
typedef struct {
   class CDAVItem* item;
   LWindow*  originatingWindow;
} msg_DAVItemUnlockedStruct;


// ---------------------------------------------------------------------------
//		¥ msg_DAVItemDeleted
const MessageT  msg_DAVItemDeleted      = 2005;
typedef struct {
   std::string   href;
   LWindow*  originatingWindow;   
} msg_DAVItemDeletedStruct;

// ---------------------------------------------------------------------------
//		¥ msg_DAVResetTransaction
const MessageT msg_DAVResetTransaction  = 2006;
typedef struct {
   class CDAVTableItem* activeItem;
   LWindow*  originatingWindow;   
} msg_DAVResetTransactionStruct;

// ---------------------------------------------------------------------------
//		¥ msg_DAVItemRenamed
const MessageT msg_DAVItemRenamed  = 2007;
typedef struct {
   class CDAVItem* item;
   std::string originalHREF;
} msg_DAVItemRenamedStruct;


// ---------------------------------------------------------------------------
//		¥ msg_HTTPStart
const MessageT  msg_HTTPStart           = 2010;
typedef struct {
   LWindow *originatingWindow;
} msg_HTTPStartStruct;

// ---------------------------------------------------------------------------
//		¥ msg_HTTPEnd
const MessageT  msg_HTTPEnd             = 2011;
typedef struct {
   LWindow *originatingWindow;
} msg_HTTPEndStruct;

// ---------------------------------------------------------------------------
//		¥ msg_DAVConnectionInfo
const MessageT  msg_DAVConnectionInfo  = 2012;
typedef struct {
   LWindow *originatingWindow;
   class CDAVInfo *connectionInfo;
} msg_DAVConnectionInfoStruct;
#endif
 
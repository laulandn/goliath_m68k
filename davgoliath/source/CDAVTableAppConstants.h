/* ==================================================================================================
 * CDAVTableAppConstants.h															   
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

#ifndef __CDAVTABLECONSTANTS_H__
#define __CDAVTABLECONSTANTS_H__

#pragma once
#include <UTables.h>

/* Application Commands */
#define OPENIDISKCMD			  9000
#define NEWFOLDERCMD             10000
#define UPLOADITEMCMD            10001
#define DOWNLOADITEMSCMD         10002
#define DOWNLOADITEMWITHOPTCMD   10003
#define LOCKITEMSCMD             10004
#define LOCKANDDOWNLOADITEMSCMD  10005
#define UNLOCKITEMSCMD           10006
#define UNLOCKANDUPLOADITEMSCMD  10007
#define DELETEITEMSCMD           10008
#define REFRESHVIEWCMD           10009
#define EDITPROPSCMD             10010
#define MODDAV_SETEXECUTABLETRUE 10015
#define MODDAV_SETEXECUTABLEFALSE 10016
#define EDITITEM_SUBMENU             10017
#define LOCKS_SUBMENU             10018
#define VIEWLOCKINFOCMD          11004
#define CLAIMLOCKCMD             11005
#define SYNCHRONIZEITEMCMD       11007
#define DUPLICATEITEMCMD         11008
#define CMD_COPYITEMSURL         11009
#define LOGHTTPREQUESTSCMD       11010
#define EDITCONNECTIONSETTINGS   11011
#define EDITCLIENTCERTIFICATES	 11012


#define EDIT_IN_APP_CFG			14000
#define EDIT_IN_APP_FILES		14001
#define EDIT_IN_APP_CHECK_EDT		14002
#define EDIT_IN_APP_START		14003
#define EDIT_IN_APP_RANGE		100
#define EDIT_IN_APP_END			EDIT_IN_APP_START + EDIT_IN_APP_RANGE

/*  Client Settings Dialog */
#define CLIENTSETTINGSDLOG 1202
#define CLIENTSETTINGSDLOG_CLSID 'CONS'
#define CSDADVOPTVIEW 'PPNE'
#define CSDHOSTEDITFIELD 'HOST'
#define CSDUSEREDITFIELD 'USER'
#define CSDPASSEDITFIELD 'PASS'
#define CSDPROXYCHECKBOX 'PROX'
#define CSDPROXYHOSTEDITFIELD 'PHST'
#define CSDPROXYPORTEDITFIELD 'PPRT'
#define CSDPROXYAUTCHCHECKBOX 'PATH'
#define CSDPROXYUSEREDITFIELD 'PUSR'
#define CSDPROXYPASSEDITFIELD 'PPSS'
#define CSDOKBUTTON 'OPEN'
#define CDSAPPLEDOUBLECHECKBOX 'ADBL'

/*  New Folder Dialog */
#define NEWFOLDERNAMEDLOG 1301
#define NEWFOLDEREDITFIELD 'RRES'
#define URLOPTSDLOG 1302
#define URLOPTSEDITFIELD 'RRES'


/* Preferences Dialog */
#define PREFSDLOG       1307
#define PREFSOWNERFIELD 'LOWN'
#define PREFSNEWCONN    'OCON'
#define PREFSOPENCONN   'NCON'
#define PREFSIDISKCONN  'IDSK'
#define PREFSNONE       'NONE'

/* View Locks Dialog */
#define VIEWLOCKSDLOG    1308
#define VIEWLOCKSDLOGURLFIELD  'URI1'
#define VIEWLOCKSDLOGTYPE 'type'
#define VIEWLOCKSDLOGSCOPE 'scop'
#define VIEWLOCKSDLOGDURATION 'llen'
#define VIEWLOCKSDLOGTOKEN 'tokn'
#define VIEWLOCKSDLOGOWNER 'lown'

/* View/Edit Properties Dialog */
#ifdef PP_Target_Carbon
#define EDITPROPSDLOG    1402
#else
#define EDITPROPSDLOG    1400
#endif

// iDisk settings dlog
#define IDISKSETTINGSDLOG 1403

const PaneIDT EDITPROPSDLOGTBLTHDR	= FOUR_CHAR_CODE('dhdr');
const PaneIDT EDITPROPSDLOGTABLE	= FOUR_CHAR_CODE('flvt');
const PaneIDT EDITPROPSDLOGTEXTHDR  = FOUR_CHAR_CODE('hdrt');
const PaneIDT EDITPROPSDLOGSORT     = FOUR_CHAR_CODE('sort');
const MessageT kPropNameColMessage  = FOUR_CHAR_CODE('pnme');
const MessageT kPropValColMessage   = FOUR_CHAR_CODE('pval');
#define ADDNEWPROPDLOG   1401
#define NEWPROP_CMD      1000
const PaneIDT PNAME_EDITFIELD     = FOUR_CHAR_CODE('NAME');
const PaneIDT PNAMESPACE_EDITFIELD     = FOUR_CHAR_CODE('PNSP');
const PaneIDT PVAL_EDITFIELD     = FOUR_CHAR_CODE('PVAL');
const PaneIDT PVAL_OKBUTTON     = FOUR_CHAR_CODE('OPEN');


//SSL Client Certs dialog
#define CLIENTCERTSDLOG	1409
//SSL Client Cert Password dialog
#define CLIENTCERTPASSWORDDLOG 1410


/* Main Frame */
const TableIndexT kNameColumn = 1;
const TableIndexT kDateColumn = 2;
const TableIndexT kSizeColumn = 3;
const TableIndexT kKindColumn = 4;
const TableIndexT kLockOwnerColumn = 5;

const UInt16 kDefaultNameColumnWidth = 235;
const UInt16 kDefaultDateColumnWidth = 178;
const UInt16 kDefaultSizeColumnWidth = 70;
const UInt16 kDefaultKindColumnWidth = 100;
const UInt16 kDefaultLockOwnerColumnWidth = 120;

const MessageT kNameColMessage = FOUR_CHAR_CODE('name');
const MessageT kDateColMessage = FOUR_CHAR_CODE('date');
const MessageT kSizeColMessage = FOUR_CHAR_CODE('size');
const MessageT kKindColMessage = FOUR_CHAR_CODE('kind');
const MessageT kLockOwnerColMessage = FOUR_CHAR_CODE('lown');
const MessageT kSortMessage = FOUR_CHAR_CODE('sort');

const PaneIDT kDAVWindowHeading	= FOUR_CHAR_CODE('dhdr');
const PaneIDT kDAVTable	= FOUR_CHAR_CODE('flvt');
const PaneIDT kRootLockIcon	= FOUR_CHAR_CODE('LOCK');
const PaneIDT kActivityArrows = FOUR_CHAR_CODE('prg1');



/** menu resource ids **/
const ResIDT kFileMenuID       = 129;
const ResIDT kEditMenuID       = 130;
const ResIDT kLogMenuResId     = 8;

const ResIDT kWebMenuID        = 131;
const ResIDT kDeleteMenuItemID = 6;
const ResIDT kWindowMenuResID  = 132;

/** STR resouce constants **/
const ResIDT str_FileNameStrings    = 1002;
const ResIDT str_GoliathLockDatabase = 1;
const ResIDT str_GoliathPreferences  = 2;
const ResIDT str_iDiskHostName  = 3;
const ResIDT str_iDiskPathString  = 4;
const ResIDT str_GoliathSSLCertDBFileName = 5;
const ResIDT str_GoliathClientCertDBFileName = 6;
const ResIDT str_GoliathExternalAppListFileName = 7;
const ResIDT str_GoliathExternalAppEditRootName = 8;
const ResIDT str_GoliathExternalEditListFileName = 9;

/** Locking Icons Resource Ids **/
const SInt16 kLocalUserLockIcon = 133;
const SInt16 kLockIcon = 134;

/** mod_dav 1.0 executable support icons **/
const SInt16 kExecutableIcon = 136;

/* Editor*/
const ResIDT		PPob_TextWindow					= 1000;
const PaneIDT		kTextScroller					= 1;
const PaneIDT		kTextView						= 2;

const ResIDT		PPob_TextPrintout				= 1100;
const PaneIDT		kTextPlaceholder				= 1;

#endif

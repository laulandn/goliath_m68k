/* ==================================================================================================
 * ParseConnectionDocument.h														   
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
 #ifndef __PARSECONNECTIONDOCUMENT_H__
 #define __PARSECONNECTIONDOCUMENT_H__
 
 // James Clark's Expat parser 
 #include <xmlparse.h>
 #include <vector>
 #include <string>
 #include <LPane.h>
 
 //***teb - FIX THIS; make all std::string objects here
 typedef struct {
    std::string mResource;
    std::string mHost;
    SInt32  mPort;
    Boolean mHasUserCredentials;
    LStr255 mUser;
    LStr255 mPassword;
    Boolean mHasProxy;
    LStr255 mProxyHost;
    SInt32  mProxyPort; 
    std::vector<UInt16> mColWidths;
    SDimension16 mFrameSize;
    Point mFrameLocation;
    Boolean mHasFrameInformation;
    Boolean mHasColumnInformation;
    Boolean mHasProxyCredentials;
    LStr255 mProxyUser;
    LStr255 mProxyPassword;
    Boolean mForceSecure;
    Boolean mEncodeAppleDouble;
 } ConnectionDocumentData;
 

extern const char *CONNECTION_INFO;
extern const char *URI;
extern const char *HOST;
extern const char *PORT;
extern const char *PROXYINFO;
extern const char *PROXY_PORT;
extern const char *PROXY_HOST;
extern const char *CREDENTIALS;
extern const char *PROXYCREDENTIALS;
extern const char *LOGIN;
extern const char *PASSWORD;
extern const char *FORCESECURE;
extern const char *DISABLEAPPLEDOUBLE;



								
void InitConnectionDocumentData(ConnectionDocumentData& inDocData);

XML_Error ParseConnectionDocument(FSSpec *theDoc, ConnectionDocumentData *docData);
XML_Error ParseConnectionData(std::string& inStr, ConnectionDocumentData *docData);

Boolean WriteConnectionDocument(std::string& resource, class CDAVContext *context, 
                                 SDimension16& frameSize, Point& frameLocation,
                                 FSSpec* inTheSpec,
                                 std::vector<UInt16> *colWidths=NULL,
                                 bool inEncodeMacResources = true);

void BuildConnectionData(std::string& outData,
							std::string& resource, class CDAVContext *context, 
                            SDimension16& frameSize, Point& frameLocation,
                            std::vector<UInt16> *colWidths=NULL,
                            bool inEncodeMacResources = true);
 #endif
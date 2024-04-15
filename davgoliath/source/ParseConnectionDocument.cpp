/* ==================================================================================================
 * ParseConnectionDocument.cpp														   
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
#include "ParseConnectionDocument.h"
#endif
#include <string.h>
#include <string>
#include <CDAVTableApp.h>

#include <CNavServicesUtils.h>
#include <CDAVContext.h>
#include <LDynamicBuffer.h>
#include <CDAVLibUtils.h>
#include <cctype>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

/*****************************************************************
   
   <!-- DTD for Goliath Connection Documents  -->
   <!ELEMENT connection.info (uri, host, port, forcesecure, proxyinfo?, credentials?, proxycredentials? ) >
   <!ELEMENT uri        (#PCDATA)>
   <!ELEMENT host       (#PCDATA)>
   <!ELEMENT port       (#PCDATA)>
   
   <!ELEMENT proxyinfo (proxy.host, proxy.port)>
   <!ELEMENT proxy.host (#PCDATA)>
   <!ELEMENT proxy.port (#PCDATA)>
   
   <!ELEMENT credentials (login, password)>
   <!ELEMENT login      (#PCDATA)>
   <!ELEMENT password   (#PCDATA)>
   
   <!ELEMENT proxycredentials (login, password)>
   
   <!ELEMENT window.size (width, length, top, left)>
   <!ELEMENT width      (#PCDATA)>
   <!ELEMENT length     (#PCDATA)>
   <!ELEMENT top        (#PCDATA)>
   <!ELEMENT left       (#PCDATA)>

   <!ELEMENT column.settings (column.width*)>
   <!ELEMENT column.width (#PCDATA)>
   <!ATTLIST column.width index>
   
   <!ELEMENT forcesecure (EMPTY)>
   <!ELEMENT disableappldbl (EMPTY)>

*****************************************************************/


// constants for parse events
const char *CONNECTION_INFO   = "connection.info";
const char *URI               = "uri";
const char *HOST              = "host";
const char *PORT              = "port";
const char *PROXYINFO         = "proxyinfo";
const char *PROXY_PORT        = "proxy.port";
const char *PROXY_HOST        = "proxy.host";
const char *CREDENTIALS       = "credentials";
const char *LOGIN             = "login";
const char *PASSWORD          = "password";
const char *PROXYCREDENTIALS  = "proxycredentials";
const char *COLUMN_SETTINGS   = "column.settings";
const char *COLUMN_WIDTH      = "column.width";
const char *WINDOW_SIZE       = "window.size";
const char *WIDTH             = "width";
const char *LENGTH            = "length";
const char *TOP               = "top";
const char *LEFT              = "left";
const char *FORCESECURE       = "forcesecure";
const char *DISABLEAPPLEDOUBLE = "disableappldbl";
 
//Expat XML parser events
static void pdc_starthandler(void *userdata, const char *name, const char **attrs);
static void pdc_endhandler(void *userdata, const char *name);
static void pdc_cdatahandler(void *userdata, const char *cdata, int len);

Boolean saveFileNavSrv(FSSpec *theSpec);

typedef enum {
   none,
   uri,
   host, 
   port,
   proxy_port,
   proxy_host,
   login,
   password,
   column_settings,
   column_width,
   window_size,
   width,
   length,
   top,
   left,
   proxylogin,
   proxypassword
} pdc_tokens;


typedef struct {
   pdc_tokens currToken;
   ConnectionDocumentData *docData;
   std::string mTmpString;
   Boolean mIsProxyHandler;
} pdc_data;


// ---------------------------------------------------------------------------
//		¥ pdc_starthandler()
// ---------------------------------------------------------------------------
//
static void pdc_starthandler(void *userdata, const char *name, const char** /* attrs*/ ) {
   pdc_data *myData = reinterpret_cast<pdc_data*>(userdata);
   if (strcmp(name, URI) == 0) {
      myData->currToken = uri;
   } else if (strcmp(name, HOST) == 0) {
      myData->currToken = host;
   } else if (strcmp(name, PORT) == 0) {
      myData->currToken = port;
      myData->mTmpString = "";
   } else if (strcmp(name, PROXYINFO) == 0) {
      myData->docData->mHasProxy = true;
   } else if (strcmp(name, PROXY_PORT) == 0) {
      myData->currToken = proxy_port;
      myData->mTmpString = "";
   } else if (strcmp(name, PROXY_HOST) == 0) {
      myData->currToken = proxy_host;
   } else if (strcmp(name, CREDENTIALS) == 0) {
      myData->docData->mHasUserCredentials = true;
   } else if (strcmp(name, LOGIN) == 0) {
      if (myData->mIsProxyHandler)
         myData->currToken = proxylogin;
      else
         myData->currToken = login;
   } else if (strcmp(name, PASSWORD) == 0) {
      if (myData->mIsProxyHandler)
         myData->currToken = proxypassword;
      else
         myData->currToken = password;
   } else if (strcmp(name, WIDTH) == 0) {
      myData->currToken = width;
      myData->mTmpString = "";
   } else if (strcmp(name, LENGTH) == 0) {
      myData->currToken = length;
      myData->mTmpString = "";
   } else if (strcmp(name, TOP) == 0) {
      myData->currToken = top;
      myData->mTmpString = "";
   } else if (strcmp(name, LEFT) == 0) {
      myData->currToken = left;
      myData->mTmpString = "";
   } else if (strcmp(name, COLUMN_SETTINGS) == 0) {
      myData->docData->mHasColumnInformation = true;
   } else if (strcmp(name, WINDOW_SIZE) == 0) {
      myData->docData->mHasFrameInformation = true;
   } else if (strcmp(name, COLUMN_WIDTH) == 0) {
      myData->currToken  = column_width;
      myData->mTmpString = "";
   }  else if (strcmp(name, PROXYCREDENTIALS)==0) {
      myData->mIsProxyHandler = true;
      myData->docData->mHasProxyCredentials = true;
   }
}


// ---------------------------------------------------------------------------
//		¥ pdc_endhandler()
// ---------------------------------------------------------------------------
//
static void pdc_endhandler(void *userdata, const char * name) {
   pdc_data *myData = reinterpret_cast<pdc_data*>(userdata);
   if (port == myData->currToken) {
      myData->docData->mPort = atoi(myData->mTmpString.c_str());
   } else if (proxy_port == myData->currToken) {
      myData->docData->mProxyPort = atoi(myData->mTmpString.c_str());
   } else if (width == myData->currToken) {
      myData->docData->mFrameSize.width = atoi(myData->mTmpString.c_str());
   } else if (length == myData->currToken) {
      myData->docData->mFrameSize.height = atoi(myData->mTmpString.c_str());
   } else if (top == myData->currToken) {
      myData->docData->mFrameLocation.v = atoi(myData->mTmpString.c_str());
   } else if (left == myData->currToken) {
      myData->docData->mFrameLocation.h = atoi(myData->mTmpString.c_str());
   } else if (column_width  == myData->currToken) {
      myData->docData->mColWidths.push_back(atoi(myData->mTmpString.c_str()));
   }
   if (strcmp(name, PROXYCREDENTIALS)==0)
      myData->mIsProxyHandler = false;
   if (strcmp(name, FORCESECURE) == 0) 
      myData->docData->mForceSecure = true;
   if (strcmp(name, DISABLEAPPLEDOUBLE) == 0) 
      myData->docData->mEncodeAppleDouble = false;
   myData->currToken = none;
}


// ---------------------------------------------------------------------------
//		¥ pdc_cdatahandler()
// ---------------------------------------------------------------------------
//
static void pdc_cdatahandler(void *userdata, const char *cdata, int len) {
   pdc_data *myData = reinterpret_cast<pdc_data*>(userdata);
   switch(myData->currToken) {
      case uri:
         myData->docData->mResource.append(cdata, len);
      break;
      case host:
         myData->docData->mHost.append(cdata, len);
      break;
      case port:
         myData->mTmpString.append(cdata, len);
      break;
      case proxy_port:
         myData->mTmpString.append(cdata, len);
      break;
      case proxy_host:
         myData->docData->mProxyHost.Append(cdata, len);
      break;
      case login:
         myData->docData->mUser.Append(cdata, len);
      break;
      case password:
         myData->docData->mPassword.Append(cdata, len);
      break;
      case proxylogin:
         myData->docData->mProxyUser.Append(cdata, len);
      break;
      case proxypassword:
         myData->docData->mProxyPassword.Append(cdata, len);
      break;
      case width:
      case length:
      case top:
      case left:
      case column_width:
         myData->mTmpString.append(cdata, len);
      break;
   }
     
}

// ---------------------------------------------------------------------------
//		¥ InitConnectionDocumentData()
// ---------------------------------------------------------------------------
//
void InitConnectionDocumentData(ConnectionDocumentData& inDocData) {
	inDocData.mHasUserCredentials = false;
	inDocData.mHasProxy = false;
}

// ---------------------------------------------------------------------------
//		¥ ParseConnectionData()
// ---------------------------------------------------------------------------
//
XML_Error ParseConnectionData(std::string& inStr, ConnectionDocumentData *docData) {
   pdc_data myData;
   XML_Parser parser;
   docData->mHasFrameInformation = false;
   docData->mHasColumnInformation = false;
   
   myData.currToken = none;
   myData.docData = docData;
   myData.mIsProxyHandler = false;
   myData.docData->mForceSecure = false;
   myData.docData->mEncodeAppleDouble = true;
   
   
   parser = XML_ParserCreate(NULL);
   if (NULL == parser) {
      return XML_ERROR_NO_MEMORY;
   }

   XML_SetUserData(parser, &myData);
   XML_SetElementHandler(parser, pdc_starthandler, pdc_endhandler);
   XML_SetCharacterDataHandler(parser, pdc_cdatahandler);

   XML_Error errCode = XML_ERROR_NONE;
   int resp;
   resp = XML_Parse(parser, inStr.c_str(), inStr.size(), 0);
   if (0 == resp) {
      errCode = XML_GetErrorCode(parser);
   }
   return errCode; 
}

// ---------------------------------------------------------------------------
//		¥ ParseConnectionDocument()
// ---------------------------------------------------------------------------
//
XML_Error ParseConnectionDocument(FSSpec *theDoc, ConnectionDocumentData *docData) {
   pdc_data myData;
   XML_Parser parser;
   docData->mHasFrameInformation = false;
   docData->mHasColumnInformation = false;
   
   myData.currToken = none;
   myData.docData = docData;
   myData.mIsProxyHandler = false;
   myData.docData->mForceSecure = false;
   myData.docData->mEncodeAppleDouble = true;
   
   
   parser = XML_ParserCreate(NULL);
   if (NULL == parser) {
      return XML_ERROR_NO_MEMORY;
   }

   XML_SetUserData(parser, &myData);
   XML_SetElementHandler(parser, pdc_starthandler, pdc_endhandler);
   XML_SetCharacterDataHandler(parser, pdc_cdatahandler);

   LFileStream stream(*theDoc);
   stream.OpenDataFork(fsRdPerm);
   
   SInt32 streamSize = stream.GetLength() - stream.GetMarker();
   SInt32 bufferSize = ::MaxBlock();
   bufferSize = (bufferSize < streamSize) ? bufferSize : streamSize;
   StPointerBlock tempSwap(bufferSize);
   SInt32 bytesRead;
   XML_Error errCode = XML_ERROR_NONE;

   int resp;
   while (streamSize > 0) {
      bytesRead = stream.ReadData(tempSwap, bufferSize);
      resp = XML_Parse(parser, tempSwap, bytesRead, 0);
      if (0 == resp) {
         errCode = XML_GetErrorCode(parser);
      }
      streamSize -= bytesRead;
   }

   XML_ParserFree(parser);   
   stream.CloseDataFork();
   
   return errCode; 
}


void ConstructConnectionData(LDynamicBuffer& dynBuf,
								std::string& resource, CDAVContext *context, 
								SDimension16& frameSize, Point& frameLocation,
								bool inEncodeMacResources,
								std::vector<UInt16> *colWidths);
								
void ConstructConnectionData(LDynamicBuffer& dynBuf,
							    std::string& resource, CDAVContext *context, 
								SDimension16& frameSize, Point& frameLocation,
								bool inEncodeMacResources,
								std::vector<UInt16> *colWidths) {
   char buf[15];
   LStr255 tmpStr;
   std::string tmpCstr;

   
   CXMLStringUtils::beginXMLDAVBody(dynBuf);
   CXMLStringUtils::startElement(dynBuf, CONNECTION_INFO);
   CXMLStringUtils::startElement(dynBuf, URI);
   dynBuf.ConcatenateBuffer(resource.c_str());
   CXMLStringUtils::endElement(dynBuf, URI);
   CXMLStringUtils::startElement(dynBuf, HOST);
   dynBuf.ConcatenateBuffer(context->GetServerName().c_str());
   CXMLStringUtils::endElement(dynBuf, HOST);
   CXMLStringUtils::startElement(dynBuf, PORT);
   tmpStr=context->GetPort();
   dynBuf.ConcatenateBuffer(DAVp2cstr(tmpStr));
   CXMLStringUtils::endElement(dynBuf, PORT);
   
   if (context->GetUseProxy()) {
      CXMLStringUtils::startElement(dynBuf, PROXYINFO);
      CXMLStringUtils::startElement(dynBuf, PROXY_PORT);
      tmpStr = context->GetProxyPort();
      dynBuf.ConcatenateBuffer(DAVp2cstr(tmpStr));
      CXMLStringUtils::endElement(dynBuf, PROXY_PORT);
      CXMLStringUtils::startElement(dynBuf, PROXY_HOST);
      
      context->GetProxyServer(tmpCstr);
      dynBuf.ConcatenateBuffer(tmpCstr.c_str());
      CXMLStringUtils::endElement(dynBuf, PROXY_HOST);
      CXMLStringUtils::endElement(dynBuf, PROXYINFO);
   }
   
   if (context->GetHasUserCredentials()) {
      CXMLStringUtils::startElement(dynBuf, CREDENTIALS);
      CXMLStringUtils::startElement(dynBuf, LOGIN);
      dynBuf.ConcatenateBuffer(context->GetLogin());
      CXMLStringUtils::endElement(dynBuf, LOGIN);
      CXMLStringUtils::startElement(dynBuf, PASSWORD);
      dynBuf.ConcatenateBuffer(context->GetPassword());
      CXMLStringUtils::endElement(dynBuf, PASSWORD);
      CXMLStringUtils::endElement(dynBuf, CREDENTIALS);
   }


   if (context->GetHasProxyCredentials()) {
      CXMLStringUtils::startElement(dynBuf, PROXYCREDENTIALS);
      CXMLStringUtils::startElement(dynBuf, LOGIN);
      dynBuf.ConcatenateBuffer(context->GetProxyLogin());
      CXMLStringUtils::endElement(dynBuf, LOGIN);
      CXMLStringUtils::startElement(dynBuf, PASSWORD);
      dynBuf.ConcatenateBuffer(context->GetProxyPassword());
      CXMLStringUtils::endElement(dynBuf, PASSWORD);
      CXMLStringUtils::endElement(dynBuf, PROXYCREDENTIALS);
   }

   if (context->GetForceSecure()) {
      CXMLStringUtils::addEmptyElement(dynBuf, FORCESECURE);
   }
   	
   CXMLStringUtils::startElement(dynBuf, WINDOW_SIZE);
   CXMLStringUtils::startElement(dynBuf, WIDTH);
   sprintf(buf, "%u", frameSize.width);
   dynBuf.ConcatenateBuffer(buf);
   CXMLStringUtils::endElement(dynBuf, WIDTH);
   CXMLStringUtils::startElement(dynBuf, LENGTH);
   sprintf(buf, "%u", frameSize.height);   
   dynBuf.ConcatenateBuffer(buf);
   CXMLStringUtils::endElement(dynBuf, LENGTH);
   CXMLStringUtils::startElement(dynBuf, TOP);
   sprintf(buf, "%u", frameLocation.v);   
   dynBuf.ConcatenateBuffer(buf);
   CXMLStringUtils::endElement(dynBuf, TOP);
   CXMLStringUtils::startElement(dynBuf, LEFT);
   sprintf(buf, "%u", frameLocation.h);   
   dynBuf.ConcatenateBuffer(buf); 
   CXMLStringUtils::endElement(dynBuf, LEFT);
   
   CXMLStringUtils::endElement(dynBuf, WINDOW_SIZE);
   
    
   if (colWidths) {
       CXMLStringUtils::startElement(dynBuf, COLUMN_SETTINGS);
       std::vector<UInt16>::iterator iter = colWidths->begin();
       SInt32 i=1;
       while (iter != colWidths->end()) {
          dynBuf.ConcatenateBuffer("<");
          dynBuf.ConcatenateBuffer(COLUMN_WIDTH);
          dynBuf.ConcatenateBuffer(" index=\"");
          sprintf(buf, "%u", i);
          dynBuf.ConcatenateBuffer(buf);
          dynBuf.ConcatenateBuffer("\">");
          sprintf(buf, "%u", *iter);
          dynBuf.ConcatenateBuffer(buf);
          CXMLStringUtils::endElement(dynBuf, COLUMN_WIDTH);
          iter++;
          i++;
       }
       CXMLStringUtils::endElement(dynBuf, COLUMN_SETTINGS);
   }
   if (!inEncodeMacResources) {
      CXMLStringUtils::addEmptyElement(dynBuf, DISABLEAPPLEDOUBLE);
   }
   CXMLStringUtils::endElement(dynBuf, CONNECTION_INFO);
   
}

// ---------------------------------------------------------------------------
//		¥ BuildConnectionData()
// ---------------------------------------------------------------------------
//
void BuildConnectionData(std::string& outData,
							std::string& resource, class CDAVContext *context, 
                            SDimension16& frameSize, Point& frameLocation,
                            std::vector<UInt16> *colWidths,
                            bool inEncodeMacResources)
{
   LDynamicBuffer dynBuf;
	
   ConstructConnectionData(dynBuf, resource, context, frameSize, frameLocation,
								inEncodeMacResources, colWidths);

   Handle ourHandle;
   dynBuf.GetBufferHandle(ourHandle);
   LStream *mStream = dynBuf.GetBufferStream();
   StHandleLocker locked(ourHandle);
   outData.append(*ourHandle, mStream->GetLength());

	
}

// ---------------------------------------------------------------------------
//		¥ WriteConnectionDocument()
// ---------------------------------------------------------------------------
//
Boolean WriteConnectionDocument(std::string& resource, CDAVContext *context, 
                                SDimension16& frameSize, Point& frameLocation,
                                FSSpec* inTheSpec,
                                std::vector<UInt16> *colWidths,
                                bool inEncodeMacResources) {

   LDynamicBuffer dynBuf;
	
   ConstructConnectionData(dynBuf, resource, context, frameSize, frameLocation,
								inEncodeMacResources, colWidths);
								
   LFile outFile(*inTheSpec);
   
   std::string tmpsname =  context->GetServerName(), srvrname;
   for (int i=0; i<tmpsname.length(); i++) {
      if (islower(tmpsname[i]))
         srvrname+=tolower(tmpsname[i]);
      else
         srvrname+=tmpsname[i];
   }
   
   try {
      if (strstr(srvrname.c_str(), ".driveway.com")!= NULL) 
         outFile.CreateNewFile(AppCreatorCode, 'DRVW');
      else
         outFile.CreateNewFile(AppCreatorCode, DocTypeCode);
   } catch (...) {
      OSErr	__theErr = ::ResError();
   	  if (__theErr != dupFNErr)
   		Throw_(__theErr);
   }
   
   LStream *mStream = dynBuf.GetBufferStream();
   
	mStream->SetMarker(0, streamFrom_Start);
	
	
	SInt16 fileRef = outFile.OpenDataFork(fsRdWrPerm);
   
	Handle ourHandle;
	Boolean mHandleStream = dynBuf.GetBufferHandle(ourHandle);

	//Optimization for copies from LHandleStream
	if (mHandleStream) {
		StHandleLocker locked(ourHandle);
		outFile.WriteDataFork(*ourHandle, mStream->GetLength());
	} else {
		//Try to allocate some swap space to speed things up
		//	Get the smaller of available memory or the stream size
		SInt32 streamSize = mStream->GetLength();
		SInt32 bufferSize = ::MaxBlock();
		bufferSize = (bufferSize < streamSize) ? bufferSize : streamSize;
		
		OSErr	err = ::SetFPos(fileRef, fsFromStart, 0);
		ThrowIfOSErr_(err);
		
		StPointerBlock tempSwap(bufferSize);
		SInt32	bytesWritten;
		while (streamSize > 0) {
			bytesWritten = mStream->ReadData(tempSwap, bufferSize);
			err = ::FSWrite(fileRef, &bytesWritten, tempSwap);
			ThrowIfOSErr_(err);
			streamSize -= bytesWritten;
		}
		
        //***teb - here's the bug in powerplant
        //::SetEOF(fileRef, streamSize);
 		::SetEOF(fileRef, mStream->GetLength());
		ThrowIfOSErr_(err);
	}

   outFile.CloseDataFork();
   return true;
}

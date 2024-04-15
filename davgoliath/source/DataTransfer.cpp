/* ==================================================================================================
 * DataTransfer.cpp															   
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
 
#include "DataTransfer.h"
#include <CDAVLibUtils.h>
#include "ParseConnectionDocument.h"
#include <CDAVContext.h>
#include <string.h>
#include <stdlib.h>

/*****************************************************************
   
   <!-- DTD for Goliath Clipboard info  -->
   <!ELEMENT goliath.clipinfo (connection.info, iscut?, resource.list) >
   
   <!ELEMENT connection.info (host, port, proxyinfo?, credentials?, proxycredentials? ) >
   <!ELEMENT host       (#PCDATA)>
   <!ELEMENT port       (#PCDATA)>
   
   <!ELEMENT proxyinfo (proxy.host, proxy.port)>
   <!ELEMENT proxy.host (#PCDATA)>
   <!ELEMENT proxy.port (#PCDATA)>
   
   <!ELEMENT credentials (login, password)>
   <!ELEMENT login      (#PCDATA)>
   <!ELEMENT password   (#PCDATA)>
   
   <!ELEMENT proxycredentials (login, password)>
	
   <!ELEMENT resource.list (resource*)>
   <!ELEMENT resource        (#PCDATA)>

   
*****************************************************************/

const char *GOLIATH_CLIPINFO = "goliath.clipinfo";
const char *ISCUT = "iscut";
const char *RESOURCE_LIST = "resource.list";
const char *RESOURCE = "resource";

//Expat XML parser events
static void pclip_starthandler(void *userdata, const char *name, const char **attrs);
static void pclip_endhandler(void *userdata, const char *name);
static void pclip_cdatahandler(void *userdata, const char *cdata, int len);


typedef enum {
   none,
   host, 
   port,
   proxy_port,
   proxy_host,
   login,
   password,
   proxylogin,
   proxypassword,
   iscut, 
   resource
} pclip_tokens;

typedef struct {
   pclip_tokens currToken;
   DAVItemData *clipData;
   std::string mTmpString;
   Boolean mIsProxyHandler;
} pclip_data;

// ---------------------------------------------------------------------------
//		¥ DAVItemData()
// ---------------------------------------------------------------------------
//
DAVItemData::DAVItemData():port(80), hasProxy(false), hasAuth(false), hasProxyAuth(false),
   isCut(false) {

}

// ---------------------------------------------------------------------------
//		¥ pclip_starthandler()
// ---------------------------------------------------------------------------
//
static void pclip_starthandler(void *userdata, const char *name, const char** /* attrs*/ ) {
   pclip_data *myData = reinterpret_cast<pclip_data*>(userdata);

   if (strcmp(name, HOST) == 0) {
      myData->currToken = host;
   } else if (strcmp(name, PORT) == 0) {
      myData->currToken = port;
      myData->mTmpString = "";
   } else if (strcmp(name, PROXYINFO) == 0) {
      myData->clipData->hasProxy = true;
   } else if (strcmp(name, PROXY_PORT) == 0) {
      myData->currToken = proxy_port;
      myData->mTmpString = "";
   } else if (strcmp(name, PROXY_HOST) == 0) {
      myData->currToken = proxy_host;
   } else if (strcmp(name, CREDENTIALS) == 0) {
      myData->clipData->hasAuth = true;
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
   }  else if (strcmp(name, PROXYCREDENTIALS)==0) {
      myData->mIsProxyHandler = true;
      myData->clipData->hasProxyAuth = true;
   }  else if (strcmp(name, ISCUT)==0) {
      myData->clipData->isCut = true;
   } else if (strcmp(name, RESOURCE)==0) {
      myData->currToken = resource;
   }

}

// ---------------------------------------------------------------------------
//		¥ pclip_endhandler()
// ---------------------------------------------------------------------------
//
static void pclip_endhandler(void *userdata, const char *name) {
   pclip_data *myData = reinterpret_cast<pclip_data*>(userdata);
   
   if (port == myData->currToken) {
      myData->clipData->port = atoi(myData->mTmpString.c_str());
   } else if (proxy_port == myData->currToken) {
      myData->clipData->proxyport = atoi(myData->mTmpString.c_str());
   } else if (resource == myData->currToken) {
      myData->clipData->sourceURIs.push_back(myData->mTmpString);
   }

   if (strcmp(name, PROXYCREDENTIALS)==0)
      myData->mIsProxyHandler = false;
   myData->currToken = none;
}

// ---------------------------------------------------------------------------
//		¥ pclip_cdatahandler()
// ---------------------------------------------------------------------------
//
static void pclip_cdatahandler(void *userdata, const char *cdata, int len) {
   pclip_data *myData = reinterpret_cast<pclip_data*>(userdata);

   switch(myData->currToken) {
      case host:
         myData->clipData->server.append(cdata, len);
      break;
      case port:
         myData->mTmpString.append(cdata, len);
      break;
      case proxy_port:
         myData->mTmpString.append(cdata, len);
      break;
      case proxy_host:
         myData->clipData->proxyserver.append(cdata, len);
      break;
      case login:
         myData->clipData->login.append(cdata, len);
      break;
      case password:
         myData->clipData->password.append(cdata, len);
      break;
      case proxylogin:
         myData->clipData->proxylogin.append(cdata, len);
      break;
      case proxypassword:
         myData->clipData->proxypassword.append(cdata, len);
      break;
      case resource:
         myData->mTmpString.append(cdata, len);
      break;
   }
}

// ---------------------------------------------------------------------------
//		¥ ParseDAVItemClip()
// ---------------------------------------------------------------------------
//
XML_Error DAVItemData::ParseDAVItemClip(Ptr inClipData, SInt32 clipSize, DAVItemData *outTheData) {
   pclip_data pclip;
   pclip.currToken = none;
   pclip.clipData = outTheData;
   pclip.mIsProxyHandler = false;
    
   XML_Parser parser;
   parser = XML_ParserCreate(NULL);
   if (NULL == parser) {
      return XML_ERROR_NO_MEMORY;
   }

   XML_SetUserData(parser, &pclip);
   XML_SetElementHandler(parser, pclip_starthandler, pclip_endhandler);
   XML_SetCharacterDataHandler(parser, pclip_cdatahandler);
   XML_Error errCode = XML_ERROR_NONE;

   int resp = XML_Parse(parser, inClipData, clipSize, 0);;
   if (0 == resp) {
         errCode = XML_GetErrorCode(parser);
   }

   XML_ParserFree(parser);   
   return errCode; 
}



// ---------------------------------------------------------------------------
//		¥ DAVItemsToClip()
// ---------------------------------------------------------------------------
//
void DAVItemData::DAVItemsToClip(CDAVItemVector &inVector, CDAVContext *inContext, bool inIsCut, 
                     std::string& outClip) {

   std::string tmpStr;
   LStr255 tmpPStr;

   CXMLStringUtils::beginXMLDAVBody(outClip);

   CXMLStringUtils::startElement(outClip, GOLIATH_CLIPINFO);
   
   CXMLStringUtils::startElement(outClip, CONNECTION_INFO);
   CXMLStringUtils::startElement(outClip, HOST);   
   CXMLStringUtils::filterXMLContent(outClip, inContext->GetServerName());
   CXMLStringUtils::endElement(outClip, HOST);
   CXMLStringUtils::startElement(outClip, PORT);
   tmpPStr = inContext->GetPort();
   CXMLStringUtils::filterXMLContent(outClip, DAVp2cstr(tmpPStr));
   CXMLStringUtils::endElement(outClip, PORT);

   if (inContext->GetUseProxy()) {
      CXMLStringUtils::startElement(outClip, PROXYINFO);
      CXMLStringUtils::startElement(outClip, PROXY_PORT);
      tmpPStr = inContext->GetProxyPort();
      CXMLStringUtils::filterXMLContent(outClip, DAVp2cstr(tmpPStr));
      CXMLStringUtils::endElement(outClip, PROXY_PORT);
      CXMLStringUtils::startElement(outClip, PROXY_HOST);
      inContext->GetProxyServer(tmpStr);
      CXMLStringUtils::filterXMLContent(outClip, tmpStr);
      CXMLStringUtils::endElement(outClip, PROXY_HOST);
      CXMLStringUtils::endElement(outClip, PROXYINFO);
   }

   if (inContext->GetHasUserCredentials()) {
      CXMLStringUtils::startElement(outClip, CREDENTIALS);
      CXMLStringUtils::startElement(outClip, LOGIN);
      CXMLStringUtils::filterXMLContent(outClip, inContext->GetLogin());
      CXMLStringUtils::endElement(outClip, LOGIN);
      CXMLStringUtils::startElement(outClip, PASSWORD);
      CXMLStringUtils::filterXMLContent(outClip, inContext->GetPassword());
      CXMLStringUtils::endElement(outClip, PASSWORD);
      CXMLStringUtils::endElement(outClip, CREDENTIALS);
   }
   
   if (inContext->GetHasProxyCredentials()) {
      CXMLStringUtils::startElement(outClip, PROXYCREDENTIALS);
      CXMLStringUtils::startElement(outClip, LOGIN);
      CXMLStringUtils::filterXMLContent(outClip, inContext->GetProxyLogin());
      CXMLStringUtils::endElement(outClip, LOGIN);
      CXMLStringUtils::startElement(outClip, PASSWORD);
      CXMLStringUtils::filterXMLContent(outClip, inContext->GetProxyPassword());
      CXMLStringUtils::endElement(outClip, PASSWORD);
      CXMLStringUtils::endElement(outClip, PROXYCREDENTIALS);   
   }
   CXMLStringUtils::endElement(outClip, CONNECTION_INFO);
   
   if (inIsCut) {
      CXMLStringUtils::addEmptyElement(outClip, ISCUT);
   }
   
   CDAVItemVector::iterator iter = inVector.begin();
   CXMLStringUtils::startElement(outClip, RESOURCE_LIST);
   while (iter != inVector.end()) {
      CXMLStringUtils::startElement(outClip, RESOURCE);
      CXMLStringUtils::filterXMLContent(outClip, iter->GetURI());
      CXMLStringUtils::endElement(outClip, RESOURCE);
      iter++;
   }
   CXMLStringUtils::endElement(outClip, RESOURCE_LIST);

   CXMLStringUtils::endElement(outClip, GOLIATH_CLIPINFO);

}

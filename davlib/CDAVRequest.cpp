/* ===========================================================================
 *	CDAVRequest.cpp			   
 *
 *  This file is part of the DAVLib package
 *  Copyright (C) 1999-2000  Thomas Bednarzär
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
 */
 
#include <map>
#include "CDAVRequest.h"
#include "CDAVProperty.h"
#include "LMacBinary.h"
#include "parsexml.h"
#include "CDAVContext.h"
#include <string.h>
#include <UInternetConfig.h>
#include <CDAVLibUtils.h>
#include "OpenSSLGlue.h"
#include <Timer.h>

using namespace std;

const char* XMLMIMETYPE = "text/xml";

std::string CDAVRequest::mUserAgent;
std::string CDAVRequest::mDefaultUserAgent;

//=========================================================================
// ***teb - some helper functions.
/*void _beginXMLDAVBody(string &msgBody);
void _startElement(string &str, const char* elem);
void _startElementWithDAVNS(string &str, const char* elemname);
void _endElement(string &str, const char* elem);
void _addEmptyElement(string &str, const char* elemname) ;*/

void _addPropFindElement(string &msgBody, CDAVPropertyVector &props);
CDAVProperty  *_getBuiltinPropertyType(CElement *elem);
bool  _isBuiltinPropertyType(CElement *elem);


//
//=========================================================================


// ---------------------------------------------------------------------------
//		¥ _addPropFindElement()
// ---------------------------------------------------------------------------
//
void _addPropFindElement(string &msgBody, CDAVPropertyVector &props) {
   CXMLStringUtils::startElementWithDAVNS(msgBody, DAVTypes::PROPFIND);
   CDAVPropertyVector::iterator iter = props.begin();
   CXMLStringUtils::startElement(msgBody, DAVTypes::PROP);
   
   while (iter != props.end()) {
      //Each property is of the form:  <prop><resourcetype/></prop>\r\n

      //***teb - fix this later - we only grok DAV properties...
      if ((*iter).getIsInDAVNamespace()) {
          CXMLStringUtils::addEmptyElement(msgBody,(*iter).GetPropertyName().c_str());
      } else {
         msgBody.append("<");
         msgBody.append((*iter).GetPropertyName().c_str());
         msgBody.append(" xmlns=\"");
         msgBody.append((*iter).GetNamespace().c_str());
         msgBody.append("\"/>");
      }
      iter++;
   }
   CXMLStringUtils::endElement(msgBody, DAVTypes::PROP);
   CXMLStringUtils::endElement(msgBody, DAVTypes::PROPFIND);
}

// ---------------------------------------------------------------------------
//		¥ _isBuiltinPropertyType()
// ---------------------------------------------------------------------------
//
bool _isBuiltinPropertyType(CElement *elem) {
	string& name = elem->getName();

	for (int i=0; i<DAVTypes::mPropertyStringsCount; i++)
		if (name.compare(DAVTypes::mPropertyStrings[i]) == 0)
			return true;
				
	for (int i=0; i<DAVTypes::mPropertyStringsExtCount; i++)
		if (name.compare(DAVTypes::mExtPropertyStrings[i]) == 0)
			return true;
			
	return false;
}

// ---------------------------------------------------------------------------
//		¥ _getBuiltinPropertyType()
// ---------------------------------------------------------------------------
//
CDAVProperty  *_getBuiltinPropertyType(CElement *elem) {
   std::string& name =elem->getName();
   if (name.compare(DAVTypes::GETLASTMODIFIED)==0) {
      return &LastModified;
   } else if ((name.compare(DAVTypes::GETCONTENTLENGTH)==0)) {
      return &ContentLength;
   } else if ((name.compare(DAVTypes::RESOURCETYPE)==0)) {
      return &ResourceType;
   } else if ((name.compare(DAVTypes::GETCONTENTTYPE)==0)) {
      return &ContentType;
   } else if ((name.compare(DAVTypes::CREATIONDATE)==0)) {
      return &CreationDate;
   } else if ((name.compare(DAVTypes::DISPLAYNAME)==0)) {
      return &DisplayName;
   } else if ((name.compare(DAVTypes::GETETAG)==0)) {
      return &ETag;
   } else if ((name.compare(DAVTypes::LOCKDISCOVERY)==0)) {
      return &LockDiscovery;
   } else if ((name.compare(DAVTypes::SUPPORTEDLOCK)==0)) {
      return &SupportedLock;
   } else if ((name.compare(DAVTypes::GETCONTENTLANGUAGE)==0)) {
      return &ContentLanguage;
   } else if ((name.compare(DAVTypes::SOURCE)==0)) {
      return &Source;
   } else if (((name.compare(mod_davExecutable.GetPropertyName())==0)) &&
                ((elem->getNamespace().compare(mod_davExecutable.GetNamespace())==0))) {
      return &mod_davExecutable;
   } 
   return NULL;
}


string _buildIfHeaderForLock(const char* srvrName, const char* itemURI, const char* lockToken);

string _buildIfHeaderForLock(const char* srvrName, const char* itemURI, const char* lockToken) {
	string tmpStr("<http://");
   tmpStr.append(srvrName);      
   tmpStr.append(itemURI);
   tmpStr.append("> (<");
   tmpStr.append(lockToken);
   tmpStr.append(">)");
   return tmpStr;
}



// ---------------------------------------------------------------------------
//		¥ CDAVRequest()
// ---------------------------------------------------------------------------
//	Constructor
CDAVRequest::CDAVRequest(CDAVContext *ctx): m_context(ctx), m_connection(nil), mCancel(false), mLastResponse(0), mAttemptRetry(false) {
	_init();
}

// ---------------------------------------------------------------------------
//		¥ CDAVRequest()
// ---------------------------------------------------------------------------
//	Constructor
CDAVRequest::CDAVRequest(LThread & inThread, CDAVContext *ctx): m_context(ctx), mCancel(false), mLastResponse(0), mAttemptRetry(false)
{
	LTCPEndpoint *endPoint;
	
#if DAVLIB_SSL
	if ((m_context->GetPort() == kHTTPSPort) || (m_context->GetForceSecure()))
		endPoint = USecureNetworkFactory::CreateSecureTCPEndpoint ();
	else
#endif
		endPoint = UNetworkFactory::CreateTCPEndpoint ();

	m_connection = new CDAVConnection (inThread, endPoint, ctx->GetCookieList(), m_context->GetForceSecure());
	_init();
}

// ---------------------------------------------------------------------------
//		¥ _init()
// ---------------------------------------------------------------------------
//	
void CDAVRequest::_init() {
	if (mDefaultUserAgent.size() == 0) {
		mDefaultUserAgent = "DAVLib ";
		GetDAVLibVersionDisplayString(mDefaultUserAgent);
		mDefaultUserAgent += " (Macintosh; PPC)";
	}
}

// ---------------------------------------------------------------------------
//		¥ CDAVRequest()
// ---------------------------------------------------------------------------
//	Constructor
CDAVRequest::CDAVRequest(const CDAVRequest &rhs) {
   m_context = rhs.m_context;
   mCancel   = rhs.mCancel;
   mLastResponse = rhs.mLastResponse;
   m_connection = nil;
   mAttemptRetry = rhs.mAttemptRetry;
}


// ---------------------------------------------------------------------------
//		¥ ~CDAVRequest()
// ---------------------------------------------------------------------------
//	Destructor
CDAVRequest::~CDAVRequest() {
	if (!m_connection)
		return;
	try
	{
		m_connection->GetEndpoint()->SendDisconnect();
	}
	catch (...) {}
	delete m_connection;
}

// ---------------------------------------------------------------------------
//		¥ CancelTransaction()
// ---------------------------------------------------------------------------
//
void CDAVRequest::CancelTransaction() {
   mCancel = true;
}

// ---------------------------------------------------------------------------
//		¥ GetServerOptions()
// ---------------------------------------------------------------------------
// This method behaves a bit differently than all of the others in that 
//	often it is used to query a server to determine if DAV operations
//	are supported.	So, it is very possible to get an HTTP response code
//	that isn't 200.		Since this is the case, WARNING is returned on any
//	non connection related responses not equal to 200 (HTTPRequestOK).	
//	Display of the dialog is also made optional.  Most likely, any time
//	a HTTP response of 200 is not returned, the server isn't an HTTP 1.1 
//	server

extern "C" const char *ERR_reason_error_string(unsigned long e);
extern "C" unsigned long ERR_get_error(void );

CDAVRequest::ReqStatus CDAVRequest::GetServerOptions(LThread& inThread, 
						const string& resource, CDAVInfo &options,
             Boolean showDlog, LListener *listener) {
   CDAVMessage theMessage;    
   LHTTPResponse theResponse;
	string theHost, theResource = resource;
	SInt32 thePort;

	_PrepareHostFields(theHost, thePort, theResource, theMessage);

	bool doCommunication = true;
	while (doCommunication) {
	    mAttemptRetry = false;
		doCommunication = false;
		try {
			StConnection connection (this, inThread, listener);
			connection.GetConnection().Options(
						theHost,
						theResource,
						theMessage,
						theResponse,
						thePort);
		
		} catch (LSSLException sslE) {
	    	_HandleSSLexception(theResponse, sslE);
	    	return FAILURE;
		} catch (...) {
			if (mAttemptRetry) {
				doCommunication = true;
			} else {
				return FAILURE;
			}
		}
	}
	
	mLastResponse = theResponse.GetResponseCode();
	if (mLastResponse != kHTTPRequestOK) {
      if (showDlog)
         _OnDavRequestError(theResponse);
      return WARNING;
   } 
   LHTTPMessage* retMsg = theResponse.GetReturnMessage();
   LHeaderField hdrField;
   
   if (retMsg->GetArbitraryField(CDAVConnection::kAllow, &hdrField)) {
		string body = hdrField.GetBody();
       //***teb - ok, i vaguely remember using some algorithm or other class
		// that was part of STL to tokenize a basic_string.	I can't seem to
       // remember the class name and 'grep' isn't helping me much right now.
       // So, as much as I hate to do it, strtok works for me.  
       char* tokTmp=NULL, *tokStr=const_cast<char*>(body.c_str());
       SInt32 opTmp=CDAVInfo::noOperations;
       tokTmp=strtok(tokStr, ", ");
       while (tokTmp!=NULL) {
          
			if (strcmp(tokTmp, kHTTPGet)==0) {
             opTmp |= CDAVInfo::HTTP_Get;      
			} else if (strcmp(tokTmp, kHTTPHead)==0) {
             opTmp |= CDAVInfo::HTTP_Head;
			} else if (strcmp(tokTmp, kHTTPPost)==0) {
             opTmp |= CDAVInfo::HTTP_Post;          
			} else if (strcmp(tokTmp, kHTTPPut)==0) {
             opTmp |= CDAVInfo::HTTP_Put;          
          } else if (strcmp(tokTmp, CDAVConnection::kHTTPOptions)==0) {
             opTmp |= CDAVInfo::HTTP_Options;          
          } else if (strcmp(tokTmp, CDAVConnection::kDAVPropFind)==0) {
             opTmp |= CDAVInfo::DAV_PropFind;          
          } else if (strcmp(tokTmp, CDAVConnection::kDAVMkcol)==0) {
             opTmp |= CDAVInfo::DAV_Mkcol;          
          } else if (strcmp(tokTmp, CDAVConnection::kDAVDelete)==0) {
             opTmp |= CDAVInfo::DAV_Delete;          
          } else if (strcmp(tokTmp, CDAVConnection::kDAVCopy)==0) {
             opTmp |= CDAVInfo::DAV_Copy;          
          } else if (strcmp(tokTmp, CDAVConnection::kDAVMove)==0) {
             opTmp |= CDAVInfo::DAV_Move;          
          } else if (strcmp(tokTmp, CDAVConnection::kDAVPropPatch)==0) {
             opTmp |= CDAVInfo::DAV_PropPatch;          
          } else if (strcmp(tokTmp, CDAVConnection::kDAVLock)==0) {
             opTmp |= CDAVInfo::DAV_Lock;          
          } else if (strcmp(tokTmp, CDAVConnection::kDAVUnlock)==0) {
             opTmp |= CDAVInfo::DAV_Unlock;          
          }
          tokTmp=strtok(NULL, ",");
          if ((NULL != tokTmp) && (' ' == *tokTmp))
             tokTmp++;
       }
       options.setSupportedOperations(opTmp);
   }

   if (retMsg->GetArbitraryField(CDAVConnection::kDAV, &hdrField)) {
       string body = hdrField.GetBody();
       options.setDavClassSupport(body);
   }

   if (retMsg->GetArbitraryField(CDAVConnection::kMSAuthorVia, &hdrField)) {
       string body = hdrField.GetBody();
       options.setMSHeaderFields(body);
   }
   

   if (retMsg->GetArbitraryField(CDAVConnection::kServer, &hdrField)) {
       string body = hdrField.GetBody();
       options.setServerType(body);
   }
   
   
   return SUCCESS;
}

// ---------------------------------------------------------------------------
//		¥ GetResourceExists()
// ---------------------------------------------------------------------------
// 
CDAVRequest::ReqStatus CDAVRequest::GetResourceExists(LThread& inThread, const string& resource,
                                                      Boolean inResourceIsCollection, 
                                                      Boolean& exists, LListener *listener) {
    CDAVMessage theMessage;    
	LHTTPResponse theResponse;
	string theHost, theResource = resource;
    SInt32 thePort;
    CDAVPropertyVector props;
    props.push_back(ETag);
 
    _PrepareHostFields(theHost, thePort, theResource, theMessage);
    
    if (inResourceIsCollection) {
		string msgBody;
		CXMLStringUtils::beginXMLDAVBody(msgBody);
		_addPropFindElement(msgBody, props);
		theMessage.SetMessageBody(msgBody.c_str());
		theMessage.SetContentType(XMLMIMETYPE); 
		theMessage.SetDepth(DAVTypes::ZERO);
	}
	
	bool doCommunication = true;
	while (doCommunication) {
		doCommunication = false;
		mAttemptRetry = false;
		try {
			StConnection connection (this, inThread, listener);
			if (inResourceIsCollection) {
				connection.GetConnection().PropFind(
											theHost,
											theResource,
											theMessage,
											theResponse,
											thePort);
			} else {
				connection.GetConnection().Head(
											theHost,
											theResource,
											theMessage,
											theResponse,
											thePort);
			}
		} catch (LSSLException sslE) {
	    	_HandleSSLexception(theResponse, sslE);
	    	return FAILURE;
		} catch (...) {
			if (mAttemptRetry)
				doCommunication = true;
		}
	}
	
	mLastResponse = theResponse.GetResponseCode();
	if ((mLastResponse != 200/*CDAVConnection::kHTTPMultiStatus*/) &&
		(mLastResponse != CDAVConnection::kHTTPMultiStatus) && 
		(mLastResponse != CDAVConnection::kHTTPNotFound))
	{
		return FAILURE;
	}
	
	if ((mLastResponse == 200/*CDAVConnection::kHTTPMultiStatus*/) || 
		(mLastResponse == CDAVConnection::kHTTPMultiStatus))
      exists = true;
	else if (mLastResponse == CDAVConnection::kHTTPNotFound)
      exists = false;
   
   return SUCCESS;                                 
}                                                      

// ---------------------------------------------------------------------------
//		¥ FindAllProperties()
// ---------------------------------------------------------------------------
//	
CDAVRequest::ReqStatus CDAVRequest::FindAllProperties(LThread& inThread, const string& resource, 
				CDAVPropertyVector &props, LListener *listener) {

	CDAVMessage theMessage;		
	LHTTPResponse theResponse;
	string theHost, theResource = resource;
	SInt32 thePort;

	_PrepareHostFields(theHost, thePort, theResource, theMessage);
	string msgBody;
   
   CXMLStringUtils::beginXMLDAVBody(msgBody);
   CXMLStringUtils::startElementWithDAVNS(msgBody, DAVTypes::PROPFIND);
   CXMLStringUtils::addEmptyElement(msgBody, DAVTypes::PROPNAME);
   CXMLStringUtils::endElement(msgBody, DAVTypes::PROPFIND);
      
   //theMessage.SetMessageBody("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n<propfind xmlns=\"DAV:\">\r\n<propname/>\r\n</propfind>\r\n");
   theMessage.SetMessageBody(msgBody.c_str());
   theMessage.SetContentType(XMLMIMETYPE);	
   theMessage.SetDepth(DAVTypes::ZERO);

	bool doCommunication = true;
	while (doCommunication) {
		doCommunication = false;
		mAttemptRetry = false;
	    try {
			StConnection connection (this, inThread, listener);
			connection.GetConnection().PropFind(
								theHost,
								theResource,
								theMessage,
								theResponse,
								thePort);
		
		} catch (LSSLException sslE) {
	    	_HandleSSLexception(theResponse, sslE);
	    	return FAILURE;
		} catch (...) {
			if (mAttemptRetry)
				doCommunication = true;
		}
	}
	
	mLastResponse = theResponse.GetResponseCode();
	if (mLastResponse != CDAVConnection::kHTTPMultiStatus) {
		_OnDavRequestError(theResponse);
		return FAILURE;

	} 

	CXMLDocument document;
	
	LDynamicBuffer *outBuf=	theResponse.GetReturnMessage()->GetMessageBody();
	LStream &stream = *(outBuf->GetBufferStream());
	
	XML_Error parseResult = ParseXMLDocument(stream, document);
	delete outBuf;
		
	if (XML_ERROR_NONE != parseResult)
		return XMLERROR;

	if (false == document.validateRoot(DAVTypes::MULTISTATUS, DAVTypes::NS_DAV))
		return XMLERROR;

	vector<CElement*> propelems;
    document.getRoot()->findAllChildren(DAVTypes::PROP, DAVTypes::NS_DAV, propelems, true/*recurse*/);
	vector<CElement*>::iterator psiter = propelems.begin();
    while (psiter != propelems.end()) {
		vector<CElement*>::iterator pnames = (*psiter)->getChildren().begin();
       while (pnames != (*psiter)->getChildren().end()) {
			string &ns =		(*pnames)->getNamespace();
          char* ns_c=const_cast<char*>(DAVTypes::NS_DAV);
          if (ns.compare("")!=0)
             ns_c = const_cast<char*>(ns.c_str());
          props.push_back(CDAVProperty((*pnames)->getName().c_str(), const_cast<const char*>(ns_c)));          
          pnames++;
       }
       psiter++;
   }
    
   return SUCCESS;
}

// ---------------------------------------------------------------------------
//		¥ GetItemProperties()
// ---------------------------------------------------------------------------
//	
CDAVRequest::ReqStatus CDAVRequest::GetItemProperties(LThread& inThread,  
                          CDAVItem &resource,
                          CDAVPropertyVector &props, LListener *listener) {

    CDAVMessage theMessage;    
	LHTTPResponse theResponse;
	string theHost, theResource = resource.GetHREF();
	SInt32 thePort;

	_PrepareHostFields(theHost, thePort, theResource, theMessage);
	
	string msgBody;
	CXMLStringUtils::beginXMLDAVBody(msgBody);
	_addPropFindElement(msgBody, props);
	theMessage.SetMessageBody(msgBody.c_str());
	theMessage.SetIsMIME(false);
	theMessage.SetContentType(XMLMIMETYPE); 
	theMessage.SetDepth(DAVTypes::ZERO);
	
	bool doCommunication = true;
	while (doCommunication) {
		doCommunication = false;
		mAttemptRetry = false;
		try {
			StConnection connection (this, inThread, listener);
			connection.GetConnection().PropFind(
										theHost,
										theResource,
										theMessage,
										theResponse,
										thePort);
		} catch (LSSLException sslE) {
		    _HandleSSLexception(theResponse, sslE);
		    return FAILURE;
		} catch (...) {
			if (mAttemptRetry)
				doCommunication = true;
		}
	}	
	mLastResponse = theResponse.GetResponseCode();
	if (mLastResponse != CDAVConnection::kHTTPMultiStatus) {
		_OnDavRequestError(theResponse);
		return FAILURE;
	}
		
	CXMLDocument document;
	
	LDynamicBuffer *outBuf=	theResponse.GetReturnMessage()->GetMessageBody();
	LStream &stream = *(outBuf->GetBufferStream());

	XML_Error parseResult = ParseXMLDocument(stream, document);
	delete outBuf;
	
	if (XML_ERROR_NONE != parseResult)
		return XMLERROR;
	
	vector<CElement*> responses;
	document.getRoot()->findAllChildren(DAVTypes::RESPONSE, DAVTypes::NS_DAV, responses, true/*recurse*/);
	vector<CElement*>::iterator respIter = responses.begin();
    //***teb - should this be ever the case?
    if (respIter == responses.end())
       return SUCCESS;
       
    CElement *propElem, *respElem = *respIter;
              
    propElem = respElem->findFirstChild(DAVTypes::PROP, DAVTypes::NS_DAV, true/*recurse*/);

	vector<CElement*>::iterator psiter;
	for (psiter = propElem->getChildren().begin(); psiter != propElem->getChildren().end(); psiter++) {		
		//***teb - this bit right here is one big ugly hack.   It works for now
		//since I arbitrarily decided to not support non DAV namespaces in 
		//properties for the time being.   Ugly, ugly, ugly......
		
		string rawPval;
       (*psiter)->getMixedContentAsXML(rawPval);
		CDAVProperty *prop = _getBuiltinPropertyType(*psiter);
       if (prop) {
          if (prop == &LockDiscovery) {
             CElement *activeLock = (*psiter)->findFirstChild(DAVTypes::ACTIVELOCK, DAVTypes::NS_DAV, false /*rescurse*/);
             if (NULL != activeLock) {
                _fillItemLockInfoFromElement(activeLock, resource);
             }
          } 
          std::string pval;
          std::vector<std::string>& chunks = (*psiter)->getCDATA();
          std::vector<std::string>::iterator iter = chunks.begin();
          while (iter != chunks.end()) {
             pval.append((*iter));
             iter++;
          }
          resource.SetPropertyValue(*prop, pval);
          resource.SetRawPropertyValue(*prop, rawPval);
      
      } else {
         std::string name = (*psiter)->getName(), ns = (*psiter)->getNamespace();
         std::string pval;
         std::vector<std::string>& chunks = (*psiter)->getCDATA();
         std::vector<std::string>::iterator iter = chunks.begin();
         while (iter != chunks.end()) {
            pval.append((*iter));
            iter++;
         }
         CDAVProperty prop(name.c_str(), ns.c_str());
         resource.SetPropertyValue(prop, pval);
         resource.SetRawPropertyValue(prop, rawPval);
      }

   }
   _OnDavItemDataChange(resource);
   return SUCCESS;
}

// ---------------------------------------------------------------------------
//		¥ SetItemProperty()
// ---------------------------------------------------------------------------
//	
CDAVRequest::ReqStatus CDAVRequest::SetItemProperty(LThread& inThread, CDAVItem &resource, 
                          CDAVProperty &prop, string &propVal,
                          LListener *listener) {
                          
    CDAVMessage theMessage;    
	LHTTPResponse theResponse;
    string theHost, theResource = resource.GetHREF();
    SInt32 thePort;

    _PrepareHostFields(theHost, thePort, theResource, theMessage);
    
    string msgBody;
    CXMLStringUtils::beginXMLDAVBody(msgBody);
    CXMLStringUtils::startElementWithDAVNS(msgBody, DAVTypes::PROPERTYUPDATE);
    CXMLStringUtils::startElementWithDAVNS(msgBody, DAVTypes::SET);
    CXMLStringUtils::startElementWithDAVNS(msgBody, DAVTypes::PROP);
    
    if (prop.getIsInDAVNamespace()) {
       CXMLStringUtils::startElementWithDAVNS(msgBody, prop.GetPropertyName().c_str());
    } else {
       CXMLStringUtils::startElementWithOtherNS(msgBody, prop.GetPropertyName().c_str(),
                        prop.GetNamespace().c_str());
    }
    msgBody+=propVal;
    CXMLStringUtils::endElement(msgBody, prop.GetPropertyName().c_str());

    CXMLStringUtils::endElement(msgBody, DAVTypes::PROP);
    CXMLStringUtils::endElement(msgBody, DAVTypes::SET);
    CXMLStringUtils::endElement(msgBody, DAVTypes::PROPERTYUPDATE);

    theMessage.SetMessageBody(msgBody.c_str());
    theMessage.SetIsMIME(false);
    theMessage.SetContentType(XMLMIMETYPE);	
     
	bool doCommunication = true;
	while (doCommunication) {
		doCommunication = false;
		mAttemptRetry = false;
		try {
			StConnection connection (this, inThread, listener);
			connection.GetConnection().PropPatch(
						theHost,
						theResource,
						theMessage,
						theResponse,
						thePort);
		} catch (LSSLException sslE) {
		    _HandleSSLexception(theResponse, sslE);
		    return FAILURE;
	    } catch (...) {
			if (mAttemptRetry)
				doCommunication = true;
	    }
	}    
	
	mLastResponse = theResponse.GetResponseCode();
	if (mLastResponse != CDAVConnection::kHTTPMultiStatus) {
		_OnDavRequestError(theResponse);
		return FAILURE;
	}

   CXMLDocument document;
    
   LDynamicBuffer *outBuf=	theResponse.GetReturnMessage()->GetMessageBody();
   LStream &stream = *(outBuf->GetBufferStream());

    XML_Error parseResult = ParseXMLDocument(stream, document);
    delete outBuf;
    
    if (XML_ERROR_NONE != parseResult)
       return XMLERROR;
    
    vector<CElement*> responses;
    document.getRoot()->findAllChildren(DAVTypes::RESPONSE, DAVTypes::NS_DAV, responses, true/*recurse*/);
    vector<CElement*>::iterator respIter = responses.begin();
    //***teb - should this be ever the case?
    if (respIter == responses.end())
       return SUCCESS;
       
    CElement *respElem = *respIter, *statElem;
              
    statElem = respElem->findFirstChild(DAVTypes::STATUS, DAVTypes::NS_DAV, true/*recurse*/);
    
    if (statElem) {
       string resp;
       statElem->getCDATAasString(resp);
       long respCode;
	   char * p = strtok(const_cast<char*>(resp.c_str()), " ");
	   p = strtok(NULL, " ");
	   if (p) {
		   respCode = atol(p);
		   if (respCode==200) {
		      resource.SetPropertyValue(prop, propVal);
		      resource.SetRawPropertyValue(prop, propVal);
		   }
	   }
    }
        
	return SUCCESS;
}

// ---------------------------------------------------------------------------
//		¥ RemoveItemProperty()
// ---------------------------------------------------------------------------
//	
CDAVRequest::ReqStatus CDAVRequest::RemoveItemProperty(LThread& inThread, CDAVItem &resource, 
                                CDAVProperty &prop, LListener *listener) {

    CDAVMessage theMessage;    
	LHTTPResponse theResponse;
    string theHost, theResource = resource.GetHREF();
    SInt32 thePort;

    _PrepareHostFields(theHost, thePort, theResource, theMessage);
    
    string msgBody;
    CXMLStringUtils::beginXMLDAVBody(msgBody);
    CXMLStringUtils::startElementWithDAVNS(msgBody, DAVTypes::PROPERTYUPDATE);
    CXMLStringUtils::startElementWithDAVNS(msgBody, DAVTypes::REMOVE);
    CXMLStringUtils::startElementWithDAVNS(msgBody, DAVTypes::PROP);
    
    if (prop.getIsInDAVNamespace()) {
       CXMLStringUtils::startElementWithDAVNS(msgBody, prop.GetPropertyName().c_str());
    } else {
       CXMLStringUtils::startElementWithOtherNS(msgBody, prop.GetPropertyName().c_str(),
                        prop.GetNamespace().c_str());
    }
    CXMLStringUtils::endElement(msgBody, prop.GetPropertyName().c_str());

    CXMLStringUtils::endElement(msgBody, DAVTypes::PROP);
    CXMLStringUtils::endElement(msgBody, DAVTypes::REMOVE);
    CXMLStringUtils::endElement(msgBody, DAVTypes::PROPERTYUPDATE);

    theMessage.SetMessageBody(msgBody.c_str());
    theMessage.SetIsMIME(false);
    theMessage.SetContentType(XMLMIMETYPE);	
     
	bool doCommunication = true;
	while (doCommunication) {
		doCommunication = false;
		mAttemptRetry = false;
		try {
			StConnection connection (this, inThread, listener);
			connection.GetConnection().PropPatch(
						theHost,
						theResource,
						theMessage,
						theResponse,
						thePort);
		} catch (LSSLException sslE) {
		    _HandleSSLexception(theResponse, sslE);
		    return FAILURE;
	    } catch (...) {
			if (mAttemptRetry)
				doCommunication = true;
	    }
	}
		    
   mLastResponse = theResponse.GetResponseCode();
   if (mLastResponse != CDAVConnection::kHTTPMultiStatus) {
      _OnDavRequestError(theResponse);
      return FAILURE;
   }

   CXMLDocument document;
    
   LDynamicBuffer *outBuf=	theResponse.GetReturnMessage()->GetMessageBody();
   LStream &stream = *(outBuf->GetBufferStream());

    XML_Error parseResult = ParseXMLDocument(stream, document);
    delete outBuf;
    
    if (XML_ERROR_NONE != parseResult)
       return XMLERROR;
    
    vector<CElement*> responses;
    document.getRoot()->findAllChildren(DAVTypes::RESPONSE, DAVTypes::NS_DAV, responses, true/*recurse*/);
    vector<CElement*>::iterator respIter = responses.begin();
    //***teb - should this be ever the case?
    if (respIter == responses.end())
       return SUCCESS;
       
    CElement *respElem = *respIter, *statElem;
              
    statElem = respElem->findFirstChild(DAVTypes::STATUS, DAVTypes::NS_DAV, true/*recurse*/);
    
    if (statElem) {
       string resp;
       statElem->getCDATAasString(resp);
       long respCode;
       
	   char * p = strtok(const_cast<char*>(resp.c_str()), " ");
	   p = strtok(NULL, " ");
	   if (p) {
		   respCode = atol(p);
		   if (respCode==200) {
		      resource.RemoveProperty(&prop);
		   }
	   }
    }
        
	return SUCCESS;
}

// ---------------------------------------------------------------------------
//		¥ ListDirectory()
// ---------------------------------------------------------------------------
//	
CDAVRequest::ReqStatus CDAVRequest::ListDirectory(LThread& inThread,  
							const string& resource, CDAVItemVector &children,
                          CDAVPropertyVector &props, LListener *listener,
                          DAVTypes::PropertyDepth depth) {
                          
    CDAVMessage theMessage;    
	LHTTPResponse theResponse;
	string theHost, theResource = resource;
	SInt32 thePort;

	_PrepareHostFields(theHost, thePort, theResource, theMessage);
	
	string msgBody;
    
    CDAVPropertyVector lsProps(props);
	lsProps.push_back (ResourceType);
    CXMLStringUtils::beginXMLDAVBody(msgBody);
    _addPropFindElement(msgBody, lsProps);
    theMessage.SetMessageBody(msgBody.c_str());
    
    theMessage.SetContentType(XMLMIMETYPE);
    theMessage.SetDepth(depth);
    
	bool doCommunication = true;
	while (doCommunication) {
		doCommunication = false;
		mAttemptRetry = false;
		try {
			StConnection connection (this, inThread, listener);
			connection.GetConnection().PropFind(
										theHost,
										theResource,
										theMessage,
										theResponse,
										thePort);
		} catch (LSSLException sslE) {
		    _HandleSSLexception(theResponse, sslE);
		    return FAILURE;
		} catch (...) {
			if (mAttemptRetry)
				doCommunication = true;
		}
	}	
	
	mLastResponse = theResponse.GetResponseCode();
	if (mLastResponse != CDAVConnection::kHTTPMultiStatus) {
		_OnDavRequestError(theResponse);
		return FAILURE;
	}
		
	CXMLDocument document;

	LDynamicBuffer *outBuf=	theResponse.GetReturnMessage()->GetMessageBody();
	LStream &stream = *(outBuf->GetBufferStream());

	XML_Error parseResult = ParseXMLDocument(stream, document);
	delete outBuf;

	if (XML_ERROR_NONE != parseResult)
		return XMLERROR;

	if (false == document.validateRoot(DAVTypes::MULTISTATUS, DAVTypes::NS_DAV))
		return XMLERROR;

	vector<CElement*> responses;
	document.getRoot()->findAllChildren(DAVTypes::RESPONSE, DAVTypes::NS_DAV, responses, true/*recurse*/);
	vector<CElement*>::iterator respIter = responses.begin();
    
    while (respIter != responses.end()) {
       CElement *hrefElem, *propElem, *respElem = *respIter;
       respIter++;
              
       hrefElem = respElem->findFirstChild(DAVTypes::HREF, DAVTypes::NS_DAV, false/*no recurse*/);
		if (NULL == hrefElem)
			continue;
			
		//i used to use a reference to the hrefElem's CDATA, but now that it's
          //sent through the normalization fxn, a local copy is made
          //std::string href = *hrefElem->getCDATA().begin();
		string href;
		vector<string> cd = hrefElem->getCDATA();
		vector<string>::iterator cd_iter;
		for (cd_iter = cd.begin(); cd_iter != cd.end(); cd_iter++) {
			href += *cd_iter;
		}

		_NormalizeHREF(href, theResource);
		
          //ShareMation returns the fully qualified URL in the HREF - need to normalize 
          // this.  Currently, DAVLib constructs protocol and host/port from the request
          // object.  
           
		CDAVItem newItem (href);
		if (newItem.GetHREF() == resource)
			continue;

		propElem = respElem->findFirstChild(DAVTypes::PROP, DAVTypes::NS_DAV, true/*recurse*/);
		vector<CElement*>::iterator psiter;

		for (psiter = propElem->getChildren().begin(); psiter != propElem->getChildren().end(); psiter++) {			
             //***teb - this bit right here is one big ugly hack.  It works for now
             //since I arbitrarily decided to not support non DAV namespaces in 
             //properties for the time being.  Ugly, ugly, ugly......
			
			//***teb - removed this on 12/30/2000; I think that the changes to the 
			// classes CDAVProperty and CDAVItem eliminate the dependancy on the
			// pointer comparisons that caused this previous comment.
			//if (!_isBuiltinPropertyType(*psiter))
			//	continue;
				
			CDAVProperty prop ((*psiter)->getName().c_str(), (*psiter)->getNamespace().c_str());
			if (prop.GetPropertyName().compare(DAVTypes::LOCKDISCOVERY) == 0)
			{
				CElement *activeLock = (*psiter)->findFirstChild(DAVTypes::ACTIVELOCK, DAVTypes::NS_DAV, false /*recurse*/);
				if (NULL != activeLock) {
                      _fillItemLockInfoFromElement(activeLock, newItem);
                }
			}
			else
			{
				string pval;
				vector<string>& chunks = (*psiter)->getCDATA();
				vector<string>::iterator iter;
				for (iter = chunks.begin(); iter != chunks.end(); iter++) {
					pval.append((*iter));
                }
                newItem.SetPropertyValue(prop, pval);
				if (prop.GetPropertyName().compare(DAVTypes::RESOURCETYPE)==0) {
					vector<CElement*> rtKids = (*psiter)->getChildren();
					vector<CElement*>::iterator rtIter = rtKids.begin();
                   while (rtIter != rtKids.end()) {
                      if ((*rtIter)->getName().compare(DAVTypes::COLLECTION)==0)
                         newItem.SetItemType(CDAVItem::COLLECTION);
                      rtIter++;
                   }
                }
             }
          }

          _OnDavItemCreated(newItem);
          children.push_back(newItem);
       }
    
	return SUCCESS;
                             
}



// ---------------------------------------------------------------------------
//		¥ ListLocks()
// ---------------------------------------------------------------------------
//	
CDAVRequest::ReqStatus CDAVRequest::ListLocks(LThread& inThread, const string& resource, LListener *listener) {
							
	CDAVMessage theMessage;		
	LHTTPResponse theResponse;
	string theHost, theResource = resource;
	SInt32 thePort;

	_PrepareHostFields(theHost, thePort, theResource, theMessage);
	string msgBody;
    CXMLStringUtils::beginXMLDAVBody(msgBody);
    CXMLStringUtils::startElementWithDAVNS(msgBody, DAVTypes::PROPFIND);
    CXMLStringUtils::startElement(msgBody, DAVTypes::PROP);
    CXMLStringUtils::addEmptyElement(msgBody, DAVTypes::LOCKDISCOVERY);
    CXMLStringUtils::endElement(msgBody, DAVTypes::PROP);
    CXMLStringUtils::endElement(msgBody, DAVTypes::PROPFIND);
    
    //theMessage.SetMessageBody("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n<propfind xmlns=\"DAV:\">\r\n<prop><lockdiscovery/></prop>\r\n</propfind>\r\n");
    theMessage.SetMessageBody(msgBody.c_str());
    theMessage.SetContentType(XMLMIMETYPE);	
    theMessage.SetDepth(DAVTypes::PROPINFINITY);
    
	bool doCommunication = true;
	while (doCommunication) {
		doCommunication = false;
		mAttemptRetry = false;
		try {
			StConnection connection (this, inThread, listener);
			connection.GetConnection().PropFind(
										theHost,
										theResource,
										theMessage,
										theResponse,
										thePort);
		} catch (LSSLException sslE) {
		    _HandleSSLexception(theResponse, sslE);
		    return FAILURE;
		} catch (...) {
			if (mAttemptRetry)
				doCommunication = true;	
		}
	}	
	mLastResponse = theResponse.GetResponseCode();
	if (mLastResponse != CDAVConnection::kHTTPMultiStatus) {
		_OnDavRequestError(theResponse);
		return FAILURE;
	}
	

	CXMLDocument document;
	
	LDynamicBuffer *outBuf= theResponse.GetReturnMessage()->GetMessageBody();
    LStream &stream = *(outBuf->GetBufferStream());
    
    XML_Error parseResult = ParseXMLDocument(stream, document);
    delete outBuf;
        
    if (XML_ERROR_NONE != parseResult)
       return XMLERROR;
    
    CElement *root = document.getRoot();
    //make sure we have a multi-status
    if ((root->getNamespace().compare(DAVTypes::NS_DAV)!=0) && 
        (root->getName().compare(DAVTypes::MULTISTATUS)!=0))
       return XMLERROR;
    
    
    //***teb - gee, this does a lot :)
	return SUCCESS;                         
}


// ---------------------------------------------------------------------------
//		¥ CreateDirectory()
// ---------------------------------------------------------------------------
//	Create a new collection on the DAV server
CDAVRequest::ReqStatus CDAVRequest::CreateDirectory(LThread& inThread, const string& resource,
                                                    const char* lockToken, LListener *listener) {
   CDAVMessage theMessage;    
   LHTTPResponse theResponse;
	string theHost, theResource = resource;
	SInt32 thePort;
							
	_PrepareHostFields(theHost, thePort, theResource, theMessage);
	theMessage.SetIsMIME(false);

	if (NULL != lockToken) {
		string pnt (resource);
		string::size_type idx = pnt.rfind('/');
       if (idx != 0) {
			pnt.resize (idx);
			string tmpStr = _buildIfHeaderForLock(m_context->GetServerName().c_str(), pnt.c_str(), lockToken);
         theMessage.SetIfHeader(tmpStr.c_str());
       }
   }
   
	bool doCommunication = true;
	while (doCommunication) {
		doCommunication = false;
		mAttemptRetry = false;
		try {
			StConnection connection (this, inThread, listener);
			connection.GetConnection().MkCol(theHost, theResource, theMessage, theResponse, thePort);
		} catch (LSSLException sslE) {
		    _HandleSSLexception(theResponse, sslE);
		    return FAILURE;
		} catch (...) {
			if (mAttemptRetry)
				doCommunication = true;		
		}
	}	
	
	mLastResponse = theResponse.GetResponseCode();   
	if (mLastResponse != CDAVConnection::kHTTPCreated) {
		_OnDavRequestError(theResponse);
		return FAILURE;
	}
	return SUCCESS;		
}

// ---------------------------------------------------------------------------
//		¥ DeleteResource()
// ---------------------------------------------------------------------------
//	Remove a resource from the DAV server
CDAVRequest::ReqStatus CDAVRequest::DeleteResource(LThread& inThread, const string& resource,
                                                   LListener *listener) {
	CDAVMessage theMessage;		
	LHTTPResponse theResponse;
	string theHost, theResource = resource;
	SInt32 thePort;
							
	_PrepareHostFields(theHost, thePort, theResource, theMessage);

	bool doCommunication = true;
	while (doCommunication) {
		doCommunication = false;
		mAttemptRetry = false;
		try {
			StConnection connection (this, inThread, listener);
			connection.GetConnection().Delete(theHost, theResource, theMessage, theResponse, thePort);
		} catch (LSSLException sslE) {
		    _HandleSSLexception(theResponse, sslE);
		    return FAILURE;
		} catch (...) {
			if (mAttemptRetry)
				doCommunication = true;	
		}
	}
		
	mLastResponse = theResponse.GetResponseCode();   
	if (	(mLastResponse != CDAVConnection::kHTTPNoContent) &&
			(mLastResponse != kHTTPRequestOK) &&
			(mLastResponse != CDAVConnection::kHTTPMultiStatus))
	{
       _OnDavRequestError(theResponse);
       return FAILURE;
    }
       
	return SUCCESS;    
}


// ---------------------------------------------------------------------------
//		¥ DeleteResource()
// ---------------------------------------------------------------------------
//	Remove a resource from the DAV server
CDAVRequest::ReqStatus CDAVRequest::DeleteResource(LThread& inThread, CDAVItem& item, const char* lockToken,
													const char* pntLockToken, LListener *listener) {
	CDAVMessage theMessage;		
	LHTTPResponse theResponse;
	string theHost, theResource = item.GetHREF();
	SInt32 thePort;
	Boolean hasIfHdr = false;
	string ifHdr;  
			
	_PrepareHostFields(theHost, thePort, theResource, theMessage);
	
	if (NULL != lockToken) {
		ifHdr = _buildIfHeaderForLock(m_context->GetServerName().c_str(), item.GetURI().c_str(), lockToken);
		hasIfHdr = true;
	}
	if (NULL != pntLockToken) {
		string pntHref;
      if (item.GetParentHREF(pntHref)) {
			string tmpPntLT = _buildIfHeaderForLock(m_context->GetServerName().c_str(), pntHref.c_str() ,pntLockToken);
			if (hasIfHdr)
				ifHdr+=' ';
			ifHdr += tmpPntLT;
			hasIfHdr = true;
		}
	}
	
	if (hasIfHdr)
		theMessage.SetIfHeader(ifHdr.c_str());

	bool doCommunication = true;
	while (doCommunication) {
		doCommunication = false;
		mAttemptRetry = false;
		try {
			StConnection connection (this, inThread, listener);
			connection.GetConnection().Delete(theHost, theResource, theMessage, theResponse, thePort);
		} catch (LSSLException sslE) {
		    _HandleSSLexception(theResponse, sslE);
		    return FAILURE;
		} catch (...) {
			if (mAttemptRetry)
				doCommunication = true;	
		}
	}	
	mLastResponse = theResponse.GetResponseCode();   
	if (	(mLastResponse != CDAVConnection::kHTTPNoContent) &&
			(mLastResponse != kHTTPRequestOK) &&
			(mLastResponse != CDAVConnection::kHTTPMultiStatus))
	{
       _OnDavRequestError(theResponse);
       return FAILURE;
    }
       
	return SUCCESS;    
}

// ---------------------------------------------------------------------------
//		¥ CopyResource()
// ---------------------------------------------------------------------------
//	Used to make a copy of a resource
CDAVRequest::ReqStatus CDAVRequest::CopyResource(LThread& inThread, const string& source, const string& destination, 
						DAVTypes::Overwrite overwrite, Boolean propsOnly, LListener *listener) {

	CDAVMessage theMessage;		
	LHTTPResponse theResponse;
	string theHost, theResource = source;
	SInt32 thePort;
							
	_PrepareHostFields(theHost, thePort, theResource, theMessage);
	
	theMessage.SetDestination(destination.c_str());
   theMessage.SetOverwrite(overwrite);
   if (propsOnly)
      theMessage.SetDepth(DAVTypes::ZERO);
      
	bool doCommunication = true;
	while (doCommunication) {
		doCommunication = false;
		mAttemptRetry = false;
		try {
			StConnection connection (this, inThread, listener);
			connection.GetConnection().Copy(theHost, theResource, theMessage, theResponse, thePort);
		} catch (LSSLException sslE) {
		    _HandleSSLexception(theResponse, sslE);
		    return FAILURE;
		} catch (...) {
			if (mAttemptRetry)
				doCommunication = true;	
		}
	}	
	mLastResponse = theResponse.GetResponseCode();

	if (mLastResponse != CDAVConnection::kHTTPCreated) {
       _OnDavRequestError(theResponse);
       return FAILURE;
    }
    return SUCCESS;
}

                       
// ---------------------------------------------------------------------------
//		¥ MoveResource()
// ---------------------------------------------------------------------------
//	Used to move a resource, including collections.
CDAVRequest::ReqStatus CDAVRequest::MoveResource(LThread& inThread, const string& source , const string& destination, 
                       DAVTypes::Overwrite overwrite, LListener *listener) {
   CDAVMessage theMessage;    
   LHTTPResponse theResponse;
	string theHost, theResource = source;
	SInt32 thePort;

	_PrepareHostFields(theHost, thePort, theResource, theMessage);
	theMessage.SetServer(m_context->GetServerName().c_str());
	
	theMessage.SetDestination(destination.c_str());
	theMessage.SetOverwrite(overwrite);

	bool doCommunication = true;
	while (doCommunication) {
		doCommunication = false;
		mAttemptRetry = false;
		try {
			StConnection connection (this, inThread, listener);
			connection.GetConnection().Move(theHost, theResource, theMessage, theResponse, thePort);
		} catch (LSSLException sslE) {
		    _HandleSSLexception(theResponse, sslE);
		    return FAILURE;
		} catch (...) {
			if (mAttemptRetry) 
				doCommunication = true;	
		}
	}
	mLastResponse = theResponse.GetResponseCode();
	if ((mLastResponse != CDAVConnection::kHTTPCreated) && 
	    (mLastResponse != CDAVConnection::kHTTPNoContent)) {
       _OnDavRequestError(theResponse);
       return FAILURE;
    }
    
    return SUCCESS;
}
                       

// ---------------------------------------------------------------------------
//		¥ PutResource()
// ---------------------------------------------------------------------------
//	uses an HTTP PUT to upload a file identified to a resource on a DAV server
CDAVRequest::ReqStatus CDAVRequest::PutResource(LThread& inThread, const string& resource, const FSSpec &inputSpec,
												bool useMacBinary, const char* locktoken, const char* pntLockToken,
												Boolean *itemCreated, LListener *listener ) {

	CDAVMessage			theMessage;
	LHTTPResponse		theResponse;
	string				theHost, theResource = resource;
	SInt32				thePort;
	Boolean				hasIfHdr = false;
	string				ifHdr;
	OSErr				err;
	ReqStatus			result = SUCCESS;
	LStream*			inStream = NULL;
	
	try
	{
		_PrepareHostFields(theHost, thePort, theResource, theMessage);
	 
		theMessage.SetServer(m_context->GetServerName().c_str());
				
		if (useMacBinary)
		{
			inStream = new LMacBinaryEncoder (inputSpec);
			theMessage.SetContentType ("application/x-macbinary");
		}
		else
		{
			FInfo fndrInfo;
			err = FSpGetFInfo(&inputSpec, &fndrInfo);
   			//if (err) continue; //***teb - Hello!! Error checking??
             
		   ICMapEntry entry;
		   OSErr ierr = UInternetConfig::PP_ICMapTypeCreator( fndrInfo.fdType, fndrInfo.fdCreator, inputSpec.name ,entry);
		   if (ierr == noErr) {
		      LStr255 tmp;
		      tmp.Assign(entry.MIMEType);
		      theMessage.SetContentType(DAVp2cstr(tmp));
		   }
			LFileStream* fileStream  = new LFileStream (inputSpec);
			fileStream->OpenDataFork(fsRdPerm);
			inStream = fileStream;
		}
		
		theMessage.SetMessageBody (inStream, inStream->GetLength(), false);
		
		if (NULL != locktoken) {
			ifHdr = _buildIfHeaderForLock(m_context->GetServerName().c_str(), resource.c_str(), locktoken);
			hasIfHdr = true;
		}
		
		if (NULL != pntLockToken) {
			string pntHref;
			if (CURLStringUtils::GetURIParent(resource, pntHref)) {
				string tmpPntLT = _buildIfHeaderForLock(m_context->GetServerName().c_str(), pntHref.c_str(), pntLockToken);
				if (hasIfHdr)
				ifHdr+=' ';
				ifHdr += tmpPntLT;
				hasIfHdr = true;
			}
		}
		
		if (hasIfHdr) 
			theMessage.SetIfHeader(ifHdr.c_str());

		bool doCommunication = true;
		while (doCommunication) {
			doCommunication = false;
			mAttemptRetry = false;
			try {
				StConnection connection (this, inThread, listener);
				connection.GetConnection().Put(theHost, theResource, theMessage, theResponse, thePort);
				if (_getWasCanceled()) 
					return CANCEL;
		    } catch (LSSLException sslE) {
			    _HandleSSLexception(theResponse, sslE);
		    return FAILURE;
			} catch (...) {
				if (mAttemptRetry)
					doCommunication = true;	
			}
		}
		mLastResponse = theResponse.GetResponseCode();
		if (	(mLastResponse != CDAVConnection::kHTTPCreated) &&
				(mLastResponse != CDAVConnection::kHTTPNoContent) && 
				(mLastResponse != kHTTPRequestOK) )
		{
       _OnDavRequestError(theResponse);
       return FAILURE;
    }
   
    if (itemCreated) {
		if ((CDAVConnection::kHTTPCreated == mLastResponse) || (CDAVConnection::kHTTPOK == mLastResponse))
          *itemCreated = true;
       else
          *itemCreated = false;
    }
	}
	catch (...)
	{
		result =  FAILURE;
	}

	delete inStream;
	return result;
 }


// ---------------------------------------------------------------------------
//		¥ GetResource()
// ---------------------------------------------------------------------------
//	uses an HTTP GET to download a resource identified in the parameter resource
//	and places the content of this into a file denoted by outputSpec

CDAVRequest::ReqStatus CDAVRequest::GetResource(LThread& inThread, const string& resource,
								const FSSpec &outputSpec, bool isMacBinary, SInt32 expectedSize, LListener *listener) {
	CDAVMessage				theMessage;		
	LHTTPResponse			theResponse;
	string					theHost, theResource = resource;
	SInt32					thePort;
	LMacBinaryDecoder		macBinaryDecoder;
	LWriteOnlyFileStream	dataFileStream;
	
	_PrepareHostFields(theHost, thePort, theResource, theMessage);
 
	theMessage.SetServer(m_context->GetServerName().c_str());
	
	if (isMacBinary)
	{	
//		OSErr err = ::FSpCreate (&outputSpec, '----', '----', smSystemScript);
//		ThrowIf_((err != noErr) && (err != dupFNErr));
		macBinaryDecoder.SetSpec (outputSpec, expectedSize);
		theResponse.SetExternalStream (&macBinaryDecoder);
	}
	
	else
	{		
		OSErr err = ::FSpCreate (&outputSpec, '????', '????', smSystemScript);
		ThrowIf_((err != noErr) && (err != dupFNErr));
		if (dupFNErr == err) {
			err = ::FSpDelete(&outputSpec);
			ThrowIf_(err != noErr);
			err = ::FSpCreate (&outputSpec, '????', '????', smSystemScript);
			ThrowIf_((err != noErr) && (err != dupFNErr));
		}
		dataFileStream.Open (outputSpec);
		theResponse.SetExternalStream (&dataFileStream);
	}
	
	bool doCommunication = true;
	while (doCommunication) {
		doCommunication = false;
		mAttemptRetry = false;
		try {
			StConnection connection (this, inThread, listener);
			connection.GetConnection().Get(theHost, theResource, theMessage, theResponse, thePort);
			if (_getWasCanceled()) 
				return CANCEL;
		} catch (LSSLException sslE) {
		    _HandleSSLexception(theResponse, sslE);
		    return FAILURE;
		} catch (...) {
			if (mAttemptRetry)
				doCommunication = true;	
		}
	}	
	
	mLastResponse = theResponse.GetResponseCode();
	if (mLastResponse != kHTTPRequestOK) {
		_OnDavRequestError(theResponse);
		return FAILURE;
	}
	
	if (!isMacBinary)
	{
		try {
			ICMapEntry	entry;
			FInfo		fndrInfo;

			FSpGetFInfo (&outputSpec, &fndrInfo);
			
#warning !!! Should use MIME type if possible
#warning !!! Should strip out directory, in case resource is bigger than 255

			Str255 pResource;
			unsigned long start = max (0L, (long)theResource.size() - 255L);
			pResource [0] = theResource.size() - start;

			BlockMoveData (theResource.c_str() + start, pResource + 1, pResource [0]);
			
			OSErr icErr = UInternetConfig::PP_ICMapFilename (pResource,  entry);
            if (noErr == icErr) {
				fndrInfo.fdCreator = entry.fileCreator;
				fndrInfo.fdType = entry.fileType;
 				FSpSetFInfo (&outputSpec, &fndrInfo);
			}			
		} catch (...) {
		
		}
	}
	return SUCCESS;
}


// ---------------------------------------------------------------------------
//		¥ LockResource()
// ---------------------------------------------------------------------------
//	
CDAVRequest::ReqStatus CDAVRequest::LockResource(LThread& inThread, CDAVItem &item, 
                                                 CDAVItem::LockScope lockScope,
                                                 const char* owner, SInt32 timeout_seconds,  
                                                 CDAVItem::LockType lockType,
                                                 LListener *listener ) {
    #pragma unused(lockType)
                                                 
    CDAVMessage theMessage;    
	LHTTPResponse theResponse;
	string theHost, theResource;
	SInt32 thePort;
	
	theResource = item.GetHREF();
	_PrepareHostFields(theHost, thePort, theResource, theMessage);
 
	theMessage.SetServer(m_context->GetServerName().c_str());
	theMessage.SetContentType(XMLMIMETYPE); 
	theMessage.SetDepth(DAVTypes::ZERO);
		
	string msgBody;
    CXMLStringUtils::beginXMLDAVBody(msgBody);
    CXMLStringUtils::startElementWithDAVNS(msgBody, DAVTypes::LOCKINFO);
    CXMLStringUtils::startElement(msgBody, DAVTypes::LOCKSCOPE);
    if (CDAVItem::EXCLUSIVE == lockScope) {
       CXMLStringUtils::addEmptyElement(msgBody, DAVTypes::EXCLUSIVE);
    } else {
       CXMLStringUtils::addEmptyElement(msgBody, DAVTypes::SHARED);
    }
    CXMLStringUtils::endElement(msgBody, DAVTypes::LOCKSCOPE);
    CXMLStringUtils::startElement(msgBody, DAVTypes::LOCKTYPE);
    CXMLStringUtils::addEmptyElement(msgBody, DAVTypes::WRITE);
    CXMLStringUtils::endElement(msgBody, DAVTypes::LOCKTYPE);
    if (NULL != owner) {
       CXMLStringUtils::startElement(msgBody, DAVTypes::OWNER);
       CXMLStringUtils::startElement(msgBody, DAVTypes::HREF);
       msgBody.append(owner);
       CXMLStringUtils::endElement(msgBody, DAVTypes::HREF);
       CXMLStringUtils::endElement(msgBody, DAVTypes::OWNER);
    }
    CXMLStringUtils::endElement(msgBody, DAVTypes::LOCKINFO);
    
    theMessage.SetMessageBody(msgBody.c_str());    
    if (0 == timeout_seconds)
       theMessage.SetTimeout("Infinite, Second-4100000000");
    else {
		string timeoutStr("Second-");
       LStr255 tmpStr(timeout_seconds);
       timeoutStr.append(DAVp2cstr(tmpStr));
       theMessage.SetTimeout(timeoutStr.c_str());
    }
    
	bool doCommunication = true;
	while (doCommunication) {
		doCommunication = false;
		mAttemptRetry = false;
		try {
			StConnection connection (this, inThread, listener);
			connection.GetConnection().Lock(theHost, theResource, theMessage, theResponse, thePort);
		} catch (LSSLException sslE) {
		    _HandleSSLexception(theResponse, sslE);
		    return FAILURE;
		} catch (...) {
			if (mAttemptRetry)
				doCommunication = true;	
		}
	}
		
	mLastResponse = theResponse.GetResponseCode();
	if (mLastResponse != kHTTPRequestOK) {
       char* errStr = NULL;
		if (CDAVConnection::kHTTPPreconditionFailed == mLastResponse)
          errStr = NULL;
		else if (CDAVConnection::kHTTPLocked == mLastResponse)
          errStr = "The specified file is already locked.";//***teb - move to STR resource
          
       _OnDavRequestError(theResponse, const_cast<char*>(errStr));
       return FAILURE;
    }
    
    CXMLDocument document;
    
	LDynamicBuffer *outBuf= theResponse.GetReturnMessage()->GetMessageBody();
    LStream &stream = *(outBuf->GetBufferStream());
         
    XML_Error parseResult = ParseXMLDocument(stream, document);
    delete outBuf;
    
    if (XML_ERROR_NONE != parseResult)
       return XMLERROR;

    CElement *activeLock = document.getRoot()->findFirstChild(DAVTypes::ACTIVELOCK, DAVTypes::NS_DAV, false /*rescurse*/);
    if (NULL != activeLock) 
        _fillItemLockInfoFromElement(activeLock, item);
    item.SetIsLocalLock(true);
        
	return SUCCESS;
}

// ---------------------------------------------------------------------------
//		¥ UnlockResource()
// ---------------------------------------------------------------------------
//	
CDAVRequest::ReqStatus CDAVRequest::UnlockResource(LThread& inThread, CDAVItem &item, LListener *listener) {
    CDAVMessage theMessage;    
	LHTTPResponse theResponse;
	string theHost, theResource;
	SInt32 thePort;
	
	theResource = item.GetHREF();
	_PrepareHostFields(theHost, thePort, theResource, theMessage);
 
	theMessage.SetServer(m_context->GetServerName().c_str());
	string tmpStr("<");
    tmpStr.append(item.GetLockToken());
    tmpStr.append(">");
    theMessage.SetLockToken(tmpStr.c_str());

	bool doCommunication = true;
	while (doCommunication) {
		doCommunication = false;
		mAttemptRetry = false;
		try {
			StConnection connection (this, inThread, listener);
			connection.GetConnection().Unlock(theHost, theResource, theMessage, theResponse, thePort);
		} catch (LSSLException sslE) {
		    _HandleSSLexception(theResponse, sslE);
		    return FAILURE;
		} catch (...) {
			if (mAttemptRetry)
				doCommunication = true;	
		}
	}
		
	mLastResponse = theResponse.GetResponseCode();
	if (CDAVConnection::kHTTPNoContent != mLastResponse) {
       return FAILURE;
    }
    item.SetIsLocalLock(false);
	return SUCCESS;
}


// ---------------------------------------------------------------------------
//		¥ UnlockResource()
// ---------------------------------------------------------------------------
//	
CDAVRequest::ReqStatus CDAVRequest::UnlockResource(LThread& inThread, const char* uri,
                                                    const char*lockToken, LListener *listener) {
    CDAVMessage theMessage;    
	LHTTPResponse theResponse;
	string theHost, theResource;
    SInt32 thePort;
    
    theResource = uri;
    _PrepareHostFields(theHost, thePort, theResource, theMessage);
 
	theMessage.SetServer(m_context->GetServerName().c_str());
	string tmpStr("<");
	tmpStr.append(lockToken);
	tmpStr.append(">");
	theMessage.SetLockToken(tmpStr.c_str());

	bool doCommunication = true;
	while (doCommunication) {
		doCommunication = false;
		mAttemptRetry = false;
		try {
			StConnection connection (this, inThread, listener);
			connection.GetConnection().Unlock(theHost, theResource, theMessage, theResponse, thePort);
		} catch (LSSLException sslE) {
		    _HandleSSLexception(theResponse, sslE);
		    return FAILURE;
		} catch (...) {
			if (mAttemptRetry)
				doCommunication = true;	
		}
	}
		
	mLastResponse = theResponse.GetResponseCode();
	if (CDAVConnection::kHTTPNoContent != mLastResponse) {
       return FAILURE;
    }

	return SUCCESS;
}

// ---------------------------------------------------------------------------
//		¥ SetUserAgent()
// ---------------------------------------------------------------------------
//	
void CDAVRequest::SetUserAgent(const char* userAgent) {
	mUserAgent = userAgent;
}

// ---------------------------------------------------------------------------
//		¥ _fillItemLockInfoFromElement()
// ---------------------------------------------------------------------------
//	
Boolean CDAVRequest::_fillItemLockInfoFromElement(CElement *activeLock, CDAVItem &item) {
   CElement *lockType = activeLock->findFirstChild(DAVTypes::LOCKTYPE, DAVTypes::NS_DAV, false /*rescurse*/);
   CElement *lockScope = activeLock->findFirstChild(DAVTypes::LOCKSCOPE, DAVTypes::NS_DAV, false /*rescurse*/);
   CElement *depth = activeLock->findFirstChild(DAVTypes::DEPTH, DAVTypes::NS_DAV, false /*rescurse*/);
   CElement *timeout = activeLock->findFirstChild(DAVTypes::TIMEOUT, DAVTypes::NS_DAV, false /*rescurse*/);
   CElement *locktoken = activeLock->findFirstChild(DAVTypes::LOCKTOKEN, DAVTypes::NS_DAV, false /*rescurse*/);
   CElement *owner = activeLock->findFirstChild(DAVTypes::OWNER, DAVTypes::NS_DAV, false /*rescurse*/);
                      
   //right now, spec only includes write locks
   CDAVItem::LockType _lockType = CDAVItem::WRITE;
                      
   CDAVItem::LockScope _lockScope = CDAVItem::EXCLUSIVE;
   if (lockScope->findFirstChild(DAVTypes::SHARED, DAVTypes::NS_DAV, false)!=NULL)
      _lockScope = CDAVItem::SHARED;
                         
   CElement *ltHref = NULL;
   if (!locktoken)
      return false;
   ltHref=locktoken->findFirstChild(DAVTypes::HREF, DAVTypes::NS_DAV, false /*rescurse*/);
   if (!ltHref)
      return false;
                         
   string _timeout, _lockToken, _owner;
   if (timeout)
      timeout->getCDATAasString(_timeout);
   if (ltHref)
      ltHref->getCDATAasString(_lockToken);
                      
   if (owner) {
      CElement *ownHref=owner->findFirstChild(DAVTypes::HREF, DAVTypes::NS_DAV, true /*rescurse*/);
      if (ownHref) {
         ownHref->getCDATAasString(_owner);
      }
   }  
                      
   item.SetLockInformation(_lockType, _lockScope, _timeout.c_str(),_lockToken.c_str(), DAVTypes::ZERO,
                              _owner.c_str());
   return true;
}

// ---------------------------------------------------------------------------
//		¥ _StartTransaction()
// ---------------------------------------------------------------------------
//	
void CDAVRequest::_StartTransaction(CDAVConnection &) {

}

// ---------------------------------------------------------------------------
//		¥ _EndTransaction()
// ---------------------------------------------------------------------------
//	
void CDAVRequest::_EndTransaction(CDAVConnection &) {

}

// ---------------------------------------------------------------------------
//		¥ _PrepareHostFields()
// ---------------------------------------------------------------------------
// used to prepare HTTP header for possible use of a proxy server
void CDAVRequest::_PrepareHostFields(string &theHost, SInt32 &thePort, string &theResource, CDAVMessage &theMessage) {
	theMessage.SetServer(m_context->GetServerName().c_str());
	
	theResource = CHTTPStringUtils::URLEncodeString(theResource);

   const char* userAgent;
   
   if (mUserAgent.size() != 0)
      userAgent = mUserAgent.c_str();
   else
   	  userAgent = mDefaultUserAgent.c_str();
   theMessage.SetUserAgent(userAgent);

	
	string path (theResource);
	
	string cookie;
	if (m_context->GetCookieList())
		cookie = m_context->GetCookieList()->GetCookieString (m_context->GetServerName().c_str(), path.c_str(), (m_context->GetPort() == kHTTPSPort));

	if (cookie.size())
		theMessage.SetCookie (cookie);

	/*else ***teb */if (m_context->GetHasUserCredentials()) {
      theMessage.SetUserName(m_context->GetLogin());
      theMessage.SetPassword(m_context->GetPassword());
   }
      
	if (!m_headerExtra.empty())
		;

   if (false == m_context->GetUseProxy()) {
      theHost = m_context->GetServerName();
      thePort = m_context->GetPort();
      return;
   }
   
   // we have a proxy
   m_context->GetProxyServer(theHost);
   thePort = m_context->GetProxyPort();
   
	char portStr[16];
	sprintf (portStr, "%d", m_context->GetPort());
	
	theResource = ((thePort == kHTTPSPort) ? "https://" : "http://") +
						m_context->GetServerName() + ':' + portStr + theResource;
   
   if (m_context->GetHasProxyCredentials()) {
      theMessage.SetProxyUserName(m_context->GetProxyLogin());
      theMessage.SetProxyPassword(m_context->GetProxyPassword());
   }
   
}


    

// ---------------------------------------------------------------------------
//		¥ _OnDavRequestError()
// ---------------------------------------------------------------------------
// 
void CDAVRequest::_OnDavRequestError(LHTTPResponse &theResponse, const char* displayString) {
#pragma unused(theResponse, displayString)

}


// ---------------------------------------------------------------------------
//		¥ _OnDavItemCreated()
// ---------------------------------------------------------------------------
// 
void CDAVRequest::_OnDavItemCreated(CDAVItem& theItem) {
#pragma unused(theItem)

}

// ---------------------------------------------------------------------------
//		¥ _OnDavItemDataChange()
// ---------------------------------------------------------------------------
// 
void CDAVRequest::_OnDavItemDataChange(CDAVItem& theItem) {
#pragma unused(theItem)

}


// ---------------------------------------------------------------------------
//		¥ _NormalizeHREF()
// ---------------------------------------------------------------------------
// 
void CDAVRequest::_NormalizeHREF(string &href, const string& parentResource) {
   href = CHTTPStringUtils::URLDecodeString(href);
   std::string pnthref = parentResource;
   pnthref = CHTTPStringUtils::URLDecodeString(pnthref);

   string::size_type pos, idx;
   idx	= href.find("://");
   if (idx != string::npos) {
      pos=idx+3;
      idx = href.find_first_of('/', pos);
      if (-1 !=idx)
         href=href.substr(idx);
   }
   if (pnthref.size() == 1 && pnthref[0] == '/')
      return;
      
   string tmpPnt = pnthref;
   if (tmpPnt[tmpPnt.size()-1] == '/')
      tmpPnt.erase(tmpPnt.size()-1, 1);
 
   idx	= tmpPnt.find("://");
    if (idx != string::npos) {
      pos=idx+3;
      idx = tmpPnt.find_first_of('/', pos);
      if (-1 !=idx)
         tmpPnt=tmpPnt.substr(idx);
   }
  
   int pos2 = href.find(tmpPnt);
   if (strncmp(tmpPnt.c_str(), href.c_str(), tmpPnt.size())!= 0) {
      href.insert(0, tmpPnt);
   }
}

// ---------------------------------------------------------------------------
//		¥ _HandleSSLexception()
// ---------------------------------------------------------------------------
//
void CDAVRequest::_HandleSSLexception(LHTTPResponse &theResponse, LSSLException& e) {
#pragma unused(theResponse)
#pragma unused(e)

}

// ---------------------------------------------------------------------------
//		¥ StConnection methods
// ---------------------------------------------------------------------------
//		
//	This class creates a new connection or uses the persistent connection if available.


CDAVRequest::StConnection::StConnection (CDAVRequest* inRequest, LThread& inThread, LListener *inListener)
{
	mPersistent = (inRequest->m_connection != nil);
	mListener = inListener;
	mRequest = inRequest;

	
	if (mPersistent)
		mConnection = inRequest->m_connection;
	else
		mConnection = new CDAVConnection (inThread, nil, inRequest->m_context->GetCookieList(), inRequest->m_context->GetForceSecure());
		
	if (inListener)
		mConnection->AddListener (inListener);
		
	mConnection->SetCancelFlag (&inRequest->mCancel);
	
	inRequest->_StartTransaction (*mConnection);
}

CDAVRequest::StConnection::~StConnection ()
{
	if (!mConnection)
		return;
		
	try
	{
		if (mRequest)
			mRequest->_EndTransaction (*mConnection);
		
		if (mListener)
			mConnection->RemoveListener (mListener);
	}
	catch (...) {}
	
	 mConnection->SetCancelFlag (NULL);

	if ((!mPersistent) || (mConnection->GetWasClosed())) {
		delete mConnection;
		mRequest->m_connection = NULL;
		mRequest->mAttemptRetry = true;
	}
}

// ---------------------------------------------------------------------------
//		¥ DynBufferToFile()
// ---------------------------------------------------------------------------
//    
//  There is a bug in the function LDynamicBuffer::BufferToFile in the
//  version of PowerPlant that shipped with CW5 where the resulting file
//  is zero length if the buffer in the LDynamicBuffer object is a file
//  and not a handle.
Boolean CDAVRequest::DynBufferToFile(LDynamicBuffer *outBuf, LFile& outFile) {
printf("dynbuffertofile\n");
    LStream *mStream = outBuf->GetBufferStream();
	if (!mStream)
		return false;
   	
	mStream->SetMarker(0, streamFrom_Start);
	printf("going to opendatafork\n");
	SInt16 fileRef = outFile.OpenDataFork(fsRdWrPerm);
printf("after opendatafork\n");
	Handle ourHandle;
	Boolean mHandleStream = outBuf->GetBufferHandle(ourHandle);
	
	//Optimization for copies from LHandleStream
	if (mHandleStream) {
	printf("got mhandlestream\n");
		StHandleLocker locked(ourHandle);
		printf("going to writedatafork\n");
		outFile.WriteDataFork(*ourHandle, mStream->GetLength());
		printf("after writedatafork\n");
	} else {
	printf("didn't get mhandlestream\n");
		//Try to allocate some swap space to speed things up
		//	Get the smaller of available memory or the stream size
		SInt32 streamSize = mStream->GetLength();
		SInt32 bufferSize = ::MaxBlock();
		bufferSize = (bufferSize < streamSize) ? bufferSize : streamSize;
		
		OSErr	err = ::SetFPos(fileRef, fsFromStart, 0);
		ThrowIfOSErr_(err);
		
		StPointerBlock tempSwap(bufferSize);
		SInt32	bytesWritten;
		printf("about to write\n");
		while (streamSize > 0) {
			bytesWritten = mStream->ReadData(tempSwap, bufferSize);
			err = ::FSWrite(fileRef, &bytesWritten, tempSwap);
			ThrowIfOSErr_(err);
			streamSize -= bytesWritten;
		}
		printf("done writing\n");
        //***teb - here's the bug in powerplant
        //::SetEOF(fileRef, streamSize);
 		::SetEOF(fileRef, mStream->GetLength());
		ThrowIfOSErr_(err);
	}
printf("going to closedatafork\n");
	outFile.CloseDataFork();
    return true;
}


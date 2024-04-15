/* ===========================================================================
 *	parsexml.cpp	   
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
 
//PowerPlant Includes
#include <LStream.h>
#include <UMemoryMgr.h>

// Standard C string functions  
#include <string.h>
// module definitions
#include "parsexml.h"


//Expat XML parser events
static void starthandler(void *userdata, const char *name, const char **attrs);
static void endhandler(void *userdata, const char *name);
static void cdatahandler(void *userdata, const char *cdata, int len);

// XML parsing utility
bool namespaceIsXML(std::string &name);

//user data for storing information during parse of data stream
class mydata {
   public:
      CXMLDocument *m_doc;
      CElement  *m_currentElement;
      bool      m_error;
};

const char NS_DELIM = '|';

// ---------------------------------------------------------------------------
//		¥ CNamespacedObject
// ---------------------------------------------------------------------------
//	
CNamespacedObject::CNamespacedObject(): m_namespace(NULL) {

}

// ---------------------------------------------------------------------------
//		¥ CNamespacedObject
// ---------------------------------------------------------------------------
//	
CNamespacedObject::CNamespacedObject(const char* name): m_name(name) {

}


// ---------------------------------------------------------------------------
//		¥ CNamespacedObject
// ---------------------------------------------------------------------------
//	
CNamespacedObject::CNamespacedObject(const char* name, std::string &ns): m_name(name), m_namespace(ns) {

}

// ---------------------------------------------------------------------------
//		¥ ~CNamespacedObject
// ---------------------------------------------------------------------------
//	
CNamespacedObject::~CNamespacedObject() {

}
  
// ---------------------------------------------------------------------------
//		¥ setNamespace
// ---------------------------------------------------------------------------
//	
void CNamespacedObject::setNamespace(std::string &ns) {
   m_namespace = ns;
}

// ---------------------------------------------------------------------------
//		¥ getNamespace
// ---------------------------------------------------------------------------
//	
std::string& CNamespacedObject::getNamespace() {
   return m_namespace;
}


// ---------------------------------------------------------------------------
//		¥ getName
// ---------------------------------------------------------------------------
//	
std::string& CNamespacedObject::getName() {
   return m_name;
}


// ---------------------------------------------------------------------------
//		¥ CAttribute
// ---------------------------------------------------------------------------
//	
CAttribute::CAttribute() {

}

// ---------------------------------------------------------------------------
//		¥ CAttribute
// ---------------------------------------------------------------------------
//	
CAttribute::CAttribute(const char* name, const char* value): CNamespacedObject(name), m_value(value) {

}

// ---------------------------------------------------------------------------
//		¥ ~CAttribute
// ---------------------------------------------------------------------------
//	
CAttribute::~CAttribute() {

}

// ---------------------------------------------------------------------------
//		¥ getValue
// ---------------------------------------------------------------------------
//	
std::string& CAttribute::getValue() {
   return m_value;
}
   
// ---------------------------------------------------------------------------
//		¥ CElement
// ---------------------------------------------------------------------------
//	
CElement::CElement():  m_parent(NULL)  {

}

// ---------------------------------------------------------------------------
//		¥ CElement
// ---------------------------------------------------------------------------
//	
CElement::CElement(const char* name): CNamespacedObject(name), m_parent(NULL)  {

}

// ---------------------------------------------------------------------------
//		¥ ~CElement
// ---------------------------------------------------------------------------
//	
CElement::~CElement() {
   std::vector<CElement*>::iterator iter = m_children.begin();
   while (iter != m_children.end()) {
      delete(*iter);
      iter++;
   }
}

// ---------------------------------------------------------------------------
//		¥ addAttribute
// ---------------------------------------------------------------------------
//	
void CElement::addAttribute(const char *attrName, const char* attrValue) {
   m_attributes.push_back(CAttribute(attrName, attrValue));
}

// ---------------------------------------------------------------------------
//		¥ addChild
// ---------------------------------------------------------------------------
//	
void CElement::addChild(CElement *child) {
   m_children.push_back(child);
   mMixedModel.push_back(CDATASegment(child));
}

// ---------------------------------------------------------------------------
//		¥ addCDataChunk
// ---------------------------------------------------------------------------
//	
void CElement::addCDataChunk(const char* cdata, int len) {
   m_cdataChunks.push_back(std::string(cdata, len));
   mMixedModel.push_back(CDATASegment(cdata, len));
}


// ---------------------------------------------------------------------------
//		¥ setLanguage
// ---------------------------------------------------------------------------
//	
void CElement::setLanguage(const char* lang) {
      m_lang = lang;
}

// ---------------------------------------------------------------------------
//		¥ getLanguage
// ---------------------------------------------------------------------------
//	
const char* CElement::getLanguage() {
   return m_lang.c_str();
}

// ---------------------------------------------------------------------------
//		¥ setParent
// ---------------------------------------------------------------------------
//	
void CElement::setParent(CElement *elem) {
   m_parent = elem;
}

// ---------------------------------------------------------------------------
//		¥ getParent
// ---------------------------------------------------------------------------
//	
CElement* CElement::getParent() {
   return m_parent;
}

// ---------------------------------------------------------------------------
//		¥ getAttributes
// ---------------------------------------------------------------------------
//	
std::vector<CAttribute>& CElement::getAttributes() {
   return m_attributes;
}


// ---------------------------------------------------------------------------
//		¥ getCDATA
// ---------------------------------------------------------------------------
//	
std::vector<std::string>& CElement::getCDATA() {
   return m_cdataChunks;
}


// ---------------------------------------------------------------------------
//		¥ getChildren
// ---------------------------------------------------------------------------
//	
std::vector<CElement*> & CElement::getChildren() {
   return m_children;
}

// ---------------------------------------------------------------------------
//		¥ findFirstChild
// ---------------------------------------------------------------------------
//	
CElement *CElement::findFirstChild(const char* elemName, const char* xnamespace, bool recurse) {
   std::vector<CElement*>::iterator iter = m_children.begin();
   while (iter != m_children.end()) {
      if ((*iter)->getName().compare(elemName)==0)
         return *iter;
      else if (recurse) {
         CElement *tmpRec = (*iter)->findFirstChild(elemName, xnamespace, recurse);
         if (NULL != tmpRec)
            return tmpRec;
      }
      iter++;
   }
   
   return NULL;
   
}

// ---------------------------------------------------------------------------
//		¥ findAllChildren
// ---------------------------------------------------------------------------
//
void CElement::findAllChildren(const char* elemName, const char* xnamespace, std::vector<CElement*> &kids, bool recurse) {
   std::vector<CElement*>::iterator iter = m_children.begin();
   while (iter != m_children.end()) {
      if ((*iter)->getName().compare(elemName)==0)
         kids.push_back(*iter);
      
      if (recurse) {
         (*iter)->findAllChildren(elemName, xnamespace, kids, recurse);
      }
      iter++;
   }
   

}

// ---------------------------------------------------------------------------
//		¥ getCDATAasString
// ---------------------------------------------------------------------------
//
void CElement::getCDATAasString(std::string& content) {
   std::vector<std::string>::iterator iter =  m_cdataChunks.begin();
   while (iter != m_cdataChunks.end()) {
      content.append(*iter);
      iter++;
   }
}

// ---------------------------------------------------------------------------
//		¥ getMixedContentAsXML
// ---------------------------------------------------------------------------
//	
void CElement::getMixedContentAsXML(std::string &mixedContent) {
   std::vector<CDATASegment>::iterator iter = mMixedModel.begin();
   
   while (iter != mMixedModel.end()) {
      if (iter->isCDATA()) {
         mixedContent += iter->getCDATA();
      } else {
         CElement *elem = iter->getElement();
         mixedContent += '<';
         std::string ns = elem->getNamespace();
         if (ns.length() > 0) {
            mixedContent+=ns;
         }
         mixedContent += elem->getName();
         
         //***teb - handle attributes here
         mixedContent +='>';
         
         elem->getMixedContentAsXML(mixedContent);
         mixedContent+="</";
         if (ns.length() > 0) {
            mixedContent+=ns;
         }
         mixedContent += elem->getName();
         mixedContent += '>';
         
      }
   
      iter++;
   }
}


// ---------------------------------------------------------------------------
//		¥ CXMLDocument
// ---------------------------------------------------------------------------
//	
CXMLDocument::CXMLDocument(): m_root(NULL) {
   
}

// ---------------------------------------------------------------------------
//		¥ ~CXMLDocument
// ---------------------------------------------------------------------------
//	
CXMLDocument::~CXMLDocument() {
   if (m_root)
      delete(m_root);
}


// ---------------------------------------------------------------------------
//		¥ setRoot
// ---------------------------------------------------------------------------
//	
void CXMLDocument::setRoot(CElement* root) {
   m_root = root;
} 
      
// ---------------------------------------------------------------------------
//		¥ getRoot
// ---------------------------------------------------------------------------
//	
CElement *CXMLDocument::getRoot() {
   return m_root;
}

// ---------------------------------------------------------------------------
//		¥ validateRoot
// ---------------------------------------------------------------------------
//
bool CXMLDocument::validateRoot(const char* elemName, const char* xnamespace) {
   if (!m_root)
      return false;

   bool nameMatches = (m_root->getName().compare(elemName)==0);
   if (xnamespace) {
      if (m_root->getNamespace().compare("")!=0)
         return ((m_root->getNamespace().compare(xnamespace)==0) && nameMatches);
      else
         return nameMatches;
   } else
      return nameMatches;
}

// ---------------------------------------------------------------------------
//		¥ namespaceIsXML
// ---------------------------------------------------------------------------
//	
bool namespaceIsXML(std::string &name) {
   return ( ('X' == name[0] || 'x' == name[0]) &&
   ('M' == name[0] || 'm' == name[0]) &&
   ('L' == name[0] || 'l' == name[0]) );
   
}

// ---------------------------------------------------------------------------
//		¥ starthandler
// ---------------------------------------------------------------------------
//	Expat event handler for the start of an CElement
static void starthandler(void *userdata, const char *name, const char **attrs) {
   mydata *data = reinterpret_cast<mydata*>(userdata);
   CElement *elem;
   
   if (data->m_error) 
      return;
   
   std::string tmp(name), gi, elemnamespace;

   std::string::size_type delim = tmp.find_first_of(NS_DELIM);
   if (std::string::npos == delim) {
      elemnamespace = "";
      //Is no namespace correct?  Is there a possibility of a default NS?
   } else if (namespaceIsXML(tmp)) {
      elemnamespace = "";
   } else {
      gi = tmp.substr(delim+1);
      elemnamespace=tmp.substr(0,delim);
   }

   elem = new CElement(gi.c_str());
   elem->setNamespace(elemnamespace);
      
   while (*attrs) {
      const char *attrName = *attrs++, *val = *attrs++;
      if (strcmp(attrName, "xml:lang")==0) {
         elem->setLanguage(val);
      } else
         elem->addAttribute(attrName, val);
   }
   
   if (NULL == data->m_currentElement) {
      data->m_currentElement = elem;
      data->m_doc->setRoot(elem);
   } else {
      elem->setParent(data->m_currentElement);
      data->m_currentElement->addChild(elem);
   }
   
   if ((elem->getLanguage()==NULL) && ((elem->getParent() != NULL) && (!elem->getParent()->getLanguage()!=NULL)))
      elem->setLanguage(elem->getParent()->getLanguage());
      
   data->m_currentElement = elem;
}


// ---------------------------------------------------------------------------
//		¥ endhandler
// ---------------------------------------------------------------------------
//	Expat event handler for the close of an CElement
static void endhandler(void *userdata, const char *name) {
#pragma unused(name)
   mydata *data = reinterpret_cast<mydata*>(userdata);
   
   if (data->m_error) 
      return;
   
   data->m_currentElement = data->m_currentElement->getParent();
}


// ---------------------------------------------------------------------------
//		¥ cdatahandler
// ---------------------------------------------------------------------------
//	Expat event handler for an CElement's cdata
static void cdatahandler(void *userdata, const char *cdata, int len) {
   mydata *data = reinterpret_cast<mydata*>(userdata);
   
   if (data->m_error) 
      return;

   data->m_currentElement->addCDataChunk(cdata, len);
}


// ---------------------------------------------------------------------------
//		¥ ParseXMLDocument
// ---------------------------------------------------------------------------
//	High level function which parses an LStream into a CXMLDocument
XML_Error ParseXMLDocument(LStream &stream, CXMLDocument &doc) {
    XML_Parser parser;
    mydata data;
    
    data.m_error = false;
    data.m_currentElement = NULL;
    data.m_doc = &doc;
    
    parser = XML_ParserCreateNS(NULL, NS_DELIM);
    if (NULL == parser) {
       return XML_ERROR_NO_MEMORY;
    }
    
   stream.SetMarker(0, streamFrom_Start);
   SInt32 streamSize = stream.GetLength() - stream.GetMarker();
   SInt32 bufferSize=0;
#if PP_Target_Carbon
   bufferSize = 1024;
#else
   bufferSize = ::MaxBlock();
#endif
   bufferSize = (bufferSize < streamSize) ? bufferSize : streamSize;
   if (0 == streamSize)
      return XML_ERROR_NONE; //***technically should return a no document root error
      
   StPointerBlock tempSwap(bufferSize);
   SInt32 bytesRead;
   
   int resp;
   
   XML_SetUserData(parser, &data);
   XML_SetElementHandler(parser, starthandler, endhandler);
   XML_SetCharacterDataHandler(parser, cdatahandler);

   XML_Error errCode = XML_ERROR_NONE;
   while (streamSize > 0) {
      bytesRead = stream.ReadData(tempSwap, bufferSize);
      resp = XML_Parse(parser, tempSwap, bytesRead, 0);
      if (0 == resp) {
         errCode = XML_GetErrorCode(parser);
      }
      streamSize -= bytesRead;
   }

   XML_ParserFree(parser);   
   return errCode;   
}




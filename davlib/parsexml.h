/* ===========================================================================
 *	parsexml.h			   
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
 
/*!
   @header DAVLib
	This library implements HTTP 1.1 and the WebDAV (RFC 2518) extensions to HTTP.
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#pragma once
#ifndef __PARSEXML_H__
#define __PARSEXML_H__

#include <string>
#include <vector>

// James Clark's Expat parser 
#include <xmlparse.h>



/*!
   @class CNamespacedObject
   @discussion an XML object which is knowlegable about a namespace
*/
class CNamespacedObject {
   protected:


      /*! @var m_namespace a basic_string containing the XML namespace for this object */
      std::string m_namespace;
      /*! @var m_name the generic identifier (GI) of this XML document component */
      std::string m_name;
      
   public:
      /*! 
         @function CNamespacedObject
         @discussion object constructor
      */
      CNamespacedObject();
      /*! 
         @function CNamespacedObject 
         @discussion object constructor
         @param name the generic identifier for this object
      */
      CNamespacedObject(const char* name);
      /*! 
         @function CNamespacedObject 
         @discussion object constructor
         @param name the generic identifier for this object
         @param ns the XML namespace for this object
      */
      CNamespacedObject(const char* name, std::string &ns);
      /*! 
         @function ~CNamespacedObject
         @discussion object destructor 
      */
      virtual ~CNamespacedObject();
      /*!
         @function setNamespace 
         @discussion sets the XML namespace for this object
         @param ns the new namespace
      */
      void setNamespace(std::string& ns);
      /*!
         @function getNamespace
         @discussion gets the XML namespace for this object
         @result a basic_string containing the XML namespace
      */
      std::string& getNamespace();
      /*!
         @function getNamespace
         @discussion gets the generic identifier for this object
         @result a basic_string containing the generic identifier
      */            
      std::string& getName();
};


/*!
   @class CAttribute
   @discussion an XML attribute, contained by an Element
*/
class CAttribute : public CNamespacedObject{
 
   protected:
      /* @var m_value the value of this XML attribute */
      std::string m_value;
         
   public:
      /*! 
         @function CAttribute
         @discussion object constructor
      */
      CAttribute();
      /*! 
         @function CAttribute
         @discussion object constructor
         @param name the name of this attribute
         @param value the value of this attribute
      */
      CAttribute(const char* name, const char* value);
      /*! 
         @function ~CAttribute
         @discussion object constructor
      */
      virtual ~CAttribute();
      /*! 
         @function getValue
         @discussion return the value of this attribute
         @result a basic_string containing the attribute value
      */      
      std::string& getValue();
};


/*!
   @class CDATASegment
   @discussion a representation of a CDATA section in an XML document
*/
class CDATASegment {
   public:
      /*! 
         @function CDATASegment
         @discussion object constructor; in this form, not really CDATA 
             but a container for CElement, used in the mixed-model mode of 
             a CElement.  Bad design...i know.
         @param elem a CElement object
      */
      CDATASegment(class CElement* elem):mElement(elem) {mIsCDATA=false;};
      /*! 
         @function CDATASegment
         @discussion object constructor
         @param cdata the CDATA
         @param len the character length of the CDATA
      */
      CDATASegment(const char* cdata, int len):mCDATA(cdata, len), mIsCDATA(true), mElement(NULL) {;};
      /*!
         @function getCDATA 
         @discussion accessor of the actual CDATA stored by this object
         @result a reference to a basic_string
      */
      std::string& getCDATA() {return mCDATA;};
      /*!
         @function isCDATA
         @result returns true if this is a CDATA chunk or a holder of an CElement
             (cheap, hack form of RTTI, i should be slapped pretty hard)
      */
      virtual Boolean isCDATA() {return mIsCDATA;};
      /*! 
         @function getElement
         @discussion returns the CElement stored by this object
      */      
      class CElement *getElement() {return mElement;};
   protected:
      /*! @var mCDATA the character data contained by this segment */
      std::string mCDATA;
      /*! @var mElement the element containing this character data */
      class CElement* mElement;
      /*! @var mIsCDATA  */
      Boolean mIsCDATA;
};


/*!
   @class CElement
   @discussion a class containing the parsed representation of an XML element
*/ 
class CElement : public CNamespacedObject {
  
   protected:
      /*!    @var m_lang the contents of an xml:lang attribute, if any */
      std::string m_lang;   
      /*!    @var m_cdataChunks all chunks of CDATA contained by this element */
      std::vector<std::string>  m_cdataChunks;
      /*!    @var m_children all direct children elements */
      std::vector<CElement*> m_children;
      /*!    @var m_parent the parent element */
      CElement *m_parent;
      /*!    @var m_attributes all attributes of this element */
      std::vector<CAttribute> m_attributes;
      /*!    @var nMixedModel all CDATA chunks and elements contained by this element
		  in the order encounted during the parse of the document */
      std::vector<CDATASegment> mMixedModel;
      
   public:                       
      /*! 
         @function CElement
         @discussion object constructor
      */
      CElement();
      /*! 
         @function CElement
         @discussion object constructor
         @param name the generic identifier for this element
      */
      CElement(const char* name);
      /*! 
         @function ~CElement
         @discussion object destructor
      */
      virtual ~CElement();
      
      /*!
         @function addAttribute
         @discussion add an attribute and it's value to this object
         @param attrName the name of the attribute
         @param attrValue the value of the attribute
      */
      void addAttribute(const char *attrName, const char* attrValue);
      /*!
         @function addChild
         @discussion adds a CElement to the child list of this object
         @param child a pointer to the CElement to be added
      */
      void addChild(CElement *child);
      /*!
         @function addCDataChunk
         @discussion adds character data to this element
         @param cdata the actual character string
         @param len the length of this string
      */
      void addCDataChunk(const char* cdata, int len);
      /*!
         @function setLanguage
         @param lang the language
      */            
      void setLanguage(const char* lang);
      /*!
         @function getLanguage
      */
      const char* getLanguage();
      /*!
         @function setParent
         @discussion sets the parent of this element
         @param elem a pointer to the parent element
      */
      void setParent(CElement *elem);
      /*!
         @function getParent 
         @discussion gets the parent of this element
         @result a pointer to a CElement or NULL if this is the root of the document
      */
      CElement* getParent();
      /*!
         @function getAttributes
         @discussion returns a vector containing all of the attributes of this elements
         @result a reference to a std::vector<CAttribute>
      */      
      std::vector<CAttribute>& getAttributes();
      /*!
         @function getDATA
         @discussion returns a vector of all of the CDATA chunks contained by this element
         @result a reference to a std::vector<std::string>
      */
      std::vector<std::string>& getCDATA();
      /*!
         @function getChildren
         @discussion get a vector containing all of the CElement objects of which this
             object is the immediate parent
         @result a reference to a std::vector<CElement*>
      */
      std::vector<CElement*> & getChildren();
      /*!
         @function findFirstChild
         @discussion a utility function that will find the first child that matches the 
             generic identifier passed in the function parameters
         @param elemName the generic identifier of the element to be found
         @param xnamespace the XML namespace
         @param recuse if true, recurse the tree pree order to find the first. otherwise just 
                scan immediate children 
         @result a pointer to a CElement object or NULL
      */
      CElement *findFirstChild(const char* elemName, const char* xnamespace, bool rescurse);
      /*!
         @function findAllChildren
         @discussion finds all children that match the 
             generic identifier passed in the function parameters
         @param elemName the generic identifier of the element to be found
         @param xnamespace the XML namespace
         @param recuse if true, recurse the tree pree order to find the first. otherwise just 
                scan immediate children 
         @param kids a vector of CElement* to populate with the elements that match
      */
      void findAllChildren(const char* elemName, const char* xnamespace, std::vector<CElement*> &kids, bool recurse=false);
      /*!
         @function getCDATAasString
         @discussion concatenates all of the CDATA chunks contained by this element 
         @param content an output parameter where the character data is copied into
      */
      void getCDATAasString(std::string& content);    
      /*!
         @function getMixedContentAsXML
         @discussion concatenates all of the CDATA chunks and elements contained by this element 
            and returns a syntactically valid XML document fragment
         @param mixedContent the string where the document fragment is returned
      */
      void getMixedContentAsXML(std::string &mixedContent);
};


/*!
   @class CXMLDocument
   @discussion a class containing the parsed representation of an XML document
*/
class CXMLDocument {
   protected:
      /*!    @var m_root the document's root element */
      CElement *m_root;
      
   public:
      /*! 
         @function CXMLDocument
         @discussion object constructor
      */
      CXMLDocument();
      /*! 
         @function ~CXMLDocument
         @discussion object destructor
      */
      virtual ~CXMLDocument();
      /*!
         @function validateRoot
         @param elemName the generic identifier of the element to be found
         @param xnamespace the XML namespace
         @result returns true if the root element's generic identifier and namespace
            match the values specified in the function parameters.
      */
      bool validateRoot(const char* elemName, const char* xnamespace);
      /*!
         @function setRoot
         @discussion Sets the document root element
         @param root a pointer to a CElement object that will be the root of this document
      */
      void setRoot(CElement* root);
      /*!
         @function getRoot
         @result returns a pointer to a CElement object that is the root of the document 
      */
      CElement *getRoot();
      
};


/*!
   @function ParseXMLDocument
   @discussion create a parse tree of an XML document
   @param stream a PowerPlant LStream to parse
   @param outDocument a reference to an CXMLDocument object to 
          receive the parsed XML document instance
   @result an XML_Error enum, see expat's header xmlparse.h
*/
XML_Error ParseXMLDocument(LStream &stream, CXMLDocument &outDocument);


#endif

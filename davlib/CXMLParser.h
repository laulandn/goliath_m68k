/* ==================================================================================================
 * CXMLParser.h														   
 *    Goliath - a Finder like application that implements WebDAV
 *    Copyright (C) 1999-2000  Thomas Bednarz
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

#ifndef __CXMLPARSER_H__
#define __CXMLPARSER_H__

#include <string>
#include <LDynamicBuffer.h>
#include <xmlparse.h>

/*!
   @class CXMLParser
   @discussion an abstraction of an event based XML parser.  Contains the basic functionality
       of reading in data from a PowerPlant LStream object and registration of parse event
       handlers.  Subclasses override the data handling functions; similar to the Java SAX API.
       Note that XML parsers generally are not multi-thread safe nor are they re-entrant.
*/

class CXMLParser {
   public:      
      /*! 
         @function CXMLParser
         @discussion object constructor
      */
      CXMLParser();
      /*! 
         @function ~CXMLParser
         @discussion object destructor
      */
      virtual ~CXMLParser();
       
      //Parse Events
      /*!
         @function StartElementHandler
         @discussion called by the XML parser when it encounters the start of an element
         @param name the generic identifier of the element
         @param atts the attributes of the element that is starting
      */
      virtual void StartElementHandler(const XML_Char *name, const XML_Char **atts);
      /*!
         @function EndElementHandler
         @discussion called by the XML parser when it encounters the end of an element
         @param name the generic identifier of the element
      */
      virtual void EndElementHandler(const XML_Char *name);
      /*!
         @function CharacterDataHandler
         @discussion called by the XML parser when it encounters character data
         @param s pointer to the character data
         @param len length of the character data
      */
      virtual void CharacterDataHandler(const XML_Char *s, int len);
      /*!
         @function ProcessingInstructionHandler
         @discussion called when the XML parser encounters a processing instruction
         @param target pointer to the character data
         @param len length of the character data
      */
      virtual void ProcessingInstructionHandler(const XML_Char *target, const XML_Char *data);
      /*!
         @function CommentHandler
         @discussion called when the XML parser encounters an XML comment
         @param data the content of the comment 
      */
      virtual void CommentHandler(const XML_Char *data);
      /*!
         @function StartCDataSectionHandler
         @discussion called when the XML parser encounters the start of a CDATA chunk
      */
      virtual void StartCDataSectionHandler();
      /*!
         @function EndCDataSectionHandler
         @discussion called when the XML parser encounters the end of a CDATA chunk
         @discussion 
      */
      virtual void EndCDataSectionHandler();

      /*!
         @function ParseXMLDocument
         @discussion parses an XML document 
         @param stream the data to be parsed
         @result returns an XML parser error; see xmlparse.h for more information
      */       
      XML_Error ParseXMLDocument(LStream &stream);
};

#endif

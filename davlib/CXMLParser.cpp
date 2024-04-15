/* ==================================================================================================
 * CXMLParser.cpp														   
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
 
#include "CXMLParser.h"

//Expat XML parser events
static void CXMLParser_starthandler(void *userdata, const char *name, const char **attrs);
static void CXMLParser_endhandler(void *userdata, const char *name);
static void CXMLParser_cdatahandler(void *userdata, const char *cdata, int len);
static void CXMLParser_processinginstructionHandler(void *userdata,
						                            const XML_Char *target, 
						                            const XML_Char *data);
static void CXMLParser_commenthandler(void *userdata, const XML_Char *data);
static void CXMLParser_startcdatasectionhandler(void *userdata);
static void CXMLParser_endcdatasectionhandler(void *userdata);


// ---------------------------------------------------------------------------
//		¥ CXMLParser()
// ---------------------------------------------------------------------------
//	
CXMLParser::CXMLParser() {

}

// ---------------------------------------------------------------------------
//		¥ ~CXMLParser()
// ---------------------------------------------------------------------------
//
CXMLParser::~CXMLParser() {

}
       
// ---------------------------------------------------------------------------
//		¥ StartElementHandler()
// ---------------------------------------------------------------------------
//
void CXMLParser::StartElementHandler(const XML_Char *name, const XML_Char **atts) {
   #pragma unused(name, atts)
}

// ---------------------------------------------------------------------------
//		¥ EndElementHandler()
// ---------------------------------------------------------------------------
//
void CXMLParser::EndElementHandler(const XML_Char *name) {
   #pragma unused(name)
}

// ---------------------------------------------------------------------------
//		¥ CharacterDataHandler()
// ---------------------------------------------------------------------------
//
void CXMLParser::CharacterDataHandler(const XML_Char *s, int len) {
   #pragma unused(s, len)
}

// ---------------------------------------------------------------------------
//		¥ ProcessingInstructionHandler()
// ---------------------------------------------------------------------------
//
void CXMLParser::ProcessingInstructionHandler(const XML_Char *target, const XML_Char *data) {
   #pragma unused(target, data)
}

// ---------------------------------------------------------------------------
//		¥ CommentHandler()
// ---------------------------------------------------------------------------
//
void CXMLParser::CommentHandler(const XML_Char *data) {
   #pragma unused(data)
}



// ---------------------------------------------------------------------------
//		¥ StartCDataSectionHandler()
// ---------------------------------------------------------------------------
//
void CXMLParser::StartCDataSectionHandler() {

}


// ---------------------------------------------------------------------------
//		¥ EndCDataSectionHandler()
// ---------------------------------------------------------------------------
//
void CXMLParser::EndCDataSectionHandler() {

}

// ---------------------------------------------------------------------------
//		¥ ParseXMLDocument()
// ---------------------------------------------------------------------------
//
XML_Error CXMLParser::ParseXMLDocument(LStream &stream) {
    XML_Parser parser;
        
    parser = XML_ParserCreateNS(NULL, '|');
    if (NULL == parser) {
       return XML_ERROR_NO_MEMORY;
    }
    
   SInt32 streamSize = stream.GetLength() - stream.GetMarker();
   SInt32 bufferSize;
#if PP_Target_Carbon
   bufferSize = 1024;
#else
   bufferSize = ::MaxBlock();
#endif
   if (0 == streamSize)
      return XML_ERROR_NONE; //***technically should return a no document root error

   bufferSize = (bufferSize < streamSize) ? bufferSize : streamSize;
   StPointerBlock tempSwap(bufferSize);
   SInt32 bytesRead;
   
   int resp;
   
   XML_SetUserData(parser, this);
   XML_SetElementHandler(parser, CXMLParser_starthandler, CXMLParser_endhandler);
   XML_SetCharacterDataHandler(parser, CXMLParser_cdatahandler);
   XML_SetProcessingInstructionHandler(parser, CXMLParser_processinginstructionHandler);
   XML_SetCdataSectionHandler(parser, CXMLParser_startcdatasectionhandler, 
                         CXMLParser_endcdatasectionhandler);
   XML_SetCommentHandler(parser, CXMLParser_commenthandler);
   
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

// ---------------------------------------------------------------------------
//		¥ CXMLParser_starthandler()
// ---------------------------------------------------------------------------
//
static void CXMLParser_starthandler(void *userdata, const char *name, const char **attrs) {
   CXMLParser *xmlparser = reinterpret_cast<CXMLParser*>(userdata);
   xmlparser->StartElementHandler(name, attrs);

}

// ---------------------------------------------------------------------------
//		¥ CXMLParser_endhandler()
// ---------------------------------------------------------------------------
//
static void CXMLParser_endhandler(void *userdata, const char *name) {
   CXMLParser *xmlparser = reinterpret_cast<CXMLParser*>(userdata);
   xmlparser->EndElementHandler(name);
}


// ---------------------------------------------------------------------------
//		¥ CXMLParser_cdatahandler()
// ---------------------------------------------------------------------------
//
static void CXMLParser_cdatahandler(void *userdata, const char *cdata, int len) {
   CXMLParser *xmlparser = reinterpret_cast<CXMLParser*>(userdata);
   xmlparser->CharacterDataHandler(cdata, len);
}


// ---------------------------------------------------------------------------
//		¥ CXMLParser_processinginstructionHandler()
// ---------------------------------------------------------------------------
//
static void CXMLParser_processinginstructionHandler(void *userdata,
						                            const XML_Char *target, 
						                            const XML_Char *data) {
   CXMLParser *xmlparser = reinterpret_cast<CXMLParser*>(userdata);
   xmlparser->ProcessingInstructionHandler(target, data);
}
						                            

// ---------------------------------------------------------------------------
//		¥ CXMLParser_commenthandler()
// ---------------------------------------------------------------------------
//
static void CXMLParser_commenthandler(void *userdata, const XML_Char *data) {
   CXMLParser *xmlparser = reinterpret_cast<CXMLParser*>(userdata);
   xmlparser->CommentHandler(data);
}


// ---------------------------------------------------------------------------
//		¥ CXMLParser_startcdatasectionhandler()
// ---------------------------------------------------------------------------
//
static void CXMLParser_startcdatasectionhandler(void *userdata) {
   CXMLParser *xmlparser = reinterpret_cast<CXMLParser*>(userdata);
   xmlparser->StartCDataSectionHandler();
}


// ---------------------------------------------------------------------------
//		¥ CXMLParser_endcdatasectionhandler()
// ---------------------------------------------------------------------------
//
static void CXMLParser_endcdatasectionhandler(void *userdata) {
   CXMLParser *xmlparser = reinterpret_cast<CXMLParser*>(userdata);
   xmlparser->EndCDataSectionHandler();
}


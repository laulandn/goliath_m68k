/* ==================================================================================================
 * CWindowManager.h															   
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
 
 #ifndef __CWINDOWMANAGER_H__
 #define __CWINDOWMANAGER_H__

#include <LBroadcaster.h>
#include <LAttachment.h>
#include <LPane.h>
#include <LWindow.h>
#include <string>

/**
   This class is designed to get around the issue of having multiple windows
   that may be displaying the same data (ie, a folder is expanded in one window
   displaying it's children and there's also a window displaying that folder
   as the root); an operation in one window should update the other.  Also, all
   DAV operations happen in a thread; having a messaging system like this gets around
   the sticky case where a user closes a window before the thread finishes; instead
   of the thread having a pointer to the window and possibly using a bad pointer, it
   simply messages out to all known windows at this point.
   
   
**/

class CWindowManager : public LBroadcaster, public LAttachment {
   public:
      CWindowManager();
      virtual ~CWindowManager();      
      
      void SetWindowMenuID(ResIDT windowMenu);

      virtual void 		ExecuteSelf( MessageT inMessage, void *ioParam );

      virtual void 		Rebuild();
      virtual void 		SelectIndexedWindow( SInt16 index );

      virtual void		Stack();
      virtual void		Tile();
      virtual void		TileVertical();
      virtual void		Zoom();
	
	
      virtual void		StackWindowsForScreen( Rect &inScreenRect, LArray &windows );

      virtual void		TileWindowsForScreen( Rect &inScreenRect, LArray &windows,
										SDimension16 tileDimensions );
										
      virtual void		CalculateTileDimensions( SInt16 numberOfWindows,
													SDimension16 &outSize );
	
	
      virtual void 		Exclude( LWindow *window );
      virtual void		DontExclude( LWindow *window );
	

      virtual void		BuildScreenList( LArray &outList );
      virtual void		BuildFrontToBackWindowList( LArray &outList, bool includeDialogs = false );
      virtual void		SubtractExcludedWindows( LArray &ioList );
      virtual GDHandle	ScreenToStackOn( LWindow *window );
      virtual void		ExtractWindowsForScreen( LArray &inList, LArray &outList, GDHandle gdh );
	
      virtual void		GetWindowSizes( LWindow *window, Point topLeft, Rect &inScreenRect,
											SDimension16 &maxSize,
											SDimension16 &stdSize,
											SDimension16 &minSize );
      virtual Rect		GetWindowBorder( LWindow *window );
      virtual void		MoveAndSizeWindow( LWindow *window, Point where,
											SInt16 width, SInt16 height );

      class CDAVTableWindow* FindWindowForURI(class CDAVContext* theCtx, std::string &theRes);
   protected:
	
      ResIDT		mMenuID;
      MenuHandle	mMenuHandle;
      SInt16		mItemCountWhenEmpty;
	
      LArray		mExclusionList;
      LArray		mKnownWindowList;
	
      static Boolean	sInitialized;
};

// ------------------------------------------------
//	¥	Command definitions
// ------------------------------------------------
//
//	Use these command numbers in your window menu if you
//	want CWindowMenu to handle stack, tile, tile vertical,
//	and zoom menu items.
//

const CommandT	cmd_Stack			= 65100;
const CommandT	cmd_Tile			= 65101;
const CommandT	cmd_TileVertical	= 65102;
const CommandT	cmd_Zoom			= 65103;


#endif


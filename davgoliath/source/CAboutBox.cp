/* ==================================================================================================
 * CAboutBox.cp															   
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
 
#include "CAboutBox.h"
#include "CDAVTableApp.h"
#include <UReanimator.h>
#include <LControl.h>
#include <LPushButton.h>
#include <PP_KeyCodes.h>
#include <UKeyFilters.h>
#include <LStaticText.h>
#include "CDAVTableApp.h"
#include <CDAVLibUtils.h>

// ---------------------------------------------------------------------------
//		¥ CreateFromStream()
// ---------------------------------------------------------------------------
//
CAboutBox* CAboutBox::CreateFromStream(LStream* inStream)
{
	return (new CAboutBox(inStream));
}


// ---------------------------------------------------------------------------
//		¥ CAboutBox()
// ---------------------------------------------------------------------------
//
CAboutBox::CAboutBox()
{
}

// ---------------------------------------------------------------------------
//		¥ CAboutBox()
// ---------------------------------------------------------------------------
//
CAboutBox::CAboutBox(LStream* inStream) : LWindow(inStream)
{
}

// ---------------------------------------------------------------------------
//		¥ ~CAboutBox()
// ---------------------------------------------------------------------------
//
CAboutBox::~CAboutBox()
{
}


// ---------------------------------------------------------------------------
//		¥ FinishCreateSelf()
// ---------------------------------------------------------------------------
//
void CAboutBox::FinishCreateSelf()
{
	UReanimator::LinkListenerToControls(this, this, win_About);
    LStaticText *heading = reinterpret_cast<LStaticText*>(FindPaneByID( 'STXT' ));
    LStr255 valText;
    heading->GetText(valText);
    valText.Append(GoliathAppVersion);
#if PP_Target_Carbon
    valText.Append(" (Carbon)");
#endif
    heading->SetText(valText);
    
    heading = reinterpret_cast<LStaticText*>(FindPaneByID( 'DTXT' ));
    heading->GetText(valText);
    
    std::string dlibVerString;
    GetDAVLibVersionDisplayString(dlibVerString);
    valText.Append(dlibVerString.c_str());
    heading->SetText(valText);
}

// ---------------------------------------------------------------------------
//		¥ ListenToMessage()
// ---------------------------------------------------------------------------
//
void CAboutBox::ListenToMessage(MessageT inMessage, void *)
{
	switch(inMessage)
	{
		case 'okay':
			AttemptClose();
			break;
	}
}

// ---------------------------------------------------------------------------
//		¥ HandleKeyPress()
// ---------------------------------------------------------------------------
//
Boolean CAboutBox::HandleKeyPress(const EventRecord &inKeyEvent)
{
	Boolean  keyHandled = false;
	LControl *keyButton = nil;
	
	switch (inKeyEvent.message & charCodeMask)
	{
		case char_Enter:
		case char_Return:
			keyButton = (LControl*)FindPaneByID('okay');
			break;	
		case char_Escape:
			if ((inKeyEvent.message & keyCodeMask) == vkey_Escape)
				keyButton = (LControl*)FindPaneByID('okay');
			break;
		default:
			if (UKeyFilters::IsCmdPeriod(inKeyEvent))
				keyButton = (LControl*)FindPaneByID('okay');
			else
				keyHandled = LWindow::HandleKeyPress(inKeyEvent);
			break;
	}
			
	if (keyButton)
	{
		keyButton->SimulateHotSpotClick(kControlButtonPart);
		keyHandled = true;
	}
	return keyHandled;
}
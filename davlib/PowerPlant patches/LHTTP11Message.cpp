/*
	LHTTPMessage.cpp
	Copyright (C) 2000 i-drive.com (email: opensource@mail.idrive.com)

	This library is free software; you can redistribute it and/or
	modify it under the terms of version 2.1 of the the GNU Lesser General
	Public License as published by the Free Software Foundation.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <LHTTP11Message.h>
#include <cctype>

#define kFieldCookie		"Cookie"
#define kFieldHost			"Host"
#define kFieldConnection	"Connection"

using namespace std;
  
// ---------------------------------------------------------------------------
//		¥ BuildHeader
// ---------------------------------------------------------------------------
//
void LHTTP11Message::BuildHeader(LDynamicBuffer * outHeader)
{
	// Handle the custom header
	LHTTPMessage::BuildHeader(outHeader);

	// If header is custom we use it "as is"
	if (mCustomHeader)
		return;

	if (GetServer())
		AddFieldToBuffer(kFieldHost, mServer.c_str(), outHeader);	

	if (GetCookie()->size())
		AddFieldToBuffer (kFieldCookie, mCookie.c_str(), outHeader);
		
	if (!mPersistent)
		AddFieldToBuffer(kFieldConnection, "Close", outHeader);
}

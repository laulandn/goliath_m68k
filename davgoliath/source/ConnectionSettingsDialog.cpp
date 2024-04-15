/* ==================================================================================================
 * ConnectionSettingsDialog.cpp															   
 *    Goliath - a Finder like application that implements WebDAV
 *    Copyright (C) 1999-2002  Thomas Bednarz
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

#ifndef __ConnectionSettingsDialog_h__
#include "ConnectionSettingsDialog.h"
#endif
#ifndef _H_UModalDialogs
#include <UModalDialogs.h>
#endif
#ifndef _H_LEditText
#include <LEditText.h>
#endif
#ifndef _H_LCheckBox
#include <LCheckBox.h>
#endif
#ifndef _H_LPushButton
#include <LPushButton.h>
#endif
#ifndef _H_LURL
#include <LURL.h>
#endif
#ifndef __CDAVTableApp_h__ 
#include "CDAVTableApp.h"
#endif
#ifndef __CDAVTABLECONSTANTS_H__
#include "CDAVTableAppConstants.h"
#endif
#ifndef __CGOLIATHPREFERENCES_H__
#include "CGoliathPreferences.h"
#endif
#include <vector>
#include <stdio.h>
#include <string.h>

using namespace std;

// ==================================================================================

static bool gRegisteredConnectionDlog = false;
static bool gRegisteredIDiskDlog = false;

const char* sHTTPProtocol  = "http://";
const char* sHTTPSProtocol = "https://";
const SInt16 sHTTPPort = 80;
const SInt16 sHTTPSPort = 443;

// ==================================================================================

class CConnectionSettingsDialog : public LDialogBox {
public:
	enum { class_ID = FOUR_CHAR_CODE(CLIENTSETTINGSDLOG_CLSID) };

	CConnectionSettingsDialog(LStream* inStream);
	virtual ~CConnectionSettingsDialog() {;};
	
	bool DoDialogLoop(StDialogHandler& inDlogHandler, DisplaySettingsData& outData);
	
	void SetInitialValues(const DisplaySettingsData& inData);
	void SetProxyDefaults();
protected:
	virtual void		FinishCreateSelf();
	
	void ExtractData(DisplaySettingsData& outData);
	
private:
	LEditText*  mHostEditField;
	LEditText*	mUserField;
	LEditText*	mPasswordField;
	LCheckBox*  mProxyOptionBox;
	LEditText*  mProxyHostField;
	LEditText*  mProxyPortField;
	LCheckBox*  mProxyAuthOptionBox;
	LEditText*  mProxyUserField;
	LEditText*  mProxyPasswordField;
	LCheckBox*  mDisableAppleDblBox;
	
	LView*      mAdvOptionsPane;
	LPushButton* mOKButton;
	
	bool		mAdvOptionsPaneShowing;
	SDimension16 mAdvOptsViewSize;
};


class CIDiskSettingsDialog : public LDialogBox {
public:
	enum { class_ID = FOUR_CHAR_CODE('idsk') };

	CIDiskSettingsDialog(LStream* inStream);
	virtual ~CIDiskSettingsDialog() {;};
	
	bool DoDialogLoop(StDialogHandler& inDlogHandler, DisplaySettingsData& outData);

protected:
	virtual void		FinishCreateSelf();
	
	void ExtractData(DisplaySettingsData& outData);

private:
	LEditText*	mUserField;
	LEditText*	mPasswordField;
	LPushButton* mOKButton;
};

// ==================================================================================


// ---------------------------------------------------------------------------------
//		¥ CConnectionSettingsDialog
// ---------------------------------------------------------------------------------
//	
CConnectionSettingsDialog::CConnectionSettingsDialog(LStream* inStream):
	LDialogBox(inStream), 
	mHostEditField(NULL),
	mUserField(NULL),
	mPasswordField(NULL),
	mProxyOptionBox(NULL),
	mProxyHostField(NULL),
	mProxyPortField(NULL),
	mProxyAuthOptionBox(NULL),
	mProxyUserField(NULL),
	mProxyPasswordField(NULL),
	mDisableAppleDblBox(NULL),
	mAdvOptionsPane(NULL),
	mOKButton(NULL),
	mAdvOptionsPaneShowing(false) 
{

}


// ---------------------------------------------------------------------------------
//		¥ FinishCreateSelf
// ---------------------------------------------------------------------------------
//	
void CConnectionSettingsDialog::FinishCreateSelf() {
	mAdvOptionsPane = dynamic_cast<LView*>(FindPaneByID(CSDADVOPTVIEW));
	ThrowIfNil_(mAdvOptionsPane);

	mAdvOptionsPane->GetFrameSize(mAdvOptsViewSize);
	mAdvOptionsPane->Hide();
	ResizeWindowBy(0, -mAdvOptsViewSize.height);

	mAdvOptionsPaneShowing = false;

	mHostEditField = dynamic_cast<LEditText*>(FindPaneByID(CSDHOSTEDITFIELD));
	ThrowIfNil_ (mHostEditField);

	mUserField = dynamic_cast<LEditText*>(FindPaneByID(CSDUSEREDITFIELD));
	ThrowIfNil_ (mUserField);
	
    mPasswordField = dynamic_cast<LEditText*>(FindPaneByID(CSDPASSEDITFIELD));
	ThrowIfNil_ (mPasswordField);

	mProxyOptionBox = dynamic_cast<LCheckBox*>(FindPaneByID(CSDPROXYCHECKBOX));
	ThrowIfNil_ (mProxyOptionBox);
	
	mProxyHostField = dynamic_cast<LEditText*>(FindPaneByID(CSDPROXYHOSTEDITFIELD));
	ThrowIfNil_ (mProxyHostField);
	
	mProxyPortField = dynamic_cast<LEditText*>(FindPaneByID(CSDPROXYPORTEDITFIELD));
	ThrowIfNil_ (mProxyPortField);

    mProxyHostField->Disable();
	mProxyPortField->Disable();

	mProxyAuthOptionBox = dynamic_cast<LCheckBox*>(FindPaneByID(CSDPROXYAUTCHCHECKBOX));
	ThrowIfNil_ (mProxyAuthOptionBox);
	
    mProxyUserField = dynamic_cast<LEditText*>(FindPaneByID(CSDPROXYUSEREDITFIELD));
	ThrowIfNil_ (mProxyUserField);
	mProxyUserField->Disable();
	
	mProxyPasswordField = dynamic_cast<LEditText*>(FindPaneByID(CSDPROXYPASSEDITFIELD));
	ThrowIfNil_ (mProxyPasswordField);
    mProxyPasswordField->Disable();

	
	mOKButton = dynamic_cast<LPushButton*>(FindPaneByID(CSDOKBUTTON));
	ThrowIfNil_ (mOKButton);
	
	mDisableAppleDblBox = dynamic_cast<LCheckBox*>(FindPaneByID(CDSAPPLEDOUBLECHECKBOX));
	ThrowIfNil_ (mDisableAppleDblBox);
}

// ---------------------------------------------------------------------------------
//		¥ DoDialogLoop
// ---------------------------------------------------------------------------------
//	
bool CConnectionSettingsDialog::DoDialogLoop(StDialogHandler& inDlogHandler, DisplaySettingsData& outData) {
    Show();
    LStr255 tmpStr;
	while (true) {
	    mHostEditField->GetText(tmpStr);
	    if (tmpStr.Length() == 0) 
	       mOKButton->Disable();
	    else
	       mOKButton->Enable();
	       
		MessageT hitMessage = inDlogHandler.DoDialog();
		if (hitMessage == PP_PowerPlant::msg_Cancel)
			return false;
		else if (hitMessage == PP_PowerPlant::msg_OK)
			break;
		else if (hitMessage == 1001) {
		   if (mProxyOptionBox->GetValue()) {
		      mProxyHostField->Enable();
		      mProxyPortField->Enable();
		   } else {
		      mProxyHostField->Disable();
		      mProxyPortField->Disable();
		   }
		} else if (hitMessage == 1002) {
		   if (mProxyAuthOptionBox->GetValue()) {
		      mProxyUserField->Enable();
		      mProxyPasswordField->Enable();
		   } else {
		      mProxyUserField->Disable();
		      mProxyPasswordField->Disable();
		   }
		} else if ('DTRI' == hitMessage	) {
	        mAdvOptionsPaneShowing = !mAdvOptionsPaneShowing;
			if (mAdvOptionsPaneShowing) {
			   mAdvOptionsPane->Show();
			   ResizeWindowBy(0, mAdvOptsViewSize.height);
			}
			else {
			   mAdvOptionsPane->Hide();
			   ResizeWindowBy(0, -mAdvOptsViewSize.height);
            }
		}
	}
	ExtractData(outData);
    Hide();
    return true;
}

// ---------------------------------------------------------------------------------
//		¥ SetInitialValues
// ---------------------------------------------------------------------------------
//	
void CConnectionSettingsDialog::SetInitialValues(const DisplaySettingsData& inData) {
	string tmpStr;
	
	if (inData.mForceSecure) {
		tmpStr.append(sHTTPSProtocol);
	} else {
		tmpStr.append(sHTTPProtocol);
	}
	tmpStr.append(inData.mHost);
	if ((inData.mPort != sHTTPPort) && (inData.mPort != sHTTPSPort)) {
		tmpStr.append(":");
		char buf[10];
		sprintf(&buf[0], "%d\0", inData.mPort);
		tmpStr.append(buf);
	}
	tmpStr.append(inData.mPath);
	mHostEditField->SetText(tmpStr.c_str(), tmpStr.size());
	
	mUserField->SetText(inData.mUserName.c_str(), inData.mUserName.size());
	mPasswordField->SetText(inData.mPassWord.c_str(), inData.mPassWord.size());

	mProxyOptionBox->SetValue(inData.mUseProxy);
	mProxyHostField->SetText(inData.mProxyServer.c_str(), inData.mProxyServer.size());
	
	if (0 != inData.mProxyPort) {
		char proxbuf[10];
		sprintf(&proxbuf[0], "%d\0", inData.mProxyPort);
		mProxyPortField->SetText(&proxbuf[0], strlen(proxbuf));
	}
	
	mProxyAuthOptionBox->SetValue(inData.mUseProxyAuth);
	mProxyUserField->SetText(inData.mProxyUser.c_str(), inData.mProxyUser.size());
	mProxyPasswordField->SetText(inData.mProxyPass.c_str(), inData.mProxyPass.size());

	mDisableAppleDblBox->SetValue(inData.mDisableAppleDoubleEncoding);
}

// ---------------------------------------------------------------------------------
//		¥ SetInitialValues
// ---------------------------------------------------------------------------------
//	
void CConnectionSettingsDialog::SetProxyDefaults() {
	bool checkSrvr=false, checkAuth=false;
	CGoliathPreferences* preferences = GetApplicationInstance()->GetPreferencesManager();
	std::string tmpStr = preferences->GetPrefValue(CGoliathPreferences::PROXY_SERVER);							
	if (0 != tmpStr.size()) {
		mProxyHostField->SetText(tmpStr.c_str(), tmpStr.size());
		checkSrvr = true;
	}
	
    tmpStr = preferences->GetPrefValue(CGoliathPreferences::PROXY_SRVRPORT);							
	if (0 != tmpStr.size()) {
		mProxyPortField->SetText(tmpStr.c_str(), tmpStr.size());
		checkSrvr = true;
	}
	
    tmpStr = preferences->GetPrefValue(CGoliathPreferences::PROXY_USER);							
	if (0 != tmpStr.size()) {
		mProxyUserField->SetText(tmpStr.c_str(), tmpStr.size());
		checkAuth = true;
	}

    tmpStr = preferences->GetPrefValue(CGoliathPreferences::PROXY_PASSWORD);							
	if (0 != tmpStr.size()) {
		mProxyPasswordField->SetText(tmpStr.c_str(), tmpStr.size());
		checkAuth = true;
	}

	if (checkSrvr) {
		mProxyOptionBox->SetValue(true);
		mProxyHostField->Enable();
		mProxyPortField->Enable();
	}
	
	if (checkAuth) {
		mProxyAuthOptionBox->SetValue(true);
		mProxyUserField->Enable();
		mProxyPasswordField->Enable();
	}
}

// ---------------------------------------------------------------------------------
//		¥ ExtractData
// ---------------------------------------------------------------------------------
//	
static std::string GetEditTextData(LEditText* inField) {
	Size theSize;
	inField->GetText(NULL, 0, &theSize);
	vector<char> textArray(theSize+1);
	inField->GetText(&textArray[0], theSize, &theSize);
	textArray[theSize]=0;
	
	return std::string(reinterpret_cast<char*>(&textArray[0]));
}

// ---------------------------------------------------------------------------------
//		¥ ExtractData
// ---------------------------------------------------------------------------------
//	
void CConnectionSettingsDialog::ExtractData(DisplaySettingsData& outData) {
	string theHost;
	theHost = GetEditTextData(mHostEditField);
	if ((theHost.find(sHTTPProtocol)!=0) && (theHost.find(sHTTPSProtocol)!=0)) {
	   theHost.assign(sHTTPProtocol+theHost);
	}
	
	LURL theUrl(theHost.c_str(), theHost.size());
	outData.mHost = theUrl.GetHost();
	outData.mPath = theUrl.GetPath();
	if (outData.mPath[0] != '/') {
		outData.mPath = "/" + outData.mPath;
	}
	outData.mPort=theUrl.GetPort();
	
	outData.mForceSecure = false;
	if (outData.mPort == 0) {
		if (theHost.find(sHTTPSProtocol)==0) {
			outData.mPort=sHTTPSPort;
            theUrl.SetPort(sHTTPSPort);  
            outData.mForceSecure = true;
		} else {
			outData.mPort=sHTTPPort;
			theUrl.SetPort(sHTTPPort);      
		}
	}
	if (theHost.find(sHTTPSProtocol)==0) {
      outData.mForceSecure = true;
	}
	
	outData.mUserName = GetEditTextData(mUserField);
	outData.mPassWord = GetEditTextData(mPasswordField);
	
	outData.mUseProxy = mProxyOptionBox->GetValue();
	outData.mProxyServer = GetEditTextData(mProxyHostField);
	
	string tmpStr;
	tmpStr = GetEditTextData(mProxyPortField);
	outData.mProxyPort = atoi(tmpStr.c_str());
	
	
	outData.mUseProxyAuth = mProxyAuthOptionBox->GetValue();
	outData.mProxyUser = GetEditTextData(mProxyUserField);
	outData.mProxyPass = GetEditTextData(mProxyPasswordField);
	outData.mDisableAppleDoubleEncoding = mDisableAppleDblBox->GetValue();
	
}

// ---------------------------------------------------------------------------------
//		¥ DisplayConnectionSettingsDialog
// ---------------------------------------------------------------------------------
//	
bool DisplayConnectionSettingsDialog(DisplaySettingsData& inOutData, bool inEditing) {
    
	if (!gRegisteredConnectionDlog) {
		RegisterClass_(CConnectionSettingsDialog);
		gRegisteredConnectionDlog = true;
	}
	
	StDialogHandler dlogHandler(CLIENTSETTINGSDLOG, GetApplicationInstance());
	CConnectionSettingsDialog* theDlog = dynamic_cast<CConnectionSettingsDialog*>(dlogHandler.GetDialog());
	Assert_(theDlog != NULL);
	if (NULL == theDlog)
		return false;
	
	if (inEditing) {
		theDlog->SetInitialValues(inOutData);
		LStr255 newtitle(str_UIStrings, str_EditConnectionSettingsDlogTitle);
		theDlog->SetDescriptor(newtitle);
	} else {
		theDlog->SetProxyDefaults();
	}
	
	return theDlog->DoDialogLoop(dlogHandler, inOutData);
}



// ---------------------------------------------------------------------------------
//		¥ CIDiskSettingsDialog
// ---------------------------------------------------------------------------------
//	
CIDiskSettingsDialog::CIDiskSettingsDialog(LStream* inStream):
	LDialogBox(inStream),
	mUserField(NULL),
	mPasswordField(NULL),
	mOKButton(NULL)
{

}

// ---------------------------------------------------------------------------------
//		¥ FinishCreateSelf
// ---------------------------------------------------------------------------------
//	
void CIDiskSettingsDialog::FinishCreateSelf() {
	mUserField = dynamic_cast<LEditText*>(FindPaneByID(CSDUSEREDITFIELD));
	ThrowIfNil_ (mUserField);
	
    mPasswordField = dynamic_cast<LEditText*>(FindPaneByID(CSDPASSEDITFIELD));
	ThrowIfNil_ (mPasswordField);

	mOKButton = dynamic_cast<LPushButton*>(FindPaneByID(CSDOKBUTTON));
	ThrowIfNil_ (mOKButton);
	
}

// ---------------------------------------------------------------------------------
//		¥ CIDiskSettingsDialog
// ---------------------------------------------------------------------------------
//	
bool CIDiskSettingsDialog::DoDialogLoop(StDialogHandler& inDlogHandler, DisplaySettingsData& outData) {
    Show();
    LStr255 tmpStr, tmpStr2;
	while (true) {
	    mUserField->GetText(tmpStr);
	    mPasswordField->GetText(tmpStr2);
	    
	    if (tmpStr.Length() == 0 || tmpStr2.Length() == 0) 
	       mOKButton->Disable();
	    else
	       mOKButton->Enable();
	       
		MessageT hitMessage = inDlogHandler.DoDialog();
		if (hitMessage == PP_PowerPlant::msg_Cancel)
			return false;
		else if (hitMessage == PP_PowerPlant::msg_OK)
			break;
	}
	ExtractData(outData);
    Hide();
    return true;
}

// ---------------------------------------------------------------------------------
//		¥ ExtractData
// ---------------------------------------------------------------------------------
//	
void CIDiskSettingsDialog::ExtractData(DisplaySettingsData& outData) {
	LStr255 rezHost(str_FileNameStrings, str_iDiskHostName);
	outData.mHost.append(rezHost.ConstTextPtr(),  rezHost.Length());

	outData.mPort=sHTTPPort;
	outData.mForceSecure = false;
	 
	outData.mUserName = GetEditTextData(mUserField);
	outData.mPassWord = GetEditTextData(mPasswordField);

	LStr255 pathPrototype(str_FileNameStrings, str_iDiskPathString);
	pathPrototype[pathPrototype.Length()+1] = NULL;
	char *pathbuf = new char[pathPrototype.Length() + outData.mUserName.size()+10];

	sprintf(pathbuf, pathPrototype.ConstTextPtr(), outData.mUserName.c_str());
	
	outData.mPath = pathbuf;
	delete[] pathbuf;
	
	outData.mUseProxy = false;
	CGoliathPreferences* preferences = GetApplicationInstance()->GetPreferencesManager();
	std::string tmpStr = preferences->GetPrefValue(CGoliathPreferences::PROXY_SERVER);							
	if (0 != tmpStr.size()) {
		outData.mUseProxy = true;
		outData.mProxyServer = tmpStr;
	}
	
    tmpStr = preferences->GetPrefValue(CGoliathPreferences::PROXY_SRVRPORT);							
	if (0 != tmpStr.size()) {
		outData.mProxyPort = atoi(tmpStr.c_str());
	}
	
    outData.mUseProxyAuth = false;
    tmpStr = preferences->GetPrefValue(CGoliathPreferences::PROXY_USER);							
	if (0 != tmpStr.size()) {
		outData.mUseProxyAuth = true;
		outData.mProxyUser = tmpStr;
	}

    tmpStr = preferences->GetPrefValue(CGoliathPreferences::PROXY_PASSWORD);							
	if (0 != tmpStr.size()) {
		outData.mProxyPass = tmpStr;
	}
	
	outData.mDisableAppleDoubleEncoding = false;
}

// ---------------------------------------------------------------------------------
//		¥ DisplayConnectionSettingsDialog
// ---------------------------------------------------------------------------------
//	
bool DisplayIDiskSettingsDialog(DisplaySettingsData& inOutData) {
	if (!gRegisteredIDiskDlog) {
		RegisterClass_(CIDiskSettingsDialog);
		gRegisteredIDiskDlog = true;
	}
	
	StDialogHandler dlogHandler(IDISKSETTINGSDLOG, GetApplicationInstance());
	CIDiskSettingsDialog* theDlog = dynamic_cast<CIDiskSettingsDialog*>(dlogHandler.GetDialog());
	Assert_(theDlog != NULL);
	if (NULL == theDlog)
		return false;
		
	return theDlog->DoDialogLoop(dlogHandler, inOutData);
	
}


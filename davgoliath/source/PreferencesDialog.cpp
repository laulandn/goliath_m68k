/* ==================================================================================================
 * PreferencesDialog.cpp												   
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

#ifndef __PreferencesDialog_h__
#include "PreferencesDialog.h"
#endif
#ifndef _H_UModalDialogs
#include <UModalDialogs.h>
#endif
#ifndef _H_LCheckBox
#include <LCheckBox.h>
#endif
#ifndef _H_LRadioButton
#include <LRadioButton.h>
#endif
#ifndef _H_LMultiPanelView
#include <LMultiPanelView.h>
#endif
#ifndef _H_LEditText
#include <LEditText.h>
#endif
#ifndef __CDAVLIBUTILS_H__
#include "CDAVLibUtils.h"
#endif
#ifndef __CGOLIATHPREFERENCES_H__
#include "CGoliathPreferences.h"
#endif
#ifndef __CDAVTableApp_h__
#include "CDAVTableApp.h"
#endif
#ifndef __CDAVTABLECONSTANTS_H__
#include "CDAVTableAppConstants.h"
#endif
#ifndef _VECTOR
#include <vector>
#endif
#include <string.h>

static bool gRegisteredPrefsDlog = false;

class PrefPanel : public LView {
public:
	PrefPanel(LStream* inStream);
	virtual ~PrefPanel() {;};
	
	virtual void SetUIFromPrefs()=0;
	virtual void SetPrefsFromUI()=0;
	
	CGoliathPreferences* mPreferences;
};

class CGeneralPrefsView : public PrefPanel {

public:
	enum { class_ID = FOUR_CHAR_CODE('GPRF') };

	CGeneralPrefsView(LStream* inStream);
	virtual ~CGeneralPrefsView() {;};
	virtual void SetUIFromPrefs();
	virtual void SetPrefsFromUI();

protected:
	virtual void	FinishCreateSelf();
	LRadioButton*	mNewConnection;
	LRadioButton*	mOpenConnection;
	LRadioButton*	mIDiskConnection;
	LRadioButton*	mNoConnection;
	LCheckBox*		mShowHidden;
	LCheckBox*		mSortCaseInsensitive;
};

class CLockPrefsView : public PrefPanel {

public:
	enum { class_ID = FOUR_CHAR_CODE('LPRF') };

	CLockPrefsView(LStream* inStream);
	virtual ~CLockPrefsView() {;};
	virtual void SetUIFromPrefs();
	virtual void SetPrefsFromUI();

protected:
	virtual void	FinishCreateSelf();
	
	 PP_PowerPlant::LEditText*	mOwnerInfo;
	 PP_PowerPlant::LCheckBox*  mSetOwner;
};

class CProxyPrefsView :  public PrefPanel {

public:
	enum { class_ID = FOUR_CHAR_CODE('PPRF') };

	CProxyPrefsView(LStream* inStream);
	virtual ~CProxyPrefsView() {;};
	virtual void SetUIFromPrefs();
	virtual void SetPrefsFromUI();

protected:
	virtual void	FinishCreateSelf();

	LEditText*	mProxyHost;
	LEditText*	mProxyPort;
	LEditText*	mProxyUser;
	LEditText*	mProxyPassword;
};


class CPreferencesDialog : public LDialogBox {
public:
	enum { class_ID = FOUR_CHAR_CODE('PRFD') };

	CPreferencesDialog(LStream* inStream);
	virtual ~CPreferencesDialog() {;};

	void RegisterPanel(PrefPanel* prefPanel);
	void SetPrefsFromUI();
	
private:
	std::vector<PrefPanel*>	mActivatedPanels;
};


// ---------------------------------------------------------------------------------
//		¥ CPreferencesDialog
// ---------------------------------------------------------------------------------
//	
CPreferencesDialog::CPreferencesDialog(LStream* inStream) : LDialogBox(inStream) {

}

// ---------------------------------------------------------------------------------
//		¥ RegisterPanel
// ---------------------------------------------------------------------------------
//	
void CPreferencesDialog::RegisterPanel(PrefPanel* prefPanel) {
	mActivatedPanels.push_back(prefPanel);
}

// ---------------------------------------------------------------------------------
//		¥ SetPrefsFromUI
// ---------------------------------------------------------------------------------
//	
void CPreferencesDialog::SetPrefsFromUI() {
	for (std::vector<PrefPanel*>::iterator iter = mActivatedPanels.begin(); iter != mActivatedPanels.end(); ++iter) {
		(*iter)->SetPrefsFromUI();
	}
}

// ---------------------------------------------------------------------------------
//		¥ PrefPanel
// ---------------------------------------------------------------------------------
//	
PrefPanel::PrefPanel(LStream* inStream):LView(inStream)  {
	mPreferences = GetApplicationInstance()->GetPreferencesManager();
	LView* superView = GetSuperView();
	while (NULL != superView) {
		CPreferencesDialog *dlogPtr = dynamic_cast<CPreferencesDialog*>(superView);
		if (NULL != dlogPtr) {
			dlogPtr->RegisterPanel(this);
			break;
		} 
		superView = superView->GetSuperView();
	}
}

// ---------------------------------------------------------------------------------
//		¥ CGeneralPrefsView
// ---------------------------------------------------------------------------------
//	
CGeneralPrefsView::CGeneralPrefsView(LStream* inStream): PrefPanel(inStream), 
	mNewConnection(NULL), mOpenConnection(NULL), mIDiskConnection(NULL),
	mNoConnection(NULL), mShowHidden(NULL),
	mSortCaseInsensitive(NULL) {

}

// ---------------------------------------------------------------------------------
//		¥ SetUIFromPrefs
// ---------------------------------------------------------------------------------
//	
void CGeneralPrefsView::SetUIFromPrefs() {
    std::string showDotFiles = mPreferences->GetPrefValue(CGoliathPreferences::SHOWDOTFILES);
    if (strcmp(showDotFiles.c_str(), CGoliathPreferences::TRUE_VALUE)==0)
       mShowHidden->SetValue(Button_On);
    else
       mShowHidden->SetValue(Button_Off);

    std::string strtAction = mPreferences->GetPrefValue(CGoliathPreferences::STARTUP_ACTION);
    if (strcmp(strtAction.c_str(), CGoliathPreferences::STARTUP_ACTION_NEW)==0)
       mNewConnection->SetValue(Button_On);
    else if (strcmp(strtAction.c_str(), CGoliathPreferences::STARTUP_ACTION_OPEN)==0)
       mOpenConnection->SetValue(Button_On);
    else if (strcmp(strtAction.c_str(), CGoliathPreferences::STARTUP_ACTION_NONE)==0)
       mNoConnection->SetValue(Button_On);
    else if (strcmp(strtAction.c_str(), CGoliathPreferences::STARTUP_ACTION_IDISK)==0)
       mIDiskConnection->SetValue(Button_On);
       
    std::string sortCaseInsensitive = mPreferences->GetPrefValue(CGoliathPreferences::SORT_CASEINSENSITIVE);
    if (strcmp(sortCaseInsensitive.c_str(), CGoliathPreferences::TRUE_VALUE)==0)
       mSortCaseInsensitive->SetValue(Button_On);
    else
       mSortCaseInsensitive->SetValue(Button_Off);

}

// ---------------------------------------------------------------------------------
//		¥ SetPrefsFromUI
// ---------------------------------------------------------------------------------
//	
void CGeneralPrefsView::SetPrefsFromUI() {
     std::string strtAction;
     if (Button_On == mNewConnection->GetValue()) 
       strtAction = CGoliathPreferences::STARTUP_ACTION_NEW;
    else if (Button_On == mOpenConnection->GetValue()) 
       strtAction = CGoliathPreferences::STARTUP_ACTION_OPEN;
    else if (Button_On == mNoConnection->GetValue()) 
       strtAction = CGoliathPreferences::STARTUP_ACTION_NONE;
    else if (Button_On == mIDiskConnection->GetValue())
       strtAction = CGoliathPreferences::STARTUP_ACTION_IDISK;
    
    mPreferences->SetPrefValue(CGoliathPreferences::STARTUP_ACTION, strtAction.c_str());
    
    if (Button_On == mShowHidden->GetValue() ) {
       mPreferences->SetPrefValue(CGoliathPreferences::SHOWDOTFILES, CGoliathPreferences::TRUE_VALUE);
    } else {
       mPreferences->SetPrefValue(CGoliathPreferences::SHOWDOTFILES, CGoliathPreferences::FALSE_VALUE);
    }
    
    if (Button_On == mSortCaseInsensitive->GetValue() ) {
       mPreferences->SetPrefValue(CGoliathPreferences::SORT_CASEINSENSITIVE, CGoliathPreferences::TRUE_VALUE);
    } else {
       mPreferences->SetPrefValue(CGoliathPreferences::SORT_CASEINSENSITIVE, CGoliathPreferences::FALSE_VALUE);    
    }

}

// ---------------------------------------------------------------------------------
//		¥ FinishCreateSelf
// ---------------------------------------------------------------------------------
//	
void CGeneralPrefsView::FinishCreateSelf() {
    mNewConnection = dynamic_cast<PP_PowerPlant::LRadioButton*>
										(FindPaneByID(PREFSNEWCONN));
    Assert_(mNewConnection != nil);

    mOpenConnection = dynamic_cast<PP_PowerPlant::LRadioButton*>
										(FindPaneByID(PREFSOPENCONN));
    Assert_(mOpenConnection != nil);

	mIDiskConnection = dynamic_cast<PP_PowerPlant::LRadioButton*>
										(FindPaneByID(PREFSIDISKCONN));
    Assert_(mIDiskConnection != nil);
    
    mNoConnection = dynamic_cast<PP_PowerPlant::LRadioButton*>
										(FindPaneByID(PREFSNONE));
    Assert_(mNoConnection != nil);
    
    mShowHidden =  dynamic_cast<PP_PowerPlant::LCheckBox*>
										(FindPaneByID('hide'));
    Assert_(mShowHidden != nil);
    
    mSortCaseInsensitive = dynamic_cast<PP_PowerPlant::LCheckBox*>
										(FindPaneByID('sort'));
    Assert_(mSortCaseInsensitive != nil);

   	SetUIFromPrefs();
}


// ---------------------------------------------------------------------------------
//		¥ CLockPrefsView
// ---------------------------------------------------------------------------------
//	
CLockPrefsView::CLockPrefsView(LStream* inStream): PrefPanel(inStream),
	mOwnerInfo(NULL),
	mSetOwner(NULL) {

}

// ---------------------------------------------------------------------------------
//		¥ SetUIFromPrefs
// ---------------------------------------------------------------------------------
//	
void CLockPrefsView::SetUIFromPrefs() {
    LStr255 ownerStr(mPreferences->GetPrefValue(CGoliathPreferences::LOCK_OWNER));							
    if (mOwnerInfo)
    	mOwnerInfo->SetDescriptor(ownerStr);

    std::string setlock = mPreferences->GetPrefValue(CGoliathPreferences::SET_LOCK);
    if (strcmp(setlock.c_str(), CGoliathPreferences::TRUE_VALUE)==0)
       mSetOwner->SetValue(Button_On);
    else
       mSetOwner->SetValue(Button_Off);
}

// ---------------------------------------------------------------------------------
//		¥ SetPrefsFromUI
// ---------------------------------------------------------------------------------
//	
void CLockPrefsView::SetPrefsFromUI() {
    LStr255 ownerStr;
    mOwnerInfo->GetDescriptor(ownerStr);
    mPreferences->SetPrefValue(CGoliathPreferences::LOCK_OWNER, DAVp2cstr(ownerStr));

    if (Button_On == mSetOwner->GetValue() ) {
       mPreferences->SetPrefValue(CGoliathPreferences::SET_LOCK, CGoliathPreferences::TRUE_VALUE);
    } else {
       mPreferences->SetPrefValue(CGoliathPreferences::SET_LOCK, CGoliathPreferences::FALSE_VALUE);
    }
}

// ---------------------------------------------------------------------------------
//		¥ FinishCreateSelf
// ---------------------------------------------------------------------------------
//	
void CLockPrefsView::FinishCreateSelf() {
	PrefPanel::FinishCreateSelf();
	LPane* theCB = this->FindPaneByID('sown');
    mSetOwner =  dynamic_cast<PP_PowerPlant::LCheckBox*>(theCB);
    Assert_(mSetOwner);
	mOwnerInfo = dynamic_cast<PP_PowerPlant::LEditText*>(FindPaneByID('LOWN'));
	Assert_(mOwnerInfo);
	SetUIFromPrefs();
}


// ---------------------------------------------------------------------------------
//		¥ CProxyPrefsView
// ---------------------------------------------------------------------------------
//	
CProxyPrefsView::CProxyPrefsView(LStream* inStream): PrefPanel(inStream),
   mProxyHost(NULL), mProxyPort(NULL), mProxyUser(NULL), mProxyPassword(NULL) {

}

// ---------------------------------------------------------------------------------
//		¥ SetUIFromPrefs
// ---------------------------------------------------------------------------------
//	
void CProxyPrefsView::SetUIFromPrefs() {
    LStr255 tmpStr;
    tmpStr.Assign(mPreferences->GetPrefValue(CGoliathPreferences::PROXY_SERVER));							
    if (mProxyHost)
    	mProxyHost->SetDescriptor(tmpStr);

    tmpStr.Assign(mPreferences->GetPrefValue(CGoliathPreferences::PROXY_SRVRPORT));							
    if (mProxyPort)
    	mProxyPort->SetDescriptor(tmpStr);

    tmpStr.Assign(mPreferences->GetPrefValue(CGoliathPreferences::PROXY_USER));							
    if (mProxyUser)
    	mProxyUser->SetDescriptor(tmpStr);

    tmpStr.Assign(mPreferences->GetPrefValue(CGoliathPreferences::PROXY_PASSWORD));							
    if (mProxyPassword)
    	mProxyPassword->SetDescriptor(tmpStr);

}

// ---------------------------------------------------------------------------------
//		¥ SetPrefsFromUI
// ---------------------------------------------------------------------------------
//	
void CProxyPrefsView::SetPrefsFromUI() {
    LStr255 tmpStr;
    
    mProxyHost->GetDescriptor(tmpStr);
    mPreferences->SetPrefValue(CGoliathPreferences::PROXY_SERVER, DAVp2cstr(tmpStr));

    mProxyPort->GetDescriptor(tmpStr);
    mPreferences->SetPrefValue(CGoliathPreferences::PROXY_SRVRPORT, DAVp2cstr(tmpStr));

    mProxyUser->GetDescriptor(tmpStr);
    mPreferences->SetPrefValue(CGoliathPreferences::PROXY_USER, DAVp2cstr(tmpStr));

    mProxyPassword->GetDescriptor(tmpStr);
    mPreferences->SetPrefValue(CGoliathPreferences::PROXY_PASSWORD, DAVp2cstr(tmpStr));

}

// ---------------------------------------------------------------------------------
//		¥ FinishCreateSelf
// ---------------------------------------------------------------------------------
//	
void CProxyPrefsView::FinishCreateSelf() {
   mProxyHost = dynamic_cast<PP_PowerPlant::LEditText*>(FindPaneByID('PHST')); 
   Assert_(mProxyHost);
   mProxyPort = dynamic_cast<PP_PowerPlant::LEditText*>(FindPaneByID('PPRT'));
   Assert_(mProxyPort);
   mProxyUser = dynamic_cast<PP_PowerPlant::LEditText*>(FindPaneByID('PUSR'));
   Assert_(mProxyUser);
   mProxyPassword = dynamic_cast<PP_PowerPlant::LEditText*>(FindPaneByID('PPSS'));
   Assert_(mProxyPassword);

   SetUIFromPrefs();   
}

// ---------------------------------------------------------------------------------
//		¥ DisplayPreferencesDialog
// ---------------------------------------------------------------------------------
//	
void DisplayPreferencesDialog() {

	if (!gRegisteredPrefsDlog) {
		RegisterClass_(CGeneralPrefsView);
		RegisterClass_(CLockPrefsView);
		RegisterClass_(CProxyPrefsView);
		RegisterClass_(CPreferencesDialog);
		gRegisteredPrefsDlog = true;
	}
	
	std::vector<PrefPanel*> prefPanels;
	
    CGoliathPreferences* prefs = GetApplicationInstance()->GetPreferencesManager();
	PP_PowerPlant::StDialogHandler dialog(PREFSDLOG, GetApplicationInstance());
	CPreferencesDialog* prefDlog = dynamic_cast<CPreferencesDialog*>(dialog.GetDialog());
	Assert_(prefDlog != nil);
	
	prefDlog->Show();
	
	while (true) {
		PP_PowerPlant::MessageT hitMessage = dialog.DoDialog();
		if (hitMessage == PP_PowerPlant::msg_Cancel)
			return;
		else if (hitMessage == PP_PowerPlant::msg_OK)
			break;
	}

	prefDlog->SetPrefsFromUI();
    prefDlog->Hide();

	prefs->CommitPreferences();
}
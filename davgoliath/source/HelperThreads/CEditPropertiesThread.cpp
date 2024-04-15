/* ==================================================================================================
 * CEditPropertiesThread.cpp															   
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
#include <UModalDialogs.h>
#include <CDAVItem.h>
#include "CDAVTableApp.h"
#include "CDAVTableWindow.h"
#include <LThread.h>
#include <LProgressBar.h>
#include <CDAVRequest.h>
#include <LOutlineItem.h>
#include "CDAVTableItem.h"
#include "CEditPropertiesThread.h"
#include <CDAVTranscodeUtils.h>
#include <CProgressDialog.h>
#include <CWindowManager.h>
#include <CDAVHeadingTable.h>
#include <CPropDisplayTable.h>
#include <CDAVTableAppConstants.h>
#include <LStaticText.h>
#include<LEditText.h>
#include<LPushButton.h>

// ---------------------------------------------------------------------------
//		¥ CEditPropertiesThread()
// ---------------------------------------------------------------------------
//
CEditPropertiesThread::CEditPropertiesThread(CDAVContext *ctx, CDAVTableWindow *wnd, CDAVItem& item)
 :CFLVThread(ctx, wnd) {
    mItem = item;
}

// ---------------------------------------------------------------------------
//		¥ _onAddProp()
// ---------------------------------------------------------------------------
//
void CEditPropertiesThread::_onAddProp(LWindow* pnt) {
	PP_PowerPlant::StDialogHandler dialog(ADDNEWPROPDLOG, pnt);
	LWindow *dlogPtr = dialog.GetDialog();
	Assert_(dlogPtr != nil);
    
    PP_PowerPlant::LEditText* propName = dynamic_cast<PP_PowerPlant::LEditText*>
									(dlogPtr->FindPaneByID(PNAME_EDITFIELD));
	Assert_(propName != nil);

    PP_PowerPlant::LEditText* propNS = dynamic_cast<PP_PowerPlant::LEditText*>
									(dlogPtr->FindPaneByID(PNAMESPACE_EDITFIELD));									
	Assert_(propNS != nil);

    PP_PowerPlant::LEditText* propVal = dynamic_cast<PP_PowerPlant::LEditText*>
									(dlogPtr->FindPaneByID(PVAL_EDITFIELD));
	Assert_(propVal != nil);
									
    PP_PowerPlant::LPushButton* okBtn = dynamic_cast<PP_PowerPlant::LPushButton*>
									(dlogPtr->FindPaneByID(PVAL_OKBUTTON));
	Assert_(okBtn != nil);
	okBtn->Disable();
	LStr255 pname, pns, pval;
	
    while (true) {
		PP_PowerPlant::MessageT hitMessage = dialog.DoDialog();
		if (hitMessage == PP_PowerPlant::msg_Cancel)
			return;
		else if (hitMessage == PP_PowerPlant::msg_OK)
			return;
	    else {
	       propName->GetText(pname);
	       propNS->GetText(pns);
	       propVal->GetText(pval);
	       if ((pname.Length()>0) && (pns.Length() > 0) && (pval.Length() > 0))
	          okBtn->Enable();
	       else
	          okBtn->Disable();
	    }
	}
    
}

// ---------------------------------------------------------------------------
//		¥ _executeDAVTransaction()
// ---------------------------------------------------------------------------
//		
void* CEditPropertiesThread::_executeDAVTransaction() {
   CDAVPropertyVector allProps;
   if (CDAVRequest::SUCCESS != 
      mRequest.FindAllProperties(*this, mItem.GetHREF(), allProps)) {
      mWnd->ResetShowEditPropsState();
      return nil;
   }

   if (CDAVRequest::SUCCESS != 
      mRequest.GetItemProperties(*this, mItem, allProps)) {
      mWnd->ResetShowEditPropsState();
      return nil;
    }
    
	PP_PowerPlant::StDialogHandler dialog(EDITPROPSDLOG, mWnd);
	Assert_(dialog.GetDialog() != nil);

	CDAVHeadingTable *heading = reinterpret_cast<CDAVHeadingTable*>(dialog.GetDialog()->FindPaneByID( EDITPROPSDLOGTBLTHDR ));
	CPropDisplayTable *table = reinterpret_cast<CPropDisplayTable*>(dialog.GetDialog()->FindPaneByID(EDITPROPSDLOGTABLE));
    
 	LBevelButton *button = reinterpret_cast<LBevelButton*>(dialog.GetDialog()->FindPaneByID('sort'));

    table->Setup(button, heading);
    
    heading->InsertCols(2, 0, nil, nil, false);
	heading->SetColumnHeading( 1, LStr255(1000, 16), kControlBevelButtonAlignTextCenter, 0);
	heading->SetColWidth(180,  1,  1);
	heading->SetColumnHeadingValueMessage(1, kPropNameColMessage);
	heading->SetColumnListener(1, table);	
	
	heading->SetColumnHeading( 2, LStr255(1000, 17), kControlBevelButtonAlignTextCenter, 0);
	heading->SetColWidth(180,  2,  2);
	heading->SetColumnHeadingValueMessage(2, kPropValColMessage);
	heading->SetColumnListener(2, table);	

	LStaticText *hdrtext = reinterpret_cast<LStaticText*>(dialog.GetDialog()->FindPaneByID( EDITPROPSDLOGTEXTHDR));
	
	std::string hdrTextStr, href = CDAVTranscodeUtils::TranscodeUTF8ToSystemScript(mItem.GetHREF());								
	
	LStr255 hdrTextVal(1000, 18);
	hdrTextStr.append(hdrTextVal.ConstTextPtr(), hdrTextVal.Length());
	hdrTextStr.append(href);
	
	if (hdrtext)
	   hdrtext->SetText(const_cast<char*>(hdrTextStr.c_str()), hdrTextStr.size());

	CDAVPropertyVector::iterator iter = allProps.begin();
	while (iter != allProps.end()) {
	  std::string val =  mItem.GetRawPropertyValue(*iter);
	  table->AddPropVal(*iter, val);
	  iter++;
	}
	
	dialog.GetDialog()->Show();
	mWnd->ResetShowEditPropsState();
	
	while (true) {
		PP_PowerPlant::MessageT hitMessage = dialog.DoDialog();
		if (hitMessage == PP_PowerPlant::msg_Cancel)
			return nil;
		else if (hitMessage == PP_PowerPlant::msg_OK)
			return nil;
	    else if (hitMessage == NEWPROP_CMD) {
	       _onAddProp(dialog.GetDialog());
	    }
	}
	
    return nil;   
}
/* ==================================================================================================
 * CDAVTableWindow.h													   
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

#ifndef __CDAVTableWindow_h__
#define __CDAVTableWindow_h__

#pragma once

#include <LWindow.h>
#include <LListener.h>
#include "CDAVTable.h"
#include "CUploadFileThread.h"
#include <CDAVInfo.h>
#include "DataTransfer.h"
#include <string>

#ifndef __PARSECONNECTIONDOCUMENT_H__
#include "ParseConnectionDocument.h"
#endif
class DisplaySettingsData;

class CDAVTableWindow : public LWindow, public LListener {
	public:
		enum {class_ID = 'DAVW'};
		
		CDAVTableWindow();
		CDAVTableWindow(LStream* inStream);
		~CDAVTableWindow();
		
		static CDAVTableWindow*	CreateFromStream(LStream* inStream);
		
		virtual	void 		FinishCreateSelf();
		virtual void		UpdatePort();
		virtual void 		ListenToMessage(MessageT inMessage, void* ioParam);
 		
 		//***teb - FIX THIS; Unify these 2 structs
 		void SetDisplaySettings(DisplaySettingsData* inDisplaySettings);
		void SetDisplaySettings(ConnectionDocumentData* inDisplaySettings, FSSpec *inSpec);

        class CDAVContext *GetDAVContext(); 
   	    void SetDAVInfo(CDAVInfo& info);
   	    
   	    CDAVPropertyVector& GetRequiredProperties();
   	    void AddToRequiredProperties(CDAVProperty& newProp);
   	    
	    void LoadInitialResource(std::string& resource);
        void OnFolderExpansion(std::string& res, LOutlineItem *pnt, Boolean isInitExpansion=false);

		virtual Boolean	ObeyCommand(CommandT inCommand, void* ioParam);	
		virtual void FindCommandStatus(CommandT inCommand, Boolean &outEnabled, Boolean &outUsesMark, UInt16 &outMark, Str255 outName);
	    void SetAppPtr(LApplication *app);
	    std::string& GetBaseResource();
	    
	    void UploadFSSpec(FSSpec *theSpec, CDAVTableItem *pntItem = NULL);
	    void UploadFSSpecVector(FSSpecVector& theSpecs, CDAVTableItem *pntItem=NULL);

        void SetColumnWidths(std::vector<UInt16> colWidths);
        
        void ResetShowEditPropsState() {mOpeningPropsDlog = false;};
        
        CDAVInfo& GetCurrentInfo() {return mDavInfo;};
        
        const CDAVTable* GetTable() const {return mFLVTable;};
        
        void DisableAppleEncoding();
        void EnableAppleEncoding();
        
        bool EncodeResources() const;
	protected:
	   void SetDAVContext(class CDAVContext *ctx);

	   void _init();
	  
	   void _OnNewFolderCommand();
	   void _OnUploadNewFile();
	   void _OnRefreshView();
	   void _OnDeleteItems();
	   void _OnDownloadItems();
	   void _OnDownloadItemWithOptions();
	   void _OnLockItems();
	   void _OnUnlockItems();
	   void _OnLockAndDownload();
	   void _OnUnlockAndUpload();
	   void _OnClaimLockCommand();
	   void _OnViewLockInfoCommand();
	   void _OnSychronizeItem();
	   void _OnEditProperties();
	   void _OnDuplicateItemCmd();
	   void _OnCopyItemURLS();
	   void _OnDavItemCreated(void *ioParam);
	   void _OnDavItemChanged(void *ioParam);
	   void _OnDavItemDeleted(void *ioParam);
	   void _OnDavItemLocked(void *ioParam);
	   void _OnDavItemUnlocked(void *ioParam);
	   
	   void _OnRowDoubleClick(void *ioParam);
       void _OnSaveConnection(bool inSaveAs=true);

       void _OnCutCopy(Boolean isCut);
       void _OnPaste();
       
       void _OnSetExecutable(Boolean exec);
       void _OnEditItems(CommandT inEditCmd);
       
       void _OnEditConnectionSettings();
       
	   void _BeginHTTPTransaction();
	   void _EndHTTPTransaction();
	   
	   void _updateHeaderText();
	   
	   void SetWindowName(std::string& inHost, SInt32 inPort, std::string& inPath, bool inForceSecure);

	private:
		int				         mSortIcon;
		CDAVTable*		         mFLVTable;
		LView*                   mFLVTableHeader;
        class LChasingArrows*    mArrows;
        std::string                  mResource;
        class LStaticText*       mHeaderText;
        class LIconControl       *mLockIcon;
        
   protected:
		class CDAVContext*       mContext;
		UInt32                   mThreadCnt;
		CDAVInfo                 mDavInfo;
		Boolean                  mOpeningPropsDlog;
		
		CDAVPropertyVector       mRequiredProperties;
		bool					 mEncodeResources;
		
   private:
        LMutexSemaphore          mAccess; // access 
        LApplication*            mApp;
		LFile*					 mFile;
		bool					 mChanged;
   friend class CDAVTableItem;
   friend class CDAVTable;

};


#endif
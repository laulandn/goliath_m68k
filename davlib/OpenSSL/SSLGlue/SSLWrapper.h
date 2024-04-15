/*
	SSLWrapper.cpp
	Copyright (C) 2000 i-drive.com
	Copyright (C) 2001-2002 Thomas Bednarz Jr. <tombednarz@hotmail.com>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
	The purpose of the wrapper class is to avoid compiling any of
	the OpenSSL code in our main app. OpenSSL uses both <...> and "..."
	references to local include files; this may cause difficulties with other code.
*/

#pragma once

#include <map>
#include <string>
#include <Files.h>

class CsslManager
{
public:
	CsslManager (bool FullStrings = false);
	~CsslManager ();
	static bool Initialized (void) {return mInitialized;}

private:
	static bool mInitialized;
};

void SetCertDBPath(const char* inCertDBPath);

typedef bool (*SSLCertificateCallback)(	void *inUserData,
										const std::map<std::string, std::string>& inOwnerInfo,
										const std::map<std::string, std::string>& inIssuerInfo,
										const std::string& inValidFromDate,
										const std::string& inValidToDate,
										const std::string& inFingerprint,
										bool& outAcceptOnce);

typedef int (*SSLClientCertCallback)(void* userData,
										std::string& outTheCertData,
										std::string& outThePrivKey);



class Cssl
{
public:
	Cssl (void *socket);
	~Cssl ();
	
	void AddCertificate ( const char *cert, const int certLen );
	int Connect (void);
	int Receive (void *buf, int size, int *err);
	int Send (void *buf, int size, int *err);
	    
    static void InstallCertCallback(SSLCertificateCallback, void* inUserData);

	static SSLCertificateCallback GetCertCallback() {return mCertCallback;}
	static void* GetCertCallbackData() {return mCertCallbackUserData;};
		
	static void InstallClientCertCallback(SSLClientCertCallback inCallBack, void* inUserData);
	static SSLClientCertCallback GetClientCertificateCallback() {return mClientCertCallback;};
	static void* GetClientCertCallbackData() {return mClientCertUserData;};
	
	enum CertParseState {
		eOK=0,
		eBadBata,
		eBadPass,
		eUnknown
	};
	
	static CertParseState ParseClientCert(void* data, int len, const std::string& inPassword, std::string& outIssuedTo, std::string& outIssuedBy, std::string& outExpirationDate, std::wstring& outFriendlyName, std::string& outLocalKeyID);
private:
	void Clear (void);

	void *ssl_ctx;
	void *ssl;

	void *cert_store;
	static SSLCertificateCallback	mCertCallback;
	static void*					mCertCallbackUserData;
	static SSLClientCertCallback	mClientCertCallback;	
	static void*					mClientCertUserData;
};

/* ===========================================================================
 *	SSLClientCertCache.h		   
 *
 *  This file is part of the DAVLib package
 *  Copyright (C) 1999-2002  Thomas Bednarzär
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
 */

#include "SSLWrapper.h"

#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/pkcs12.h"

#include "SSLCertificates.h"
#ifndef __SSLClientCertCache_h__
#include "SSLClientCertCache.h"
#endif
#include "PKCSUtils.h"
#include "SSLCertificates.h"
#include "LException.h"
#include <vector>


//---------------------------------------------------

bool CsslManager::mInitialized = false;
SSLCertificateCallback Cssl::mCertCallback = NULL;
void*	Cssl::mCertCallbackUserData = NULL;
std::map<std::string, std::string> sClientCertMap;
SSLClientCertCallback	Cssl::mClientCertCallback;	
void*					Cssl::mClientCertUserData;

SSLClientCertCache 		sClientCertCache;



CsslManager::CsslManager (bool FullStrings)
{
	if (mInitialized)
		return;
	if (FullStrings)
		SSL_load_error_strings ();
	OpenSSL_add_ssl_algorithms ();
	OpenSSL_add_all_algorithms();
	mInitialized = true;
}

CsslManager::~CsslManager ()
{
	if (!mInitialized)
		return;
	EVP_cleanup();
	ERR_free_strings();
	mInitialized = false;
}


Cssl::CertParseState Cssl::ParseClientCert(void* data, int len, const std::string& inPassword, std::string& outIssuedTo, std::string& outIssuedBy, std::string& outExpirationDate, std::wstring& outFriendlyName, std::string& outLocalKeyID)
{
   BIO *memBio = BIO_new_mem_buf(data, len);

   unsigned long errCode = 0;
   PKCS12 *p12 = d2i_PKCS12_bio(memBio, NULL);
   if (!p12) {
      unsigned long e = ERR_get_error_line_data(NULL, NULL, NULL, NULL);
      int l,r;

      l=ERR_GET_LIB(e);
      r=ERR_GET_REASON(e);
      if (ERR_LIB_ASN1 == l && ERR_R_EXPECTING_AN_ASN1_SEQUENCE == r) {
         return eBadBata;
      }
      return eUnknown;
   }
   
   X509* x509;
   EVP_PKEY *pkey;
   int ret = PKCS12_parse(p12, inPassword.c_str(), &pkey, &x509, NULL);
   if (1 != ret) {
      unsigned long e = ERR_get_error_line_data(NULL, NULL, NULL, NULL);
      int l,r;

      l=ERR_GET_LIB(e);
      r=ERR_GET_REASON(e);
      if (ERR_LIB_PKCS12 == l && PKCS12_R_MAC_VERIFY_ERROR == r) {
         return eBadPass;
      }
      return eUnknown;
   }
 
 	char buf[256];
 	X509_NAME_oneline(X509_get_subject_name(x509), buf, 256);
 	outIssuedTo = x509_get_part (buf, "/CN=");
 	

 	X509_NAME_oneline(X509_get_issuer_name(x509), buf, 256);
 	outIssuedBy = x509_get_part (buf, "/CN=");

  	outExpirationDate = asn1time_to_string (X509_get_notAfter (x509));

	GetFriendlyName(p12, inPassword.c_str(), (int)inPassword.size(), outFriendlyName);
	GetLocalKeyID(p12, inPassword.c_str(), (int)inPassword.size(), outLocalKeyID);

	PKCS12_free(p12);
	return eOK;
}

static int Cssl_ClientCertCB(SSL *ssl, X509 **x509, EVP_PKEY **pkey) {
	SSLClientCertCallback cb = Cssl::GetClientCertificateCallback();
	
	if (!cb) {
		return -1;
	}

    X509* peerCert = SSL_get_peer_certificate(ssl);
    if (!peerCert) {
       return -1;
    }
    
    X509_NAME *subject = X509_get_subject_name(peerCert);
    
	if (sClientCertCache.HaveCert(subject)) {
		sClientCertCache.OutputCert(subject, x509, pkey);
		return 1;
	}
	
	std::string outTheCertData, outThePassword;
	if (!(*cb)(Cssl::GetClientCertCallbackData(), outTheCertData, outThePassword)) {
		return -1;
	}
	
	BIO *memBio = BIO_new_mem_buf(&outTheCertData[0], (int)outTheCertData.size());

	PKCS12 *p12 = d2i_PKCS12_bio(memBio, NULL);
	if (!p12)
		return 0;
		
	int ret = PKCS12_parse(p12, outThePassword.c_str(), pkey, x509, NULL);
	if (1 == ret)
		sClientCertCache.AddCert(subject, p12, outThePassword.c_str());
	return (ret == 1);
}

//---------------------------------------------------

Cssl::Cssl (void* socket) : ssl (0), ssl_ctx (0)
{	
	if (!CsslManager::Initialized())
		goto err;

	ssl_ctx = SSL_CTX_new (/*SSLv3_client_method*/SSLv23_client_method());
	if (!ssl_ctx)
		goto err;

    SSL *tmpSSL = SSL_new ((SSL_CTX*)ssl_ctx);

	ssl = tmpSSL;
	
	if (!ssl)
		goto err;
    
	if (!SSL_set_fd ((SSL*)ssl, (long)socket))
		goto err;
	
	cert_store =  X509_STORE_new ();
	if (GetClientCertificateCallback()) {
		//WTF is this?  OpenSSL 0.95 defines SSL_CTX->client_cert_cb to be int(*)() although 
		// it's really int(*)(SSL *ssl, X509 **x509, EVP_PKEY **pkey).  A little hack here
		//  so we can compile properly.
		typedef int(*hackFxnType)();
		hackFxnType hack = (hackFxnType)Cssl_ClientCertCB;
		SSL_CTX_set_client_cert_cb((SSL_CTX*)ssl_ctx, hack);	
	}
	return;
	
err:
	Clear ();
}

Cssl::~Cssl ()
{
    X509_STORE_free (reinterpret_cast<X509_STORE*>(cert_store));
	Clear();
}

void Cssl::AddCertificate (const char *cert, const int certLen)
{
	X509	*x509;
	int		ret;
	
	if (!ssl)
		return;

	
	BIO *bio = BIO_new (BIO_s_mem());
	if (bio == NULL)
		return;
		
	ret = BIO_write (bio, cert, certLen);					if (ret < 0) goto end;
	x509 = PEM_read_bio_X509_AUX (bio,NULL,NULL,NULL);		if (x509 == NULL) goto end;
	X509_STORE_add_cert (reinterpret_cast<X509_STORE*>(cert_store)/*((SSL*)ssl)->ctx->cert_store*/, x509);

end:	
	BIO_free (bio);
}

void Cssl::Clear (void)
{
	if (ssl)
	{
		SSL_free ((SSL*)ssl);
		ssl = NULL;
	}
	
	if (ssl_ctx)
	{
		SSL_CTX_free ((SSL_CTX*)ssl_ctx);
		ssl_ctx = NULL;
	}
}


int Cssl::Connect (void)
{
	int ret;
	
	if (!ssl)
		return -1;
		
	ret = SSL_connect ((SSL*)ssl);
	if (ret < 0)
		ret = SSL_get_error ((SSL*)ssl, ret);
	else
		ret = 0;

	if (ret == 0) {
		X509 *cert = SSL_get_peer_certificate ((SSL*)ssl);
		if (!ssl_check_certificate(cert, reinterpret_cast<X509_STORE*>(cert_store)))
			ret = -1;
	}
	
	return ret;
}

int Cssl::Receive (void *buf, int size, int *err)
{
	int ret;
	
	if (!ssl)
		return -1;
		
	ret = SSL_read ((SSL*)ssl, (char*)buf, size);
	if (ret < 0)
	{
		*err = SSL_get_error ((SSL*)ssl, ret);
		if (*err == SSL_ERROR_WANT_READ || *err == SSL_ERROR_WANT_WRITE)
		{
			*err = 0;
			ret = 0;
		}
	}
	else if (ret == 0 && !SSL_pending((SSL*)ssl))
	{
		ret = -1;
		*err = 0;
	}
	return ret;
}

int Cssl::Send (void *buf, int size, int *err)
{
	int ret;
	
	if (!ssl)
		return -1;
	ret = SSL_write ((SSL*)ssl, (char*)buf, size);
	if (ret < 0)
	{
		*err = SSL_get_error ((SSL*)ssl, ret);
		if (*err == SSL_ERROR_WANT_READ || *err == SSL_ERROR_WANT_WRITE)
		{
			*err = 0;
			ret = 0;
		}
	}
	return ret;
}

void Cssl::InstallCertCallback(SSLCertificateCallback inCallBack, void* inUserData) {
	mCertCallback = inCallBack;
	mCertCallbackUserData = inUserData;
}


void Cssl::InstallClientCertCallback(SSLClientCertCallback inCallBack, void* inUserData) {
	mClientCertCallback = inCallBack;
	mClientCertUserData = inUserData;
}

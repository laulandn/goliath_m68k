/* crypto/engine/hw_aep.c */
/*
 */
/* ====================================================================
 * Copyright (c) 1999 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    licensing@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

#include <stdio.h>
#include <openssl/bn.h>
#include <string.h>

#include <openssl/e_os.h>
#ifndef MSDOS
#include <unistd.h>
#else
#include <process.h>
typedef int pid_t;
#endif

#include <openssl/crypto.h>
#include "cryptlib.h"
#include <openssl/dso.h>
#include "engine_int.h"
#include <openssl/engine.h>

#ifndef NO_HW
#ifndef NO_HW_AEP
#ifdef FLAT_INC
#include "aep.h"
#else
#include "vendor_defns/aep.h"
#endif

static int aep_init(void);
static int aep_finish(void);

static AEP_RV aep_get_connection(AEP_CONNECTION_HNDL_PTR hConnection);
static AEP_RV aep_return_connection(AEP_CONNECTION_HNDL hConnection);
static AEP_RV aep_close_connection(AEP_CONNECTION_HNDL hConnection);
static AEP_RV aep_close_all_connections(int use_engine_lock, int *in_use);

/* BIGNUM stuff */
static int aep_mod_exp(BIGNUM *r, BIGNUM *a, const BIGNUM *p,
		       const BIGNUM *m, BN_CTX *ctx);
static AEP_RV aep_mod_exp_crt(BIGNUM *r, BIGNUM *a,
			      const BIGNUM *p, const BIGNUM *q,
			      const BIGNUM *dmp1, const BIGNUM *dmq1,
			      const BIGNUM *iqmp, BN_CTX *ctx);

/* RSA stuff */
#ifndef NO_RSA
static int aep_rsa_mod_exp(BIGNUM *r0, BIGNUM *I, RSA *rsa);
#endif

/* This function is aliased to mod_exp (with the mont stuff dropped). */
static int aep_mod_exp_mont(BIGNUM *r, BIGNUM *a, const BIGNUM *p,
			    const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *m_ctx);

/* DSA stuff */
#ifndef NO_DSA
static int aep_dsa_mod_exp(DSA *dsa, BIGNUM *rr, BIGNUM *a1,
			   BIGNUM *p1, BIGNUM *a2, BIGNUM *p2, BIGNUM *m,
			   BN_CTX *ctx, BN_MONT_CTX *in_mont);

static int aep_mod_exp_dsa(DSA *dsa, BIGNUM *r, BIGNUM *a,
			   const BIGNUM *p, const BIGNUM *m, BN_CTX *ctx,
			   BN_MONT_CTX *m_ctx);
#endif

/* DH stuff */
#ifndef NO_DH
/* This function is aliased to mod_exp (with the DH and mont dropped). */
static int aep_mod_exp_dh(DH *dh, BIGNUM *r, BIGNUM *a, const BIGNUM *p,
			  const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *m_ctx);
#endif
	
#ifdef AEPRAND
/* rand stuff   */
static int aep_rand(unsigned char *buf, int num);
static int aep_rand_status(void);
#endif

/* Bignum conversion stuff */
static AEP_RV GetBigNumSize(AEP_VOID_PTR ArbBigNum, AEP_U32* BigNumSize);
static AEP_RV MakeAEPBigNum(AEP_VOID_PTR ArbBigNum, AEP_U32 BigNumSize,
			    unsigned char* AEP_BigNum);
static AEP_RV ConvertAEPBigNum(void* ArbBigNum, AEP_U32 BigNumSize,
			       unsigned char* AEP_BigNum);


#ifndef NO_RSA
/* Our internal RSA_METHOD that we provide pointers to */
static RSA_METHOD aep_rsa =
{
  "Aep RSA method",
  NULL,                /*rsa_pub_encrypt*/
  NULL,                /*rsa_pub_decrypt*/
  NULL,                /*rsa_priv_encrypt*/
  NULL,                /*rsa_priv_encrypt*/
  aep_rsa_mod_exp,     /*rsa_mod_exp*/
  aep_mod_exp_mont,    /*bn_mod_exp*/
  NULL,                /*init*/
  NULL,                /*finish*/
  0,                   /*flags*/
  NULL,                /*app_data*/
  NULL,                /*rsa_sign*/
  NULL                 /*rsa_verify*/
};
#endif

#ifndef NO_DSA
/* Our internal DSA_METHOD that we provide pointers to */
static DSA_METHOD aep_dsa =
{
  "Aep DSA method",
  NULL,                /* dsa_do_sign */
  NULL,                /* dsa_sign_setup */
  NULL,                /* dsa_do_verify */
  aep_dsa_mod_exp,     /* dsa_mod_exp */
  aep_mod_exp_dsa,     /* bn_mod_exp */
  NULL,                /* init */
  NULL,                /* finish */
  0,                   /* flags */
  NULL                 /* app_data */
};
#endif

#ifndef NO_DH
/* Our internal DH_METHOD that we provide pointers to */
static DH_METHOD aep_dh =
{
  "Aep DH method",
  NULL,
  NULL,
  aep_mod_exp_dh,
  NULL,
  NULL,
  0,
  NULL
};
#endif

#ifdef AEPRAND
/* our internal RAND_method that we provide pointers to  */
static RAND_METHOD aep_random =
{
  /*"AEP RAND method", */
  NULL,
  aep_rand,
  NULL,
  NULL,
  aep_rand,
  aep_rand_status,
};
#endif

/* Our ENGINE structure. */
static ENGINE engine_aep =
{
  "aep",
  "Aep hardware engine support",
  &aep_rsa,
  &aep_dsa,
  &aep_dh,
#ifdef AEPRAND
  &aep_random,
#else
  NULL,
#endif
  aep_mod_exp,
  NULL,
  aep_init,
  aep_finish,
  NULL, /* no ctrl() */
  NULL, /* no load_privkey() */
  NULL, /* no load_pubkey() */
  0, /* no flags */
  0, 0, /* no references */
  NULL, NULL /* unlinked */
};

/*Define an array of structures to hold connections*/
static AEP_CONNECTION_ENTRY aep_app_conn_table[MAX_PROCESS_CONNECTIONS];

/*Used to determine if this is a new process*/
static int    recorded_pid = 0;

#ifdef AEPRAND
static AEP_U8   rand_block[RAND_BLK_SIZE];
static AEP_U32  rand_block_bytes = 0;
#endif

static int max_key_len = 2176;


/* As this is only ever called once, there's no need for locking
 * (indeed - the lock will already be held by our caller!!!) */
ENGINE *ENGINE_aep()
{
#ifndef NO_RSA
  RSA_METHOD  *meth1;
#endif
#ifndef NO_DSA
  DSA_METHOD  *meth2;
#endif
#ifndef NO_DH
  DH_METHOD   *meth3;
#endif

  /* We know that the "PKCS1_SSLeay()" functions hook properly
   * to the aep-specific mod_exp and mod_exp_crt so we use
   * those functions. NB: We don't use ENGINE_openssl() or
   * anything "more generic" because something like the RSAref
   * code may not hook properly, and if you own one of these
   * cards then you have the right to do RSA operations on it
   * anyway! */
#ifndef NO_RSA
  meth1 = RSA_PKCS1_SSLeay();
  aep_rsa.rsa_pub_enc = meth1->rsa_pub_enc;
  aep_rsa.rsa_pub_dec = meth1->rsa_pub_dec;
  aep_rsa.rsa_priv_enc = meth1->rsa_priv_enc;
  aep_rsa.rsa_priv_dec = meth1->rsa_priv_dec;
#endif


  /* Use the DSA_OpenSSL() method and just hook the mod_exp-ish
   * bits. */
#ifndef NO_DSA
  meth2 = DSA_OpenSSL();
  aep_dsa.dsa_do_sign    = meth2->dsa_do_sign;
  aep_dsa.dsa_sign_setup = meth2->dsa_sign_setup;
  aep_dsa.dsa_do_verify  = meth2->dsa_do_verify;

  aep_dsa = *DSA_get_default_openssl_method(); 
  aep_dsa.dsa_mod_exp = aep_dsa_mod_exp; 
  aep_dsa.bn_mod_exp = aep_mod_exp_dsa;      
#endif

  /* Much the same for Diffie-Hellman */
#ifndef NO_DH
  meth3 = DH_OpenSSL();
  aep_dh.generate_key = meth3->generate_key;
  aep_dh.compute_key  = meth3->compute_key;
  aep_dh.bn_mod_exp   = meth3->bn_mod_exp;
#endif

  return &engine_aep;
}

/* This is a process-global DSO handle used for loading and unloading
 * the Aep library. NB: This is only set (or unset) during an
 * init() or finish() call (reference counts permitting) and they're
 * operating with global locks, so this should be thread-safe
 * implicitly. */
static DSO *aep_dso = NULL;

/* These are the static string constants for the DSO file name and the function
 * symbol names to bind to. 
*/
static const char *AEP_LIBNAME = "aep";

static const char *AEP_F1    = "AEP_ModExp";
static const char *AEP_F2    = "AEP_ModExpCrt";
#ifdef AEPRAND
static const char *AEP_F3    = "AEP_GenRandom";
#endif
static const char *AEP_F4    = "AEP_Finalize";
static const char *AEP_F5    = "AEP_Initialize";
static const char *AEP_F6    = "AEP_OpenConnection";
static const char *AEP_F7    = "AEP_SetBNCallBacks";
static const char *AEP_F8    = "AEP_CloseConnection";

/* These are the function pointers that are (un)set when the library has
 * successfully (un)loaded. */
static t_AEP_OpenConnection    *p_AEP_OpenConnection  = NULL;
static t_AEP_CloseConnection   *p_AEP_CloseConnection = NULL;
static t_AEP_ModExp            *p_AEP_ModExp          = NULL;
static t_AEP_ModExpCrt         *p_AEP_ModExpCrt       = NULL;
#ifdef AEPRAND
static t_AEP_GenRandom         *p_AEP_GenRandom       = NULL;
#endif
static t_AEP_Initialize        *p_AEP_Initialize      = NULL;
static t_AEP_Finalize          *p_AEP_Finalize        = NULL;
static t_AEP_SetBNCallBacks    *p_AEP_SetBNCallBacks  = NULL;

/* (de)initialisation functions. */
static int aep_init(void)
{
  t_AEP_ModExp          *p1;
  t_AEP_ModExpCrt       *p2;
#ifdef AEPRAND
  t_AEP_GenRandom       *p3;
#endif
  t_AEP_Finalize        *p4;
  t_AEP_Initialize      *p5;
  t_AEP_OpenConnection  *p6;
  t_AEP_SetBNCallBacks  *p7;
  t_AEP_CloseConnection *p8;

  int to_return = 0;
 
 
  if(aep_dso != NULL)
    {
      ENGINEerr(ENGINE_F_AEP_INIT,ENGINE_R_ALREADY_LOADED);
      goto err;
    }
  /* Attempt to load libaep.so. */

  aep_dso = DSO_load(NULL, AEP_LIBNAME, NULL,
		     DSO_FLAG_NAME_TRANSLATION);
  
  if(aep_dso == NULL)
    {
      ENGINEerr(ENGINE_F_AEP_INIT,ENGINE_R_DSO_FAILURE);
      goto err;
    }

  if(!(p1 = (t_AEP_ModExp *)         DSO_bind_func( aep_dso,AEP_F1))  ||
     !(p2 = (t_AEP_ModExpCrt*)       DSO_bind_func( aep_dso,AEP_F2))  ||
#ifdef AEPRAND
     !(p3 = (t_AEP_GenRandom*)       DSO_bind_func( aep_dso,AEP_F3))  ||
#endif
     !(p4 = (t_AEP_Finalize*)        DSO_bind_func( aep_dso,AEP_F4))  ||
     !(p5 = (t_AEP_Initialize*)      DSO_bind_func( aep_dso,AEP_F5))  ||
     !(p6 = (t_AEP_OpenConnection*)  DSO_bind_func( aep_dso,AEP_F6))  ||
     !(p7 = (t_AEP_SetBNCallBacks*)  DSO_bind_func( aep_dso,AEP_F7))  ||
     !(p8 = (t_AEP_CloseConnection*) DSO_bind_func( aep_dso,AEP_F8)))
    {
     
      ENGINEerr(ENGINE_F_AEP_INIT,ENGINE_R_DSO_FAILURE);
      goto err;
    }

  /* Copy the pointers */
  
  p_AEP_ModExp           = p1;
  p_AEP_ModExpCrt        = p2;
#ifdef AEPRAND
  p_AEP_GenRandom        = p3;
#endif
  p_AEP_Finalize         = p4;
  p_AEP_Initialize       = p5;
  p_AEP_OpenConnection   = p6;
  p_AEP_SetBNCallBacks   = p7;
  p_AEP_CloseConnection  = p8;
 
  to_return = 1;
 
  return to_return;

 err: 

  if(aep_dso)
    DSO_free(aep_dso);
		
  p_AEP_OpenConnection    = NULL;
  p_AEP_ModExp            = NULL;
  p_AEP_ModExpCrt         = NULL;
#ifdef AEPRAND
  p_AEP_GenRandom         = NULL;
#endif
  p_AEP_Initialize        = NULL;
  p_AEP_Finalize          = NULL;
  p_AEP_SetBNCallBacks    = NULL;
  p_AEP_CloseConnection   = NULL;

  return to_return;
 
}

static int aep_finish(void)
{
  int to_return = 0, in_use;
  AEP_RV rv;

  if(aep_dso == NULL)
    {
      ENGINEerr(ENGINE_F_AEP_FINISH,ENGINE_R_NOT_LOADED);
      goto err;
    }

  rv = aep_close_all_connections(0, &in_use);
  if (rv != AEP_R_OK)
    {
      ENGINEerr(ENGINE_F_AEP_FINISH,ENGINE_R_CLOSE_HANDLES_FAILED);
      goto err;
    }
  if (in_use)
    {
      ENGINEerr(ENGINE_F_AEP_FINISH,ENGINE_R_CONNECTIONS_IN_USE);
      goto err;
    }

  rv = p_AEP_Finalize();
  if (rv != AEP_R_OK)
    {
      ENGINEerr(ENGINE_F_AEP_FINISH,ENGINE_R_FINALIZE_FAILED);
      goto err;
    }


  if(!DSO_free(aep_dso))
    {
      ENGINEerr(ENGINE_F_AEP_FINISH,ENGINE_R_DSO_FAILURE);
      goto err;
    }

  aep_dso = NULL;
  p_AEP_CloseConnection   = NULL;
  p_AEP_OpenConnection    = NULL;
  p_AEP_ModExp            = NULL;
  p_AEP_ModExpCrt         = NULL;
#ifdef AEPRAND
  p_AEP_GenRandom         = NULL;
#endif
  p_AEP_Initialize        = NULL;
  p_AEP_Finalize          = NULL;
  p_AEP_SetBNCallBacks    = NULL;

  to_return = 1;
 err:
  return to_return;
}

static int aep_mod_exp(BIGNUM *r, BIGNUM *a, const BIGNUM *p,
		       const BIGNUM *m, BN_CTX *ctx)
{
  int to_return = 0;
  int r_len = 0;
  AEP_CONNECTION_HNDL hConnection;
  AEP_RV rv;

  r_len = BN_num_bits(m);

  /* Perform in software if modulus is too large for hardware. */

  if (r_len > max_key_len)
    {
      ENGINE *e;
      e = ENGINE_openssl();
      to_return = e->bn_mod_exp(r, a, p, m, ctx);
      goto err;
    }

  /*Grab a connection from the pool*/
  rv = aep_get_connection(&hConnection);
  if (rv != AEP_R_OK)
    {
      ENGINE *e;
      ENGINEerr(ENGINE_F_AEP_MOD_EXP,ENGINE_R_GET_HANDLE_FAILED);

      e = ENGINE_openssl();
      to_return = e->bn_mod_exp(r, a, p, m, ctx);
      goto err;
    }

  /*To the card with the mod exp*/
  rv = p_AEP_ModExp(hConnection,(void*)a, (void*)p,(void*)m, (void*)r,NULL);
                            
  if (rv !=  AEP_R_OK)
    {
      ENGINE *e;

      aep_close_connection(hConnection);

      ENGINEerr(ENGINE_F_AEP_MOD_EXP,ENGINE_R_MOD_EXP_FAILED);

      e = ENGINE_openssl();
      to_return = e->bn_mod_exp(r, a, p, m, ctx);

      goto err;
    }

  /*Return the connection to the pool*/
  rv = aep_return_connection(hConnection);
  if (rv != AEP_R_OK)
  {
    ENGINEerr(ENGINE_F_AEP_RAND,ENGINE_R_RETURN_CONNECTION_FAILED); 
    goto err;
  }

  to_return = 1;
 err:
  return to_return;
}

static AEP_RV aep_mod_exp_crt(BIGNUM *r, BIGNUM *a,
			      const BIGNUM *p, const BIGNUM *q ,
			      const BIGNUM *dmp1,const BIGNUM *dmq1,
			      const BIGNUM *iqmp, BN_CTX *ctx)
{
  AEP_RV rv = AEP_R_OK;
  AEP_CONNECTION_HNDL hConnection;

  /* Perform in software if modulus is too large for hardware. */
 
  if (BN_num_bits(p) > max_key_len || BN_num_bits(q) > max_key_len){
    ENGINE *e;
    e = ENGINE_openssl();
    rv = e->bn_mod_exp_crt(r, a, p, q, dmp1, dmq1, iqmp, ctx);
    goto err;
  }

  /*Grab a connection from the pool*/
  rv = aep_get_connection(&hConnection);
  if (rv != AEP_R_OK)
    {
      ENGINE *e;
 
      ENGINEerr(ENGINE_F_AEP_MOD_EXP_CRT,ENGINE_R_GET_HANDLE_FAILED);
 	  
      e = ENGINE_openssl();
 
      if (e->bn_mod_exp_crt(r, a, p, q, dmp1, dmq1, iqmp, ctx) > 0)
	rv = AEP_R_OK;
      else
	rv = AEP_R_GENERAL_ERROR;
  
      goto err;
    }

  /*To the card with the mod exp*/
  rv = p_AEP_ModExpCrt(hConnection,(void*)a, (void*)p, (void*)q, (void*)dmp1,(void*)dmq1,
		       (void*)iqmp,(void*)r,NULL);
  if (rv != AEP_R_OK)
    {
      ENGINE *e;
 
      aep_close_connection(hConnection);
 	  
      ENGINEerr(ENGINE_F_AEP_MOD_EXP_CRT,ENGINE_R_MOD_EXP_CRT_FAILED);
 
      e = ENGINE_openssl();
 
      if (e->bn_mod_exp_crt(r, a, p, q, dmp1, dmq1, iqmp, ctx) > 0)
	rv = AEP_R_OK;
      else
	rv = AEP_R_GENERAL_ERROR;
 
      goto err;
    }

  /*Return the connection to the pool*/
  rv = aep_return_connection(hConnection);
  if (rv != AEP_R_OK)
  {
      ENGINEerr(ENGINE_F_AEP_RAND,ENGINE_R_RETURN_CONNECTION_FAILED); 
 	goto err;
  }
 
 err:
  return rv;
}

#ifdef AEPRAND
static int aep_rand(unsigned char *buf,int len )
{
  AEP_RV rv = AEP_R_OK;
  AEP_CONNECTION_HNDL hConnection;

  CRYPTO_w_lock(CRYPTO_LOCK_RAND);

  /*Can the request be serviced with what's already in the buffer?*/
  if (len <= rand_block_bytes)
    {
      memcpy(buf, &rand_block[RAND_BLK_SIZE - rand_block_bytes], len);
      rand_block_bytes -= len;
      CRYPTO_w_unlock(CRYPTO_LOCK_RAND);
    }
  else
  /*If not the get another block of random bytes*/
    {
      CRYPTO_w_unlock(CRYPTO_LOCK_RAND);

      rv = aep_get_connection(&hConnection);
      if (rv !=  AEP_R_OK)
	  { 
          ENGINEerr(ENGINE_F_AEP_RAND,ENGINE_R_GET_HANDLE_FAILED);             
	  goto err_nounlock;
	  }

      if (len > RAND_BLK_SIZE)
	  {
	    rv = p_AEP_GenRandom(hConnection, len, 2, buf, NULL);
	    if (rv !=  AEP_R_OK)
	    {  
              ENGINEerr(ENGINE_F_AEP_RAND,ENGINE_R_GET_RANDOM_FAILED); 
	      goto err_nounlock;
	    }
	  }
      else
	  {
	    CRYPTO_w_lock(CRYPTO_LOCK_RAND);

	    rv = p_AEP_GenRandom(hConnection, RAND_BLK_SIZE, 2, &rand_block[0], NULL);
	    if (rv !=  AEP_R_OK)
	    {       
              ENGINEerr(ENGINE_F_AEP_RAND,ENGINE_R_GET_RANDOM_FAILED); 
	      
	      goto err;
	    }

	    rand_block_bytes = RAND_BLK_SIZE;
	    memcpy(buf, &rand_block[RAND_BLK_SIZE - rand_block_bytes], len);
	    rand_block_bytes -= len;

	    CRYPTO_w_unlock(CRYPTO_LOCK_RAND);
	  }

      rv = aep_return_connection(hConnection);
      if (rv != AEP_R_OK)
	  {
          ENGINEerr(ENGINE_F_AEP_RAND,ENGINE_R_RETURN_CONNECTION_FAILED); 
	  
	  goto err_nounlock;
	  }
    }
  
  return 1;
 err:
  CRYPTO_w_unlock(CRYPTO_LOCK_RAND);
 err_nounlock:
  return 0;
}
	
static int aep_rand_status(void)
{
	return 1;
}
#endif

#ifndef NO_RSA
static int aep_rsa_mod_exp(BIGNUM *r0, BIGNUM *I, RSA *rsa)
{
  BN_CTX *ctx = NULL;
  int to_return = 0;
  AEP_RV rv = AEP_R_OK;

  if ((ctx = BN_CTX_new()) == NULL)
    goto err;

  if (!aep_dso)
    {
      ENGINEerr(ENGINE_F_AEP_RSA_MOD_EXP,ENGINE_R_NOT_LOADED);
      goto err;
    }

  /*See if we have all the necessary bits for a crt*/
  if (rsa->q && rsa->dmp1 && rsa->dmq1 && rsa->iqmp)
    {
      rv =  aep_mod_exp_crt(r0,I,rsa->p,rsa->q, rsa->dmp1,rsa->dmq1,rsa->iqmp,ctx);
      if (rv != AEP_R_OK)
	goto err;
    }
  else
    {
       if (!rsa->d || !rsa->n)
	{
	  ENGINEerr(ENGINE_F_AEP_RSA_MOD_EXP,ENGINE_R_MISSING_KEY_COMPONENTS);
	  goto err;
	}
 
       rv = aep_mod_exp(r0,I,rsa->d,rsa->n,ctx);
       if  (rv != AEP_R_OK)
             goto err;
	
    }

   to_return = 1;

 err:
  if(ctx)
    BN_CTX_free(ctx);
  return to_return;
}
#endif
		
#ifndef NO_DSA
static int aep_dsa_mod_exp(DSA *dsa, BIGNUM *rr, BIGNUM *a1,
			   BIGNUM *p1, BIGNUM *a2, BIGNUM *p2, BIGNUM *m,
			   BN_CTX *ctx, BN_MONT_CTX *in_mont)
{
  BIGNUM t;
  int to_return = 0;
  BN_init(&t);

  /* let rr = a1 ^ p1 mod m */
  if (!aep_mod_exp(rr,a1,p1,m,ctx)) goto end;
  /* let t = a2 ^ p2 mod m */
  if (!aep_mod_exp(&t,a2,p2,m,ctx)) goto end;
  /* let rr = rr * t mod m */
  if (!BN_mod_mul(rr,rr,&t,m,ctx)) goto end;
  to_return = 1;
 end: 
  BN_free(&t);
  return to_return;
}


static int aep_mod_exp_dsa(DSA *dsa, BIGNUM *r, BIGNUM *a,
			   const BIGNUM *p, const BIGNUM *m, BN_CTX *ctx,
			   BN_MONT_CTX *m_ctx)
{  
  return aep_mod_exp(r, a, p, m, ctx); 
 
}
#endif

/* This function is aliased to mod_exp (with the mont stuff dropped). */
static int aep_mod_exp_mont(BIGNUM *r, BIGNUM *a, const BIGNUM *p,
			    const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *m_ctx)
{
     return aep_mod_exp(r, a, p, m, ctx);

}

#ifndef NO_DH
/* This function is aliased to mod_exp (with the dh and mont dropped). */
static int aep_mod_exp_dh(DH *dh, BIGNUM *r, BIGNUM *a, const BIGNUM *p,
			  const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *m_ctx)
{
    return aep_mod_exp(r, a, p, m, ctx);
}
#endif

static AEP_RV aep_get_connection(AEP_CONNECTION_HNDL_PTR phConnection)
{
  int count;
  AEP_RV rv = AEP_R_OK;

  /*Get the current process id*/
  int curr_pid;

  CRYPTO_w_lock(CRYPTO_LOCK_ENGINE);

  curr_pid = getpid();

  /*Check if this is the first time this is being called from the current
    process*/
  if (recorded_pid != curr_pid)
    {
      /*Remember our pid so we can check if we're in a new process*/
      recorded_pid = curr_pid;

      /*Call Finalize to make sure we have not inherited some data from a parent
	process*/
      p_AEP_Finalize();
     
      /*Initialise the AEP API*/
      rv = p_AEP_Initialize(NULL);

      if (rv != AEP_R_OK)
	{
	  ENGINEerr(ENGINE_F_AEP_INIT,ENGINE_R_AEP_INIT_FAILURE);
	  recorded_pid = 0;
	  goto end;
	}

      /*Set the AEP big num call back functions*/
      rv = p_AEP_SetBNCallBacks(&GetBigNumSize, &MakeAEPBigNum, &ConvertAEPBigNum);

      if (rv != AEP_R_OK)
	{
	 
	  ENGINEerr(ENGINE_F_AEP_INIT,ENGINE_R_SETBNCALLBACK_FAILURE);
	  recorded_pid = 0;
	  goto end;
	}

#ifdef AEPRAND
      /*Reset the rand byte count*/
      rand_block_bytes = 0;
#endif

      /*Init the structures*/
      for (count = 0;count < MAX_PROCESS_CONNECTIONS;count ++)
	{
	  aep_app_conn_table[count].conn_state = NotConnected;
	  aep_app_conn_table[count].conn_hndl  = 0;
	}

      /*Open a connection*/
      rv = p_AEP_OpenConnection(phConnection);

      if (rv != AEP_R_OK)
	{
          ENGINEerr(ENGINE_F_AEP_INIT,ENGINE_R_UNIT_FAILURE);
	  recorded_pid = 0;
	  goto end;
	}

      aep_app_conn_table[0].conn_state = InUse;
      aep_app_conn_table[0].conn_hndl = *phConnection;
      goto end;
    }
  /*Check the existing connections to see if we can find a free one*/
  for (count = 0;count < MAX_PROCESS_CONNECTIONS;count ++)
    {
      
      if (aep_app_conn_table[count].conn_state == Connected)
	{
	  aep_app_conn_table[count].conn_state = InUse;
	  *phConnection = aep_app_conn_table[count].conn_hndl;
	  goto end;
	}
    }
  /*If no connections available, we're going to have to try to open a new one*/
  for (count = 0;count < MAX_PROCESS_CONNECTIONS;count ++)
    {
        if (aep_app_conn_table[count].conn_state == NotConnected)
	{
	  /*Open a connection*/
	  rv = p_AEP_OpenConnection(phConnection);

	  if (rv != AEP_R_OK)
	    {	      
               ENGINEerr(ENGINE_F_AEP_INIT,ENGINE_R_UNIT_FAILURE);
	       goto end;
	    }

	  aep_app_conn_table[count].conn_state = InUse;

	  aep_app_conn_table[count].conn_hndl = *phConnection;
	  goto end;
	}
    }

  rv = AEP_R_GENERAL_ERROR;
 end:
  CRYPTO_w_unlock(CRYPTO_LOCK_ENGINE);
  return rv;
}


static AEP_RV aep_return_connection(AEP_CONNECTION_HNDL hConnection)
{
  int count;

  CRYPTO_w_lock(CRYPTO_LOCK_ENGINE);

  /*Find the connection item that matches this connection handle*/
  for(count = 0;count < MAX_PROCESS_CONNECTIONS;count ++)
    {
      if (aep_app_conn_table[count].conn_hndl == hConnection)
	{
	  aep_app_conn_table[count].conn_state = Connected;
	  break;
	}
    }

  CRYPTO_w_unlock(CRYPTO_LOCK_ENGINE);

  return AEP_R_OK;
}

static AEP_RV aep_close_connection(AEP_CONNECTION_HNDL hConnection)
{
  int count;
  AEP_RV rv = AEP_R_OK;

  CRYPTO_w_lock(CRYPTO_LOCK_ENGINE);

  /*Find the connection item that matches this connection handle*/
  for(count = 0;count < MAX_PROCESS_CONNECTIONS;count ++)
    {
      if (aep_app_conn_table[count].conn_hndl == hConnection)
	{
	  rv = p_AEP_CloseConnection(aep_app_conn_table[count].conn_hndl);
	  if (rv != AEP_R_OK)
	    goto end;
	  aep_app_conn_table[count].conn_state = NotConnected;
	  aep_app_conn_table[count].conn_hndl  = 0;
	  break;
	}
    }

 end:
  CRYPTO_w_unlock(CRYPTO_LOCK_ENGINE);
  return AEP_R_OK;
}

static AEP_RV aep_close_all_connections(int use_engine_lock, int *in_use)
{
  int count;
  AEP_RV rv = AEP_R_OK;

  *in_use = 0;
  if (use_engine_lock) CRYPTO_w_lock(CRYPTO_LOCK_ENGINE);
  for (count = 0;count < MAX_PROCESS_CONNECTIONS;count ++)
    {
      switch (aep_app_conn_table[count].conn_state)
	{
	case Connected:
	  rv = p_AEP_CloseConnection(aep_app_conn_table[count].conn_hndl);
	  if (rv != AEP_R_OK)
	    goto end;
	  aep_app_conn_table[count].conn_state = NotConnected;
	  aep_app_conn_table[count].conn_hndl  = 0;
	  break;
	case InUse:
	  (*in_use)++;
	  break;
	case NotConnected:
	  break;
	}
    }
 end:
  if (use_engine_lock) CRYPTO_w_unlock(CRYPTO_LOCK_ENGINE);
  return AEP_R_OK;
}

/*BigNum call back functions, used to convert OpenSSL bignums into AEP bignums.
  Note only 32bit Openssl build support*/

static AEP_RV GetBigNumSize(AEP_VOID_PTR ArbBigNum, AEP_U32* BigNumSize)
{
  BIGNUM* bn;

  /*Cast the ArbBigNum pointer to our BIGNUM struct*/
  bn = (BIGNUM*) ArbBigNum;

#ifdef SIXTY_FOUR_BIT_LONG
  *BigNumSize = bn->top << 3;
#else
  /*Size of the bignum in bytes is equal to the bn->top (no of 32 bit
    words) multiplies by 4*/
  *BigNumSize = bn->top << 2;
#endif

  return AEP_R_OK;
}

static AEP_RV MakeAEPBigNum(AEP_VOID_PTR ArbBigNum, AEP_U32 BigNumSize,
			    unsigned char* AEP_BigNum)
{
  BIGNUM* bn;

#ifndef SIXTY_FOUR_BIT_LONG
  unsigned char* buf;
  int i;
#endif

  /*Cast the ArbBigNum pointer to our BIGNUM struct*/
  bn = (BIGNUM*) ArbBigNum;

#ifdef SIXTY_FOUR_BIT_LONG
  	memcpy(AEP_BigNum, bn->d, BigNumSize);
#else
  /*Must copy data into a (monotone) least significant byte first format
	performing endian conversion if necessary*/
  for(i=0;i<bn->top;i++)
    {
      buf = (unsigned char*)&bn->d[i];

      *((AEP_U32*)AEP_BigNum) = (AEP_U32)
	((unsigned) buf[1] << 8 | buf[0]) |
	((unsigned) buf[3] << 8 | buf[2])  << 16;

      AEP_BigNum += 4;
    }
#endif

  return AEP_R_OK;
}

/*Turn an AEP Big Num back to a user big num*/
static AEP_RV ConvertAEPBigNum(void* ArbBigNum, AEP_U32 BigNumSize,
			       unsigned char* AEP_BigNum)
{
  BIGNUM* bn;
#ifndef SIXTY_FOUR_BIT_LONG
  int i;
#endif

  bn = (BIGNUM*)ArbBigNum;

  /*Expand the result bn so that it can hold our big num.  Size is in bits*/
  bn_expand(bn, (int)(BigNumSize << 3));

#ifdef SIXTY_FOUR_BIT_LONG
  bn->top = BigNumSize >> 3;
	
  if((BigNumSize & 7) != 0)
  	bn->top++;

  memset(bn->d, 0, bn->top << 3);	

  memcpy(bn->d, AEP_BigNum, BigNumSize);
#else
  bn->top = BigNumSize >> 2;
 
  for(i=0;i<bn->top;i++)
    {
      bn->d[i] = (AEP_U32)
	((unsigned) AEP_BigNum[3] << 8 | AEP_BigNum[2]) << 16 |
	((unsigned) AEP_BigNum[1] << 8 | AEP_BigNum[0]);
      AEP_BigNum += 4;
    }
#endif

  return AEP_R_OK;
}	
	
#endif /* !NO_HW_AEP */
#endif /* !NO_HW */

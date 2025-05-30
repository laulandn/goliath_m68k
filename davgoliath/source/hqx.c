
/* file hqx.c 
	BinHex decoder/encoder routines, implementation.
	Copyright (c) 1995, 1996, 1997 by John Montbriand.  All Rights Reserved.
	Permission hereby granted for public use.
    	Distribute freely in areas where the laws of copyright apply.
	USE AT YOUR OWN RISK.
	DO NOT DISTRIBUTE MODIFIED COPIES.
	Comments/questions/postcards* to the author at the address:
	  John Montbriand
	  P.O. Box. 1133
	  Saskatoon Saskatchewan Canada
	  S7K 3N2
	or by email at:
	  tinyjohn@sk.sympatico.ca
	*if you mail a postcard, then I will provide you with technical support
	regarding questions you may have about this file.
*/

#include "hqx.h"
#include <string.h>
#include <Errors.h>
#include <TextUtils.h>
#include <PLStringFuncs.h>
#include <Finder.h>

	/* constants */
#define kBinHexMagic 0x1021
#define kRLEFlag 0x90
#define kCopyBufLen 4096
#define HT ((char) 9) /* horizontal tab */
#define LF ((char) 10) /* line feed, newline */
#define CR ((char) 13) /* carriage return */

	/* types */
typedef struct {
	unsigned char fInBuffer[4*1024];
	unsigned char fOutBuffer[4*1024];
	unsigned char *fOutBufp;
	long fBitBuffer, fBitBufferBits;
	long fNOutChars;
	long fRLECount;
	unsigned char fRLECharacter;
	unsigned short fCRC;
	HQXSink fSink;
	long fSinkRefCon;
} HQXEncodeVars, *HQXEncVarsPtr;

typedef struct {
	unsigned char fInBuffer[4*1024];
	unsigned char fOutBuffer[4*1024];
	unsigned char *fInBufp, *fInBufMax;
	long fBitBuffer, fBitBufferBits;
	long fRLECount;
	unsigned char fRLECharacter;
	Boolean fInHqxData;
	unsigned short fCRC;
	HQXSource fSource;
	long fSourceRefCon;
} HQXDecodeVars, *HQXDecVarsPtr;

	/* globals */
static Boolean gHQXinited = false;
static char *gHQX = "!\"#$%&'()*+,-012345689@ABCDEFGHIJKLMNPQRSTUVXYZ[`abcdefhijklmpqr";
static short hqxtab[256];
static short ihqxtab[256];
static unsigned short crctab[256];


/* CRC CALCULATION */

static void BuildCRCTable(unsigned short magic_number) {
	unsigned long i, magic, mgc, val, bit;
	for (i = 0; i < 256; i++) {
		magic = (magic_number << 8);
		mgc = (magic >> 1);
		val = (i << 16);
		for (bit = 23; bit > 15; bit--) {
			if ((val & (1 << bit)) != 0) val ^= mgc;
			mgc >>= 1;
		}
		crctab[i] = val & 0xFFFF;
	}
}

static unsigned short initial_crc(void) {
	return 0;
}

static unsigned short crc_byte(unsigned short crc, unsigned char byte) {
	return ( (crctab[(crc >> 8) & 255] ^ ((crc << 8) | byte)) & 0xFFFF );
}

static unsigned short crc_run(unsigned short crc, void* buf, long len) {
	unsigned char *ch = buf;
	long i;
	unsigned short crcv = crc;
	for (i = 0; i < len; i++)
		 crcv = (crctab[(crcv >> 8) & 255] ^ ((crcv << 8) | (*ch++))) & 0xFFFF;
	return crcv;
}

static void HQXInit(void) {
	if (!gHQXinited) {
		long i, n;
		for (i = 0;i < 256;i++) {
			hqxtab[i] = -1;
			ihqxtab[i] = -1;
		}
		for (i = 0, n = strlen(gHQX); i < n; i++) {
			hqxtab[gHQX[i]] = i;
			ihqxtab[i] = gHQX[i];
		}
		hqxtab[CR] = -2;
		hqxtab[LF] = -2;
		hqxtab[HT] = -2;
		hqxtab[' '] = -2;
		hqxtab[':'] = -3;
		BuildCRCTable(kBinHexMagic);
		gHQXinited = true;
	}
}


/* BUFFERED OUTPUT */


static OSErr BUFFWrite(HQXEncVarsPtr encv, void* buffer, long count) {
	unsigned char* ch;
	long i;
	OSErr err;
	ch = (unsigned char*) buffer;
	for (i = 0;i < count;i++) {
		if (encv->fOutBufp - encv->fOutBuffer == sizeof(encv->fOutBuffer)) {
			if ((err = encv->fSink(encv->fOutBuffer, sizeof(encv->fOutBuffer), encv->fSinkRefCon)) != noErr) return err;
			encv->fOutBufp = encv->fOutBuffer;
		}
		*encv->fOutBufp++ = *ch++;
	}
	return noErr;
}

static OSErr BUFFWriteEnd(HQXEncVarsPtr encv) {
	if (encv->fOutBufp - encv->fOutBuffer > 0)
		return encv->fSink(encv->fOutBuffer, encv->fOutBufp - encv->fOutBuffer, encv->fSinkRefCon);
	else return noErr;
}


/* HQX OUTPUT */

static OSErr HQXWrite(HQXEncVarsPtr encv, void* buffer, long count) {
	unsigned char* ch, code;
	long i, v;
	OSErr err;
	ch = (unsigned char*) buffer;
	for (i = 0; i < count; i++, ch++) {
		encv->fBitBuffer = (encv->fBitBuffer << 8) | ((*ch)&255);
		encv->fBitBufferBits += 8;
		while (encv->fBitBufferBits >= 6) {
			v = (encv->fBitBuffer >> (encv->fBitBufferBits-6)) & 0x003F;
			encv->fBitBufferBits -= 6;
			code = ihqxtab[v];
			if ((err = BUFFWrite(encv, &code, 1)) != noErr) return err;
			encv->fNOutChars++;
			if ((encv->fNOutChars % 64) == 0) {
				if ((err = BUFFWrite(encv, "\n", 1)) != noErr) return err;
			}
		}
	}
	return noErr;
}

static OSErr HQXWriteEnd(HQXEncVarsPtr encv) {
	unsigned char code;
	OSErr err;
	if (encv->fBitBufferBits > 0) {
		code = ihqxtab[encv->fBitBuffer<<(6-encv->fBitBufferBits) & 0x03F];
		if ((err = BUFFWrite(encv, &code, 1)) != noErr) return err;
		encv->fNOutChars++;
	}
	return BUFFWrite(encv, ":\n", 2);
}


/* RLE OUTPUT */


static OSErr FlushRLE(HQXEncVarsPtr encv) {
	long n;
	OSErr err;
	unsigned char buffer[10], *putp;
	while (encv->fRLECount > 0) {
		n = encv->fRLECount > 255 ? 255 : encv->fRLECount;
		putp = buffer;
		*putp++ = encv->fRLECharacter;
		if (encv->fRLECharacter == kRLEFlag) {
			*putp++ = 0;	/* literal flag value */
			if (n == 2) {
				*putp++ = kRLEFlag;
				*putp++ = 0;	/* literal flag value */
			} else if (n > 2) {
				*putp++ = kRLEFlag;
				*putp++ = n;	/* repeat count */
			}
		} else {
			if (n == 2)
				*putp++ = encv->fRLECharacter;
			else if (n == 3) {
				*putp++ = encv->fRLECharacter;
				*putp++ = encv->fRLECharacter;
			} else if (n > 3) {
				*putp++ = kRLEFlag;
				*putp++ = n;
			}
		}
		if ((err = HQXWrite(encv, buffer, putp - buffer)) != noErr) return err;
		encv->fRLECount -= n;
	}
	return err;
}

static OSErr RLEWrite(HQXEncVarsPtr encv, void* buffer, long count) {
	unsigned char *ch;
	long i;
	OSErr err;
	ch = (unsigned char*) buffer;
	err = noErr;
	for (ch = (unsigned char*) buffer, i = 0; i < count; i++, ch++) {
		if (encv->fRLECount == 0) {
			encv->fRLECharacter = *ch;
			encv->fRLECount = 1;
		} else if (*ch == encv->fRLECharacter)
			encv->fRLECount += 1;
		else {
			if ((err = FlushRLE(encv)) != noErr) return err;
			encv->fRLECharacter = *ch;
			encv->fRLECount = 1;
		}
	}
	return err;
}

static OSErr RLEWriteEnd(HQXEncVarsPtr encv) {
	OSErr err;
	if ((err = FlushRLE(encv)) != noErr) return err;
	if ((err = HQXWriteEnd(encv)) != noErr) return err;
	return BUFFWriteEnd(encv);
}


static OSErr WriterInit(HQXEncVarsPtr encv, HQXSink dst, long refcon) {
	OSErr err;
	char *p;
	p = "(This file must be converted with BinHex 4.0)\n:";
	encv->fRLECount = 0;
	encv->fOutBufp = encv->fOutBuffer;
	encv->fSink = dst;
	encv->fSinkRefCon = refcon;
	if ((err = BUFFWrite(encv, p, strlen(p))) != noErr) return err;
	encv->fNOutChars = 1;
	encv->fBitBuffer = encv->fBitBufferBits = 0;
	return noErr;
}

/* CRC OUTPUT */


static OSErr CRCWriteInit(HQXEncVarsPtr encv) {
	encv->fCRC = initial_crc();
	return noErr;
}

static OSErr CRCWrite(HQXEncVarsPtr encv, void* buffer, long count) {
	encv->fCRC = crc_run(encv->fCRC, buffer, count);
	return RLEWrite(encv, buffer, count);
}

static OSErr CRCWriteEnd(HQXEncVarsPtr encv) {
	encv->fCRC = crc_byte(encv->fCRC, 0);
	encv->fCRC = crc_byte(encv->fCRC, 0);
	return RLEWrite(encv, &encv->fCRC, 2);
}


OSErr HQXEncode(StringPtr name, short vol, long dir, HQXSink dst, long refcon) {
	OSErr err;
	long zero, bytecount, actcount;
	short refnum;
	HParamBlockRec pb;
	HQXEncVarsPtr encv;
	
		/* set up */
	zero = 0;
	refnum = 0;
	encv = NULL;
	HQXInit();
	encv = (HQXEncVarsPtr) NewPtrClear(sizeof(HQXEncodeVars));
	if ((err = MemError()) != noErr) goto bail;
	encv->fRLECount = 0;
	
	err = WriterInit(encv, dst, refcon);
	if (err != noErr) goto bail;

		/* hqx file header */
	err = CRCWriteInit(encv);
	if (err != noErr) goto bail;
		memset(&pb, 0, sizeof(pb));
		pb.fileParam.ioNamePtr = name;
		pb.fileParam.ioDirID = dir;
		pb.fileParam.ioVRefNum = vol;
		pb.fileParam.ioFDirIndex = 0;
		pb.fileParam.ioFVersNum = 0;
		err = PBHGetFInfoSync(&pb);
		if (err != noErr) goto bail;
		err = CRCWrite(encv, name, *name + 1);
		if (err != noErr) goto bail;
		err = CRCWrite(encv, &zero, 1);
		if (err != noErr) goto bail;
		err = CRCWrite(encv, &pb.fileParam.ioFlFndrInfo, 10);
		if (err != noErr) goto bail;
		err = CRCWrite(encv, &pb.fileParam.ioFlLgLen, 4);
		if (err != noErr) goto bail;
		err = CRCWrite(encv, &pb.fileParam.ioFlRLgLen, 4);
		if (err != noErr) goto bail;
	err = CRCWriteEnd(encv);
	if (err != noErr) goto bail;

		/*  file data fork */
	err = CRCWriteInit(encv);
	if (err != noErr) goto bail;
		err = HOpenDF(vol, dir, name, fsRdPerm, &refnum);
		if (err != noErr) goto bail;
		for (bytecount = 0; bytecount < pb.fileParam.ioFlLgLen; bytecount += actcount) {
			actcount = pb.fileParam.ioFlLgLen - bytecount;
			if (actcount > sizeof(encv->fInBuffer)) actcount = sizeof(encv->fInBuffer);
			err = FSRead(refnum, &actcount, encv->fInBuffer);
			if (err != noErr) goto bail;
			err = CRCWrite(encv, encv->fInBuffer, actcount);
			if (err != noErr) goto bail;
		}
		FSClose(refnum); refnum = 0;
	err = CRCWriteEnd(encv);
	if (err != noErr) goto bail;

		/* file resource fork */
	err = CRCWriteInit(encv);
	if (err != noErr) goto bail;
		err = HOpenRF(vol, dir, name, fsRdPerm, &refnum);
		if (err != noErr) goto bail;
		for (bytecount = 0; bytecount < pb.fileParam.ioFlRLgLen; bytecount += actcount) {
			actcount = pb.fileParam.ioFlRLgLen - bytecount;
			if (actcount > sizeof(encv->fInBuffer)) actcount = sizeof(encv->fInBuffer);
			err = FSRead(refnum, &actcount, encv->fInBuffer);
			if (err != noErr) goto bail;
			err = CRCWrite(encv, encv->fInBuffer, actcount);
			if (err != noErr) goto bail;
		}
		FSClose(refnum); refnum = 0;
	err = CRCWriteEnd(encv);
	if (err != noErr) goto bail;
	
	err = RLEWriteEnd(encv);
	if (err != noErr) goto bail;
	DisposePtr((Ptr) encv);
	
	return noErr;
	
bail:
	if (refnum != 0) FSClose(refnum);
	if (encv != NULL) DisposePtr((Ptr) encv);
	return err;
}
	



/* BINHEX DECODER */


/* HQX INPUT, HQXSource calls */

static OSErr GetNextCharacter(HQXDecVarsPtr decv, unsigned char* the_char) {
	OSErr err;
	long bytes;
	if (decv->fInBufp >= decv->fInBufMax) {
		err = decv->fSource(decv->fInBuffer, (bytes = sizeof(decv->fInBuffer), &bytes), decv->fSourceRefCon);
		if (err == eofErr) err = noErr;
		if (err != noErr) return err;
		decv->fInBufp = decv->fInBuffer;
		decv->fInBufMax = decv->fInBuffer + bytes;
		if (decv->fInBufp >= decv->fInBufMax) return eofErr;
	}
	*the_char = *decv->fInBufp++;
	return noErr;
}

static OSErr HQXRead(HQXDecVarsPtr decv, void* buffer, long count) {
	unsigned char *dst, code;
	long i;
	long readcode;
	OSErr err;

	dst = (unsigned char*) buffer;
	for ( i = 0; i < count; i++) {
		while (decv->fBitBufferBits < 8)  {
			readcode = -1;
			while (readcode < 0) {
					/* get the next character */
				err = GetNextCharacter(decv, &code);
				if (err != noErr) return err;
					/* look up the code in the table and dispatch on the value */
				readcode = hqxtab[code];
				if (readcode == -3) {
					decv->fInHqxData = !decv->fInHqxData;
				} else if (!decv->fInHqxData)
					readcode = -1;
			}			
			decv->fBitBuffer = (decv->fBitBuffer<<6) | readcode;
			decv->fBitBufferBits += 6;
		}
		*dst++ = (decv->fBitBuffer>>(decv->fBitBufferBits-8));
		decv->fBitBufferBits -= 8;
	}
	
	return noErr;
}


/* RLE INPUT,  HQXRead calls */

static OSErr RLERead(HQXDecVarsPtr decv, void* buffer, long count) {
	unsigned char *ch, nextchar, countvalue;
	long i;
	OSErr err;

	ch = (unsigned char*) buffer;
	err = noErr;
	for ( i=0; i < count; )
		if (decv->fRLECount > 0) {
			*ch++ = decv->fRLECharacter;
			decv->fRLECount--;
			i++;
		} else {
			if ((err = HQXRead(decv, &nextchar, 1)) != noErr) return err;
			if (nextchar == kRLEFlag) {
				if ((err = HQXRead(decv, &countvalue, 1)) != noErr) return err;
				if (countvalue == 0) {
					decv->fRLECharacter = kRLEFlag;
					decv->fRLECount = 1;
				} else decv->fRLECount = countvalue - 1;
			} else {
				decv->fRLECharacter = nextchar;
				decv->fRLECount = 1;
			}
		}
		
	return noErr;
}


/* CRC INPUT, RLERead calls */

static OSErr CRCReadInit(HQXDecVarsPtr decv) {
	decv->fCRC = initial_crc();
	return noErr;
}

static OSErr CRCRead(HQXDecVarsPtr decv, void* buffer, long count) {
	OSErr err;
	if ((err = RLERead(decv, buffer, count)) != noErr) return err;
	decv->fCRC = crc_run(decv->fCRC, buffer, count);
	return noErr;
}

static OSErr CRCReadEnd(HQXDecVarsPtr decv) {
	OSErr err;
	unsigned short saved_crc;
	if ((err = RLERead(decv, &saved_crc, 2)) != noErr) return err;
	decv->fCRC = crc_byte(decv->fCRC, 0);
	decv->fCRC = crc_byte(decv->fCRC, 0);
	if (saved_crc != decv->fCRC) return paramErr; else return noErr;
}



/* FILE INPUT, CRCRead calls */

OSErr HQXDecode(HQXSource src, HQXNameFilter fname, Boolean can_replace, Boolean header_search, long refcon) {
	OSErr err;
	FInfo info, tinfo;
	long zero, bytecount, actcount, dir, data_length, rsrc_length;
	short refnum, vol;
	Str255 name;
	Boolean file_exists;
	HQXDecVarsPtr decv;
	
		/* set up */
	HQXInit();
	decv = NULL;
	zero = 0;
	refnum = 0;
	vol = 0;
	dir = 0;
	file_exists = false;
		
		/* allocate shared variables */
	decv = (HQXDecVarsPtr) NewPtrClear(sizeof(HQXDecodeVars));
	if ((err = MemError()) != noErr) goto bail;

		/* reader globals */
	decv->fRLECount = 0;
	decv->fInBufMax = decv->fInBufp = decv->fInBuffer;
	decv->fSource = src;
	decv->fSourceRefCon = refcon;
	decv->fBitBuffer = decv->fBitBufferBits = 0;
	decv->fInHqxData = false;
	
		/* search for the header string */
	if (header_search) {
		char *header, *headerp, inputchar;
		headerp = header = "(This file must be converted with BinHex 4.0)";
		while (*headerp != '\0') {
			err = GetNextCharacter(decv, (unsigned char*) &inputchar);
			if (err != noErr) goto bail;
			if (inputchar == *headerp) headerp++; else headerp = header;
		}
	}
	
		/* hqx file header */
	err = CRCReadInit(decv);
	if (err != noErr) goto bail;
	err = CRCRead(decv, name, 1);
	if (err != noErr) goto bail;
	err = CRCRead(decv, name+1, *name);
	if (err != noErr) goto bail;
	err = CRCRead(decv, &zero, 1);
	if (err != noErr) goto bail;
	err = CRCRead(decv, &info, 10);
	if (err != noErr) goto bail;
	err = CRCRead(decv, &data_length, 4);
	if (err != noErr) goto bail;
	err = CRCRead(decv, &rsrc_length, 4);
	if (err != noErr) goto bail;
	err = CRCReadEnd(decv);
	if (err != noErr) goto bail;
	
  		/* create destination file */
	if (fname != NULL) {
		err = fname(name, &vol, &dir, refcon);
		if (err != noErr) goto bail;
	}
	if (can_replace) {
		err = HDelete(vol, dir, name);
		if (err == fnfErr) err = noErr;
		if (err != noErr) goto bail;
	}
	err = HCreate(vol, dir, name, info.fdCreator, info.fdType);
	if (err != noErr) goto bail;
	file_exists = true;
	err = HGetFInfo(vol, dir, name, &tinfo);
	if (err != noErr) goto bail;
	tinfo.fdFlags = info.fdFlags & (~kHasBeenInited);
	err = HSetFInfo(vol, dir, name, &tinfo);
	if (err != noErr) goto bail;

		/* file data fork */
	err = CRCReadInit(decv);
	if (err != noErr) goto bail;
	err = HOpenDF(vol, dir, name, fsRdWrPerm, &refnum);
	if (err != noErr) goto bail;
	for (bytecount = 0; bytecount < data_length; bytecount += actcount) {
		actcount = data_length - bytecount;
		if (actcount > sizeof(decv->fOutBuffer)) actcount = sizeof(decv->fOutBuffer);
		err = CRCRead(decv, decv->fOutBuffer, actcount);
		if (err != noErr) goto bail;
		err = FSWrite(refnum, &actcount, decv->fOutBuffer);
		if (err != noErr) goto bail;
	}
	FSClose(refnum); refnum = 0;
	err = CRCReadEnd(decv);
	if (err != noErr) goto bail;

		/* file resource fork */
	err = CRCReadInit(decv);
	if (err != noErr) goto bail;
	err = HOpenRF(vol, dir, name, fsRdWrPerm, &refnum);
	if (err != noErr) goto bail;
	for (bytecount = 0; bytecount < rsrc_length; bytecount += actcount) {
		actcount = rsrc_length - bytecount;
		if (actcount > sizeof(decv->fOutBuffer)) actcount = sizeof(decv->fOutBuffer);
		err = CRCRead(decv, decv->fOutBuffer, actcount);
		if (err != noErr) goto bail;
		err = FSWrite(refnum, &actcount, decv->fOutBuffer);
		if (err != noErr) goto bail;
	}
	FSClose(refnum); refnum = 0;
	err = CRCReadEnd(decv);
	if (err != noErr) goto bail;

	DisposePtr((Ptr) decv);
	return noErr;
	
bail:
	if (refnum != 0) FSClose(refnum);
	if (file_exists) HDelete(vol, dir, name);
	if (decv != NULL) DisposePtr((Ptr) decv);
	return err;
}


/* end of file hqx.c */

/***********************************************************************
*
* syCwmpCrypto.h
*
* Copyright (C) 2013 Lin jieling <mycourage0@126.com>
* Copyright (C) 2003-2013 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/
#ifndef SYCWMPCRYPTO_H
#define SYCWMPCRYPTO_H

#define SY_MAX_MD_SIZE			64	/* longest known is SHA512 */

enum syMd5Action
{
    SY_MD5_INIT,
    SY_MD5_UPDATE,
    SY_MD5_FINAL,
    SY_MD5_DELETE
};

enum sySha256Action
{
    SY_SHA256_INIT,
    SY_SHA256_UPDATE,
    SY_SHA256_FINAL,
    SY_SHA256_DELETE
};


int SyMd5Handler(struct soap *soap, void **context,
                 enum syMd5Action action, char* buffer, unsigned int length);
unsigned char *SySha256(const unsigned char *d, unsigned int n, unsigned char *md);
long SyBase64Encode(char *to, char *from, unsigned int len);
long SyBase64Decode(char *to, char *from, unsigned int len);
#if 0
int SySha256Handler(struct soap *soap, void **context,
                    enum sySha256Action action, char* buffer, unsigned int length);
#endif
#endif

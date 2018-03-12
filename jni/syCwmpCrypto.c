/***********************************************************************
*
* syCwmpCrypto.c
*
* Copyright (C) 2013 Lin jieling <mycourage0@126.com>
* Copyright (C) 2003-2013 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/
#include "md5.h"
#include "base64.h"
#include "sha256.h"
#include "syCwmpCommon.h"
#include "syCwmpCrypto.h"
#include "stdsoap2.h"


int SyMd5Handler(struct soap* soap, void** context,
                 enum syMd5Action action, char* buffer, unsigned int length)
{
    unsigned char lHash[SY_MAX_MD_SIZE] = {0};
    md5_state_t* pms = NULL;

#ifdef SY_TEST_DEMO
    DPrint("action:%d\n", action);
#endif

    switch(action)
    {
    case SY_MD5_INIT:
        if (!*context)
        {
            *context = (void*)SOAP_MALLOC(soap, sizeof(md5_state_t));
        }
#ifdef SY_TEST_DEMO
        DPrint("1\n");
#endif
        pms = (md5_state_t*)*context;
#ifdef SY_TEST_DEMO
        DPrint("pms:0x%x\n", pms);
#endif
        md5_init(pms);
        break;
    case SY_MD5_UPDATE:
        pms = (md5_state_t*)*context;

#ifdef SY_TEST_DEMO
        DPrint("length:%d, buffer:%s\n", length, buffer);
#endif
        md5_append(pms, (md5_byte_t *)buffer, length);
        break;
    case SY_MD5_FINAL:
        pms = (md5_state_t*)*context;
        md5_finish(pms, (md5_byte_t *)&lHash);
        memcpy(buffer, lHash, 16);
        break;
    case SY_MD5_DELETE:
        pms = (md5_state_t*)*context;
        if (pms)
        {
            SOAP_FREE(soap, pms);
        }
        *context = NULL;
        break;
    }
#ifdef SY_TEST_DEMO
    DONE;
#endif

    return SY_SUCCESS;
}

unsigned char *SySha256(const unsigned char *d, unsigned int n, unsigned char *md)
{
    sha256_context shaStr;
    LOCAL unsigned char m[SY_MAX_MD_SIZE] = {0};

    if (md == NULL)
        md = m;

    sha256_starts(&shaStr);
    sha256_update(&shaStr, (uint8 *)d, (uint32)n);
    sha256_finish(&shaStr, (uint8 *)&m);
    memcpy(md, m, 32);

    return md;
}

long SyBase64Encode(char *to, char *from, unsigned int len)
{
    return base64_encode(to, from, len);
}

long SyBase64Decode(char *to, char *from, unsigned int len)
{
    return base64_decode(to, from, len);
}

#if 0
int SySha256Handler(struct soap *soap, void **context,
                    enum sySha256Action action, char* buffer, unsigned int length)
{

    unsigned char lHash[SY_MAX_MD_SIZE] = {0};
    sha256_context* pShaStr = NULL;

    DPrint("action:%d\n", action);
    switch(action)
    {
    case SY_SHA256_INIT:
        if (!*context)
        {
            *context = (void*)SOAP_MALLOC(soap, sizeof(sha256_context));
        }
        pShaStr = (sha256_context*)*context;
        sha256_starts(pShaStr);
        break;
    case SY_SHA256_UPDATE:
        pShaStr = (sha256_context*)*context;

        DPrint("length:%d, buffer:%s\n", length, buffer);
        sha256_update(pShaStr, (uint8 *)buffer, length);
        break;
    case SY_SHA256_FINAL:
        pShaStr= (sha256_context*)*context;
        sha256_finish(pShaStr, (uint8 *)&lHash);
        memcpy(buffer, lHash, 32);
        break;
    case SY_SHA256_DELETE:
        pShaStr = (sha256_context*)*context;
        if (pShaStr)
        {
            SOAP_FREE(soap, pShaStr);
        }
        *context = NULL;
        break;
    }

    return SY_SUCCESS;
}
#endif

#if 0
int main(int argc, char **argv)
{
    int i = 0;
    char* userId = "courage";
    char aOutStr[512] = {0};
    char TmpHa1[17] = {0};
    void* context = NULL;
    struct soap soap;

    SyMd5Handler(&soap, &context, SY_MD5_INIT, NULL, 0);
    DPrint("1\n");
    SyMd5Handler(&soap, &context, SY_MD5_UPDATE, userId, strlen(userId));
    SyMd5Handler(&soap, &context, SY_MD5_FINAL, TmpHa1, 0);

    DPrint("TmpHa1 len:%d\n", strlen(TmpHa1));
    for (i = 0; i < strlen(TmpHa1); i++)
    {
        DPrint("TmpHa1[%d]:%2x\n", i, TmpHa1[i]);
    }

    SySha256((unsigned char *)userId, strlen(userId),
             (unsigned char *)aOutStr);
    DPrint("aOutStr len:%d\n", strlen(aOutStr));
    for (i = 0; i < strlen(aOutStr); i++)
    {
        DPrint("TmpHa1[%d]:%x\n", i, aOutStr[i]);
    }

}
#endif

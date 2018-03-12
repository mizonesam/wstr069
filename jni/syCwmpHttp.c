/***********************************************************************
*
* syCwmpHttp.c
*
* Copyright (C) 2012 Lin jieling <mycourage0@126.com>
* Copyright (C) 2003-2012 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/
#include <pthread.h>
#include <sys/types.h>
#include "syCwmpCommon.h"
#include "syCwmpHttp.h"
#include "syCwmpCrypto.h"
#include "syCwmpManagement.h"

extern int gSyGetting;
extern int gSyIsCPEStart;
extern syGlobalParmStruct gsyGlobalParmStru;
extern syAcsCpeParamStruct gsyAcsCpeParamStru;

const char syHttpId[12] = "HTTP_SY_1.0";
char Sz_Nonce[32] = {0}, Sz_Opaque[32] = {0};
LOCAL char gSyNonce[11] = {0};
LOCAL char gSyAespasswd[17] = {0};
LOCAL syHttpSessionStruct *gSyHttpSeesion = NULL;
LOCAL pthread_mutex_t gSyHttpSessionLock;

LOCAL int HttpInit(struct soap *soap, syHttpData* data);
LOCAL void HttpDelete(struct soap *soap, struct soap_plugin *plugin);
LOCAL int HttpCopy(struct soap *soap, struct soap_plugin *pluginDst, struct soap_plugin *pluginSrc);
LOCAL int HttpPostHeader(struct soap *soap, const char* key, const char* value);
LOCAL int HttpParseHeader(struct soap *soap, const char* key, const char* value);
LOCAL int HttpPrepInitSend(struct soap *soap);
LOCAL int HttpPrepInitRecv(struct soap *soap);
LOCAL int HttpPrepareRecv(struct soap *soap, const char* buffer, unsigned int len);
LOCAL int HttpPrepareSend(struct soap *soap, const char* buffer, unsigned int len);
LOCAL int HttpDisconnect(struct soap *soap);
LOCAL int SyGetAesPassword(syHttpData *HttpData);
LOCAL void HttpHA1Calc(struct soap* soap, void** context,
                         char* alg, char* userId, char* realm, char* passwd, char* nonce,
                         char* cnonce, char HA1hex[33]);
LOCAL void HttpResponseCalc(struct soap* Soap, void** Context,
                              char HA1hex[33], char* Nonce, char* Ncount, char* Cnonce, char* Qop,
                              char* Method, char* Uri, char EntityHAhex[33], char Response[33]);
LOCAL void HttpNonceCalc(struct soap *soap, char nonce[21]);
LOCAL void HttpOpaqueCalc(struct soap *soap, char opaque[9]);
LOCAL void HttpCnonceCalc(struct soap *soap, void** Context,
                            char* cNonce, char hex[33]);
LOCAL void HttpStartSession(const char *realm, const char *nonce, const char *opaque);
LOCAL void HttpCleanSession();
LOCAL int HttpUpdateSession(const char *realm, const char *nonce,
                              const char *opaque, const char *cnonce, const char *ncount);
LOCAL int HttpMethodVerify(struct soap* soap, char* method, char* password);
LOCAL int DoDigestAuth(struct soap *soap);


void HttpRestore(struct soap* soap, syHttpHeaderInfo* httpInfo)
{
    syHttpData* data = (syHttpData*)soap_lookup_plugin(soap, syHttpId);

    if (!data)
    {
        EPrint("data is NULL\n");
        return;
    }

    soap->authrealm = httpInfo->Realm;
    soap->userid = httpInfo->Username;
    soap->passwd = httpInfo->Password;
    data->Qop = httpInfo->Qop;
    data->Nonce = httpInfo->Nonce;
    data->Opaque = httpInfo->Opaque;
    data->Algorithm = httpInfo->Algorithm;
}

void HttpRelease(struct soap* soap, syHttpHeaderInfo* httpInfo)
{
    syHttpData* data = (syHttpData*)soap_lookup_plugin(soap, syHttpId);

    DO;

    if (!data)
    {
        DPrint("data is NULL\n");
        return;
    }

    soap->authrealm = NULL;
    soap->userid = NULL;
    soap->passwd = NULL;
    data->Qop = NULL;
    data->Nonce = NULL;
    data->Opaque = NULL;
    data->Algorithm = NULL;

    if (httpInfo->Realm)
    {
        free(httpInfo->Realm);
        httpInfo->Realm = NULL;
    }

    if (httpInfo->Username)
    {
        free(httpInfo->Username);
        httpInfo->Username = NULL;
    }

    if (httpInfo->Password)
    {
        free(httpInfo->Password);
        httpInfo->Password = NULL;
    }

    if (httpInfo->Qop)
    {
        free(httpInfo->Qop);
        httpInfo->Qop = NULL;
    }

    if (httpInfo->Algorithm)
    {
        free(httpInfo->Algorithm);
        httpInfo->Algorithm = NULL;
    }

    if (httpInfo->Nonce)
    {
        free(httpInfo->Nonce);
        httpInfo->Nonce = NULL;
    }

    if (httpInfo->Opaque)
    {
        free(httpInfo->Opaque);
        httpInfo->Opaque = NULL;
    }
}

void HttpSave(struct soap* soap, syHttpHeaderInfo* httpInfo,
                const char* realm, const char* userId, const char* passwd)
{
    syHttpData* data = (syHttpData*)soap_lookup_plugin(soap, syHttpId);

    if (!data)
    {
        EPrint("data is NULL\n");
        return;
    }

    httpInfo->Realm = soap_strdup(NULL, realm);
    httpInfo->Username = soap_strdup(NULL, userId);
    httpInfo->Password = soap_strdup(NULL, passwd);
    soap->authrealm = httpInfo->Realm;
    soap->userid = httpInfo->Username;
    soap->passwd = httpInfo->Password;
    httpInfo->Nonce= soap_strdup(NULL, data->Nonce);
    httpInfo->Opaque = soap_strdup(NULL, data->Opaque);
    httpInfo->Qop = soap_strdup(NULL, data->Qop);
    httpInfo->Algorithm = soap_strdup(NULL, data->Algorithm);
}

LOCAL int HttpMethodVerify(struct soap* soap, char* method, char* password)
{
    char  TmpHa1[33] = {0};
    char  Response[33] = {0};
    char  EntityHAHex[33] = {0};
    syHttpData* data = (syHttpData*)soap_lookup_plugin(soap, syHttpId);
    DO;

    if (!data)
    {
        DPrint("data is NULL\n");
        return SOAP_PLUGIN_ERROR;
    }

    if (!soap->authrealm || !soap->userid || soap->passwd)
    {
        DPrint("Use None or Basic Authentication.\n");
        return SY_FAILED;
    }

    if (!data->Qop)
    {
        DPrint("Require Auth Qop to Prevent replay attacks\n");
        return SY_FAILED;
    }

    if (HttpUpdateSession(soap->authrealm, data->Nonce, data->Opaque,
                            data->Cnonce, data->Ncount))
    {
        DPrint("Update Http Session Error\n");
        return SY_FAILED;
    }

    HttpHA1Calc(soap, &data->Context, NULL, (char*)soap->userid, (char*)soap->authrealm,
                  password, data->Nonce, data->Cnonce, TmpHa1);
    DPrint("HA1:%s\n", TmpHa1);

    if (!soap_tag_cmp(data->Qop, "auth-int"))
    {
        soap_s2hex(soap, (unsigned char*)data->Digest, EntityHAHex, 16);
    }

    HttpResponseCalc(soap, &data->Context, TmpHa1, data->Nonce,
                       data->Ncount, data->Cnonce, data->Qop, method, soap->path,
                       EntityHAHex, Response);
    DPrint("data:%s, Response:%s\n", data->Response, Response);
    if (strcmp(data->Response, Response))
    {
        DPrint("Auth Fail.\n");
        return SY_FAILED;
    }

    return SY_SUCCESS;
}

int HttpPostVerify(struct soap* soap, char* password)
{
    DO;

    return HttpMethodVerify(soap, "POST", password);
}

int HttpGetVerify(struct soap* soap, char* password)
{
    DO;

    return HttpMethodVerify(soap, "GET", password);
}

int HttpD(struct soap* soap, struct soap_plugin* plugin, void* arg)
{
    DO;
    plugin->id = syHttpId;
    plugin->data = (void*)SOAP_MALLOC(soap, sizeof(syHttpData));

    DPrint("data:%p\n", plugin->data);

    plugin->fdelete = HttpDelete;
    plugin->fcopy = HttpCopy;
    if (NULL != plugin->data)
    {
        if (HttpInit(soap, (syHttpData*)plugin->data))
        {
            SOAP_FREE(soap, plugin->data);
            plugin->data = NULL;
            return SY_FAILED;
        }
    }
    DONE;
    return SY_SUCCESS;
}

LOCAL int HttpInit(struct soap *soap, syHttpData* data)
{
#ifdef SY_TEST_DEMO
    DO;
#endif

    data->PostHeaderFunc = soap->fposthdr;
#ifdef SY_TEST_DEMO
    DPrint("soap->fposthdr:%p, PostHeaderFunc:%p\n",
           soap->fposthdr, data->PostHeaderFunc);
#endif
    data->ParseHeaderFunc = soap->fparsehdr;
    data->PrepareInitRecvFunc = soap->fprepareinitrecv;
    data->PrepareInitSendFunc = soap->fprepareinitsend;
    soap->fposthdr = HttpPostHeader;
#ifdef SY_TEST_DEMO
    DPrint("soap->fposthdr:%p, HttpPostHeader:%p\n",
           soap->fposthdr, HttpPostHeader);
#endif
    soap->fparsehdr = HttpParseHeader;
    soap->fprepareinitsend = HttpPrepInitSend;
    soap->fprepareinitrecv = HttpPrepInitRecv;

    data->Context = NULL;
    //memset(data->Digest, 0x00, sizeof(data->Digest));
    memcpy(data->Digest, "hitler", strlen("hitler"));
#ifdef SY_TEST_DEMO
    DONE;
#endif
    return SY_SUCCESS;
}

LOCAL void HttpDelete(struct soap *soap, struct soap_plugin *plugin)
{
    syHttpData* data = (syHttpData*)soap_lookup_plugin(soap, syHttpId);
#ifdef SY_TEST_DEMO
    DO;
#endif
    DPrint("data:%p\n", data);
    if (data)
    {
        if (data->Context)
        {
            SyMd5Handler(soap, &data->Context, SY_MD5_DELETE, NULL, 0);
        }
        SOAP_FREE(soap, data);
        data = NULL;
    }
    DONE;
}

LOCAL int HttpCopy(struct soap *soap, struct soap_plugin *pluginDst, struct soap_plugin *pluginSrc)
{
    syHttpData* tmpHttpDate = NULL;
#ifdef SY_TEST_DEMO
    DO;
#endif
    *pluginDst = *pluginSrc;
    //pluginDst->id = syHttpId;
    pluginDst->data = (void*)SOAP_MALLOC(soap, sizeof(syHttpData));
#ifdef SY_TEST_DEMO
    DPrint("pluginDst->data:%p\n", pluginDst->data);
#endif
    memcpy(pluginDst->data, pluginSrc->data, sizeof(syHttpData));
    tmpHttpDate = (syHttpData*)pluginDst->data;
#ifdef SY_TEST_DEMO
    DPrint("Digest length:%d\n", sizeof(tmpHttpDate->Digest));
#endif
    memset(tmpHttpDate->Digest, 0x00,  sizeof(tmpHttpDate->Digest));
    tmpHttpDate->Nonce = NULL;
    tmpHttpDate->Uri = NULL;
    tmpHttpDate->Response = NULL;
    tmpHttpDate->Algorithm = NULL;
    tmpHttpDate->Opaque = NULL;
    tmpHttpDate->Cnonce = NULL;
    tmpHttpDate->Ncount = NULL;
    tmpHttpDate->Qop = NULL;
    tmpHttpDate->Nc = 0;

    DONE;

    return SY_SUCCESS;
}

LOCAL int DoDigestAuth(struct soap *soap)
{

    DO;

    DPrint("userid:%s, passwd:%s, authrealm:%s\n",
           soap->userid, soap->passwd, soap->authrealm);
    DPrint("ConnectionRequestUsername:%s, ConnectionRequestPassword:%s, Realm:%s\n",
           gsyAcsCpeParamStru.ConnectionRequestUsername, gsyAcsCpeParamStru.ConnectionRequestPassword,
           gsyAcsCpeParamStru.Realm);

    if (soap->userid && soap->passwd)   //Basic
    {
        if(!strcmp(soap->userid, gsyAcsCpeParamStru.ConnectionRequestUsername)
                && !(strcmp(soap->passwd, gsyAcsCpeParamStru.ConnectionRequestPassword)))
            return SOAP_OK;
    }
    // Digest
    if(soap->authrealm && soap->userid)
    {

        if(!strcmp(soap->authrealm, gsyAcsCpeParamStru.Realm)
                && !strcmp(soap->userid, gsyAcsCpeParamStru.ConnectionRequestUsername))
        {
            if(!HttpGetVerify(soap, gsyAcsCpeParamStru.ConnectionRequestPassword))
                return SOAP_OK;
        }

        syHttpData* HttpData = (syHttpData*)soap_lookup_plugin(soap, syHttpId);

        DPrint("Sz_Nonce:%s,Sz_Opaque:%s,Nonce:%s,Opaque:%s",
               Sz_Nonce,
               Sz_Opaque,
               HttpData->Nonce,
               HttpData->Opaque);
        if(strncmp(Sz_Nonce,HttpData->Nonce,strlen(HttpData->Nonce)) == 0
                && strncmp(Sz_Opaque,HttpData->Opaque,strlen(HttpData->Opaque)) == 0)
        {
            extern int NATDetected;
            extern int syNATCmd;
            if(SY_TRUE != NATDetected)
                syNATCmd =1;
            return SOAP_OK;
        }
    }

    DONE;

    return SY_HTTP_UNAUTHORIZED;
}

int HttpGet(struct soap *soap)
{
    int nRet = SOAP_OK;

    DO;

    nRet = DoDigestAuth(soap);

    if (SOAP_OK == nRet)
    {
        gSyGetting = 0;
        soap->keep_alive = 0;
        soap->count = 0;
        soap->length = 0;
        soap_response(soap, SY_HTTP_OK);
        soap_end_send(soap);
        gsyGlobalParmStru.SessionState |=   SY_SESSION_IDLE;
        gsyGlobalParmStru.SessionEvent |=   SY_EVENT_START;
        gsyGlobalParmStru.SessionTrigger |= SY_TRIGGER_CONNECTION_REQUEST;
        gsyGlobalParmStru.SessionFlags |=   SY_STATE_CONNECTREQURL;
        if (0 == gSyIsCPEStart)
            gSyIsCPEStart = 1;
    }
    else if(SY_HTTP_UNAUTHORIZED == nRet)
    {
        soap->keep_alive = 0;
        soap->count = 0;
        soap->length = 0;
        soap_response(soap, SY_HTTP_UNAUTHORIZED);
        soap_end_send(soap);
        gSyGetting = 1;
        if (0 == gSyIsCPEStart)
            gSyIsCPEStart = 1;
    }

    DONE;

    return SOAP_OK;
}

LOCAL int HttpPostHeader(struct soap *soap, const char* key, const char* value)
{
    char* Qop = NULL;
    char* Method = NULL;
    char  Nonce[21] = {0};
    char  Opaque[9] = {0};
    char  Ncount[9] = {0};
    char  Cnonce[21] = {0};
    char  TmpHa1[33] = {0};
    char  Response[33] = {0};
    char  EntityHAHex[33] = {0};
    syHttpData* data = (syHttpData*)soap_lookup_plugin(soap, syHttpId);

    if (!data)
    {
        EPrint("data is NULL\n");
        return SOAP_PLUGIN_ERROR;
    }

    DPrint("%s = %s", key, value);

    if (key)
    {

        if (0 == strcmp(key, "Authorization"))
        {
            DPrint("AuthType:%s\n", gsyAcsCpeParamStru.AuthType);
            if (0 == strcmp(gsyAcsCpeParamStru.AuthType, "Basic"))
            {
                DPrint("2\n");
                return data->PostHeaderFunc(soap, key, soap->tmpbuf);
            }

            SyMd5Handler(soap, &data->Context, SY_MD5_FINAL, data->Digest, 0);
            strncpy(Cnonce, gSyNonce, strlen(gSyNonce));
            DPrint("Cnonce:%s, gSyNonce:%s\n", Cnonce, gSyNonce);
            HttpHA1Calc(soap, &data->Context, data->Algorithm, (char*)soap->userid,
                          (char*)soap->authrealm, (char*)soap->passwd, (char*)data->Nonce, Cnonce, TmpHa1);
            if (data->Qop && !soap_tag_cmp(data->Qop, "*auth-int*"))
            {
                Qop = "auth-int";
                soap_s2hex(soap, (unsigned char *)data->Digest, EntityHAHex, 16);
            }
            else if (data->Qop)
            {
                Qop = "auth";
            }
            else
            {
                Qop = NULL;
            }

            if(soap->status == SOAP_GET)
                Method = "GET";
            else
                Method = "POST";

            sprintf(Ncount, "%8.8lx", data->Nc++);
            HttpResponseCalc(soap, &data->Context, TmpHa1, data->Nonce,
                               Ncount, Cnonce, Qop, Method, soap->path, EntityHAHex,
                               Response);
            //SyHttpCnonceCalc(soap, &data->Context, gSyNonce, Cnonce);
            sprintf(soap->tmpbuf, "Digest realm=\"%s\", username=\"%s\", algorithm=\"MD5\", nonce=\"%s\", uri=\"%s\", nc=%s, cnonce=\"%s\", response=\"%s\"",
                    soap->authrealm, soap->userid, data->Nonce, soap->path, Ncount, Cnonce, Response);
            if (data->Opaque)
            {
                sprintf(soap->tmpbuf + strlen(soap->tmpbuf), ", opaque=\"%s\"", data->Opaque);
            }
            if (Qop)
            {
                sprintf(soap->tmpbuf + strlen(soap->tmpbuf), ", qop=\"%s\"", Qop);
            }
            return data->PostHeaderFunc(soap, key, soap->tmpbuf);
        }
        else if (0 == strcmp(key, "WWW-Authenticate"))
        {
            HttpNonceCalc(soap, Nonce);
            HttpOpaqueCalc(soap, Opaque);

            DPrint("authrealm:%s, gsyAcsCpeParamStru.Realm:%s\n",
                   soap->authrealm,
                   gsyAcsCpeParamStru.Realm);

            if (NULL != soap->authrealm)
            {
                free((char *)soap->authrealm);
            }

            /*
             * 之前是soap->authrealm = soap_malloc(soap, strlen("Digest Authentication")), 看不懂...
             */
            soap->authrealm = soap_strdup(soap, "Digest Authentication");
            //soap->authrealm = soap_malloc(soap, strlen("Digest Authentication"));

            if (NULL != gsyAcsCpeParamStru.Realm)
            {
                free(gsyAcsCpeParamStru.Realm);
            }
            gsyAcsCpeParamStru.Realm = strdup("Digest Authentication");

            HttpStartSession(soap->authrealm, Nonce, Opaque);
            sprintf(soap->tmpbuf, "Digest realm=\"%s\", domain=\"%s\", qop=\"auth\",algorithm=\"MD5\", nonce=\"%s\", opaque=\"%s\"",
                    soap->authrealm, "/", Nonce, Opaque);
            strncpy(Sz_Nonce,Nonce,strlen(Nonce)+1);
            strncpy(Sz_Opaque,Opaque,strlen(Opaque)+1);
            return data->PostHeaderFunc(soap, key, soap->tmpbuf);
        }
    }

    //DPrint("PostHeaderFunc:%p\n", data->PostHeaderFunc);


    return data->PostHeaderFunc(soap, key, value);
}
LOCAL int HttpParseHeader(struct soap *soap, const char* key, const char* value)
{
    int i = 0;
    int tmpLen = 0;
    char* pos = NULL;
    char* pNonce = NULL;
    char tmpBuf[4] = {0};

    const char* date[12] =
    {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    DPrint("key:%s, value:%s\n", key, value);

    syHttpData* HttpData = (syHttpData*)soap_lookup_plugin(soap, syHttpId);
    if (!HttpData)
    {
        EPrint("data is NULL\n");
        return SOAP_PLUGIN_ERROR;
    }
    //NONCE  "Tue, 06 Mar 2012 07:28:30 GMT"
    if (!strncmp(key, "Date", 4))
    {

        pNonce = gSyNonce;
        for (i = 0; i < 12; i++)
        {
            strncpy(tmpBuf, date[i], 3);

            pos = strstr((char*)value, tmpBuf);
            if (pos != NULL)
            {
                break;
            }
        }
        //DPrint("pos:%s\n", pos);
        sprintf(pNonce, "%d", i + strlen("Date: Wed, "));
        pNonce += 2;
        if (pos != NULL)
        {
            pos = pos - 3;
            strncpy(pNonce, pos, 2);
            pos = strstr((char*)value, "20");//hava bug, need fix later, hitler 2013-06-14
            pos = pos + 5;
            strncpy(pNonce, pos, 2);
            pos = pos + 3;
            pNonce = pNonce + 2;
            strncpy(pNonce, pos, 2);
            pos = pos + 3;
            pNonce = pNonce + 2;
            strncpy(pNonce, pos, 2);
        }
        else
        {
            strncpy(pNonce, "25131313", 8);
        }
        DPrint("pNonce:%s\n", pNonce);
    }

    tmpLen = strlen("Digest ");
    if (!soap_tag_cmp(key, "Authorization") &&
            !soap_tag_cmp(value, "Digest *"))
    {

        soap->authrealm = soap_strdup(soap,
                                      soap_get_header_attribute(soap, value + tmpLen, "realm"));
        soap->userid = soap_strdup(soap,
                                   soap_get_header_attribute(soap, value + tmpLen, "username"));
        soap->passwd = NULL;
        HttpData->Nonce = soap_strdup(soap,
                                      soap_get_header_attribute(soap, value + tmpLen, "nonce"));
        HttpData->Opaque = soap_strdup(soap,
                                       soap_get_header_attribute(soap, value + tmpLen, "opaque"));
        HttpData->Qop = soap_strdup(soap,
                                    soap_get_header_attribute(soap, value + tmpLen, "qop"));
        HttpData->Ncount = soap_strdup(soap,
                                       soap_get_header_attribute(soap, value + tmpLen, "nc"));
        HttpData->Cnonce = soap_strdup(soap,
                                       soap_get_header_attribute(soap, value + tmpLen, "cnonce"));
        HttpData->Response = soap_strdup(soap,
                                         soap_get_header_attribute(soap, value + tmpLen, "response"));
        HttpData->Algorithm = NULL;
        if (HttpData->Qop && !soap_tag_cmp(HttpData->Qop, "auth-int"))
        {
            if (soap->fpreparerecv != HttpPrepareRecv)
            {
                HttpData->PrepareRecvFunc = soap->fpreparerecv;
                soap->fpreparerecv = HttpPrepareRecv;
            }
            if (soap->fdisconnect != HttpDisconnect)
            {
                HttpData->DisconnectFunc = soap->fdisconnect;
                soap->fdisconnect = HttpDisconnect;
            }
            SyMd5Handler(soap, &HttpData->Context, SY_MD5_INIT, NULL, 0);
        }
        return SY_SUCCESS;
    }

    if (!soap_tag_cmp(key, "WWW-Authenticate") && \
            !soap_tag_cmp(value, "Digest *"))
    {

        if (gsyAcsCpeParamStru.AuthType != NULL)
        {
            free(gsyAcsCpeParamStru.AuthType);
            gsyAcsCpeParamStru.AuthType = strdup("Digest");
        }
        if (gsyAcsCpeParamStru.Realm != NULL)
        {
            free(gsyAcsCpeParamStru.Realm);
        }
        gsyAcsCpeParamStru.Realm = strdup( \
                                           soap_get_header_attribute(soap, value + tmpLen, "realm"));

        HttpData->Nonce = soap_strdup(soap, \
                                      soap_get_header_attribute(soap, value + tmpLen, "nonce"));
        HttpData->Opaque = soap_strdup(soap, \
                                       soap_get_header_attribute(soap, value + tmpLen, "opaque"));
        HttpData->Algorithm = soap_strdup(soap, \
                                          soap_get_header_attribute(soap, value + tmpLen, "algorithm"));
        HttpData->Qop = soap_strdup(soap, \
                                    soap_get_header_attribute(soap, value + tmpLen, "qop"));
        HttpData->Ncount = NULL;
        HttpData->Cnonce = NULL;
        HttpData->Response = NULL;
        HttpData->Nc = 1;

        SyGetAesPassword(HttpData);

        return SY_SUCCESS;
    }

    tmpLen = strlen("Basic ");
    if (!soap_tag_cmp(key, "WWW-Authenticate") &&
            !soap_tag_cmp(value, "Basic *"))
    {

        if (gsyAcsCpeParamStru.AuthType != NULL)
        {
            free(gsyAcsCpeParamStru.AuthType);
            gsyAcsCpeParamStru.AuthType = strdup("Basic");
        }

        if (gsyAcsCpeParamStru.Realm != NULL)
        {
            free(gsyAcsCpeParamStru.Realm);
            gsyAcsCpeParamStru.Realm = strdup(soap_get_header_attribute(soap, value + tmpLen, "realm"));
        }

        DPrint("authrealm:%s\n", soap->authrealm);

        return SY_SUCCESS;
    }


    return HttpData->ParseHeaderFunc(soap, key, value);
}

LOCAL int HttpPrepInitSend(struct soap *soap)
{

    //DO;

    syHttpData* data = (syHttpData*)soap_lookup_plugin(soap, syHttpId);

    //DPrint("data:%p\n", data);

    if (!data)
    {
        E("data is NULL\n");
        return SOAP_PLUGIN_ERROR;
    }

    if ((SOAP_IO_STORE != (soap->mode & SOAP_IO))
            && (soap->mode & (SOAP_ENC_DIME | SOAP_ENC_MIME)))
    {

        soap->mode &= ~SOAP_IO;
        soap->mode |= SOAP_IO_STORE;
    }
    else
    {

        //DPrint("userid:%s, passwd:%s\n", soap->userid, soap->passwd);

        if(soap->userid && soap->passwd)
        {
            SyMd5Handler(soap, &data->Context, SY_MD5_INIT, NULL, 0);
            if(soap->fpreparesend != HttpPrepareSend)
            {
                data->PrepareSendFunc = soap->fpreparesend;
                soap->fpreparesend = HttpPrepareSend;
            }
        }

        D("PrepareInitSendFunc:%p\n", data->PrepareInitSendFunc);

        if (data->PrepareInitSendFunc)
            return data->PrepareInitSendFunc(soap);
    }

    return SY_SUCCESS;
}

LOCAL int HttpPrepInitRecv(struct soap *soap)
{

    DO;

    syHttpData* data = (syHttpData*)soap_lookup_plugin(soap, syHttpId);

    //DPrint("data:%p\n", data);

    if (!data)
    {
        EPrint("data is NULL\n");
        return SOAP_PLUGIN_ERROR;
    }

    if ((SOAP_IO_STORE != (soap->mode & SOAP_IO))
            && (soap->mode & (SOAP_ENC_DIME | SOAP_ENC_MIME)))
    {

        soap->mode &= ~SOAP_IO;
        soap->mode |= SOAP_IO_STORE;

        DPrint("mode = [%#x]\n", soap->mode);

    }
    else
    {

        if (soap->fpreparerecv == HttpPrepareRecv)
        {
            soap->fpreparerecv = data->PrepareRecvFunc;
        }
        if (soap->fdisconnect == HttpDisconnect)
        {
            soap->fdisconnect = data->DisconnectFunc;
        }

        if (data->PrepareInitRecvFunc)
            return data->PrepareInitRecvFunc(soap);
    }
    return SY_SUCCESS;
}

LOCAL int HttpPrepareRecv(struct soap *soap, const char* buffer, unsigned int len)
{

    DO;

    syHttpData* data = (syHttpData*)soap_lookup_plugin(soap, syHttpId);
    if (!data)
    {
        DPrint("data is NULL\n");
        return SOAP_PLUGIN_ERROR;
    }

    SyMd5Handler(soap, &data->Context, SY_MD5_UPDATE, (char*)buffer, len);

    if (data->PrepareRecvFunc)
    {
        return data->PrepareRecvFunc(soap, buffer, len);
    }

    return SY_SUCCESS;
}

LOCAL int HttpPrepareSend(struct soap *soap, const char* buffer, unsigned int len)
{
    syHttpData* data = (syHttpData*)soap_lookup_plugin(soap, syHttpId);
    if (!data)
    {
        DPrint("data is NULL\n");
        return SOAP_PLUGIN_ERROR;
    }

    SyMd5Handler(soap, &data->Context, SY_MD5_UPDATE, (char*)buffer, len);

    if (data->PrepareSendFunc)
    {
        return data->PrepareSendFunc(soap, buffer, len);
    }

    return SY_SUCCESS;
}

LOCAL void HttpStartSession(const char *realm, const char *nonce,
                              const char *opaque)
{
    time_t CurrenTime = 0L;
    syHttpSessionStruct* HttpSession;
    DO;

    CurrenTime = time(NULL);

    if (0 == CurrenTime % 10)
    {
        HttpCleanSession();
    }
    pthread_mutex_lock(&gSyHttpSessionLock);
    HttpSession = (syHttpSessionStruct*)malloc(sizeof(syHttpSessionStruct));
    if (HttpSession)
    {
        HttpSession->NextHttpSession = gSyHttpSeesion;
        HttpSession->Modified = CurrenTime;
        HttpSession->Realm = soap_strdup(NULL, realm);
        HttpSession->Nonce = soap_strdup(NULL, nonce);
        HttpSession->Opaque = soap_strdup(NULL, opaque);
        HttpSession->Nc = 0;
        gSyHttpSeesion = HttpSession;
    }
    pthread_mutex_unlock(&gSyHttpSessionLock);
}

LOCAL int HttpUpdateSession(const char *realm, const char *nonce,
                              const char *opaque, const char *cnonce, const char *ncount)
{
    syHttpSessionStruct* HttpSession;

    DO;

    if (!realm || !nonce || !opaque || !cnonce || !ncount)
    {
        DPrint("Some params is NULL\n");
        return SY_FAILED;
    }

    DPrint("realm:%s, nonce:%s, opaque:%s, cnonce:%s, ncount:%s\n",
           realm, nonce, opaque, cnonce, ncount);

    pthread_mutex_lock(&gSyHttpSessionLock);
    for (HttpSession = gSyHttpSeesion; HttpSession;
            HttpSession = HttpSession->NextHttpSession)
    {
        DPrint("NonceNonce:%s, Opaque:%s\n",
               HttpSession->Nonce, HttpSession->Opaque);
        if (!strcmp(HttpSession->Realm, realm) &&
                !strcmp(HttpSession->Nonce, nonce) &&
                !strcmp(HttpSession->Opaque, opaque))
        {
            break;
        }
    }

    if (HttpSession)
    {
        unsigned long Nc = soap_strtoul(ncount, NULL, 16);
        if (HttpSession->Nc >= Nc)
        {
            HttpSession->Modified = 0;
            HttpSession = NULL;
        }
        else
        {
            HttpSession->Nc = Nc;
            HttpSession->Modified = time(NULL);
        }
    }
    pthread_mutex_unlock(&gSyHttpSessionLock);
    if (!HttpSession)
    {
        return SY_FAILED;
    }
    return SY_SUCCESS;
}

LOCAL void HttpCleanSession()
{
    time_t CurrenTime = 0L;
    syHttpSessionStruct** HttpSession;
    syHttpSessionStruct* tmpSession;

    DO;


    pthread_mutex_lock(&gSyHttpSessionLock);

    HttpSession = &gSyHttpSeesion;
    while(*HttpSession != NULL)
    {
        if ((*HttpSession)->Modified + SY_HTTP_SESSION_TIMEOUT < CurrenTime)
        {
            tmpSession = *HttpSession;
            if(tmpSession->Nonce)
            {
                free(tmpSession->Nonce);
            }
            if (tmpSession->Opaque)
            {
                free(tmpSession->Opaque);
            }

            *HttpSession = tmpSession->NextHttpSession;
            free(tmpSession);
        }
        else
        {
            HttpSession = &(*HttpSession)->NextHttpSession;
        }
    }
    pthread_mutex_unlock(&gSyHttpSessionLock);
}

LOCAL int HttpDisconnect(struct soap *soap)
{

    DO;

    syHttpData* data = (syHttpData*)soap_lookup_plugin(soap, syHttpId);
    if (!data)
    {
        DPrint("data is NULL\n");
        return SOAP_PLUGIN_ERROR;
    }

    SyMd5Handler(soap, &data->Context, SY_MD5_FINAL, data->Digest, 0);

    soap->fpreparerecv = data->PrepareRecvFunc;
    soap->fdisconnect = data->DisconnectFunc;

    if (data->DisconnectFunc)
    {
        return data->DisconnectFunc(soap);
    }

    return SY_SUCCESS;
}

LOCAL void HttpHA1Calc(struct soap* soap, void** context,
                         char* alg, char* userId, char* realm, char* passwd, char* nonce,
                         char* cnonce, char HA1hex[33])
{
    char TmpHa1[17] = {0};

    DO;

    SyMd5Handler(soap, context, SY_MD5_INIT, NULL, 0);
    SyMd5Handler(soap, context, SY_MD5_UPDATE, userId, strlen(userId));
    SyMd5Handler(soap, context, SY_MD5_UPDATE, ":", 1);
    SyMd5Handler(soap, context, SY_MD5_UPDATE, realm, strlen(realm));
    SyMd5Handler(soap, context, SY_MD5_UPDATE, ":", 1);
    SyMd5Handler(soap, context, SY_MD5_UPDATE, passwd, strlen(passwd));
    SyMd5Handler(soap, context, SY_MD5_FINAL, TmpHa1, 0);

    //DPrint("alg:%s\n", alg);

    if (alg && !soap_tag_cmp(alg, "MD5-sess"))
    {
        SyMd5Handler(soap, context, SY_MD5_INIT, NULL, 0);
        SyMd5Handler(soap, context, SY_MD5_UPDATE, TmpHa1, strlen(TmpHa1));
        SyMd5Handler(soap, context, SY_MD5_UPDATE, ":", 1);
        SyMd5Handler(soap, context, SY_MD5_UPDATE, nonce, strlen(nonce));
        SyMd5Handler(soap, context, SY_MD5_UPDATE, ":", 1);
        SyMd5Handler(soap, context, SY_MD5_UPDATE, cnonce, strlen(cnonce));
        SyMd5Handler(soap, context, SY_MD5_FINAL, TmpHa1, 0);
    }

    soap_s2hex(soap, (unsigned char *)TmpHa1, HA1hex, 16);

    DPrint("HA1 hex:%s\n", HA1hex);

}

LOCAL void HttpResponseCalc(struct soap* soap, void** Context,
                              char HA1hex[33], char* Nonce, char* Ncount, char* Cnonce, char* Qop,
                              char* Method, char* Uri, char EntityHAhex[33], char Response[33])
{
    char TmpHa2[17] = {0};
    char ResponseHa[17] = {0};
    char TmpHa2Hex[33] = {0};

    DO;

    SyMd5Handler(soap, Context, SY_MD5_INIT, NULL, 0);
    SyMd5Handler(soap, Context, SY_MD5_UPDATE, Method, strlen(Method));
    SyMd5Handler(soap, Context, SY_MD5_UPDATE, ":", 1);
    SyMd5Handler(soap, Context, SY_MD5_UPDATE, Uri, strlen(Uri));
    if (Qop)
    {
        if (!soap_tag_cmp(Qop, "auth-int"))
        {
            SyMd5Handler(soap, Context, SY_MD5_UPDATE, ":", 1);
            SyMd5Handler(soap, Context, SY_MD5_UPDATE, EntityHAhex, 32);
        }
    }

    DPrint("Nonce:%s\n", Nonce);

    SyMd5Handler(soap, Context, SY_MD5_FINAL, TmpHa2, 0);
    soap_s2hex(soap, (unsigned char *)TmpHa2, TmpHa2Hex, 16);

    SyMd5Handler(soap, Context, SY_MD5_INIT, NULL, 0);
    SyMd5Handler(soap, Context, SY_MD5_UPDATE, HA1hex, 32);
    SyMd5Handler(soap, Context, SY_MD5_UPDATE, ":", 1);
    SyMd5Handler(soap, Context, SY_MD5_UPDATE, Nonce==NULL?(char*)"":Nonce,
                 strlen(Nonce==NULL?(char*)"":Nonce));
    SyMd5Handler(soap, Context, SY_MD5_UPDATE, ":", 1);

    DPrint("Qop:%s\n", Qop);

    if (Qop && *Qop)
    {
        SyMd5Handler(soap, Context, SY_MD5_UPDATE, Ncount, strlen(Ncount));
        SyMd5Handler(soap, Context, SY_MD5_UPDATE, ":", 1);
        SyMd5Handler(soap, Context, SY_MD5_UPDATE, Cnonce, strlen(Cnonce));
        SyMd5Handler(soap, Context, SY_MD5_UPDATE, ":", 1);
        SyMd5Handler(soap, Context, SY_MD5_UPDATE, Qop, strlen(Qop));
        SyMd5Handler(soap, Context, SY_MD5_UPDATE, ":", 1);
    }

    SyMd5Handler(soap, Context, SY_MD5_UPDATE, TmpHa2Hex, 32);
    SyMd5Handler(soap, Context, SY_MD5_FINAL, ResponseHa, 0);
    soap_s2hex(soap, (unsigned char *)ResponseHa, Response, 16);

    DONE;

}

LOCAL void HttpNonceCalc(struct soap *soap, char nonce[21])
{
    static short Count = 0xCA53;
    //static short Count = 0x53CA;

    sprintf(nonce, "%8.8x%4.4hx%8.8x", (int)time(NULL), Count++, soap_random);
}

LOCAL void HttpOpaqueCalc(struct soap *soap, char opaque[9])
{
    sprintf(opaque, "%8.8x", soap_random);
}

LOCAL void HttpCnonceCalc(struct soap *soap, void** Context,
                            char* cNonce, char hex[33])
{
    char tmpHa[17] = {0};

    SyMd5Handler(soap, Context, SY_MD5_INIT, NULL, 0);
    SyMd5Handler(soap, Context, SY_MD5_UPDATE, cNonce, strlen(cNonce==NULL?(char*)"":cNonce));
    SyMd5Handler(soap, Context, SY_MD5_FINAL, tmpHa, 0);
    soap_s2hex(soap, (unsigned char *)tmpHa, hex, 16);
}

//aShaLaws = SerialNumber + Initvector + passwd + random
LOCAL int SyGetAesPassword(syHttpData *HttpData)
{
    int i = 0;
    int nLen = 0;
    char SN[24] = {0};
    char aPasswd[24] = {0};
    char aShaLaws[512] = {0};
    char aOutStr[512] = {0};
    char* pShaLaws = aShaLaws;

    memset(aShaLaws, 0x00, sizeof(aShaLaws));

    SyGetNodeValue("Device.DeviceInfo.SerialNumber", SN);
    nLen = strlen(SN);

    for (i = 0; i < nLen; i++)
    {
        if (SN[i] >= 'A' && SN[i] <= 'Z')
        {
            SN[i] += 32;
        }
    }

    DPrint("SN:%s\n", SN);

    strncpy(pShaLaws, SN, nLen);
    pShaLaws = pShaLaws + nLen;
    strncpy(pShaLaws, "12319999", 8); //99991231
    pShaLaws = pShaLaws + 8;
    SyGetNodeValue("Device.ManagementServer.Password", aPasswd);
    nLen = strlen(aPasswd);
    strncpy(pShaLaws, aPasswd, nLen);
    pShaLaws = pShaLaws + nLen;

    DPrint("Opaque:%s\n", HttpData->Opaque);
    if (NULL != HttpData->Opaque)
    {
        nLen = strlen(HttpData->Opaque);
        DPrint("OpaqueLen:%d\n", nLen);
        strncpy(pShaLaws, HttpData->Opaque, nLen);
    }

    SySha256((uchar*)aShaLaws, strlen(aShaLaws), \
             (uchar*)aOutStr);

    if (0 == strlen(aOutStr))
    {
        DPrint("aOutStr is NULL\n");
    }
    else
    {
        memset(gSyAespasswd, 0, sizeof(gSyAespasswd));
        strncpy(gSyAespasswd, aOutStr, sizeof(gSyAespasswd)-1);
    }

    return SY_SUCCESS;
}


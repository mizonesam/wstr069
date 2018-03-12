/***********************************************************************
*
* syCwmpHttp.h
*
* Copyright (C) 2013 Lin jieling <mycourage0@126.com>
* Copyright (C) 2003-2013 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/
#ifndef SYCWMPHTTP_H
#define SYCWMPHTTP_H

#define SY_HTTP_SESSION_TIMEOUT                 600

#define SY_HTTP_CONTINUE                        100
#define SY_HTTP_SWITCHING_PROTOCOLS             101
#define SY_HTTP_PROCESSING                      102
#define SY_HTTP_OK                              200
#define SY_HTTP_CREATED                         201
#define SY_HTTP_ACCEPTED                        202
#define SY_HTTP_NON_AUTHORITATIVE               203
#define SY_HTTP_NO_CONTENT                      204
#define SY_HTTP_RESET_CONTENT                   205
#define SY_HTTP_PARTIAL_CONTENT                 206
#define SY_HTTP_MULTI_STATUS                    207
#define SY_HTTP_ALREADY_REPORTED                208
#define SY_HTTP_IM_USED                         226
#define SY_HTTP_MULTIPLE_CHOICES                300
#define SY_HTTP_MOVED_PERMANENTLY               301
#define SY_HTTP_MOVED_TEMPORARILY               302
#define SY_HTTP_SEE_OTHER                       303
#define SY_HTTP_NOT_MODIFIED                    304
#define SY_HTTP_USE_PROXY                       305
#define SY_HTTP_TEMPORARY_REDIRECT              307
#define SY_HTTP_PERMANENT_REDIRECT              308
#define SY_HTTP_BAD_REQUEST                     400
#define SY_HTTP_UNAUTHORIZED                    401
#define SY_HTTP_PAYMENT_REQUIRED                402
#define SY_HTTP_FORBIDDEN                       403
#define SY_HTTP_NOT_FOUND                       404
#define SY_HTTP_METHOD_NOT_ALLOWED              405
#define SY_HTTP_NOT_ACCEPTABLE                  406
#define SY_HTTP_PROXY_AUTHENTICATION_REQUIRED   407
#define SY_HTTP_REQUEST_TIME_OUT                408
#define SY_HTTP_CONFLICT                        409
#define SY_HTTP_GONE                            410
#define SY_HTTP_LENGTH_REQUIRED                 411
#define SY_HTTP_PRECONDITION_FAILED             412
#define SY_HTTP_REQUEST_ENTITY_TOO_LARGE        413
#define SY_HTTP_REQUEST_URI_TOO_LARGE           414
#define SY_HTTP_UNSUPPORTED_MEDIA_TYPE          415
#define SY_HTTP_RANGE_NOT_SATISFIABLE           416
#define SY_HTTP_EXPECTATION_FAILED              417
#define SY_HTTP_UNPROCESSABLE_ENTITY            422
#define SY_HTTP_LOCKED                          423
#define SY_HTTP_FAILED_DEPENDENCY               424
#define SY_HTTP_UPGRADE_REQUIRED                426
#define SY_HTTP_PRECONDITION_REQUIRED           428
#define SY_HTTP_TOO_MANY_REQUESTS               429
#define SY_HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE 431
#define SY_HTTP_INTERNAL_SERVER_ERROR           500
#define SY_HTTP_NOT_IMPLEMENTED                 501
#define SY_HTTP_BAD_GATEWAY                     502
#define SY_HTTP_SERVICE_UNAVAILABLE             503
#define SY_HTTP_GATEWAY_TIME_OUT                504
#define SY_HTTP_VERSION_NOT_SUPPORTED           505
#define SY_HTTP_VARIANT_ALSO_VARIES             506
#define SY_HTTP_INSUFFICIENT_STORAGE            507
#define SY_HTTP_LOOP_DETECTED                   508
#define SY_HTTP_NOT_EXTENDED                    510
#define SY_HTTP_NETWORK_AUTHENTICATION_REQUIRED 511

typedef struct _syHttpHeaderInfo
{
    char* Username;
    char* Password;
    char* Realm;
    char* Nonce;
    char* Uri;
    char* Response;
    char* Algorithm;
    char* Opaque;
    char* Cnonce;
    char* Qop;
} syHttpHeaderInfo;

typedef struct _syHttpData
{
    char* Realm;
    char* Nonce;
    char* Uri;
    char* Response;
    char* Algorithm;
    char* Opaque;
    char* Cnonce;
    char* Ncount;
    char* Qop;
    unsigned long Nc;
    char  Digest[17];
    void* Context;
    int (*PrepareInitSendFunc)(struct soap*);
    int (*PrepareInitRecvFunc)(struct soap*);
    int (*DisconnectFunc)(struct soap*);
    int (*PostHeaderFunc)(struct soap*, const char*, const char*);
    int (*ParseHeaderFunc)(struct soap*, const char*, const char*);
    int (*PrepareSendFunc)(struct soap*, const char*, unsigned int);
    int (*PrepareRecvFunc)(struct soap*, const char*, unsigned int);
} syHttpData;

typedef struct _syHttpSessionStruct
{
    struct _syHttpSessionStruct* NextHttpSession;
    time_t Modified;
    char *Realm;
    char *Nonce;
    char *Opaque;
    unsigned long Nc;
} syHttpSessionStruct;


void HttpRestore(struct soap* soap, syHttpHeaderInfo* httpInfo);
void HttpRelease(struct soap* soap, syHttpHeaderInfo* httpInfo);
void HttpSave(struct soap* soap, syHttpHeaderInfo* httpInfo, const char* realm,
                const char* userId, const char* password);
int HttpPostVerify(struct soap* soap, char* password);
int HttpGetVerify(struct soap* soap, char* password);
int HttpGet(struct soap *soap);
int HttpD(struct soap* soap, struct soap_plugin* plugin, void* arg);
#endif

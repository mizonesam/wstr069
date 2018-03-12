/***********************************************************************
*
* syCwmpClient.c
*
* Copyright (C) 2012 Lin jieling <mycourage0@126.com>
* Copyright (C) 2003-2012 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/
#include "soapStub.h"
#include "syCwmpCommon.h"
#include "cwmp.nsmap"

//extern int g_msg2cmdProc_fd;

int GetRPCMethods(const char *server, void *req, struct cwmp__GetRPCMethodsResponse *res)
{
    DPrint("--->\n");
    int i;
    int size = 0;
    int result = 0;
    struct soap soap;

    soap_init(&soap);
    soap_set_namespaces(&soap, namespaces);
    DPrint("soap 0\n");
    soap.header = (struct SOAP_ENV__Header *)malloc(sizeof(struct SOAP_ENV__Header));
    soap.header->cwmp__ID = (char*)malloc(64);
    DPrint("soap 1\n");
    memcpy(soap.header->cwmp__ID, "12345", strlen("12345"));
    DPrint("soap 2\n");

    soap_call_cwmp__GetRPCMethods(&soap, server, NULL, NULL, res);
    size = res->__ptrMethodList.__size;
    for(i = 0; i < size; i++)
    {
        DPrint("element%d : %s\n", i, res->__ptrMethodList.__ptrstring[i]);
    }
    free(soap.header->cwmp__ID);
    free(soap.header);
    if (soap.error)
    {
        DPrint("soap error: %d, %s, %s\n", soap.error,
               *soap_faultcode(&soap), *soap_faultstring(&soap));
        result = soap.error;
    }
    soap_end(&soap);
    soap_done(&soap);
    return result;
}

int GetParameterValues(const char *server, struct ParameterNames *ParameterNames,
                       struct cwmp__GetParameterValuesResponse *res)
{
    DPrint("--->\n");
    int i;
    int size = 0;
    int result = 0;
    struct soap soap;
    struct cwmp__ParameterValueStruct* paraValStruct = NULL;


    soap_init(&soap);
    soap_set_namespaces(&soap, namespaces);
    soap_call_cwmp__GetParameterValues(&soap, server, NULL, ParameterNames, res);
    size = res->ParameterList->__size;
    DPrint("size=%d\n", size);
    for(i = 0; i < size; i++)
    {
        paraValStruct = res->ParameterList->__ptrParameterValueStruct[i];
        DPrint("element%d : name=%s, type=%s, value=%s\n", i,
               paraValStruct->Name,
               paraValStruct->Type,
               paraValStruct->Value);
    }

    if (soap.error)
    {
        DPrint("soap error: %d, %s, %s\n", soap.error,
               *soap_faultcode(&soap), *soap_faultstring(&soap));
        result = soap.error;
    }
    soap_end(&soap);
    soap_done(&soap);
    return result;
}

int SetParameterValues(const char *server,
                       struct ParameterValueList *ParameterList,
                       char *ParameterKey,
                       struct cwmp__SetParameterValuesResponse *res)
{
    DPrint("--->\n");
    int result = 0;
    struct soap soap;

    soap_init(&soap);
    soap_set_namespaces(&soap, namespaces);
    soap_call_cwmp__SetParameterValues(&soap, server, NULL, ParameterList, ParameterKey, res);

    DPrint("status=%d\n", res->Status);

    if (soap.error)
    {
        DPrint("soap error: %d, %s, %s\n", soap.error,
               *soap_faultcode(&soap), *soap_faultstring(&soap));
        result = soap.error;
    }
    soap_end(&soap);
    soap_done(&soap);
    return result;
}

int GetParameterNames(const char *server,
                      char **ParameterPath,
                      char *NextLevel,
                      struct cwmp__GetParameterNamesResponse *res)
{
    DPrint("--->\n");
    int result = 0;
    struct soap soap;

    soap_init(&soap);
    soap_set_namespaces(&soap, namespaces);

    soap_call_cwmp__GetParameterNames(&soap, server, NULL, ParameterPath, NextLevel, res);

    if (soap.error)
    {
        DPrint("soap error: %d, %s, %s\n", soap.error,
               *soap_faultcode(&soap), *soap_faultstring(&soap));
        result = soap.error;
    }
    soap_end(&soap);
    soap_done(&soap);
    return result;
}

int  Reboot(const char *server,
            char *CommandKey,
            struct cwmp__RebootResponse *res)
{
    DPrint("--->\n");
    int result = 0;
    struct soap soap;

    soap_init(&soap);
    soap_set_namespaces(&soap, namespaces);
    soap_call_cwmp__Reboot(&soap, server, NULL, CommandKey, res);

    if (soap.error)
    {
        DPrint("soap error: %d, %s, %s\n", soap.error,
               *soap_faultcode(&soap), *soap_faultstring(&soap));
        result = soap.error;
    }
    soap_end(&soap);
    soap_done(&soap);
    return result;
}

int Upload(const char *server,
           char *CommandKey,
           char *FileType,
           char *URL,
           char *Username,
           char *Password,
           int DelaySeconds,
           struct cwmp__UploadResponse *res)
{
    DPrint("--->\n");
    int result = 0;
    struct soap soap;

    soap_init(&soap);
    soap_set_namespaces(&soap, namespaces);
    soap_call_cwmp__Upload(&soap, server, NULL, CommandKey, FileType,
                           URL, Username, Password, DelaySeconds, res);

    if (soap.error)
    {
        DPrint("soap error: %d, %s, %s\n", soap.error,
               *soap_faultcode(&soap), *soap_faultstring(&soap));
        result = soap.error;
    }
    soap_end(&soap);
    soap_done(&soap);
    return result;
}



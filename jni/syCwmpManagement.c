/***********************************************************************
*
* syCwmpManagement.c
*
* Copyright (C) 2013 Lin jieling <mycourage0@126.com>
* Copyright (C) 2003-2013 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/
#include <pthread.h>
#include <math.h>
#include <jni.h>


#include "cwmp.nsmap"
#include "soapStub.h"
#include "syCwmpHttp.h"
#include "syCwmpFault.h"
#include "syCwmpCommon.h"
#include "syCwmpSocket.h"
#include "syCwmpUtil.h"
#include "syCwmpManagement.h"
#include "syCwmpTaskQueue.h"
#include "eventParse.h"
#include "paramParse.h"

extern int StartupInfoFlag;
extern int zeroConfigFlag;
extern int gSyUpdateSuccess;
extern int gSyChangeNetMode;
extern int syCwmpSession;
extern int gSyIsTmStart;

int syNATCmd;
int gSyGetting = 0;
int gSyIsFirst = 1;
int gSyIsCPEStart = 0;
int gSyIsFirstHeartBeat = 1;
int gSyHeartInterval = 360;
int syEPGmodifySupported = 0;
int gSyIsZeroConfig = 0;

/* sendInform标志，如果成功1，失败0，默认0,强制切换到调度2 */
int gSendInfomResult = 0;

time_t gSyCPEStartTime = 0;
pthread_t gSockThreadId;
pthread_t gClientSockThreadId;
pthread_t gCwmpProcessThreadId;

syHttpHeaderInfo            gSyHttpHeaderInfo = {0};
syDeviceInfoStruct          gSyDeviceInfoStu = {0};
syManagementServerStruct    gSyManagementServerStu = {0};
syLANStruct                 gSyLANStu = {0};
syTimeStruct                gSyTimeStu = {0};
syServiceInfoStruct         gSyServiceInfoStu = {0};
syGlobalParmStruct          gsyGlobalParmStru = {0};
syAcsCpeParamStruct         gsyAcsCpeParamStru = {0};


extern int NATDetected;
extern int gSyNatSession;

struct _EventStruct*            gSyEvent;
struct _DeviceIdStruct          gSyDeviceId;
struct _ParameterValueStruct*   gSyParamList = NULL;
struct soap* gSySoap = NULL;
struct soap* gSyClSoap = NULL;
struct _cwmp__Fault*   gSyCwmpFault = NULL;

int gSyErrorCodeNum = 0;
int gSyErrorCodeSwitch = 0;
int gSyErrorCodeInterval = 0;
syErrorCodeStu gSyErrorCodeArr[20] = {0};
#if 0
char* syInformEventString[] =
{
    "0 BOOTSTRAP",
    "1 BOOT",
    "2 PERIODIC",
    "4 VALUE CHANGE",
    "6 CONNECTION REQUEST",
    "7 TRANSFER COMPLETE",
    "8 DIAGNOSTICS COMPLETE",
    "9 REQUEST DOWNLOAD",
    "10 AUTONOMOUS TRANSFER COMPLETE",
    "M Reboot",
    "M Download",
    "M Upload",
    "M X_CTC_SHUT_DOWN",
    "M CTC LOG_PERIODIC",
    "X CTC ErrorCode",

};

char* syInformEventCommandKey[] =
{
    "BOOTSTRAP",
    "BOOT",
    "PERIODIC",
    "VALUE CHANGE",
    "CONNECTION REQUEST",
    "TRANSFER COMPLETE",

#ifdef SY_GD_TR069
    "DIAGNOSTIC COMPLETE"
#else
    "DIAGNOSTICS COMPLETE",
#endif

    "REQUEST DOWNLOAD",
    "AUTONOMOUS TRANSFER COMPLETE",
    "Reboot",
    "Download",
    "Upload",
    "X_CTC_SHUT_DOWN",
    "CTC LOG_PERIODIC",
    "X CTC ErrorCode",

};
#endif
EVENT_MOD_t gSendEvent[16] = {
    {SY_EVENT_BOOTSTRAP,							"0 BOOTSTRAP",							""},
    {SY_EVENT_BOOT,									"1 BOOT", 								""},
    {SY_EVENT_PERIODIC,								"2 PERIODIC",							""},
    {SY_EVENT_VALUE_CHANGE,							"4 VALUE CHANGE",						""},
    {SY_EVENT_CONNECTION_REQUEST,					"6 CONNECTION REQUEST",					""},
    {SY_EVENT_TRANSFER_COMPLETE,					"7 TRANSFER COMPLETE",					""},
    {SY_EVENT_DIAGNOSTICS_COMPLETE,					"8 DIAGNOSTICS COMPLETE",				""},
    {SY_EVENT_REQUEST_DOWNLOAD,						"9 REQUEST DOWNLOAD",					""},
    {SY_EVENT_AUTONOMOUS_TRANSFER_COMPLETE,			"10 AUTONOMOUS TRANSFER COMPLETE",		""},
    {SY_EVENT_REBOOT,								"M Reboot",					            ""},
    {SY_EVENT_DOWNLOAD,								"M Download",							""},
    {SY_EVENT_UPLOAD,								"M Upload",								""},
    {SY_EVENT_X_CTC_SHUT_DOWN,						"M X_CTC_SHUT_DOWN",					""},
    {SY_EVENT_CTC_LOG_PERIODIC,						"M CTC LOG_PERIODIC",					""},
    {SY_EVENT_X_00E0FC_ErrorCode,					"X CTC ErrorCode",						""},
};

char* syInformParameters[] =
{
    "Device.DeviceSummary",
    "Device.DeviceInfo.ModelName",
    "Device.DeviceInfo.Description",
    "Device.DeviceInfo.UpTime",
    "Device.DeviceInfo.FirstUseDate",
    "Device.DeviceInfo.HardwareVersion",

    "Device.DeviceInfo.SoftwareVersion",

    "Device.DeviceInfo.AdditionalHardwareVersion",
    "Device.DeviceInfo.AdditionalSoftwareVersion",
    "Device.ManagementServer.ConnectionRequestURL",
    "Device.ManagementServer.ParameterKey",
    "Device.LAN.IPAddress",
    "Device.LAN.MACAddress",
    "Device.X_00E0FC.STBID",
    "Device.X_00E0FC.ServiceInfo.UserID",
    "Device.X_00E0FC.ServiceInfo.AuthURL",
    "Device.X_00E0FC.ServiceInfo.PPPoEID",
    "Device.X_00E0FC.ServiceInfo.IsEncryptMark",

};


void* Malloc(struct soap *soap, int size)
{
    void *p = NULL;

    if(size > 0) {
        p = (char *)calloc(size, 1);
    }
    return p;
}

char *Strdup(struct soap *soap, char *s)
{
    char *t = NULL;
    if(s && (t = (char *)Malloc(soap, strlen(s) + 1)))
    {
        strcpy(t, s);
        t[strlen(t)] = '\0';
        return t;
    }
    return strdup("ERROR:Strdup invalid argvment OR Memory alloc failed.");
}

LOCAL int GetInformNum(const char* FilePath)
{
    DO;
    int nNum = 0;
    char tmpBuf[256];
    char *pStr = tmpBuf;
    FILE *fp = NULL;

    if(!FilePath) return 0;
    fp = fopen(FilePath, "r");
    if (fp != NULL)
    {
        memset(tmpBuf, 0x00, sizeof(tmpBuf));
        if (fgets(pStr, sizeof(tmpBuf), fp) != NULL)
        {
            while(strstr(pStr, "Device.") != NULL)
            {
                nNum++;
                memset(tmpBuf, 0x00, sizeof(tmpBuf));
                fgets(pStr, sizeof(tmpBuf), fp);
            }
        }
        fclose(fp);
    }
    DONE;
    return nNum;
}

int readUptime()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (int)ts.tv_sec;
}

/* ONLY CALL BY MallocParaList*/
LOCAL int AddIfmParam(struct soap* soap, const char* flagFile)
{
    char* pName = NULL ;
    char* pValue = NULL ;
    char name[256] = {0};
    char value[256] = {0};
    char tmpBuf[256];
    char tmpStr[1024] = {0};
    char tmpType[256] = {0};
    char *pStr = tmpBuf;
    FILE *fp = NULL ;

    DO;
    memset(tmpType, 0, sizeof(tmpType));
    memset(tmpStr, 0, sizeof(tmpStr));

    DPrint("flagFile:%s\n", flagFile);
    fp = fopen(flagFile, "r");
    if (fp != NULL)
    {
        memset(tmpBuf, 0x00, sizeof(tmpBuf));
        if(fgets(pStr, sizeof(tmpBuf), fp) != NULL)
        {
            while(strstr(pStr, "Device.") != NULL)
            {
                pName = strtok(pStr, "=");
                pValue = strtok(NULL, "=");
                memset(name, 0x00, sizeof(name));
                memset(value, 0x00, sizeof(value));
                snprintf(name, sizeof(name), "%s", pName);
                snprintf(value, sizeof(value), "%s", pValue);

                if ((0X0D == value[strlen(value) - 2]) && (0X0A == value[strlen(value) - 1]))
                {
                    value[strlen(value) - 2] = '\0';
                }
                else
                {
                    value[strlen(value) - 1] = '\0';
                }

                DPrint("name:%s, value:%s\n", name, value);

                if (0 == strcmp(name, "Device.LAN.IPAddress"))
                {
                	if (NULL != gSyParamList->__ptrParameterValueStruct[SY_INFORM_IPADDRESS].Value)
                    {
                        free(gSyParamList->__ptrParameterValueStruct[SY_INFORM_IPADDRESS].Value);
						gSyParamList->__ptrParameterValueStruct[SY_INFORM_IPADDRESS].Value = NULL;
                    }
					gSyParamList->__ptrParameterValueStruct[SY_INFORM_IPADDRESS].Value = Strdup(soap, value);
                }
				
            	SyGetNodeType(name, tmpType, "string");
                snprintf(tmpStr, sizeof(tmpStr), "%s:%s", "xsd", tmpType);
                gSyParamList->__ptrParameterValueStruct[gSyParamList->__size].Name = Strdup(soap, name);
                gSyParamList->__ptrParameterValueStruct[gSyParamList->__size].Value = Strdup(soap, value);
                gSyParamList->__ptrParameterValueStruct[gSyParamList->__size].Type = Strdup(soap, tmpStr);
                gSyParamList->__size++;
				
                memset(tmpBuf, 0x00, sizeof(tmpBuf));
                fgets(pStr, sizeof(tmpBuf), fp);
            }
        }
        fclose(fp) ;
    }
    DONE;
    return SY_SUCCESS;
}

LOCAL int FreeFault(void)
{
    int i;

    if (gSyCwmpFault->SetParameterValuesFault != NULL)
    {
        for (i = 0; i < gSyCwmpFault->__sizeSetParameterValuesFault; i++)
        {
            if (gSyCwmpFault->SetParameterValuesFault[i].ParameterName != NULL)
            {
                free(gSyCwmpFault->SetParameterValuesFault[i].ParameterName);
                gSyCwmpFault->SetParameterValuesFault[i].ParameterName = NULL;
            }
            if (gSyCwmpFault->SetParameterValuesFault[i].FaultCode != NULL)
            {
                free(gSyCwmpFault->SetParameterValuesFault[i].FaultCode);
                gSyCwmpFault->SetParameterValuesFault[i].FaultCode = NULL;
            }
            if (gSyCwmpFault->SetParameterValuesFault[i].FaultString != NULL)
            {
                free(gSyCwmpFault->SetParameterValuesFault[i].FaultString);
                gSyCwmpFault->SetParameterValuesFault[i].FaultString = NULL;
            }
        }
        free(gSyCwmpFault->SetParameterValuesFault);
        gSyCwmpFault->SetParameterValuesFault = NULL;
    }
    gSyCwmpFault->__sizeSetParameterValuesFault = 0;
    if (gSyCwmpFault->FaultCode != NULL)
    {
        free(gSyCwmpFault->FaultCode);
        gSyCwmpFault->FaultCode = NULL;
    }
    if (gSyCwmpFault->FaultString != NULL)
    {
        free(gSyCwmpFault->FaultString);
        gSyCwmpFault->FaultString = NULL;
    }

    return SY_SUCCESS;
}

LOCAL int FreeParamList(void)
{
    /* void free(void *ptr);
     * The free() function frees the memory space pointed to by ptr,
     * which must have been returned by a previous call to malloc(),
     * calloc() or realloc().
     * Otherwise, or if free(ptr) has already been called before,
     * undefined behavior occurs.
     * If ptr is NULL, no operation is performed.
     */
    DO;
    int i = 0;

    if (gSyParamList != NULL) {
    
        if (gSyParamList->__ptrParameterValueStruct != NULL) {
        
            for (i = 0; i < gSyParamList->__size; i++) {
                free(gSyParamList->__ptrParameterValueStruct[i].Name);
                gSyParamList->__ptrParameterValueStruct[i].Name = NULL;
                free(gSyParamList->__ptrParameterValueStruct[i].Type);
                gSyParamList->__ptrParameterValueStruct[i].Type = NULL;
                free(gSyParamList->__ptrParameterValueStruct[i].Value);
                gSyParamList->__ptrParameterValueStruct[i].Value = NULL;
            }
            free(gSyParamList->__ptrParameterValueStruct);
            gSyParamList->__ptrParameterValueStruct = NULL;
        }
        free(gSyParamList);
        gSyParamList = NULL;
    }

    DONE;
    return SY_SUCCESS;
}

extern lua_State* luaVM;

LOCAL int MallocParaList(struct soap* soap,
                            int type,
                            const char* flagFile1,
                            const char* flagFile2,
                            const char* flagFile3)
{
    int i = 0;
    int nSize = sizeof(syInformParameters)/sizeof(char*);
    int informSize1 = 0;
    int informSize2 = 0;
    int informSize3 = 0;
    char tmpStr[1024] = {0};
    char tmpType[256] = {0};

    DO;

    memset(tmpType, 0, sizeof(tmpType));
    memset(tmpStr, 0, sizeof(tmpStr));

    FreeParamList();

    if (NULL != gSyParamList)
    {
        DPrint("free paramList error\n");
        gSyParamList = NULL;
    }

    if (flagFile1 != NULL)
    {
        informSize1 = GetInformNum(flagFile1);
    }
    if (flagFile2 != NULL)
    {
        informSize2 = GetInformNum(flagFile2);
    }
    if (flagFile3 != NULL)
    {
        informSize3 = GetInformNum(flagFile3);
    }
#ifdef SY_TEST
	DPrint("event:%s\n", gSendEvent[type].EventStr);
	xml_event_list_t *tEventDataPt = getEvent(gSendEvent[type].EventStr);
	nSize = tEventDataPt->size;
#endif
    gSyParamList = (struct _ParameterValueStruct*)Malloc(soap, sizeof(struct _ParameterValueStruct));
    if (NULL != gSyParamList)
    {
        memset(gSyParamList, 0x00, sizeof(struct _ParameterValueStruct));
    }
    int length = (nSize + informSize1 + informSize2 + informSize3) * sizeof(struct cwmp__ParameterValueStruct);
    gSyParamList->__ptrParameterValueStruct = (struct cwmp__ParameterValueStruct *)Malloc(soap, length);
    if (NULL != gSyParamList->__ptrParameterValueStruct)
    {
        memset(gSyParamList->__ptrParameterValueStruct, 0x00, length);
    }
    gSyParamList->__size = nSize;
#ifndef SY_TEST
    for (i = 0; i < nSize; i++)
    {
        gSyParamList->__ptrParameterValueStruct[i].Name = Strdup(soap, syInformParameters[i]);
        //DPrint("name:%s\n", gSyParamList->__ptrParameterValueStruct[i].Name);
        SyGetNodeType(syInformParameters[i], tmpType, "string");
        snprintf(tmpStr, sizeof(tmpStr), "%s:%s", "xsd", tmpType);
        
        gSyParamList->__ptrParameterValueStruct[i].Type = Strdup(soap, tmpStr);
        //DPrint("type:%s\n", gSyParamList->__ptrParameterValueStruct[i].Type);
    }

    DPrint("gSyDeviceInfoStu.DeviceSummary:%s\n", gSyDeviceInfoStu.DeviceSummary);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_DEVICE_SUMMARY].Value =
        Strdup(soap, gSyDeviceInfoStu.DeviceSummary);
    DPrint("gSyDeviceInfoStu.ModelName:%s\n", gSyDeviceInfoStu.ModelName);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_MODE_NAME].Value =
        Strdup(soap, gSyDeviceInfoStu.ModelName);
    DPrint("gSyDeviceInfoStu.Description:%s\n", gSyDeviceInfoStu.Description);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_DESCRIPTION].Value =
        Strdup(soap, gSyDeviceInfoStu.Description);
    DPrint("gSyDeviceInfoStu.UpTime:%s\n", gSyDeviceInfoStu.UpTime);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_UPTIME].Value =
        Strdup(soap, gSyDeviceInfoStu.UpTime);
    DPrint("gSyDeviceInfoStu.FirstUseDate:%s\n", gSyDeviceInfoStu.FirstUseDate);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_FIRST_USEDATE].Value =
        Strdup(soap, gSyDeviceInfoStu.FirstUseDate);
    DPrint("gSyDeviceInfoStu.HardwareVersion:%s\n", gSyDeviceInfoStu.HardwareVersion);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_HARDWARE_VERSION].Value =
        Strdup(soap, gSyDeviceInfoStu.HardwareVersion);
    DPrint("gSyDeviceInfoStu.SoftwareVersion:%s\n", gSyDeviceInfoStu.SoftwareVersion);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_SOFTWARE_VERSION].Value =
        Strdup(soap, gSyDeviceInfoStu.SoftwareVersion);//"V71811104"
    DPrint("gSyDeviceInfoStu.AdditionalHardwareVersion:%s\n", gSyDeviceInfoStu.AdditionalHardwareVersion);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_ADDHARDWARE_VERSION].Value =
        Strdup(soap, gSyDeviceInfoStu.AdditionalHardwareVersion);
    DPrint("gSyDeviceInfoStu.AdditionalSoftwareVersion:%s\n", gSyDeviceInfoStu.AdditionalSoftwareVersion);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_ADDSOFTWARE_VERSION].Value =
        Strdup(soap, gSyDeviceInfoStu.AdditionalSoftwareVersion);
    DPrint("gSyManagementServerStu.ConnectionRequestURL:%s\n", gSyManagementServerStu.ConnectionRequestURL);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_CONNECTION_REQUEST_URL].Value =
        Strdup(soap, gSyManagementServerStu.ConnectionRequestURL);
    DPrint("gSyManagementServerStu.ParameterKey:%s\n", gSyManagementServerStu.ParameterKey);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_PARAMETER_KEY].Value =
        Strdup(soap, gSyManagementServerStu.ParameterKey);
    DPrint("gSyLANStu.IPAddress:%s\n", gSyLANStu.IPAddress);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_IPADDRESS].Value =
        Strdup(soap, gSyLANStu.IPAddress);
    DPrint("gSyLANStu.MACAddress:%s\n", gSyLANStu.MACAddress);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_MACAddress].Value =
        Strdup(soap, gSyLANStu.MACAddress);
    DPrint("gSyServiceInfoStu.STBID:%s\n", gSyServiceInfoStu.STBID);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_STBID].Value =
        Strdup(soap, gSyServiceInfoStu.STBID);
    DPrint("gSyServiceInfoStu.UserID:%s\n", gSyServiceInfoStu.UserID);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_USER_ID].Value =
        Strdup(soap, gSyServiceInfoStu.UserID);//"0571000465"
    //DPrint("gSyServiceInfoStu.UserID:%s\n", gSyServiceInfoStu.UserID);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_AUTHURL].Value =
        Strdup(soap, gSyServiceInfoStu.AuthURL);
    DPrint("gSyServiceInfoStu.PPPoEID:%s\n", gSyServiceInfoStu.PPPoEID);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_PPPOE_ID].Value =
        Strdup(soap, gSyServiceInfoStu.PPPoEID);
    //DPrint("gSyServiceInfoStu.PPPoEID:%s\n", gSyServiceInfoStu.PPPoEID);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_ISENCRYMARK].Value =
        Strdup(soap, "1");

    /*
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_PROVISIONINGCODE].Value =
            Strdup(soap, "1");
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_NTPSERVER1].Value =
            Strdup(soap, gSyTimeStu.NTPServer1);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_DNSSERVERS].Value =
            Strdup(soap, gSyLANStu.DNSServers);
    gSyParamList->__ptrParameterValueStruct[SY_INFORM_ADDRESSINGTYPE].Value =
            Strdup(soap, gSyLANStu.AddressingType);
    */
#else
	for(i = 0; i < nSize; i++){
		gSyParamList->__ptrParameterValueStruct[i].Name = Strdup(soap, tEventDataPt->paramName[i]);
		
		SyGetNodeType(tEventDataPt->paramName[i], tmpType, "string");
        snprintf(tmpStr, sizeof(tmpStr), "%s:%s", "xsd", tmpType);        
        gSyParamList->__ptrParameterValueStruct[i].Type = Strdup(soap, tmpStr);

		char value[256] = {0};
		xml_key_path_t tData;
		if(getParamForNode(tEventDataPt->paramName[i], &tData)){
			if(!GetValueToTM(tData.keyname, value, sizeof(value)))
				SyGetNodeValue(tEventDataPt->paramName[i], value);
		}
		gSyParamList->__ptrParameterValueStruct[i].Value = Strdup(soap, value);
		DPrint("name:%s,value:%s\n", tEventDataPt->paramName[i], value);

		free(tEventDataPt->paramName[i]);
	}

	free(tEventDataPt->paramName);
	free(tEventDataPt);
	
#endif

#ifdef SUPPORT_LUA
	callLuaFunc(luaVM, "updateParam", "p>", gSyParamList);
#endif

    if(flagFile1 != NULL)
    {
        DPrint("flagFile1 = %s\n",flagFile1);
        AddIfmParam(soap, flagFile1);
    }

    if(flagFile2 != NULL)
    {
        DPrint("flagFile2 = %s\n",flagFile2);
        AddIfmParam(soap, flagFile2);
    }

    if(flagFile3 != NULL)
    {
        DPrint("flagFile3 = %s\n",flagFile3);
        AddIfmParam(soap, flagFile3);
    }

    DONE;
    return SY_SUCCESS;
}

LOCAL int ValueChanged(struct soap* soap, void* handle)
{

    MallocParaList(soap,
                      (int)handle,
                      SY_VALUE_CHANGE_INFORM_FLAG_0,
                      SY_VALUE_CHANGE_INFORM_FLAG_1,
                      SY_VALUE_CHANGE_INFORM_FLAG_2
                     );
    return SY_SUCCESS;
}

LOCAL int Periodic(struct soap* soap, void* handle)
{
    MallocParaList(soap, (int)handle, NULL, NULL, NULL);
    return SY_SUCCESS;
}
LOCAL int LogMsgPeriod(struct soap* soap, void* handle)
{
    MallocParaList(soap, (int)handle, SY_LOG_PERIODIC_INFORM_FLAG, NULL, NULL);
    return SY_SUCCESS;
}
LOCAL int Diagnostics(struct soap* soap, void* handle)
{
    MallocParaList(soap, (int)handle, NULL, NULL, NULL);
    return SY_SUCCESS;
}

LOCAL int syErrorCode(struct soap* soap, void* handle)
{
    MallocParaList(soap, (int)handle, SY_ERRORCODEENCODETMP_FLAG, NULL, NULL);
    return SY_SUCCESS;
}


LOCAL int Inform(struct soap* soap, void* handle)
{
    MallocParaList(soap, (int)handle, "./syInform1", "./syInform2", NULL);
    return SY_SUCCESS;
}

LOCAL char* GetNewReqId(struct soap* soap)
{
    char *pSzRequestId = NULL;

    pSzRequestId = (char *)soap_malloc(soap, SY_CPE_REQUEST_ID_LENGTH);
    if(NULL != pSzRequestId)
    {
        sprintf(pSzRequestId, "%u", gsyGlobalParmStru.CurrentRequestId++);
    }
    return pSzRequestId;
}

LOCAL void CreateInformEvt(struct soap* soap,  void* handle)
{
    int nType = 0;
    int eventNum = 1;
    char tmpBuf[512] = {0};

    DO;

    nType = (int)handle;
    DPrint("event type:%d\n", nType);

    gSyEvent = (struct _EventStruct*)malloc(sizeof(struct _EventStruct));
	
    eventNum = 1;
    if (SY_EVENT_BOOT == nType)
    {
        if (1 == gsyAcsCpeParamStru.HaveReboot)
        {
            eventNum = 1;
            gSyEvent->__ptrEventStruct = (struct cwmp__EventStruct*)
                                         malloc(eventNum * sizeof(struct cwmp__EventStruct));
            gSyEvent->__ptrEventStruct[0].EventCode =
                soap_strdup(soap, gSendEvent[nType].EventStr);
            gSyEvent->__ptrEventStruct[0].CommandKey =
                soap_strdup(soap, gSendEvent[nType].CommandKey);
            /*
            gSyEvent->__ptrEventStruct[1].EventCode =
                 soap_strdup(soap, syInformEventString[SY_EVENT_CONNECTION_REQUEST]);
            gSyEvent->__ptrEventStruct[1].CommandKey =
                 soap_strdup(soap, syInformEventCommandKey[SY_EVENT_CONNECTION_REQUEST]);
            gSyEvent->__ptrEventStruct[2].EventCode =
                 soap_strdup(soap, syInformEventString[SY_EVENT_REBOOT]);
            gSyEvent->__ptrEventStruct[2].CommandKey =
                 soap_strdup(soap, syInformEventCommandKey[SY_EVENT_REBOOT]);
            gSyEvent->__ptrEventStruct[3].EventCode =
                 soap_strdup(soap, syInformEventString[SY_EVENT_VALUE_CHANGE]);
            gSyEvent->__ptrEventStruct[3].CommandKey =
                 soap_strdup(soap, syInformEventCommandKey[SY_EVENT_VALUE_CHANGE]);
            */
        }
        else
        {
            if (SY_ACS_UPDATE_SUCCESS == gsyAcsCpeParamStru.IsAcsContacted)
            {
                eventNum = 1;
                gSyEvent->__ptrEventStruct = (struct cwmp__EventStruct*)
                                             malloc(eventNum * sizeof(struct cwmp__EventStruct));
                gSyEvent->__ptrEventStruct[0].EventCode =
                    soap_strdup(soap, gSendEvent[nType].EventStr);
                gSyEvent->__ptrEventStruct[0].CommandKey =
                    soap_strdup(soap, gSendEvent[nType].CommandKey);
                /*
                gSyEvent->__ptrEventStruct[1].EventCode =
                     soap_strdup(soap, syInformEventString[SY_EVENT_VALUE_CHANGE]);
                gSyEvent->__ptrEventStruct[1].CommandKey =
                     soap_strdup(soap, syInformEventCommandKey[SY_EVENT_VALUE_CHANGE]);
                gSyEvent->__ptrEventStruct[2].EventCode =
                     soap_strdup(soap, syInformEventString[SY_EVENT_TRANSFER_COMPLETE]);
                gSyEvent->__ptrEventStruct[2].CommandKey =
                     soap_strdup(soap, syInformEventCommandKey[SY_EVENT_TRANSFER_COMPLETE]);
                gsyGlobalParmStru.SessionFlags = SY_STATE_DOWNLOAD_FIRMWARE_COMPLETE; //hitler
                */
            }
            else
            {
                eventNum = 1;
                gSyEvent->__ptrEventStruct = (struct cwmp__EventStruct*)
                                             malloc(eventNum * sizeof(struct cwmp__EventStruct));
                gSyEvent->__ptrEventStruct[0].EventCode =
                    soap_strdup(soap, gSendEvent[nType].EventStr);
                gSyEvent->__ptrEventStruct[0].CommandKey =
                    soap_strdup(soap, gSendEvent[nType].CommandKey);
                /*
                gSyEvent->__ptrEventStruct[1].EventCode =
                     soap_strdup(soap, syInformEventString[SY_EVENT_VALUE_CHANGE]);
                gSyEvent->__ptrEventStruct[1].CommandKey =
                     soap_strdup(soap, syInformEventCommandKey[SY_EVENT_VALUE_CHANGE]);
                */
            }
        }
    }
    else if (SY_ACS_VALUE_CHANGED == gsyAcsCpeParamStru.IsAcsContacted)
    {
        eventNum = 3;
        gSyEvent->__ptrEventStruct = (struct cwmp__EventStruct*)
                                     malloc(eventNum * sizeof(struct cwmp__EventStruct));
        gSyEvent->__ptrEventStruct[0].EventCode =
            soap_strdup(soap, gSendEvent[nType].EventStr);
        gSyEvent->__ptrEventStruct[0].CommandKey =
            soap_strdup(soap, gSendEvent[nType].CommandKey);
        gSyEvent->__ptrEventStruct[1].EventCode =
            soap_strdup(soap, gSendEvent[SY_EVENT_CONNECTION_REQUEST].EventStr);
        gSyEvent->__ptrEventStruct[1].CommandKey =
            soap_strdup(soap, gSendEvent[SY_EVENT_CONNECTION_REQUEST].CommandKey);
        gSyEvent->__ptrEventStruct[2].EventCode =
            soap_strdup(soap, gSendEvent[SY_EVENT_BOOT].EventStr);
        gSyEvent->__ptrEventStruct[2].CommandKey =
            soap_strdup(soap, gSendEvent[SY_EVENT_BOOT].CommandKey);
    }
    else if (1 == gSyUpdateSuccess)
    {
        gSyUpdateSuccess = 0;
        eventNum = 2;
        gSyEvent->__ptrEventStruct = (struct cwmp__EventStruct*)
                                     malloc(eventNum * sizeof(struct cwmp__EventStruct));
        gSyEvent->__ptrEventStruct[0].EventCode =
            soap_strdup(soap, gSendEvent[nType].EventStr);
        gSyEvent->__ptrEventStruct[0].CommandKey =
            soap_strdup(soap, gSendEvent[nType].CommandKey);
        gSyEvent->__ptrEventStruct[1].EventCode =
            soap_strdup(soap, gSendEvent[SY_EVENT_VALUE_CHANGE].EventStr);
        gSyEvent->__ptrEventStruct[1].CommandKey =
            soap_strdup(soap, gSendEvent[SY_EVENT_VALUE_CHANGE].CommandKey);
    }
    else if (SY_EVENT_PERIODIC == nType)
    {
        memset(tmpBuf, 0x00, sizeof(tmpBuf));
        SyGetIPAddr(tmpBuf);
        DPrint("gSyLANStu.IPAddress:%s, ip:%s",
               gSyLANStu.IPAddress,
               tmpBuf);
        if ((0 != strlen(tmpBuf)) && (0 != strcmp(gSyLANStu.IPAddress, tmpBuf))) {
            if (gSyLANStu.IPAddress != NULL)
            {
                free(gSyLANStu.IPAddress);
                gSyLANStu.IPAddress = strdup(tmpBuf);
            }
            sprintf(tmpBuf, "http://%s:%d", gSyLANStu.IPAddress, SY_CWMP_CLIENT_PORT);
            if (gSyManagementServerStu.ConnectionRequestURL != NULL)
            {
                free(gSyManagementServerStu.ConnectionRequestURL);
            }
            gSyManagementServerStu.ConnectionRequestURL = strdup(tmpBuf);
            gSyEvent->__ptrEventStruct = (struct cwmp__EventStruct*)
                                         malloc(eventNum * sizeof(struct cwmp__EventStruct));
            gSyEvent->__ptrEventStruct[0].EventCode =
                soap_strdup(soap, gSendEvent[SY_EVENT_VALUE_CHANGE].EventStr);
            gSyEvent->__ptrEventStruct[0].CommandKey =
                soap_strdup(soap, gSendEvent[SY_EVENT_VALUE_CHANGE].CommandKey);
        }
        else {
            gSyEvent->__ptrEventStruct = (struct cwmp__EventStruct*)
                                         malloc(eventNum * sizeof(struct cwmp__EventStruct));
            gSyEvent->__ptrEventStruct[0].EventCode =
                soap_strdup(soap, gSendEvent[nType].EventStr);
            gSyEvent->__ptrEventStruct[0].CommandKey =
                soap_strdup(soap, gSendEvent[nType].CommandKey);
        }
    }
    else
    {
        gSyEvent->__ptrEventStruct = (struct cwmp__EventStruct*)
                                     malloc(eventNum * sizeof(struct cwmp__EventStruct));
        gSyEvent->__ptrEventStruct[0].EventCode =
            soap_strdup(soap, gSendEvent[nType].EventStr);
        gSyEvent->__ptrEventStruct[0].CommandKey =
            soap_strdup(soap, gSendEvent[nType].CommandKey);
    }
    gSyEvent->__size = eventNum;
#ifdef SUPPORT_LUA
	DPrint("enter Lua");
	callLuaFunc(luaVM, "updateEvent", "ipppp>", nType, gSyEvent, &gSyLANStu, &gSyManagementServerStu, &gsyAcsCpeParamStru);
	DPrint("after lua event num is %d", gSyEvent->__size);
#endif
    DONE;

}

LOCAL void CreateIfmPara(struct soap* soap,   void* handle)
{
    syInformEventType* eventType = (syInformEventType*)handle;

    DO;
    DPrint("eventType:%d\n", (int)eventType);

    switch ((int)eventType)
    {
    case SY_EVENT_BOOTSTRAP:
    case SY_EVENT_BOOT:
    case SY_EVENT_CONNECTION_REQUEST:
    case SY_EVENT_TRANSFER_COMPLETE:

        Inform(soap, handle);
        break;
    case SY_EVENT_CTC_LOG_PERIODIC:
        LogMsgPeriod(soap, handle);
        break;
    case SY_EVENT_PERIODIC:
        Periodic(soap, handle);
        break;
    case SY_EVENT_VALUE_CHANGE:
        ValueChanged(soap, handle);
        break;
    case SY_EVENT_DIAGNOSTICS_COMPLETE:
        Diagnostics(soap, handle);
        break;
    case SY_EVENT_REQUEST_DOWNLOAD:
        DPrint("SY_EVENT_REQUEST_DOWNLOAD\n");
        break;
    case SY_EVENT_AUTONOMOUS_TRANSFER_COMPLETE:
        DPrint("SY_EVENT_AUTONOMOUS_TRANSFER_COMPLETE\n");
        break;
    case SY_EVENT_REBOOT:
        DPrint("SY_EVENT_REBOOT\n");
        break;
    case SY_EVENT_DOWNLOAD:
        DPrint("SY_EVENT_DOWNLOAD\n");
        break;
    case SY_EVENT_UPLOAD:
        DPrint("SY_EVENT_UPLOAD\n");
        break;
    case SY_EVENT_X_CTC_SHUT_DOWN:
        DPrint("SY_EVENT_X_CTC_SHUT_DOWN\n");
        break;
    case SY_EVENT_X_00E0FC_ErrorCode:
        syErrorCode(soap, handle);
        break;

    default:
        break;
    }
#ifdef SY_TEST_DEMO
    DONE;
#endif
}

LOCAL void CreateInform(struct soap* soap, void* handle)
{   
    DO;
    soap->header = (struct SOAP_ENV__Header *)soap_malloc(soap, sizeof(struct SOAP_ENV__Header));
    if(soap->header) {
        memset(soap->header, 0, sizeof(struct SOAP_ENV__Header));
        soap->header->cwmp__ID = (char *)soap_malloc(soap, SY_CPE_REQUEST_ID_LENGTH);
        if(soap->header->cwmp__ID) {
            memset(soap->header->cwmp__ID, 0, SY_CPE_REQUEST_ID_LENGTH);
            sprintf(soap->header->cwmp__ID, "%u", gsyGlobalParmStru.CurrentRequestId++);
        }
        else {
            EPrint("Memory allocate soap->header->cwmp__ID failed.\n");
        }
    }
    else {
        EPrint("Memory allocate soap->header failed.\n");
    }
    CreateInformEvt(soap, handle);
    CreateIfmPara(soap, handle);
    DONE;
}

int UpdateNetPara(void)
{
	char pTmpBuf[SY_BUFFER_LENGTH] = {0};
    size_t sizeOfBuf = SY_BUFFER_LENGTH;

    DO;

	while (1){
		memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
		if (!GetValue("Device.LAN.IPAddress", pTmpBuf, sizeOfBuf))
        {
            SyGetIPAddr(pTmpBuf);
        }
		if (strlen(pTmpBuf) != 0 && strcmp(pTmpBuf, "0.0.0.0") != 0){
			break;
		}
		usleep(500 * 500);
	}
	if (gSyLANStu.IPAddress != NULL){
		free(gSyLANStu.IPAddress);
		gSyLANStu.IPAddress = NULL;
	}
	gSyLANStu.IPAddress = strdup(pTmpBuf);
	DPrint("ip:%s\n", gSyLANStu.IPAddress);

	if (!GetValue("Device.LAN.AddressingType", pTmpBuf, sizeOfBuf))
    {
        SyGetNodeValue("Device.LAN.AddressingType", pTmpBuf);
    }
	if (gSyLANStu.AddressingType != NULL){
		free(gSyLANStu.AddressingType);
		gSyLANStu.AddressingType = NULL;
	}
    gSyLANStu.AddressingType = strdup(pTmpBuf);
	DPrint("AddressingType:%s.\n", gSyLANStu.AddressingType);
	
	memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
	if (!GetValue("Device.LAN.DNSServers", pTmpBuf, sizeOfBuf))
	{
		SyGetNodeValue("Device.LAN.DNSServers", pTmpBuf);
	}
	if (gSyLANStu.DNSServers != NULL){
		free(gSyLANStu.DNSServers);
		gSyLANStu.DNSServers = NULL;
	}
	gSyLANStu.DNSServers = strdup(pTmpBuf);
	DPrint("DNSServers:%s.\n", gSyLANStu.DNSServers);

	memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
	sprintf(pTmpBuf, "http://%s:%d", gSyLANStu.IPAddress, SY_CWMP_CLIENT_PORT);
	if (gSyManagementServerStu.ConnectionRequestURL != NULL){
		free(gSyManagementServerStu.ConnectionRequestURL);
		gSyManagementServerStu.ConnectionRequestURL = NULL;
	}
	gSyManagementServerStu.ConnectionRequestURL = strdup(pTmpBuf);
	DPrint("connectURL:%s.\n", gSyManagementServerStu.ConnectionRequestURL);
}

int SendInform(struct soap* soap, void* handle)
{
    int   nRetryTime   = 0;
    int   failureTime  = 0;
    char* pCurrentTime = NULL;
    char  tmpUrl[SY_BUFFER_LENGTH] = {0};
    int   nReTryCount  = 0;
    int   backupUrlMode= 0;			//0:未启用BackupUrl 1:启用了BackupUrl
    struct cwmp__InformResponse InformResp;

    DO;

	/* 添加一个每次sendInform之前都先更新下网络信息 */
	UpdateNetPara();

    memset(tmpUrl, 0, SY_BUFFER_LENGTH);
    if (NULL == strstr(gsyAcsCpeParamStru.URL, "http"))
    {
        sprintf(tmpUrl, "http://%s:%d", gsyAcsCpeParamStru.URL, SY_CWMP_CLIENT_PORT);
    }
    else
    {
        sprintf(tmpUrl, "%s", gsyAcsCpeParamStru.URL);
    }
    DPrint("tmpUrl:%s\n", tmpUrl);
	
    CreateInform(soap, handle);
    VPrint("Size of ParamList = %d\n", gSyParamList->__size);

TryToSchedule:
		
    /* 如果是调度服务器地址，且事件不是1boot或者0boot，强制切换到1boot */
    if (strstr(gsyAcsCpeParamStru.URL, "125.88.86.16:37020") || strstr(gsyAcsCpeParamStru.URL, "125.88.86.54:37020"))
    {
        if ((int)handle != SY_EVENT_BOOTSTRAP && (int)handle != SY_EVENT_BOOT)
        {
            handle = (void *)SY_EVENT_BOOT;
            gSendInfomResult = 2;
        }
    }
	for(int i = 0; i<gSyEvent->__size; i++)
   	{
        VPrint("$eventCode:%s.$", gSyEvent->__ptrEventStruct[i].EventCode);
	}
    for (int i = 0; i<gSyParamList->__size; i++)
    {
        VPrint("%s -> %s", gSyParamList->__ptrParameterValueStruct[i].Name, gSyParamList->__ptrParameterValueStruct[i].Value);
    }
    VPrint("End of ParamList dump\n");
    if (0 == strcmp(gsyAcsCpeParamStru.AuthType, "Basic"))
    {
        soap->userid = soap_strdup(soap, gsyAcsCpeParamStru.Username);
        soap->passwd = soap_strdup(soap, gsyAcsCpeParamStru.Password);
    }
    DPrint("The progress step 1/2 of 'send Inform'.\n");

    soap->connect_timeout = 5;

TryAgain_restart:

    /* 盒子没有IP肯定是soap->error = 28.无效IP也是soap->error = 28.
     * 如果tmpUrl是一个ping不通的IP soap->error = 28.
     * 如果tmpUrl是一个没有被监听的端口 soap->error = 28 而不管其是否在连接时被RST过.
     * 只有在connect成功后 被RST才会 soap->error = -1.
     */
    if(NULL != gSyEvent)
        if(NULL != gSyEvent->__ptrEventStruct)
            if(NULL != gSyEvent->__ptrEventStruct->EventCode)
                DPrint("EVENT = %s.\n", gSyEvent->__ptrEventStruct->EventCode);
    pCurrentTime = GetCurrTime();

    /* 发送成功 */
    if (SOAP_OK == soap_call_cwmp__Inform(soap,
                                          tmpUrl,
                                          NULL,
                                          &gSyDeviceId,
                                          gSyEvent,
                                          SY_MAX_ENVELOPES,
                                          pCurrentTime,
                                          nRetryTime++,
                                          gSyParamList,
                                          &InformResp))
    {

        DPrint("Call Inform OK. handle = %d.\n", (int)handle);
        if (gSendInfomResult != 2)
        {
            gSendInfomResult = 1;
        }
        return SY_SUCCESS;

    }

    DPrint("soap->error = %d.\n", soap->error);


    if(SY_HTTP_UNAUTHORIZED == soap->error)
    {
        nRetryTime = 0;
        failureTime = 0;
        DPrint("Realm:%s, Username:%s, Password:%s\n",
               gsyAcsCpeParamStru.Realm,
               gsyAcsCpeParamStru.Username,
               gsyAcsCpeParamStru.Password);
        HttpSave(soap,
                   &gSyHttpHeaderInfo,
                   gsyAcsCpeParamStru.Realm,
                   gsyAcsCpeParamStru.Username,
                   gsyAcsCpeParamStru.Password);
        HttpRestore(soap, &gSyHttpHeaderInfo);
        // CreateIfmHdr(soap);
        DPrint("The progress step 2/2 of 'send Inform'.\n");
TryAgain:
        pCurrentTime = GetCurrTime();
        DPrint(" tmpUrl:%s\n", tmpUrl);
        if (SOAP_OK == soap_call_cwmp__Inform(soap,
                                              tmpUrl,
                                              NULL,
                                              &gSyDeviceId,
                                              gSyEvent,
                                              SY_MAX_ENVELOPES,
                                              pCurrentTime,
                                              nRetryTime++,
                                              gSyParamList,
                                              &InformResp))
        {
            soap_end(soap);
            DPrint("step 2/2 finish. Call Inform successful.\n");
            if (gSendInfomResult != 2)
            {
                gSendInfomResult = 1;
            }
            return SY_SUCCESS;
        }
        else
        {
            DPrint(" soap->error = %d.\n",  soap->error);
            if(-1 == soap->error || (SOAP_TCP_ERROR == soap->error))
            {
                /* 只重试九次,共执行十次 soap_call_cwmp__Inform. */
                if(++failureTime > 9)
                {
                    EPrint("10 times step 2/2 failed. immediate return\n");
                    return SY_FAILED;
                }
                else
                {
                    if(handle != SY_EVENT_BOOTSTRAP)
                    {
                        DPrint("Retry %dth times. Set connect_timeout to %ds\n",
                               failureTime, (int)pow(2, (failureTime > 3 ? 3 : failureTime))*5);
                        soap->connect_timeout = (int)pow(2, (failureTime > 3 ? 3 : failureTime))*5;
                    }
                    else
                    {
                        //零配置的时候不按上面的规则累加时间
                        AddRouteToEth(tmpUrl);
                        sleep(1);
                    }
                    goto TryAgain;
                }
            }
            else if (SY_HTTP_NO_CONTENT == soap->error)
            {
                /* 如果到服务器的网络连接不健壮
                 * 有一种情况是:我们的Inform多次被RST后,某次成功被服务器接受以后服务器会直接回应204
                 * 服务器不回应InformReponse,也不见我们的盒子发送 Send Empty Post.
                 */
                // TODO:这里直接返回是不是不太好
                WPrint("204. [warning]\n");
                if (gSendInfomResult != 2)
                {
                    gSendInfomResult = 1;
                }
                return SY_SUCCESS;
            }
            return SY_FAILED;
        }
    }
    else if(SY_HTTP_NO_CONTENT == soap->error)
    {
        // TODO:这里直接返回是不是不太好
        WPrint("204. [warning]\n");
        if (gSendInfomResult != 2)
        {
            gSendInfomResult = 1;
        }
        return SY_SUCCESS; /* 这里应该返回 EAGIN ,而不是返回成功 */
        // TODO:...
    }
    else {
        /* Inform 发送失败
         * 503 , -1 , 28 都当做失败处理
         */
		DPrint(" soap->error third = %d.\n",  soap->error);
		if(soap->error == 28)
		{
			usleep(500 * 500);
			UpdateNetPara();
			goto TryToSchedule;
		}
        if(++failureTime < 10)
        {
            /* 只重试九次,共执行十次 soap_call_cwmp__Inform. */
            if(handle != SY_EVENT_BOOTSTRAP)
            {
                DPrint("Retry %dth times. Set connect_timeout to %ds.\n",
                       failureTime, (int)pow(2, (failureTime > 3 ? 3 : failureTime))*5);
                soap->connect_timeout = (int)pow(2, (failureTime > 3 ? 3 : failureTime))*5;
            }
            else
            {
                // 0 Boot事件(零配置),不按上面的规则累加时间
                AddRouteToEth(tmpUrl);
                sleep(1);
            }
            HttpRelease(soap, &gSyHttpHeaderInfo);
            /* ENETUNREACH: 意既 Network unreachable. 说明盒子没有 IP gateway etc.原因.
             * 因此导致了 soap_call_cwmp__Inform 立即返回. 时序不能乱，sleep 过去。
             */
            if(ENETUNREACH == soap->errnum)
            {
                WPrint("%s are unreachable.\n"
                       "maybe eth0 Link is DOWN.\n", soap->host);
                sleep((int)pow(2, (failureTime > 3 ? 3 : failureTime))*5);
            }

            goto TryAgain_restart;
        }

        //零配置的时候在连接不上当前网管平台的时候不尝试调度
        if (handle == SY_EVENT_BOOTSTRAP)
        {
            DPrint("zero config failed, immediately return!\n");
            return SY_FAILED;
        }

        if (backupUrlMode)
        {
            if(SY_HTTP_SERVICE_UNAVAILABLE == soap->error)
            {
                /* 调度服务器返回 503 */
                EPrint("Scheduler Server response 503.\n");
            }
            else
            {
                EPrint("Scheduler Server no response too.\n");
            }
            return SY_FAILED;
        }
        else if (NULL != gsyAcsCpeParamStru.URLBackup && 0 != *gsyAcsCpeParamStru.URLBackup)
        {
            WPrint("jump to Scheduler Server!\n");
            if (NULL != gsyAcsCpeParamStru.URL) free(gsyAcsCpeParamStru.URL);
            if (NULL != (gsyAcsCpeParamStru.URL = strdup(gsyAcsCpeParamStru.URLBackup)))
            {
                DPrint("Now, URL = %s\n", gsyAcsCpeParamStru.URL);
                SySetNodeValue("Device.ManagementServer.URL", gsyAcsCpeParamStru.URL);
                // TODO:是否要设置到iptv里
                backupUrlMode = 1;
                failureTime = 0;
                soap->connect_timeout = 5;
                goto TryToSchedule;
            }
            else
            {
                EPrint("strdup() failed !\n");
                /* return */
            }
        }
        else
        {
            DPrint("backupUrlMode %d(0:main 1:backup) Backup url is '%s'.\n",
                   backupUrlMode, gsyAcsCpeParamStru.URLBackup?gsyAcsCpeParamStru.URLBackup:"");
            EPrint("Internal error !\n"); // 一般也就是URLBackup不可用才会出现这句
            /* return */
        }
    }

    VPrint("leave.\n");
    return SY_FAILED;
}

LOCAL void SendHeartIfm(struct soap* soap)
{

    FILE *pFile = NULL;
    char tmpBuf[256] = {0};
    char content[256] = {0};
    SyGetNodeValue("Device.ManagementServer.PeriodicInformEnable", tmpBuf) ;
#ifdef SY_TEST_DEMO
    DPrint("tmpBuf:%s\n", tmpBuf);
#endif
    if (atoi(tmpBuf) == 1)
    {
        pFile = fopen(SY_HEART_INFORM_FLAG, "wb");
        if (NULL != pFile)
        {
            DPrint("creat heartinform file :%s\n",SY_HEART_INFORM_FLAG);
            strcpy(content, "1");
            fwrite(content, 1, strlen(content), pFile);
            fclose(pFile);
        }
        else
        {
            DPrint("creat heartinform file failed :%s\n",SY_HEART_INFORM_FLAG);
        }
    }
}

LOCAL int SendEmptyPost(struct soap* soap)
{

    DO;

    HttpRestore(soap,  &gSyHttpHeaderInfo);

    soap->length = 0;
    soap->count = 0;
    soap_connect(soap, gsyAcsCpeParamStru.URL, NULL);
    soap_end_send(soap);

    return SY_SUCCESS;
}
#if 0
LOCAL void HandleInform(struct soap* soap , char* inform)
{
    int fd = -1;
    int flag = 0;
    int nRet = SY_FAILED;
    long time1 = 0;
    char* pName = NULL ;
    char* pValue = NULL ;
    char name[256] = {0};
    char value[256] = {0};
    char tmpBuf[256] = {0};
    size_t sizeOfValue = sizeof(tmpBuf);
    char *pStr = tmpBuf;
    FILE *fp = NULL ;

    gSendInfomResult = 0;

    fd = open(inform, O_RDONLY) ;
    if(fd > 0)
    {
        close(fd);
        DPrint("SessionState:%d, SessionFlags:%d\n",
               gsyGlobalParmStru.SessionState,
               gsyGlobalParmStru.SessionFlags);
        if ((SY_SESSION_IDLE == gsyGlobalParmStru.SessionState)
                && (SY_STATE_NONE == gsyGlobalParmStru.SessionFlags))
        {

            DPrint("gCwmpProcessThreadId:%ld, gSyGetting:%d\n",
                   gCwmpProcessThreadId, gSyGetting);

            if ((gCwmpProcessThreadId != 0) && (1 == gSyGetting))
            {
                DPrint("CwmpProcessThread 3 running......\n");
                usleep(500 * 1000);
                return;
            }

            if ((0 != strcmp(inform, SY_VALUE_CHANGE_INFORM_FLAG_0))
                    && (0 != strcmp(inform, SY_VALUE_CHANGE_INFORM_FLAG_1))
                    && (0 != strcmp(inform, SY_VALUE_CHANGE_INFORM_FLAG_2))
                    && (0 != strcmp(inform, SY_ERRORCODEMANAGE_FLAG))
                    && (0 != strcmp(inform, SY_LOG_PERIODIC_INFORM_FLAG)))
            {
                DPrint("inform:%s\n", inform);
                remove(inform);
            }

            if (0 == strcmp(inform, SY_HEART_INFORM_FLAG))
            {
                time1 = time(NULL);
                DPrint("time1:%ld\n", time1);
                if (SY_SUCCESS == SendInform(soap, (void*)SY_EVENT_PERIODIC))
                {
                    DPrint("time2:%ld\n", time(NULL) - time1);
                    SendEmptyPost(soap);
                    DPrint("time3:%ld\n", time(NULL) - time1);
                }
            }
            else if (0 == strcmp(inform, SY_SCREENON_FLAG))
            {
                sleep(1); // wait network init finish
                if (SY_SUCCESS == SendInform(soap, (void*)SY_EVENT_BOOT))
                {
                    SendEmptyPost(soap);
                }
            }
            else if (0 == strcmp(inform, SY_SHUTDOWN_FLAG))
            {
                if (SY_SUCCESS == SendInform(soap, (void*)SY_EVENT_X_CTC_SHUT_DOWN))
                {
                    //SendEmptyPost(soap);
                }
            }
            else if ((0 == strcmp(inform, SY_IPPING_INFORM_FLAG))
                     || (0 == strcmp(inform, SY_TRACEROUTE_INFORM_FLAG))
                     || (0 == strcmp(inform, SY_BANDWIDTH_INFORM_FLAG))
                     || (0 == strcmp(inform, SY_DIAGNOSTICS_INFORM_FLAG)))
            {
                DPrint("fd:%d, inform:%s\n", fd, inform);
                if (SY_SUCCESS == SendInform(soap, (void*)SY_EVENT_DIAGNOSTICS_COMPLETE))
                {
                    SendEmptyPost(soap);
                }
            }

            else if ((0 == strcmp(inform, SY_ERRORCODEMANAGE_FLAG)))
            {
                if (SY_SUCCESS == SendInform(soap, (void*)SY_EVENT_X_00E0FC_ErrorCode))
                {
                    SendEmptyPost(soap);
                }
            }

            else if ((0 == strcmp(inform, SY_VALUE_CHANGE_INFORM_FLAG_0))
                     || (0 == strcmp(inform, SY_VALUE_CHANGE_INFORM_FLAG_1))
                     || (0 == strcmp(inform, SY_VALUE_CHANGE_INFORM_FLAG_2)))
            {
                if (1 == gSyChangeNetMode)
                {
                    sleep(1);
                    gSyChangeNetMode = 0;
                }
                if (SY_SUCCESS == SendInform(soap, (void*)SY_EVENT_VALUE_CHANGE))
                {
                    SendEmptyPost(soap);
                    nRet = SY_SUCCESS;
                }
                else
                {
                    nRet = SY_FAILED;
                }
            }

            else if (0 == strcmp(inform, SY_LOG_PERIODIC_INFORM_FLAG))
            {
                if (SY_SUCCESS == SendInform(soap, (void*)SY_EVENT_CTC_LOG_PERIODIC))
                {
                    SendEmptyPost(soap);
                }
            }

            if (0 != strcmp(inform, SY_SHUTDOWN_FLAG))
            {
                DPrint("time4:%ld\n", time(NULL) - time1);
                RecvRPC(soap);
                DPrint("time5:%ld\n", time(NULL) - time1);
            }

            if ((0 == strcmp(inform, SY_VALUE_CHANGE_INFORM_FLAG_0))
                    || (0 == strcmp(inform, SY_VALUE_CHANGE_INFORM_FLAG_1))
                    || (0 == strcmp(inform, SY_VALUE_CHANGE_INFORM_FLAG_2)))
            {
                if (SY_SUCCESS == nRet)
                {
                    fp = fopen(inform, "r");
                    if (fp != NULL)
                    {
                        memset(tmpBuf, 0x00, sizeof(tmpBuf));
                        if(fgets(pStr, sizeof(tmpBuf), fp) != NULL)
                        {
                            while(strstr(pStr, "Device.") != NULL)
                            {
                                pName = strtok(pStr, "=");
                                pValue = strtok(NULL, "=");
                                memset(name, 0x00, sizeof(name));
                                memset(value, 0x00, sizeof(value));
                                snprintf(name, sizeof(name), "%s", pName);
                                snprintf(value, sizeof(value), "%s", pValue);
                                if ((0X0D == value[strlen(value) - 2]) && (0X0A == value[strlen(value) - 1]))
                                {
                                    value[strlen(value) - 2] = '\0';
                                }
                                else
                                {
                                    value[strlen(value) ] = '\0';
                                }

                                DPrint("name:%s, value:%s\n", name, value);
                                if (0 == strcmp(name, "Device.LAN.IPAddress"))
                                {
                                    if (gSyLANStu.IPAddress!= NULL)
                                    {
                                        free(gSyLANStu.IPAddress);

                                        if((0 == strcmp(value, "0.0.0.0")) && 0==strcmp(gSyLANStu.AddressingType, "PPPoE")) 
                                        {
                                            memset(tmpBuf, 0x00, sizeof(tmpBuf));
                                            if (GetValue("Device.LAN.IPAddress", tmpBuf, sizeOfValue)) {
                                                gSyLANStu.IPAddress = strdup(tmpBuf);
                                            }

                                            DPrint("For PPPoE Get true IP change:%s\n", gSyLANStu.IPAddress);
                                            snprintf(value, sizeof(value), "%s", gSyLANStu.IPAddress);
                                        }
                                        gSyLANStu.IPAddress = strdup(value);
                                    }
                                    memset(tmpBuf, 0x00, sizeof(tmpBuf));
                                    sprintf(tmpBuf, "http://%s:%d", gSyLANStu.IPAddress, SY_CWMP_CLIENT_PORT);
                                    if (gSyManagementServerStu.ConnectionRequestURL != NULL)
                                    {
                                        free(gSyManagementServerStu.ConnectionRequestURL);
                                    }
                                    gSyManagementServerStu.ConnectionRequestURL = strdup(tmpBuf);
                                }
                                memset(tmpBuf, 0x00, sizeof(tmpBuf));
                                fgets(pStr, sizeof(tmpBuf), fp);
                            }
                        }
                    }
                    fclose(fp) ;
                    remove(inform);
                }
            }
            else
            {
                DPrint("gSendInfomResult:%d.\n", gSendInfomResult);
                if (gSendInfomResult == 1)
                {
                    remove(inform);
                }
            }
        }
        else
        {
            DPrint("Busy Now\n");
        }
    }
    else
    {
        //DPrint("%s open Error\n", inform);
    }
}
#else
void HandleInform(void* inform, int len) //准备上报
{
	syInformEventType handle;
	if(len != 0){
		for(int i = 0; i < sizeof(gSendEvent)/sizeof(gSendEvent[0]); i++){
			if(0 == strcmp((char*)inform, gSendEvent[i].EventStr)){
				handle = i;
				break;
			}
		}
	}
	else
		handle = (int)inform;
	DPrint("event code:%d\n", handle);
	if(gSyClSoap == NULL){
		DPrint("gSyClSoap error\n");
		return;
	}
	if(SY_SUCCESS == SendInform(gSyClSoap, (void*)handle)){
		SendEmptyPost(gSyClSoap);
	}

	if(handle != SY_EVENT_X_CTC_SHUT_DOWN){
		RecvRPC(gSyClSoap);
	}
	DPrint("<---\n");
}
#endif
LOCAL bool GetDeviceInfo()
{
    char* pTmpBuf = NULL;
    size_t sizeOfBuf = SY_BUFFER_LENGTH;

    DO;

    pTmpBuf = (char*)malloc(sizeOfBuf);
    if(NULL == pTmpBuf)
    {
        EPrint("malloc Failure.");
        return SY_FAILED;
    }

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    SyGetNodeValue("Device.DeviceInfo.ManufacturerOUI", pTmpBuf);
    // TODO:后期做到不从xml 拿任何值
    gSyDeviceId.OUI = strdup(pTmpBuf);
    DPrint("OUI:%s\n", gSyDeviceId.OUI);

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
	DPrint("start to getValues\n");

    if (!GetValue("Device.DeviceInfo.ModelName", pTmpBuf, sizeOfBuf))
    {
        getprop("ro.product.model", pTmpBuf, "noDefine"); 
		//SetValue("Device.DeviceInfo.ModelName", pTmpBuf);
		
    }
    DPrint("model = %s\n", pTmpBuf);
    gSyDeviceInfoStu.ModelName = strdup(pTmpBuf);

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    if (!GetValue("Device.DeviceInfo.ProductClass", pTmpBuf, sizeOfBuf))
    {
        getprop("ro.product.model", pTmpBuf, "noDefine"); 
    }
    gSyDeviceId.ProductClass = strdup(pTmpBuf);
    DPrint("ProductClass:%s", gSyDeviceId.ProductClass);

//
    if(strncmp(gSyDeviceId.ProductClass,"JIUZHOU_PTV8098",15) == 0)
    {
        gSyDeviceId.Manufacturer = strdup("JIUZHOU");
    }
    else if(strncmp(gSyDeviceId.ProductClass,"SKYWORTH_E900",13) == 0)
    {
        gSyDeviceId.Manufacturer = strdup("SKYWORTH");
    }
    else if(strncmp(gSyDeviceId.ProductClass,"UM_UNT400B", 8) == 0)
    {
        gSyDeviceId.Manufacturer = strdup("UNIONMAN");;
    }
    else
    {
        memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);

        if (!GetValue("Device.DeviceInfo.Manufacturer", pTmpBuf, sizeOfBuf))
        {
            getprop("ro.product.manufacturer", pTmpBuf, "noDefine");
        }
        gSyDeviceId.Manufacturer = strdup(pTmpBuf);
    }

    DPrint("Manufacturer:%s\n", gSyDeviceId.Manufacturer);
    SySetNodeValue("Device.DeviceInfo.Manufacturer", gSyDeviceId.Manufacturer);

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);

    if (!GetValue("Device.LAN.MACAddress", pTmpBuf, sizeOfBuf))
    {
        getprop("ro.mac", pTmpBuf, "noDefine");
    }
    gSyDeviceId.SerialNumber = strdup(pTmpBuf);
    DPrint("SerialNumber:%s\n", gSyDeviceId.SerialNumber);

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    if(SY_FAILED != SyGetNodeValue("Device.DeviceSummary", pTmpBuf))
    {
        gSyDeviceInfoStu.DeviceSummary = strdup(pTmpBuf);
    }
    DPrint("DeviceSummary:%s\n", gSyDeviceInfoStu.DeviceSummary);

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);

    if(SY_FAILED != SyGetNodeValue("Device.DeviceInfo.Description", pTmpBuf))
    {
        gSyDeviceInfoStu.Description = strdup(pTmpBuf);
    }
    DPrint("Description:%s\n", gSyDeviceInfoStu.Description);

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);

    if (!GetValue("Device.DeviceInfo.HardwareVersion", pTmpBuf, sizeOfBuf))
    {
        getprop("ro.build.hardware.id", pTmpBuf, "noDefine");
		//SetValue("Device.DeviceInfo.HardwareVersion", pTmpBuf);
    }

    gSyDeviceInfoStu.HardwareVersion = strdup(pTmpBuf);
    DPrint("Device.DeviceInfo.HardwareVersion:%s\n", pTmpBuf);

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);

    if (!GetValue("Device.DeviceInfo.SoftwareVersion", pTmpBuf, sizeOfBuf))
    {
        getprop("ro.build.version.release", pTmpBuf, "noDefine");
		//SetValue("Device.DeviceInfo.SoftwareVersion", pTmpBuf);
    }
    gSyDeviceInfoStu.SoftwareVersion = strdup(pTmpBuf);
    DPrint("SoftwareVersion:%s\n", pTmpBuf);

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    if (access("/data/data/com.android.smart.terminal.iptv/shared_prefs/netmanager_prefs.xml",R_OK | W_OK) == 0)
    {
        SyGetNodeValue("Device.DeviceInfo.AdditionalHardwareVersion", pTmpBuf);
    }
    else
    {
        getprop("ro.build.version.release", pTmpBuf, "noDefine");
    }
    if (NULL == pTmpBuf)
    {
        free(pTmpBuf);
        return SY_FAILED;
    }
    else
    {
        gSyDeviceInfoStu.AdditionalHardwareVersion = strdup(pTmpBuf);
    }
    DPrint("AdditionalHardwareVersion:%s\n", pTmpBuf);

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    if (access("/data/data/com.android.smart.terminal.iptv/shared_prefs/netmanager_prefs.xml",R_OK | W_OK) == 0)
    {
        SyGetNodeValue("Device.DeviceInfo.AdditionalSoftwareVersion", pTmpBuf);
    }
    else
    {
        getprop("ro.build.version.release", pTmpBuf, "noDefine");
    }
    if (NULL == pTmpBuf)
    {
        free(pTmpBuf);
        return SY_FAILED;
    }
    else
    {
        gSyDeviceInfoStu.AdditionalSoftwareVersion = strdup(pTmpBuf);
        SySetNodeValue("Device.DeviceInfo.AdditionalSoftwareVersion", pTmpBuf);
    }
    DPrint("AdditionalSoftwareVersion:%s\n", pTmpBuf);

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    SyGetNodeValue("Device.DeviceInfo.ProvisioningCode", pTmpBuf);
    if (NULL == pTmpBuf)
    {
        free(pTmpBuf);
        return SY_FAILED;
    }
    else
    {
        gSyDeviceInfoStu.ProvisioningCode = strdup(pTmpBuf);
    }
    DPrint("ProvisioningCode:%s\n", pTmpBuf);

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    SyGetNodeValue("Device.DeviceInfo.UpTime", pTmpBuf);
    if (NULL == pTmpBuf)
    {
        free(pTmpBuf);
        return SY_FAILED;
    }
    else
    {
        sprintf(pTmpBuf, "%d", readUptime());
        gSyDeviceInfoStu.UpTime = strdup(pTmpBuf);
		//SetValue("Device.DeviceInfo.UpTime", pTmpBuf);
    }

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    SyGetNodeValue("Device.DeviceInfo.FirstUseDate", pTmpBuf);
    if (NULL == pTmpBuf)
    {
        free(pTmpBuf);
        return SY_FAILED;
    }
    else
    {
        gSyDeviceInfoStu.FirstUseDate = strdup(pTmpBuf);
    }

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    SyGetNodeValue("Device.ManagementServer.URL", pTmpBuf);
    if (NULL == pTmpBuf)
    {
        free(pTmpBuf);
        return SY_FAILED;
    }
    else
    {
        gSyManagementServerStu.URL = strdup(pTmpBuf);
    }

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    SyGetNodeValue("Device.ManagementServer.ParameterKey", pTmpBuf);
    if (NULL == pTmpBuf)
    {
        free(pTmpBuf);
        return SY_FAILED;
    }
    else
    {
        gSyManagementServerStu.ParameterKey = strdup(pTmpBuf);
    }

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);

    if (!GetValue("Device.LAN.AddressingType", pTmpBuf, sizeOfBuf))
    {
        SyGetNodeValue("Device.LAN.AddressingType", pTmpBuf);
    }
    if (!*pTmpBuf) {
        free(pTmpBuf);
        return SY_FAILED;
    }
    else {
        gSyLANStu.AddressingType = strdup(pTmpBuf);
    }

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);

    if (!GetValue("Device.LAN.DNSServers", pTmpBuf, sizeOfBuf))
    {
        SyGetNodeValue("Device.LAN.DNSServers", pTmpBuf);
    }
    if (NULL == pTmpBuf)
    {
        free(pTmpBuf);
        return SY_FAILED;
    }
    else
    {
        gSyLANStu.DNSServers = strdup(pTmpBuf);
    }

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);

    /*这里有问题，不能通过判断网络连接方式来决定ip的获取方式，
    一种极端的情况，如果零配置的时候xml配置的是pppoe，
    那么这里将永远无限循环下去，因为零配置的时候iptv，lanuch都不会起来，网络连接
    方式就是从xml获取的*/
    if(0==strcmp(gSyLANStu.AddressingType, "PPPoE")) 
    {
        while(1)
        {
            if (GetValue("Device.LAN.IPAddress", pTmpBuf, sizeOfBuf))
            {
                if (NULL == pTmpBuf)
                {
                    free(pTmpBuf);
                    return SY_FAILED;
                }
                DPrint("iptv get IP --->%s\n", pTmpBuf);
                if(0 !=strcmp(pTmpBuf, "0.0.0.0"))
                    break;
                memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
                usleep(500*1000);
            }
        }
        gSyLANStu.IPAddress = strdup(pTmpBuf);
    }
    else
    {

        if (!GetValue("Device.LAN.IPAddress", pTmpBuf, sizeOfBuf))
        {
            SyGetIPAddr(pTmpBuf);
        }

        if (NULL == pTmpBuf)
        {
            free(pTmpBuf);
            return SY_FAILED;
        }
        else
        {
            gSyLANStu.IPAddress = strdup(pTmpBuf);
            DPrint("ip:%s\n", gSyLANStu.IPAddress);
        }
    }

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);

    if (!GetValue("Device.LAN.MACAddress", pTmpBuf, sizeOfBuf))
    {
        //SyGetMAC(0, pTmpBuf);
        /* ro.mac可能跟ifconfig获取到的不一样.很多时候ro.mac说了算 */
        getprop("ro.mac", pTmpBuf, "noDefine");
		//SetValue("Device.LAN.MACAddress", pTmpBuf);
		
    }

    if (NULL == pTmpBuf)
    {
        free(pTmpBuf);
        return SY_FAILED;
    }
    else
    {
        gSyLANStu.MACAddress = strdup(pTmpBuf);
        DPrint("MAC:%s\n", gSyLANStu.MACAddress);
    }

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);

    sprintf(pTmpBuf, "http://%s:%d", gSyLANStu.IPAddress, SY_CWMP_CLIENT_PORT);

    DPrint("ConnectionRequestURL:%s\n", pTmpBuf);

    if (NULL == pTmpBuf)
    {
        free(pTmpBuf);
        return SY_FAILED;
    }
    else
    {
        gSyManagementServerStu.ConnectionRequestURL = strdup(pTmpBuf);
    }
//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);

    if (!GetValue("Device.X_00E0FC.STBID", pTmpBuf, sizeOfBuf))
    {
        getprop("ro.serialno", pTmpBuf, "noDefine");
		//SetValue("Device.X_00E0FC.STBID", pTmpBuf);
		
    }

    if (NULL == pTmpBuf)
    {
        free(pTmpBuf);
        return SY_FAILED;
    }
    else
    {
        gSyServiceInfoStu.STBID = strdup(pTmpBuf);
        DPrint("STBID:%s\n", gSyServiceInfoStu.STBID);
    }

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);

    if (!GetValue("Device.X_00E0FC.ServiceInfo.PPPoEID", pTmpBuf, sizeOfBuf))
    {
        SyGetNodeValue("Device.X_00E0FC.ServiceInfo.PPPoEID", pTmpBuf);
    }
    if (NULL == pTmpBuf)
    {
        free(pTmpBuf);
        return SY_FAILED;
    }
    else
    {
        gSyServiceInfoStu.PPPoEID = strdup(pTmpBuf);
    }

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    if (!GetValue("Device.X_00E0FC.ServiceInfo.UserID", pTmpBuf, sizeOfBuf))
    {
        SyGetNodeValue("Device.X_00E0FC.ServiceInfo.UserID", pTmpBuf);
    }
    if (NULL == pTmpBuf)
    {
        free(pTmpBuf);
        return SY_FAILED;
    }
    else
    {
    	//SyGetNodeValue("Device.X_00E0FC.ServiceInfo.UserID", pTmpBuf);
        gSyServiceInfoStu.UserID = strdup(pTmpBuf);
    }
    DPrint("UserID - [%s].", pTmpBuf);

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    if (!GetValue("Device.X_00E0FC.ServiceInfo.AuthURL", pTmpBuf, sizeOfBuf))
    {
        SyGetNodeValue("Device.X_00E0FC.ServiceInfo.AuthURL", pTmpBuf);
    }
    if (!*pTmpBuf) {
        free(pTmpBuf);
		DPrint("get ServiceInfo.AuthURL error");
        return SY_FAILED;
    }
    else {
        gSyServiceInfoStu.AuthURL = strdup(pTmpBuf);
    }
	DPrint("ServiceInfo.AuthURL is- [%s].", pTmpBuf);
//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    if (!GetValue("Device.Time.NTPServer1", pTmpBuf, sizeOfBuf))
    {
        SyGetNodeValue("Device.Time.NTPServer1", pTmpBuf);
    }
    if (!*pTmpBuf) {
        free(pTmpBuf);
        return SY_FAILED;
    }
    else {
        gSyTimeStu.NTPServer1 = strdup(pTmpBuf);
    }
//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    if (!GetValue("Device.Time.LocalTimeZone", pTmpBuf, sizeOfBuf))
    {
        SyGetNodeValue("Device.Time.LocalTimeZone", pTmpBuf);
    }
    if (NULL == pTmpBuf)
    {
        free(pTmpBuf);
        return SY_FAILED;
    }
    else
    {
        gSyTimeStu.LocalTimeZone = strdup(pTmpBuf);
    }
//
    free(pTmpBuf);
	
    DONE;
	DPrint("GetDeviceInfo return True");
    return SY_SUCCESS;
}

LOCAL bool GlobalParamInit()
{
    bool ret = false;
    gsyGlobalParmStru.IsCPEInitialized = SY_CPE_NOT_INITIALIZED;
    gsyGlobalParmStru.ConnectReqUrlUpdateFlag = SY_FALSE;
    gsyGlobalParmStru.CurrentRequestId  = 1;
    gsyGlobalParmStru.SessionEvent = SY_EVENT_NONE;
    gsyGlobalParmStru.SessionState = SY_SESSION_IDLE;
    gsyGlobalParmStru.SessionFlags = SY_STATE_NONE;
    gsyGlobalParmStru.SessionTrigger = SY_TRIGGER_NONE;

    ret = GetDeviceInfo();
	if(ret == SY_SUCCESS)
		DPrint("get GetDeviceInfo is True");

	
    gsyGlobalParmStru.IsCPEInitialized = SY_CPE_INITIALIZED;
    return ret;
}

LOCAL bool PramUpdate()
{
	DPrint("enter PramUpdate.\n");
    char* pTmpBuf = (char*)malloc(SY_BUFFER_LENGTH);
    size_t sizeOfBuf = SY_BUFFER_LENGTH;
    char tmpBuf[3] = {0};

    if(NULL == pTmpBuf)
    {
        EPrint("failed to allocate memory space.\n");
        return false;
    }
//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    SyGetNodeValue("Device.ManagementServer.PeriodicInformInterval", pTmpBuf);
    if (!*pTmpBuf) {
        free(pTmpBuf);
        return false;
    }
    else {
        gsyAcsCpeParamStru.InformInterval = atoi(pTmpBuf);
    }
    gsyAcsCpeParamStru.AuthType = strdup("Digest");
    gsyAcsCpeParamStru.Realm = strdup("Digest Authentication");
    DPrint("AuthType:%s, Realm:%s\n", gsyAcsCpeParamStru.AuthType, gsyAcsCpeParamStru.Realm);

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    SyGetNodeValue("Device.ManagementServer.URL", pTmpBuf);
    if (!*pTmpBuf) {
        free(pTmpBuf);
        return false;
    }
    else {
        gsyAcsCpeParamStru.URL = strdup(pTmpBuf);
    }
    DPrint("AcsCpeParamStru.URL:%s\n", gsyAcsCpeParamStru.URL);

    AddRouteToEth(gsyAcsCpeParamStru.URL);
    DPrint("Added to the route table successfully.\n");
//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    if (!GetValue("Device.ManagementServer.URLBackup", pTmpBuf, sizeOfBuf)) {
        //DPrint("get backup url");
        SyGetNodeValue("Device.ManagementServer.URLBackup", pTmpBuf);
    }

    if (!*pTmpBuf) {
        free(pTmpBuf);
        return false;
    }
    else {
        gsyAcsCpeParamStru.URLBackup = strdup(pTmpBuf);
    }
    DPrint("AcsCpeParamStru.backupURL:%s\n", gsyAcsCpeParamStru.URL);
//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    if (!GetValue("Device.ManagementServer.Username", pTmpBuf, sizeOfBuf)) {
        //DPrint("get Username");
        SyGetNodeValue("Device.ManagementServer.Username", pTmpBuf);
    }
    if (!*pTmpBuf) {
        free(pTmpBuf);
        return false;
    }
    else {
        gsyAcsCpeParamStru.Username = strdup(pTmpBuf);
    }
    DPrint("ManagementServer.Username %s\n", pTmpBuf);
//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);

    if (!GetValue("Device.ManagementServer.Password", pTmpBuf, sizeOfBuf))
    {
        SyGetNodeValue("Device.ManagementServer.Password", pTmpBuf);
    }
	DPrint("ManagementServer.Password:%s\n", pTmpBuf);
    if (!*pTmpBuf) {
        free(pTmpBuf);
        return false;
    }
    else {
        gsyAcsCpeParamStru.Password = strdup(pTmpBuf);
    }
//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    sprintf(pTmpBuf, "http://%s:%d", gSyLANStu.IPAddress, SY_CWMP_CLIENT_PORT);
    if (!*pTmpBuf) {
        free(pTmpBuf);
        return false;
    }
    else {
        gSyManagementServerStu.ConnectionRequestURL = strdup(pTmpBuf);
    }
    DPrint("ConnectionRequestURL:%s\n", pTmpBuf);
//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    if (!GetValue("Device.ManagementServer.ConnectionRequestUsername", pTmpBuf, sizeOfBuf)) {
        SyGetNodeValue("Device.ManagementServer.ConnectionRequestUsername", pTmpBuf);
    }
    if (!*pTmpBuf) {
        free(pTmpBuf);
        return false;
    }
    else {
        gsyAcsCpeParamStru.ConnectionRequestUsername = strdup(pTmpBuf);
    }
    DPrint("ConnectionRequestUsername %s", gsyAcsCpeParamStru.ConnectionRequestUsername);
//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    if (!GetValue("Device.ManagementServer.ConnectionRequestPassword", pTmpBuf, sizeOfBuf)) {
        SyGetNodeValue("Device.ManagementServer.ConnectionRequestPassword", pTmpBuf);
    }

    if (!*pTmpBuf) {
        free(pTmpBuf);
        return false;
    }
    else {
        gsyAcsCpeParamStru.ConnectionRequestPassword = strdup(pTmpBuf);
    }
//
    FILE *fp;
    if (!(fp = fopen(SY_BOOT_FLAG, "r+")))
    {
        WPrint("First startup (syBoot open failed).\n");
        gsyAcsCpeParamStru.IsAcsContacted = SY_ACS_NO_CONTACTED;
    }
    else
    {
        fread(tmpBuf, 1, sizeof(tmpBuf), fp);
        DPrint("tmpBuf:%s\n", tmpBuf);
        if (atoi(tmpBuf) == SY_ACS_VALUE_CHANGED) {
            gsyAcsCpeParamStru.IsAcsContacted = SY_ACS_VALUE_CHANGED;
            fwrite("1", 1, 1, fp);
        }
        else if (atoi(tmpBuf) == SY_ACS_UPDATE_SUCCESS) {
            gsyAcsCpeParamStru.IsAcsContacted = SY_ACS_UPDATE_SUCCESS;
            fwrite("1", 1, 1, fp);
        }
        else {
            gsyAcsCpeParamStru.IsAcsContacted = SY_ACS_CONTACTED;
        }
        fclose(fp);
    }

    if (!(fp = fopen(SY_REBOOT_FLAG, "r"))) {

        gsyAcsCpeParamStru.HaveReboot = 0;
    }
    else {
        DPrint("Have reboot\n");
        gsyAcsCpeParamStru.HaveReboot = 1;
        fclose(fp);
        remove(SY_REBOOT_FLAG);
    }
//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    SyGetNodeValue("Device.ManagementServer.PeriodicInformEnable", pTmpBuf);
    if (!*pTmpBuf)
    {
        free(pTmpBuf);
        return false;
    }
    else
    {
        gsyAcsCpeParamStru.InformEnable = atoi(pTmpBuf);
    }
    
//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    SyGetNodeValue("Device.X_00E0FC.ErrorCodeSwitch", pTmpBuf);
    if (!*pTmpBuf) {
        gSyErrorCodeSwitch = 0;
    }
    else {
        gSyErrorCodeSwitch = atoi(pTmpBuf);
    }
//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    SyGetNodeValue("Device.X_00E0FC.ErrorCodeInterval", pTmpBuf);
    if (!*pTmpBuf) {
        gSyErrorCodeInterval = 0;
    }
    else {
        gSyErrorCodeInterval = atoi(pTmpBuf);
    }

//
    memset(pTmpBuf, 0x00, SY_BUFFER_LENGTH);
    SyGetNodeValue("Device.ManagementServer.URLModifyFlag", pTmpBuf);
    if (!*pTmpBuf) {
        syEPGmodifySupported = atoi(pTmpBuf);
    }

    return true;
}

LOCAL bool AcsConnInit(struct soap* soap)
{
    DO;

    if (SY_TRUE == gsyGlobalParmStru.ConnectReqUrlUpdateFlag)
    {
        gsyGlobalParmStru.ConnectReqUrlUpdateFlag = SY_FALSE;
    }

    if (SY_SUCCESS == SendInform(soap, (void*)SY_EVENT_CONNECTION_REQUEST)) {
        soap_end(soap);
        return true;
    }
    else {
        soap_print_fault(soap, stderr);
        soap_end(soap);
        return false;
    }

}

LOCAL bool Initialize(struct soap* soap)
{
    bool nRet = false;
    nRet = GlobalParamInit();
    if(!(nRet = PramUpdate())) {
        EPrint("Update parameter failed.\n");
    }
    return nRet;
}

LOCAL int SendFaultResp(struct soap *soap)
{
    int i;
    char tmpBuf[8096] = {0};
    struct SOAP_ENV__Header header;
    DO;
    memset(tmpBuf, 0x00, sizeof(tmpBuf));

    soap_set_fault(soap);
    sprintf(tmpBuf, "<cwmp:Fault><FaultCode>%s</FaultCode><FaultString>%s</FaultString>",
            gSyCwmpFault->FaultCode, gSyCwmpFault->FaultString);
    DPrint("tmpBuf1:%s\n", tmpBuf);
    DPrint("SetParameterValuesFault:%p, size:%d\n", gSyCwmpFault->SetParameterValuesFault,
           gSyCwmpFault->__sizeSetParameterValuesFault);
    if ((gSyCwmpFault->SetParameterValuesFault != NULL)
            &&(gSyCwmpFault->__sizeSetParameterValuesFault > 0))
    {
        for (i = 0; i < gSyCwmpFault->__sizeSetParameterValuesFault; i++)
        {
            sprintf(tmpBuf, "%s%s", tmpBuf, "<SetParameterValuesFault>");
            sprintf(tmpBuf, "%s<ParameterName>%s</ParameterName>", tmpBuf,
                    gSyCwmpFault->SetParameterValuesFault[i].ParameterName);
            sprintf(tmpBuf, "%s<FaultCode>%s</FaultCode>", tmpBuf,
                    gSyCwmpFault->SetParameterValuesFault[i].FaultCode);
            sprintf(tmpBuf, "%s<FaultString>%s</FaultString>", tmpBuf,
                    gSyCwmpFault->SetParameterValuesFault[i].FaultString);
            sprintf(tmpBuf, "%s%s", tmpBuf, "</SetParameterValuesFault>");
        }
    }
    DPrint("tmpBuf2:%s\n", tmpBuf);
    sprintf(tmpBuf, "%s%s", tmpBuf, "</cwmp:Fault>");
    DPrint("tmpBuf3:%s\n", tmpBuf);

    if ((gSyCwmpFault->FaultCode != NULL)
            &&(gSyCwmpFault->FaultString != NULL))
    {
        soap->fault->detail = (struct SOAP_ENV__Detail*)soap_malloc(soap, sizeof(struct SOAP_ENV__Detail));
        memset(soap->fault->detail, 0x00, sizeof(struct SOAP_ENV__Detail));
        soap->fault->detail->__any = soap_strdup(soap, (const char*)tmpBuf);
        DPrint("__any:%s\n", soap->fault->detail->__any);
    }
    FreeFault();

    soap_serializeheader(soap);
    soap_serializefault(soap);
    if (soap_begin_count(soap))
        return soap->error;

    if (soap->mode & SOAP_IO_LENGTH)
    {
        if (soap_envelope_begin_out(soap)
                || soap_putheader(soap)
                || soap_body_begin_out(soap)
                || soap_putfault (soap)
                || soap_body_end_out(soap)
                || soap_envelope_end_out(soap))
            return soap->error;
    }

    if (soap_end_count(soap)
            || soap_connect(soap, gsyAcsCpeParamStru.URL, NULL)
            || soap_envelope_begin_out(soap)
            || soap_putheader(soap)
            || soap_body_begin_out(soap)
            || soap_putfault (soap)
            || soap_body_end_out(soap)
            || soap_envelope_end_out(soap)
            || soap_end_send(soap))
        return soap->error;

    return SY_SUCCESS;
}

LOCAL int DispatchReq(struct soap* soap)
{
    DO;
    int tmp = soap_serve_request(soap);
    DPrint("tmp:%d\n", tmp);
    if (tmp)
    {
        DPrint("Receive Error.\n");
        SendFaultResp(soap);
        //soap_send_fault(soap);
        //return SY_FAILED;
    }

    DONE;

    return SY_SUCCESS;
}

LOCAL int DoRPC(struct soap* soap)
{
    char tmpUrl[SY_BUFFER_LENGTH] = {0};
    struct cwmp__FaultStruct cmwpFault;
    struct cwmp__TransferCompleteResponse transferResponse;

    DO;

    soap_end(soap);

    gsyGlobalParmStru.SessionState = SY_SESSION_IDLE;

    DPrint("SessionFlags:%d\n", gsyGlobalParmStru.SessionFlags);

    switch(gsyGlobalParmStru.SessionFlags)
    {
    case SY_STATE_VALUE_CHANGED:
        if (SY_SUCCESS != SendInform(soap, (void*)SY_EVENT_VALUE_CHANGE))
        {
            soap_end(soap);
        }
        break;
    case SY_STATE_UPLOAD_COMPLETE:
    case SY_STATE_DOWNLOAD_FIRMWARE_COMPLETE:
    case SY_STATE_DOWNLOAD_CONFIGURATION_COMPLETE:
        cmwpFault.FaultCode = "0";
        cmwpFault.FaultString = "";
        if (SY_SUCCESS == SendInform(soap, (void*)SY_EVENT_TRANSFER_COMPLETE))
        {
            HttpRestore(soap,  &gSyHttpHeaderInfo);

            gsyAcsCpeParamStru.StartTime = SyGetCurrentTime();
            gsyAcsCpeParamStru.CompleteTime = gsyAcsCpeParamStru.StartTime + 13;

            if (NULL == strstr(gsyAcsCpeParamStru.URL, "http"))
            {
                sprintf(tmpUrl, "http://%s:%d", gsyAcsCpeParamStru.URL, SY_CWMP_CLIENT_PORT);
            }
            else
            {
                sprintf(tmpUrl, "%s", gsyAcsCpeParamStru.URL);
            }

            if (SOAP_OK == soap_call_cwmp__TransferComplete(soap, tmpUrl, NULL, "",
                    cmwpFault, gsyAcsCpeParamStru.StartTime, gsyAcsCpeParamStru.CompleteTime,
                    &transferResponse))
            {
                SendEmptyPost(soap);
                //soap_end(soap);
            }
            if (SY_STATE_DOWNLOAD_CONFIGURATION_COMPLETE == gsyGlobalParmStru.SessionFlags)
            {
                //reboot
            }
        }
        else
        {
            soap_end(soap);
        }
        break;
    default:
        break;
    }
    gsyGlobalParmStru.SessionFlags = SY_STATE_NONE;

    DONE;


    return SY_SUCCESS;
}

LOCAL int RecvRPC(struct soap* soap)
{
    DO;

    if (NULL == gSyCwmpFault)
    {
        gSyCwmpFault = (struct _cwmp__Fault*)Malloc(soap, sizeof(struct _cwmp__Fault));
        gSyCwmpFault->__sizeSetParameterValuesFault = 0;
        gSyCwmpFault->SetParameterValuesFault = NULL;
        gSyCwmpFault->FaultCode = NULL;
        gSyCwmpFault->FaultString = NULL;
    }

    for (;;)
    {

        if (soap_begin_recv(soap)
                || soap_envelope_begin_in(soap)
                || soap_recv_header(soap)
                || soap_body_begin_in(soap))
        {

            DPrint("error:%d\n", soap->error);

            if ((SY_HTTP_NO_CONTENT != soap->error) && (SOAP_NO_DATA != soap->error))
            {

                soap_send_fault(soap);

            }

            gsyGlobalParmStru.SessionState = SY_SESSION_IDLE;
            DoRPC(soap);
            return SY_SUCCESS;
        }

        HttpRestore(soap, &gSyHttpHeaderInfo);
        if (SY_FAILED == DispatchReq(soap))
        {
            DPrint("SY_FAILED\n");
            gsyGlobalParmStru.SessionState = SY_SESSION_IDLE;
            soap_end(soap);
            return SY_FAILED;
        }
        DPrint("ReturnCode:%d\n", soap->error);
        gsyGlobalParmStru.SessionState = SY_SESSION_UP;
        //sleep(1);//说华为的盒子快，我们的盒子慢
    }

    return SY_SUCCESS;
}

LOCAL int StartSession(struct soap* soap)
{
    DO;
    if (SY_TRUE == gsyGlobalParmStru.ConnectReqUrlUpdateFlag)
    {
        gsyGlobalParmStru.ConnectReqUrlUpdateFlag = SY_FALSE;
    }

    if (SY_SUCCESS == SendInform(soap, (void*)SY_EVENT_CONNECTION_REQUEST)) {
        // soap_end(soap);
        gsyGlobalParmStru.SessionState = SY_SESSION_UP;
        SendEmptyPost(soap);
    }
    else {
        DPrint("Acs ConnectionInitiate failed.");
        soap_print_fault(soap, stderr);
        soap_end(soap);
        return SY_FAILED;
    }

    RecvRPC(soap);
    DONE;

    return SY_SUCCESS;
}

LOCAL void Run(struct soap* soap)
{
    DO;

    if (SY_CPE_INITIALIZED == gsyGlobalParmStru.IsCPEInitialized
            && (SY_TRUE == gSyNatSession || SY_FALSE == NATDetected))
    {
        if (   (SY_EVENT_START & gsyGlobalParmStru.SessionEvent)
            && (SY_SESSION_IDLE == gsyGlobalParmStru.SessionState)
            && (SY_STATE_SESSION_TEARDOWN & ~gsyGlobalParmStru.SessionFlags)
            && (SY_STATE_SESSION_INITIATE & ~gsyGlobalParmStru.SessionFlags))
        {
            StartSession(soap);

            gSyNatSession = SY_FALSE;

        }
    }

    DONE;

}
extern JavaVM*   jvm;

LOCAL void* ProcThread(void* data)
{

    DO;
	JNIEnv* env = NULL;
	(*jvm)->AttachCurrentThread(jvm, &env, NULL);

    DPrint("gSySoap:%p\n", gSySoap);


    if (NULL != gSySoap)
    {
        if (SY_CPE_INITIALIZED == gsyGlobalParmStru.IsCPEInitialized
            && (SY_TRUE == gSyNatSession || SY_FALSE == NATDetected))
        {
            if (   (SY_EVENT_START & gsyGlobalParmStru.SessionEvent)
                && (SY_SESSION_IDLE == gsyGlobalParmStru.SessionState)
                && (SY_STATE_SESSION_TEARDOWN & ~gsyGlobalParmStru.SessionFlags)
                && (SY_STATE_SESSION_INITIATE & ~gsyGlobalParmStru.SessionFlags))
            {
                StartSession(gSySoap);

                gSyNatSession = SY_FALSE;

            }
        }
        gSySoap->keep_alive = 1;
    }
    gCwmpProcessThreadId = 0;
    syCwmpSession = 1;

    DONE;
	(*jvm)->DetachCurrentThread(jvm);
	pthread_exit(NULL);

    return NULL;
}

LOCAL void* ClientThread(void* data)
{

    int  nRet = -1;
    int  random;
    long TmpCount = 0L;
    long CurrentTimeCount = 0L;
    long BaseTimeCount = 0L;
    char InformInterval[SY_BUFFER_LENGTH]  = {0};
    char InformTime[SY_BUFFER_LENGTH] = {0};
    char tmpBuf[SY_BUFFER_LENGTH] = {0};
    size_t sizeOfBuf = sizeof(tmpBuf);
    time_t CurrentTime = 0L;
    pthread_t tid;
    FILE *fp = NULL;
    struct tm BaseTime;
    struct tm* TmpTm;
    struct soap SoapClient;
    char tmpUrl[SY_BUFFER_LENGTH] = {0};


    DO;

    memcpy(&SoapClient, data, sizeof(struct soap));	
	if(gSyClSoap == NULL){
		gSyClSoap = soap_copy(&SoapClient);
	}
    /* 这里应该是在等待接收线程就绪 */
    sleep(1);

    memset(tmpBuf, 0x00, sizeof(tmpBuf));
    if (!GetValue("Device.LAN.AddressingType", tmpBuf, sizeOfBuf))
    {
        //obsolete
    }
    free(gSyLANStu.AddressingType);
    gSyLANStu.AddressingType = strdup(tmpBuf);
    DPrint("connect type:%s\n", tmpBuf);

    memset(tmpBuf, 0x00, sizeof(tmpBuf));
    if (!GetValue("Device.LAN.IPAddress", tmpBuf, sizeOfBuf))
    {
        //obsolete
    }
    free(gSyLANStu.IPAddress);

    gSyLANStu.IPAddress = strdup(tmpBuf);
    DPrint("IPaddress:%s\n", tmpBuf);

    memset(tmpBuf, 0x00, sizeof(tmpBuf));
    if(NULL != gSyLANStu.IPAddress && *gSyLANStu.IPAddress != 0) {
        sprintf(tmpBuf, "http://%s:%d", gSyLANStu.IPAddress, SY_CWMP_CLIENT_PORT);
    }
    else {
        char* tempIp = (char*)calloc(1, 16);
        if(SyGetIPAddr(tempIp)) {
            strcpy(tmpBuf, tempIp);
        }
        free(tempIp);
    }
    free(gSyManagementServerStu.ConnectionRequestURL);

    gSyManagementServerStu.ConnectionRequestURL = strdup(tmpBuf);
    DPrint("ConnectionRequestURL:%s\n", tmpBuf);
	SetValue("Device.ManagementServer.ConnectionRequestURL", tmpBuf);


    DPrint("IsAcsContacted:%d, SessionState:%d\n",
           gsyAcsCpeParamStru.IsAcsContacted,
           gsyGlobalParmStru.SessionState);

    if (SY_SESSION_IDLE == gsyGlobalParmStru.SessionState){ sleep(3); }
	if (gSyIsZeroConfig){
		HandleZeroCfg(&SoapClient);
	}
	else{
		/*
	    if (SY_ACS_VALUE_CHANGED == gsyAcsCpeParamStru.IsAcsContacted)
	    {
	        if (SY_SUCCESS == SendInform(&SoapClient, (void*)SY_EVENT_VALUE_CHANGE)) {
	            SendEmptyPost(&SoapClient);
	        }
	        else {
	            soap_end(&SoapClient);
	        }
	    }
	    else if (SY_ACS_NO_CONTACTED == gsyAcsCpeParamStru.IsAcsContacted) {
	        HandleZeroCfg(&SoapClient);
	    }*/
	    {
	        
	        if (SY_SUCCESS == SendInform(&SoapClient, (void*)SY_EVENT_BOOT))
	        {
	            DPrint("1 boot register OK\n");
	            SendEmptyPost(&SoapClient);
	        }
	        else {
	            soap_end(&SoapClient);
	        }
	    }
	    gsyGlobalParmStru.SessionState = SY_SESSION_IDLE;
	    gSyIsFirst = 1;
	    RecvRPC(&SoapClient);
	    gSyIsFirst = 0;
	}
    DPrint("Run the periodic thread");
   #if 0 
    nRet = pthread_create(&tid, 0, ProcIfmThread, (void*)&SoapClient);
    if (0 != nRet)
    {
        DPrint("Create Cwmp Process Inform thread failed");
    }
    else
    {
        pthread_detach(tid);
    }
	#endif
    SyGetNodeValue("Device.ManagementServer.PeriodicInformInterval", InformInterval);
    gSyHeartInterval = atoi(InformInterval);
    DPrint("Period interval:%d\n", gSyHeartInterval);

    int nUptimeLast = readUptime();
    int nUptimeCurrent = 0;

    VPrint("reference uptime is %d.", nUptimeLast);

    while(1)
    {

        nUptimeCurrent = readUptime();
        VPrint("current uptime is %d.", nUptimeCurrent);
        if ((nUptimeCurrent - nUptimeLast) >= gSyHeartInterval)
        {
            //DPrint("time out. Report Period.");
            char szBuf[16] = {0};
			SyGetNodeValue("Device.ManagementServer.PeriodicInformEnable", szBuf) ;
			if(atoi(szBuf) == 1){
				addEvent(EVENT_PERIODIC);
			}
           // SendHeartIfm(&SoapClient);
            nUptimeLast = nUptimeCurrent;

            VPrint("reference uptime is %d.", nUptimeLast);
        }
        /*
         * 因为服务器设置心跳间隔的的时候会更改这个全局变量,当然,改xml的话只能重启了.
         */
        //SyGetNodeValue("1Device.ManagementServer.PeriodicInformInterval", InformInterval);
        //gSyHeartInterval = atoi(InformInterval);

        sleep(10);
    }

    DONE;
    return NULL;
}

LOCAL void* ServerThread(void* data)
{

    int m;
    static int runFlag = 0;
    struct soap* pTmpSoap = NULL;
    struct soap  SoapServer;
    int    NATDetectedCOUNTS = 0;

    DPrint("parameterValue = (%p)", data);

    memcpy(&SoapServer, data, sizeof(struct soap));

    DPrint("soap.version = %d", SoapServer.version);

    D("1111 NATDetected == %d\n", NATDetected);
    while (-1 == NATDetected)
    {
        sleep(1);
        NATDetectedCOUNTS++; 
        if (NATDetectedCOUNTS >= 10)
            NATDetected = SY_FALSE;
    }
    D("2222 NATDetected == %d\n", NATDetected);

    m = soap_bind(&SoapServer, NULL, SY_CWMP_CLIENT_PORT, 100);
    if (m < 0)
    {
        soap_print_fault(&SoapServer, stderr);
        exit(-1);
    }

    for (;;)
    {
        D("gCwmpProcessThreadId:%ld, gSyIsFirst:%d, gSyGetting:%d\n",
          gCwmpProcessThreadId,
          gSyIsFirst,
          gSyGetting);

        if (SY_TRUE != NATDetected)
        {
        	EPrint("enter soap_accept.\n");
            int acceptRet = soap_accept(&SoapServer);
            if (acceptRet < 0)
            {
                soap_print_fault(&SoapServer, stderr);
				/* 因为soap设置了accept超时，所以可以accept失败的时候continue
				当然这里没有判断accept失败的原因，基本都是超时导致的，后续觉得
				有必要可以优化一下 */
				DPrint("errno:%d, err:%s.\n", errno, strerror(errno));
				continue;
            }

            DPrint("Socket connection successful: slave socket = %d\n", acceptRet);

            DPrint("pTmpSoap:%p\n", pTmpSoap);

            soap_free(pTmpSoap);
            pTmpSoap = soap_copy(&SoapServer);

			if (0 == gCwmpProcessThreadId)
            {
                DPrint("0 == gCwmpProcessThreadId\n");
                soap_free(gSySoap);
                gSySoap = soap_copy(&SoapServer);
            }
            if (NULL == gSySoap)
            {
                gSySoap = soap_copy(&SoapServer);
            }

			soap_serve(pTmpSoap);
            soap_end(pTmpSoap);
            soap_serve(gSySoap);
            soap_end(gSySoap);
        }
		else {
			DPrint("wait for nat connect!\n");
			while (SY_TRUE == NATDetected && syNATCmd == 0) {
	            usleep(500 * 1000);
	        }

			/* 如果等于0说明前面推出循环是因为NATDetected发生了变化
			(可能是网络发生了变化需要重新判断NAT)这个时候就不需要往下面走，
			直接跳到最上面的循环去 */
			if (syNATCmd == 0){
				DPrint("net is changed!\n");
				continue;
			}

			/* 如果syNATCmd不等于0等于1就会进入这里，说明是有穿透消息过来
			表示服务器有刷新或者查询动作，需要继续往下走 */
			syNATCmd = 0;
			DPrint("get nat msg!\n");
		}

        DPrint("pTmpSoap:%p, gSySoap2:%p\n", pTmpSoap, gSySoap);


        if (NULL == gSySoap)
        {
            gSySoap = soap_copy(&SoapServer);
        }
        if ((0 == gCwmpProcessThreadId) && (0 == gSyGetting))
        {
            while(gSyIsFirst != 0)
            {
                DPrint("CwmpProcessThread 2 running......\n");
                usleep(500 * 1000);
            }
            DPrint("Initial reverse connection\n");
            pthread_create(&gCwmpProcessThreadId, 0, ProcThread, (void*)&SoapServer);
        }

        if (SY_TRUE == NATDetected)
        {
            if (0 == gCwmpProcessThreadId)
            {
                DPrint("0 == gCwmpProcessThreadId\n");
                soap_free(gSySoap);
                gSySoap = soap_copy(&SoapServer);
            }
            if (NULL == gSySoap)
            {
                gSySoap = soap_copy(&SoapServer);
            }
        }

        DPrint("soap_accept\n");

    }
    DONE;

    return NULL;
}

LOCAL void *ProcIfmThread(void* data)
{
#if 0
    struct soap SoapClient;

    DO;

    memcpy(&SoapClient, data, sizeof(struct soap));
    while(1)
    {
        HandleInform(&SoapClient, SY_VALUE_CHANGE_INFORM_FLAG_1);
        HandleInform(&SoapClient, SY_VALUE_CHANGE_INFORM_FLAG_0);
        HandleInform(&SoapClient, SY_VALUE_CHANGE_INFORM_FLAG_2);
        HandleInform(&SoapClient, SY_HEART_INFORM_FLAG);
        HandleInform(&SoapClient, SY_SCREENON_FLAG);
        HandleInform(&SoapClient, SY_IPPING_INFORM_FLAG);
        HandleInform(&SoapClient, SY_TRACEROUTE_INFORM_FLAG);
        HandleInform(&SoapClient, SY_SHUTDOWN_FLAG);
        HandleInform(&SoapClient, SY_LOG_PERIODIC_INFORM_FLAG);
        HandleInform(&SoapClient, SY_BANDWIDTH_INFORM_FLAG);
        HandleInform(&SoapClient, SY_DIAGNOSTICS_INFORM_FLAG);
        HandleInform(&SoapClient, SY_ERRORCODEMANAGE_FLAG);

        sleep(2);
    }
    DONE;
#endif
    return NULL;
}

LOCAL void InitAuthProto(struct soap *soap)
{
    DPrint("AuthType %s", gsyAcsCpeParamStru.AuthType);
    if (!strcmp(gsyAcsCpeParamStru.AuthType, "Digest"))
    {
        if (soap_register_plugin(soap, HttpD))
        {
            soap_print_fault(soap, stderr);
        }

    }
}

bool CwmpInit(void)
{
    DO;
    int nRet = -1;
    bool ret = false;
    pthread_t       serverTid;
    pthread_t       clientTid;
    struct soap     soap;
    struct soap*    pSoapClient;
    struct soap*    pSoapServer;

    gSyErrorCodeManageTid = -1;

    if(GlobalParamInit() == SY_FAILED) {
        EPrint("Parameter update failed.\n");
        return false;
    }
    if(!PramUpdate()) {
        EPrint("Critical-parameter update failed.\n");
        return false;
    }

    gSyCPEStartTime = time(NULL);

    soap_init(&soap);
    soap.cookie_max = 10;
    
    if (!strcmp(gsyAcsCpeParamStru.AuthType, "Digest"))
    {
        if (soap_register_plugin(&soap, HttpD))
        {
            soap_print_fault(&soap, stderr);
        }

    }
    soap_set_namespaces(&soap, namespaces);

    soap_omode(&soap, SOAP_IO_KEEPALIVE);
    soap_imode(&soap, SOAP_IO_KEEPALIVE);

    soap.recv_timeout = 0;
    soap.send_timeout = 0;
    soap.accept_timeout = 10;
    soap.connect_timeout = 0;
    soap.state = 1;
    soap.fget = HttpGet;
    
    DPrint("httpGet %p.\n", HttpGet);

    pSoapServer = soap_copy(&soap);
    pSoapClient = soap_copy(&soap);

    remove(SY_HEART_INFORM_FLAG);

    nRet = pthread_create(&clientTid, 0, ClientThread, (void*)pSoapClient);
    if (nRet != 0) {
        DPrint("Failed to create Cwmp Client Socket thread\n");
        return false;
    }
    else {
        pthread_detach(clientTid);
    }

    nRet = pthread_create(&serverTid, 0, ServerThread, (void*)pSoapServer);
    if (nRet != 0) {
        DPrint("Failed to create Cwmp Server Socket thread\n");
        return false;
    }
    else {
        pthread_detach(serverTid);
    }
    
    nRet = pthread_create(&gSyErrorCodeManageTid, NULL, ErrCodeManageThd, NULL);
    if (nRet != 0) {
        PERROR("pthread_create:");
        return false;
    }
    else {
        pthread_detach(gSyErrorCodeManageTid);
    }

    DONE;
    return true;
}

LOCAL int HandleZeroCfg(struct soap *soap)
{
    DO;

    if (SY_SUCCESS == SendInform(soap, (void*)SY_EVENT_BOOTSTRAP))
    {
        SendEmptyPost(soap);
        DPrint("send inform success\n");
    }

    gsyGlobalParmStru.SessionState = SY_SESSION_IDLE;
    gSyIsFirst = 1;
    RecvRPC(soap);
    gSyIsFirst = 0;

    sleep(3);
    DPrint("zeroConfigFlag:%d\n", zeroConfigFlag);
    Send2zeroApk(zeroConfigFlag);

    DONE;
    return SY_SUCCESS;
}

bool CwmpMain()
{
    DO;
    int ret = 0;
#ifdef 	SY_TEST
	mxml_node_t *tEvenXmlRootPt = loadConfigXml(SY_EVENT_XML_PATH, SY_EVENT_XML_PATH_BAK, FALSE);
	DPrint("load xml result => %s.", tEvenXmlRootPt ? "success" : "failed");

	mxml_node_t *tParamXmlRootPt = loadConfigXml(SY_PARAM_XML_PATH, SY_PARAM_XML_PATH_BAK, FALSE);
	DPrint("load xml result => %s.", tParamXmlRootPt ? "success" : "failed");

	if(!tEvenXmlRootPt || !tParamXmlRootPt){
		//system("am broadcast -a loading.fail");
		DPrint("load xml fail\n");
		return false;
	}
	setEventRoot(tEvenXmlRootPt);
	setParamRoot(tParamXmlRootPt);
#endif	

    if (SY_FAILED == SyInitConfigXml()) {
        DPrint("Init Config Xml Failed\n");
        return false;
    }

    ret = pthread_create(&gSockThreadId, 0, CmdProcThread, 0);
    if (ret != 0)
    {
        DPrint("Failed to create TR069 Socket Process thread\n");
        return false;
    }
    else {
        pthread_detach(gSockThreadId);
    }

    sleep(2);
    FILE *lishiFp = NULL;

    /* 广东电信零配置调度进这里初始化 */
    if (NULL != (lishiFp = fopen(SY_ZERO_CONFIG_FLAG, "r")))
    {
        fclose(lishiFp);
        DPrint("self send init .\n");
        struct sockmsg sendMsg;
        memset(&sendMsg, 0x00, sizeof(sendMsg));
        if (0 == gSyIsTmStart)
        {
            gSyIsTmStart = 1;
            sprintf(sendMsg.msg, "syCwmpInit");
            sendMsg.len = strlen(sendMsg.msg);
            strcpy(sendMsg.user, "tr069");
            Send2CmdProcThd(&sendMsg);
        }
    }

    /* 网管平台一键信息收集进这里启动信息收集 */
    if (NULL != (lishiFp = fopen(SY_START_UP_INFO_FLAG, "r")))
    {
        StartupInfoFlag = 2;
        fclose(lishiFp);
		#if 0
        struct sockmsg sockMsg;
        sprintf(sockMsg.msg, "startupInfo");
        sockMsg.len = strlen(sockMsg.msg);
        strcpy(sockMsg.user, "tr069");
        Send2CmdProcThd(&sockMsg);
		#endif

		addEvent(EVENT_STARTUPINFO);
    }

    /* 运维APK启动开机信息收集进这里启动数据收集 */
    if (NULL != (lishiFp = fopen(SY_TS_INFO_FLAG, "rb")))
    {
        fclose(lishiFp);
		#if 0
        struct sockmsg sockMsg;
        memset(&sockMsg, 0x00, sizeof(struct sockmsg));
        sprintf(sockMsg.msg, "tsAPKStartupInfo");
        sockMsg.len = strlen(sockMsg.msg);
        strcpy(sockMsg.user, "tr069");
        Send2CmdProcThd(&sockMsg);
		#endif
		
		addEvent(EVENT_TSAPK_STARTUPINFO);
    }
    sleep(1);
	#if 0
    extern int Send2CmdProcThd(struct sockmsg *msg);
    struct sockmsg sendMsg;
    memset(&sendMsg, 0x00, sizeof(sendMsg));
    if (0 == gSyIsTmStart) {
        gSyIsTmStart = 1;
        sendMsg.len = sprintf(sendMsg.msg, "%s","syCwmpInit");
        strcpy(sendMsg.user, "tr069");
        Send2CmdProcThd(&sendMsg);
    }
	#endif
	addEvent(EVENT_INITIALIZE);
    DONE;	
    return true;
}



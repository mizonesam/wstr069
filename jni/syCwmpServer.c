/***********************************************************************
*
* syCwmpServer.c
*
* Implementation of user-space cwmp.
*
* Copyright (C) 2012 Lin jieling <mycourage0@126.com>
* Copyright (C) 2003-2012 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/
#include <pthread.h>
#include <openssl/des.h>

#include "soapH.h"
//#include "cwmp.nsmap"
#include "syCwmpFault.h"
#include "syCwmpSocket.h"
#include "syCwmpCommon.h"
#include "syCwmpManagement.h"
#include "NAT/syNATClient.h"
#include "syCwmpSocket.h"
#include "syCwmpLog.h"

extern int gSyCmdStrListLen;
extern syCmdStrStu gSyCmdStrList[];

extern int zeroConfigFlag;
extern int gSyHeartInterval;
extern int gSyIsFirstHeartBeat;
extern time_t gSyCPEStartTime;
extern syLANStruct gSyLANStu;
extern syServiceInfoStruct gSyServiceInfoStu;
extern syDeviceInfoStruct  gSyDeviceInfoStu;
extern syAcsCpeParamStruct gsyAcsCpeParamStru;
extern syManagementServerStruct gSyManagementServerStu;
extern struct _cwmp__Fault* gSyCwmpFault;
int gSyIpPing = 0;
int gSyTraceRoute = 0;
int gSyIpPingTesting = 0;
int gSyTraceRouteTesting = 0;
int gSyErrorCodeTesting = 0;
int gSyChangeNetMode = 0;
int gSyChangeURL = 0;
int gSyErrorCodeManage = 0;
int debugInfoCollectStop = 0;
int startupInfoCollectStop = 0;
int DebugInfoFlag = 0;
int StartupInfoFlag = 0;

int gSyIsMsg = 0;
int gSyEnableMsgLog = 0;
int gSyMsgLogDuration = 0;
int gSyMsgLogPeriodRunning = 0;



int gSyNatStart = 0;


extern int gSyBreakLoop;
extern int gSyDeleteCaputureFile;
extern int gSyErrorCodeSwitch;
extern int gSyErrorCodeInterval;
int gSyBandwidthDiagnostics = 0;
int gSyBandwidthDiagnosticsTesting = 0;
int gSyPacketCapture = 0;
int gSyPacketCapturing = 0;

int gSyPathNumber = 0;
char gSyPathList[204800] = {0}; //setZero In cwmp__GetParameterNames
char sendZeroBuf[1024] = {0};
// extern int SetIptvData(QOSMSG* pMsg);

int syCalcMallocNum(struct ParameterNames *ParameterNames)
{
    int i = 0;
    int j = 0;
    int szNum = 0;
    int lastPos = 0;
    int szOffset = 0;
    int mallocNum = 0;
    int NumberOfRouteHops = 0;
    char szType[64];
    char szPath[256];
    char szName[256];
    char tmpBuf[2048];
    char szParamPath[256];
    char tempStr[2048] = {0};
    char* ptr = NULL;

    DO;
    DPrint("size: %d\n", ParameterNames->__size);

    for (i = 0; i < ParameterNames->__size; i++)
    {
        lastPos = strlen(ParameterNames->__ptrstring[i]);

        if (ParameterNames->__ptrstring[i][lastPos - 1] == '.')
        {
            if (0 == strcmp(ParameterNames->__ptrstring[i], "Device.LAN.TraceRouteDiagnostics.RouteHops."))
            {
                memset(tempStr, 0, sizeof(tempStr));
                if (SyGetNodeValue("Device.LAN.TraceRouteDiagnostics.NumberOfRouteHops", tempStr))
                {
                    NumberOfRouteHops = atoi(tempStr);
                }
                mallocNum = mallocNum + NumberOfRouteHops;
            }
            else
            {
                szNum = SyGetNameList(ParameterNames->__ptrstring[i], tmpBuf, "false");

                DPrint("szNum:%d, tmpBuf:%s\n", szNum, tmpBuf);

                ptr = strstr(tmpBuf, "&");
                memcpy(szParamPath, tmpBuf, ptr - tmpBuf);
                szOffset = strlen(szParamPath) + 1;
                while (j < szNum)
                {
                    j++;
                    memset(szName, 0, sizeof(szName));
                    if(SyGetSubString(tmpBuf + szOffset, j, "+", szName) != 0)
                    {
                        continue;
                    }
                    sprintf(szPath, "%s%s", szParamPath, szName);
                    DPrint("szPath:%s\n", szPath);
                    memset(szType, 0, sizeof(szType));
                    memset(tempStr, 0, sizeof(tempStr));
                    SyGetNodeType(szPath, szType, "string");
                    snprintf(tempStr, sizeof(tempStr), "%s:%s", "xsd", szType);
                    
                    DPrint("tempStr:%s\n", tempStr);
                    if ((0 == strcmp(tempStr, "xsd:SingleInstance"))
                            || (0 == strcmp(tempStr, "xsd:multipleObject")))
                    {
                        continue;
                    }
                    mallocNum++;
                }
            }
        }
        else
        {
            mallocNum++;
        }
    }

    DPrint("In fact num of ParamNames %d\n", mallocNum);
    DONE;
    return mallocNum;
}

int __Inform(struct soap *soap, struct _DeviceIdStruct *DeviceId, struct _EventStruct *Event,
                 unsigned int MaxEnvelopes, char *CurrentTime, unsigned int RetryCount,
                 struct _ParameterValueStruct *ParameterList, struct cwmp__InformResponse *res)
{

    WPrint("Not implement!\n");

    return 0;
}

int __GetRPCMethods(struct soap *soap, void *req, struct cwmp__GetRPCMethodsResponse *res)
{
    DO;
    res->__ptrMethodList.__size = 12;
    res->__ptrMethodList.__ptrstring = (char**)soap_malloc(soap, res->__ptrMethodList.__size * sizeof(char **));
    res->__ptrMethodList.__ptrstring[0]  = soap_strdup(soap, SyMethodTypeToStr(SY_GET_RPC_METHODS));
    res->__ptrMethodList.__ptrstring[1]  = soap_strdup(soap, SyMethodTypeToStr(SY_SET_PARAMETER_VALUES));
    res->__ptrMethodList.__ptrstring[2]  = soap_strdup(soap, SyMethodTypeToStr(SY_GET_PARAMETER_VALUES));
    res->__ptrMethodList.__ptrstring[3]  = soap_strdup(soap, SyMethodTypeToStr(SY_GET_PARAMETER_NAMES));
    res->__ptrMethodList.__ptrstring[4]  = soap_strdup(soap, SyMethodTypeToStr(SY_SET_PARAMETER_ATTRIBUTES));
    res->__ptrMethodList.__ptrstring[5]  = soap_strdup(soap, SyMethodTypeToStr(SY_GET_PARAMETER_ATTRIBUTES));
    res->__ptrMethodList.__ptrstring[6]  = soap_strdup(soap, SyMethodTypeToStr(SY_ADD_OBJECT));
    res->__ptrMethodList.__ptrstring[7]  = soap_strdup(soap, SyMethodTypeToStr(SY_DELETE_OBJECT));
    res->__ptrMethodList.__ptrstring[8]  = soap_strdup(soap, SyMethodTypeToStr(SY_REBOOT));
    res->__ptrMethodList.__ptrstring[9]  = soap_strdup(soap, SyMethodTypeToStr(SY_DOWNLOAD));
    res->__ptrMethodList.__ptrstring[10] = soap_strdup(soap, SyMethodTypeToStr(SY_UPLOAD));
    res->__ptrMethodList.__ptrstring[11] = soap_strdup(soap, SyMethodTypeToStr(SY_FACTORY_RESET));
    //res->__ptrMethodList.__ptrstring[12] = soap_strdup(soap, SyMethodTypeToStr(SY_SCHEDULE_INFORM));

    return SY_SUCCESS;
}


int __GetParaValues(struct soap *soap,
                    struct ParameterNames *ParameterNames,
                    struct cwmp__GetParameterValuesResponse *res)
{
    DO;

    PVL_t* valueList  = NULL;
    cwmp_PVS_t* paraValStruct = NULL;

    if(NULL == ParameterNames->__ptrstring[0])
    {
        DPrint("NULL ParameterNames\n");
        return SY_FAILED;
    }

    int i = 0;
    int j = 0;
    int k = 0;
    int f = 1;
    int mallocNum = 0;
    int szNum = 0;
    int length = 0;
    int NumberOfRouteHops = 0;
    int szOffset = 0;
    int nSize = 0;
    int Size = 0;
    char tmpBuf[2048];
    char szType[64];
    char szPath[256];
    char szName[256];
    char szParamPath[256];
    char tempStr[2048] = {0};
    size_t sizeOfBuf = sizeof(tempStr);
    char FaultCodeStr[32] = {0};
    char FaultString[512] = {0};
    char* ptr = NULL;
    char* pCurrentTime = NULL;
    SY_INT64 rx_bytes = 0;
    SY_INT64 tx_bytes = 0;
    SY_INT64 rx_packets = 0;
    SY_INT64 tx_packets = 0;

    memset(tmpBuf, 0x00, sizeof(tmpBuf));
    memset(szParamPath, 0, sizeof(szParamPath));

    valueList = (PVL_t*)soap_malloc(soap, sizeof(PVL_t));
    valueList->__size = 0;
    //mallocNum = ParameterNames->__size;
    mallocNum = syCalcMallocNum(ParameterNames);

    if (ParameterNames->__size > 0)
    {
        valueList->ptrPVS = (cwmp_PVS_t**)soap_malloc
                    (soap, mallocNum * sizeof(cwmp_PVS_t*));
    }
    for (i = 0; i < ParameterNames->__size; i++)
    {
        length= strlen(ParameterNames->__ptrstring[i]);

        DPrint("name[%d]:%s\n", i, ParameterNames->__ptrstring[i]);

        if (ParameterNames->__ptrstring[i][length - 1] == '.')
        {
            if (0 == strcmp(ParameterNames->__ptrstring[i], "Device.LAN.TraceRouteDiagnostics.RouteHops."))
            {
                memset(tempStr, 0, sizeof(tempStr));
                if (SyGetNodeValue("Device.LAN.TraceRouteDiagnostics.NumberOfRouteHops", tempStr))
                {
                    NumberOfRouteHops = atoi(tempStr);
                }
                char *tmpName0 = NULL;
                char *tmpValue0 = NULL;
                char *tmpType0 = NULL;

                DPrint("NumberOfRouteHops:%d\n", NumberOfRouteHops);

                if (NumberOfRouteHops > 0)
                {
                    for (k = 0; k < NumberOfRouteHops; k++)
                    {
                        sprintf(tempStr, "Device.LAN.TraceRouteDiagnostics.RouteHops.%d.HopHost", k + 1);

                        DPrint("tempStr:%s\n", tempStr);

                        tmpName0 = soap_strdup(soap, tempStr);
                        memset(szType, 0, sizeof(szType));
                        memset(tempStr, 0, sizeof(tempStr));
                        SyGetNodeType(tmpName0, szType, "string");
                        snprintf(tempStr, sizeof(tempStr), "%s:%s", "xsd", szType);

                        DPrint("szType:%s\n", szType);

                        tmpType0 = soap_strdup(soap, tempStr);
                        DPrint("name:%s, tempStr:%s\n", tmpName0, tempStr);
                        if (SyGetNodeValue(tmpName0, tempStr))
                        {
                            tmpValue0 = soap_strdup(soap, tempStr);
                        }
                        else
                        {
                            tmpValue0 =  soap_strdup(soap, "null");
                        }
                        DPrint("value:%s\n", tmpValue0);
                        paraValStruct = (struct cwmp__ParameterValueStruct*)soap_malloc(soap, sizeof(struct cwmp__ParameterValueStruct));
                        valueList->__ptrParameterValueStruct[valueList->__size] = paraValStruct;
                        valueList->__size++;

                        paraValStruct->Name  = tmpName0;
                        paraValStruct->Value  = tmpValue0;
                        paraValStruct->Type = tmpType0;
                    }
                }
                else
                {
                    DPrint("NumberOfRouteHops <= 0\n");
                }
            }
            else
            {
                szNum = SyGetNameList(ParameterNames->__ptrstring[i], tmpBuf, "false");
                if (0 == szNum)
                {
                    sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_NAME);
                    sprintf(FaultString, "%s, %s", ParameterNames->__ptrstring[i], SyGetFaultString(SY_CPE_INVALID_PARAM_NAME));
                    gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);
                    gSyCwmpFault->FaultString = Strdup(soap, FaultString);
                    return SOAP_CWMP_FAULT;
                }

                DPrint("szNum:%d, tmpBuf:%s\n", szNum, tmpBuf);

                ptr = strstr(tmpBuf, "&");
                memcpy(szParamPath, tmpBuf, ptr - tmpBuf);
                szOffset = strlen(szParamPath) + 1;
                while (j < szNum)
                {
                    j++;
                    memset(szName, 0, sizeof(szName));
                    if(SyGetSubString(tmpBuf + szOffset, j, "+", szName) != 0)
                    {
                        continue;
                    }
                    sprintf(szPath, "%s%s", szParamPath, szName);

                    DPrint("szPath:%s\n", szPath);

                    char *tmpName = NULL;
                    char *tmpValue = NULL;
                    char *tmpType = NULL;
                    tmpName = soap_strdup(soap, szPath);
                    memset(szType, 0, sizeof(szType));
                    memset(tempStr, 0, sizeof(tempStr));
                    SyGetNodeType(tmpName, szType, "string");
                    snprintf(tempStr, sizeof(tempStr), "%s:%s", "xsd", szType);

                    DPrint("szType:%s\n", szType);

                    tmpType = soap_strdup(soap, tempStr);

                    DPrint("tmpBuf:%s, tmpType:%s\n", tempStr, tmpType);

                    if ((0 == strcmp(tmpType, "xsd:SingleInstance"))
                            || (0 == strcmp(tmpType, "xsd:multipleObject")))
                    {
                        continue;
                    }
                    memset(tempStr, 0, sizeof(tempStr));
                    if (!GetValue(tmpName, tempStr, sizeOfBuf))
                    {
                        DPrint("tmpName:%s, tempStr:%s\n", tmpName, tempStr);
                        if (SyGetNodeValue(tmpName, tempStr))
                        {
                            tmpValue = soap_strdup(soap, tempStr);
                        }
                        else
                        {
                            //value =  soap_strdup(soap, "");
                            sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_NAME);
                            sprintf(FaultString, "%s, %s", tmpName, SyGetFaultString(SY_CPE_INVALID_PARAM_NAME));
                            gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
                            gSyCwmpFault->FaultString = Strdup(soap, FaultString);
                            return SOAP_CWMP_FAULT;
                        }
                    }
                    else
                    {
                    	DPrint("GetValue failed/n");
                        tmpValue = soap_strdup(soap, tempStr);
                    }

                    if (valueList->__size >= mallocNum)
                    {
                        valueList->__ptrParameterValueStruct[valueList->__size] = (struct cwmp__ParameterValueStruct*)
                                soap_malloc(soap, sizeof(struct cwmp__ParameterValueStruct));
                    }
                    paraValStruct = (struct cwmp__ParameterValueStruct*)soap_malloc(soap, sizeof(struct cwmp__ParameterValueStruct));
                    valueList->__ptrParameterValueStruct[valueList->__size] = paraValStruct;
                    valueList->__size++;

                    paraValStruct->Name  = tmpName;
                    paraValStruct->Value  = tmpValue;
                    paraValStruct->Type = tmpType;
                }
            }
        }
        else
        {
            char *name = NULL;
            char *value = NULL;
            char *type = NULL;

            name = soap_strdup(soap, ParameterNames->__ptrstring[i]);
            memset(szType, 0, sizeof(szType));
            memset(tempStr, 0, sizeof(tempStr));
            SyGetNodeType(name, szType, "string");
            snprintf(tempStr, sizeof(tempStr), "%s:%s", "xsd", szType);

            DPrint("szType:%s\n", szType);

            type = soap_strdup(soap, tempStr);

            if ((0 == strcmp(name, "Device.X_00E0FC.ServiceInfo.UserIDPassword")) ||
                (0 == strcmp(name, "Device.X_00E0FC.ServiceInfo.PPPoEPassword") ) ||
                (0 == strcmp(name, "Device.X_00E0FC.ServiceInfo.DHCPPassword")  ))
            {
                value = soap_strdup(soap, "");
            }
            else if (0 == strcmp(name, "Device.X_CTC_IPTV.STBID"))
            {
                value = soap_strdup(soap, gSyServiceInfoStu.STBID);
            }
            else if (0 == strcmp(name, "Device.Time.CurrentLocalTime"))
            {
                pCurrentTime = GetCurrTime();
                value = soap_strdup(soap, pCurrentTime);
            }
            else if ((0 == strcmp(name, "Device.DeviceInfo.UpTime"))||
                     (0 == strcmp(name, "Device.LAN.Stats.ConnectionUpTime")))
            {
                char upTime[32] = {0};
                sprintf(upTime, "%d", readUptime());
                value = soap_strdup(soap, upTime);
            }
            else if (0 == strcmp(name, "Device.LAN.Stats.TotalPacketsReceived"))
            {
                SyGetIfnameNetStats(SY_ETH0, &rx_bytes, &rx_packets, &tx_bytes, &tx_packets);
                sprintf(tempStr, "%llu", rx_packets);
                value = soap_strdup(soap, tempStr);
            }
            else if (0 == strcmp(name, "Device.LAN.Stats.TotalBytesReceived"))
            {
                SyGetIfnameNetStats(SY_ETH0, &rx_bytes, &rx_packets, &tx_bytes, &tx_packets);
                sprintf(tempStr, "%llu", rx_bytes);
                value = soap_strdup(soap, tempStr);
            }
            else if (0 == strcmp(name, "Device.LAN.Stats.TotalBytesSent"))
            {
                SyGetIfnameNetStats(SY_ETH0, &rx_bytes, &rx_packets, &tx_bytes, &tx_packets);
                sprintf(tempStr, "%llu", tx_bytes);
                value = soap_strdup(soap, tempStr);
            }
            else if (0 == strcmp(name, "Device.LAN.Stats.TotalPacketsSent"))
            {
                SyGetIfnameNetStats(SY_ETH0, &rx_bytes, &rx_packets, &tx_bytes, &tx_packets);
                sprintf(tempStr, "%llu", tx_packets);
                value = soap_strdup(soap, tempStr);
            }
            else if (0 == strcmp(name, "Device.ManagementServer.ConnectionRequestURL"))
            {
                DPrint("ConnectionRequestURL is %s\n", gSyManagementServerStu.ConnectionRequestURL);
                value = soap_strdup(soap, gSyManagementServerStu.ConnectionRequestURL);
            }
            else if (0 == strcmp(name, "Device.LAN.TraceRouteDiagnostics.RouteHops."))
            {
                if (0 == NumberOfRouteHops)
                {
                    name = soap_strdup(soap, "Device.LAN.TraceRouteDiagnostics.RouteHops.1.HopHost");
                    value = soap_strdup(soap, "null");
                    type = soap_strdup(soap, "xsd:string");
                }
            }
            else if (0 == strcmp(name, "Device.X_00E0FC.ErrorCodeSwitch"))
            {
                sprintf(tempStr, "%d", gSyErrorCodeSwitch);
                value = soap_strdup(soap, tempStr);
            }
            else if (0 == strcmp(name, "Device.X_00E0FC.ErrorCodeInterval"))
            {
                sprintf(tempStr, "%d", gSyErrorCodeInterval);
                value = soap_strdup(soap, tempStr);
            }
            else if (0 == strcmp(name, "Device.X_00E0FC.STBID"))
            {
                value = soap_strdup(soap, gSyServiceInfoStu.STBID);
            }
            else if ( 0 == strcmp(name, "Device.DeviceInfo.ProcessStatus.CPUUsage") )
            {
                CPUUSAGE cpu;
                CPUUSAGE m_cpu;
                GetCPUUsage(&m_cpu);
                sleep(1);
                GetCPUUsage(&cpu);
                int CPUR;
                CPUR = GetCPUUsageRate(&m_cpu, &cpu) / 10;
                memset(tempStr, 0x00, sizeof(tempStr));
                sprintf(tempStr, "%d", CPUR);
                value = soap_strdup(soap, tempStr);
                DPrint("get cpu usage is %s\n", value);
            }
            else if (0 == strcmp(name, "Device.DeviceInfo.MemoryStatus.Free") )
            {
                int memFree = 0;
                memFree = GetMemoryfree()/1024;
                memset(tempStr, 0x00, sizeof(tempStr));
                sprintf(tempStr, "%d", memFree);
                value = soap_strdup(soap, tempStr);
                DPrint("get memery free is %s\n", value);
            }
            else if (0 == strcmp(name, "Device.DeviceInfo.MemoryStatus.Total") )
            {
                int memTotal = 0;
                memTotal = GetMemoryTotal()/1024;
                memset(tempStr, 0x00, sizeof(tempStr));
                sprintf(tempStr, "%d", memTotal);
                value = soap_strdup(soap, tempStr);
                DPrint("get memory total is %s\n", value);
            }
            /* 网络连接方式 1:有线 2:WIFI */
            else if (0 == strcmp(name, "Device.X_00E0FC.ConnectMode") )
            {
                /*
                if (!GetValue("Device.LAN.AddressingType", tempStr, sizeOfBuf)){
                	if (SY_SUCCESS == SyGetNodeValue(name, tempStr)){
                	        DPrint("name:%s, tempStr:%s\n", name, tempStr);
                	    value = soap_strdup(soap, tempStr);
                	}
                }
                else{
                	DPrint("name:%s, tempStr:%s\n", name, tempStr);
                	value = soap_strdup(soap, tempStr);
                }
                */
                value = soap_strdup(soap, "1");
            }
            else if ( 0 == strcmp(name, "Device.X_00E0FC.HDMIConnect") )
            {

                if (!GetValue("Device.X_00E0FC.HDMIConnect", tempStr, sizeOfBuf))
                {
                    DPrint("name:%s, tempStr:%s\n", name, tempStr);
                    value = soap_strdup(soap, "1");
                }
            }
            else if ((0 == strcmp(name, "Device.DeviceInfo.ProductClass"))||
                     (0 == strcmp(name, "Device.DeviceInfo.ModelName"))) 
            {
                if (!GetValue(name, tempStr, sizeOfBuf))
                {
                    getprop("ro.product.model", tempStr, "noDefine");
                }
                value = soap_strdup(soap, tempStr);
                DPrint("value:%s", tempStr);
            }
            else if (0 == strcmp(name, "Device.X_00E0FC.StorageSize"))
            {
                getprop("ro.product.flash.info", tempStr, "noDefine");
                if(0 == strcmp(tempStr, "4G"))
                {
                    strcpy(tempStr, "4194304KB");
                    DPrint("ro %s:%s\n", name, value);
                }
                else if(0 == strcmp(tempStr, "8G"))
                {
                    strcpy(tempStr, "8388608KB");
                    DPrint("ro %s:%s\n", name, value);
                }
                else
                {
                    SyGetNodeValue(name, tempStr);
                    if(NULL != strchr(tempStr, 'M'))
                    {
                        nSize = atoi(tempStr);
                        Size = nSize * 1024;
                        memset(tempStr,0,sizeof(tempStr));
                        sprintf(tempStr,"%d KB",Size);
                    }
                }
                value = soap_strdup(soap, tempStr);
                DPrint("Get %s:%s", name, value);
            }
            else if(0 == strcmp(name,"Device.X_00E0FC.PhyMemSize"))
            {
                memset(tempStr, 0, sizeof(tempStr));
                SyGetNodeValue(name, tempStr);
                if(NULL != strchr(tempStr, 'M'))
                {
                    nSize = atoi(tempStr);
                    Size = nSize * 1024;
                    memset(tempStr,0,sizeof(tempStr));
                    sprintf(tempStr,"%d KB",Size);
                }
                value = soap_strdup(soap, tempStr);
                DPrint("Get %s:%s\n", name, value);
            }
            else if (0 == strcmp(name, "Device.X_00E0FC.PlayDiagnostics.DiagnosticsState") ||
				0 == strcmp(name, "Device.X_00E0FC.PlayDiagnostics.PlayState") ||
				0 == strcmp(name, "Device.X_00E0FC.PlayDiagnostics.PlayURL") )
		    {
                GetValueToIptv(name, tempStr);
                value = soap_strdup(soap, tempStr);
            }
         	else if (0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.Startpoint") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.Endpoint") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.AuthNumbers") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.AuthFailNumbers") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.AuthFailInfo") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.MultiReqNumbers") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.MultiRRT") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.MultiFailNumbers") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.MultiFailInfo") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.VodReqNumbers") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.VodRRT") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.VodFailNumbers") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.VodFailInfo") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HTTPReqNumbers") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HTTPRRT") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HTTPFailNumbers") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HTTPFailInfo") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.MutiAbendNumbers") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.VODAbendNumbers") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.MultiAbendUPNumbers") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.VODAbendUPNumbers") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.HD_MultiAbendNumbers") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_VODAbendNumbers") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_MultiAbendUPNumbers") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_VODAbendUPNumbers") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.PlayErrorNumbers") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.PlayErrorInfo") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.MultiPacketsLostR1") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.MultiPacketsLostR2") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.MultiPacketsLostR3") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.MultiPacketsLostR4") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.MultiPacketsLostR5") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.FECMultiPacketsLostR1") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.FECMultiPacketsLostR2") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.FECMultiPacketsLostR3") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.FECMultiPacketsLostR4") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.FECMultiPacketsLostR5") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.VODPacketsLostR1") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.VODPacketsLostR2") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.VODPacketsLostR3") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.VODPacketsLostR4") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.VODPacketsLostR5") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.ARQVODPacketsLostR1") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.ARQVODPacketsLostR2") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.ARQVODPacketsLostR3") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.ARQVODPacketsLostR4") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.ARQVODPacketsLostR5") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.MultiBitRateR1") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.MultiBitRateR2") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.MultiBitRateR3") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.MultiBitRateR4") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.MultiBitRateR5") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.VODBitRateR1") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.VODBitRateR2") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.VODBitRateR3") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.VODBitRateR4") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.VODBitRateR5") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_MultiPacketsLostR1") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_MultiPacketsLostR2") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_MultiPacketsLostR3") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_MultiPacketsLostR4") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_MultiPacketsLostR5") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_FECMultiPacketsLostR1") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.HD_FECMultiPacketsLostR2") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_FECMultiPacketsLostR3") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_FECMultiPacketsLostR4") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_FECMultiPacketsLostR5") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_VODPacketsLostR1") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_VODPacketsLostR2") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_VODPacketsLostR3") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_VODPacketsLostR4") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_VODPacketsLostR5") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_ARQVODPacketsLostR1") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_ARQVODPacketsLostR2") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_ARQVODPacketsLostR3") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.HD_ARQVODPacketsLostR4") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.HD_ARQVODPacketsLostR5") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_MultiBitRateR1") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_MultiBitRateR2") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_MultiBitRateR3") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_MultiBitRateR4") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.HD_MultiBitRateR5") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_VODBitRateR1") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_VODBitRateR2") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_VODBitRateR3") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_VODBitRateR4") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.HD_VODBitRateR5") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.BufferInc") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.BufferDec") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.FramesLostR1") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.FramesLostR2") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.FramesLostR3") ||
                 0 == strcmp(name,"Device.X_00E0FC.ServiceStatistics.FramesLostR4") ||
                 0 == strcmp(name, "Device.X_00E0FC.ServiceStatistics.FramesLostR5"))
            {
                GetValueToIptv(name, tempStr);
                value = soap_strdup(soap, tempStr);
            }
            else {
                memset(tempStr, 0, sizeof(tempStr));
                if (!GetValue(name, tempStr, sizeOfBuf)) {
                    if (SyGetNodeValue(name, tempStr)) {
                        value = soap_strdup(soap, tempStr);
                    }
                    else {
                        sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_NAME);
                        sprintf(FaultString, "%s, %s", name, SyGetFaultString(SY_CPE_INVALID_PARAM_NAME));
                        gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
                        gSyCwmpFault->FaultString = Strdup(soap, FaultString);
						EPrint("__GetParaValues return is SOAP_CWMP_FAULT");
						return SOAP_CWMP_FAULT;
                    }
                }
				value = soap_strdup(soap, tempStr);
            }

            paraValStruct = (struct cwmp__ParameterValueStruct*)soap_malloc(soap, sizeof(struct cwmp__ParameterValueStruct));
            valueList->__ptrParameterValueStruct[valueList->__size] = paraValStruct;
            valueList->__size++;
            paraValStruct->Name  = name;
            paraValStruct->Value  = value;
            paraValStruct->Type = type;
        }
    }

    DPrint("list size:%d, param size:%d\n", valueList->__size, ParameterNames->__size);
    if (ParameterNames->__size > 0)
    {
        res->ParameterList = valueList;
    }
    else
    {
        sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_NAME);
        gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
        gSyCwmpFault->FaultString = Strdup(soap, (char*)SyGetFaultString(SY_CPE_INVALID_PARAM_NAME));
        return SOAP_CWMP_FAULT;
    }

    DONE;

    return SY_SUCCESS;
}

int __SetParaValues(struct soap *soap,
                    struct ParameterValueList *ParameterList,
                    char *ParameterKey,
                    struct cwmp__SetParameterValuesResponse *res)
{

    DO;

    int i;
    int size = 0;
    int invalidArgument = SY_FALSE;

    char szValueChangeBuf[512] = {0};
    char szValueChangeBufs[2024] = {0};
    char tmpBuf[512] = {0};
    char szNotify[8] = {0};
    char szValueBuf[1024] = {0};
    char szWritable[8];
    char FaultCodeStr[32] = {0};
    char FaultString[512] = {0};
    FILE *pFile = NULL;
    struct sockmsg sockMsg;


    memset(&sockMsg, 0x00, sizeof(sockMsg));

    if(NULL == ParameterList->__ptrParameterValueStruct[0])
    {
        DPrint("NULL ParameterValue\n");
        return SY_FAILED;
    }
    DPrint("size:%d\n", ParameterList->__size);
    size = ParameterList->__size;
    for(i = 0; i < size; i++)
    {
        char *name = NULL;
        char *value = NULL;
        name = soap_strdup(soap, ParameterList->__ptrParameterValueStruct[i]->Name);
        value = soap_strdup(soap, ParameterList->__ptrParameterValueStruct[i]->Value);
        DPrint("name:%s, value:%s\n", name, value);
        if (SY_SUCCESS != SyGetNodeRW(name, szWritable))
        {
            DPrint("szValueChangeBufs:%s\n", szValueChangeBufs);
            if (0 != strlen(szValueChangeBufs))
            {
                pFile = fopen(SY_VALUE_CHANGE_INFORM_FLAG_0, "wb");
                if (NULL != pFile)
                {
                    fwrite(szValueChangeBufs, 1, strlen(szValueChangeBufs), pFile);
                    fclose(pFile);
                }
                else
                {
                    DPrint("open %s Error...\n", SY_VALUE_CHANGE_INFORM_FLAG_0);
                }
            }
            sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_NAME);
            sprintf(FaultString, "%s, %s", name, SyGetFaultString(SY_CPE_INVALID_PARAM_NAME));
            gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
            gSyCwmpFault->FaultString = Strdup(soap, FaultString);
            return SOAP_CWMP_FAULT;
        }
        DPrint("szWritable:%s\n", szWritable);
        if (0 == strcmp(szWritable, "R"))
        {
            DPrint("[%s:%d] szValueChangeBufs:%s\n",
                   __FUNCTION__, __LINE__, szValueChangeBufs);
            if (0 != strlen(szValueChangeBufs))
            {
                pFile = fopen(SY_VALUE_CHANGE_INFORM_FLAG_0, "wb");
                if (NULL != pFile)
                {
                    fwrite(szValueChangeBufs, 1, strlen(szValueChangeBufs), pFile);
                    fclose(pFile);
                }
                else
                {
                    DPrint("open %s Error...\n", SY_VALUE_CHANGE_INFORM_FLAG_0);
                }
            }
            sprintf(FaultCodeStr, "%d", SY_CPE_NON_WRITABLE_PARAM);
            sprintf(FaultString, "%s, %s", name, SyGetFaultString(SY_CPE_NON_WRITABLE_PARAM));
            gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
            gSyCwmpFault->FaultString = Strdup(soap, FaultString);
            return SOAP_CWMP_FAULT;
        }

        if (0 == strcmp(name, "Device.UserInterface.CurrentLanguage"))
        {
            if (0 == strcasecmp(value, "english"))
            {
                strcpy(szValueBuf, "1");
            }
            else
            {
                strcpy(szValueBuf, "0");
            }
        }
        else
        {
            sprintf(szValueBuf, "%s", value);
        }

        DPrint("(%s) will equal (%s)", name, szValueBuf);

        SetValue(name, szValueBuf);

        if ((0 == strcmp(name, "Device.X_00E0FC.ServiceInfo.UserID") && 0 != strcmp(szValueBuf, "")) 			||
            (0 == strcmp(name, "Device.X_00E0FC.ServiceInfo.UserIDPassword") && 0 != strcmp(szValueBuf, "")) 	||
            (0 == strcmp(name, "Device.X_00E0FC.ServiceInfo.Password") && 0 != strcmp(szValueBuf, "")) 			||
            (0 == strcmp(name, "Device.X_00E0FC.ServiceInfo.PPPoEID") && 0 != strcmp(szValueBuf, "")) 			||
            (0 == strcmp(name, "Device.X_00E0FC.ServiceInfo.PPPoEPassword") && 0 != strcmp(szValueBuf, ""))		||
            (0 == strcmp(name, "Device.X_00E0FC.ServiceInfo.BroadbandAccount") && 0 != strcmp(szValueBuf, "")) 	||
            (0 == strcmp(name, "Device.X_00E0FC.ServiceInfo.BroadbandPassword") && 0 != strcmp(szValueBuf, "")) 	||
            (0 == strcmp(name, "Device.X_00E0FC.ServiceInfo.SSID") && 0 != strcmp(szValueBuf, ""))				||
            (0 == strcmp(name, "Device.X_CTC_IPTV.ServiceInfo.UserID") && 0 != strcmp(szValueBuf, "")) 			||
            (0 == strcmp(name, "Device.X_CTC_IPTV.ServiceInfo.UserIDPassword") && 0 != strcmp(szValueBuf, "")) 	||
            (0 == strcmp(name, "Device.X_CTC_IPTV.ServiceInfo.Password") && 0 != strcmp(szValueBuf, "")) 		||
            (0 == strcmp(name, "Device.X_CTC_IPTV.ServiceInfo.PPPoEID") && 0 != strcmp(szValueBuf, "")) 			||
            (0 == strcmp(name, "Device.X_CTC_IPTV.ServiceInfo.PPPoEPassword") && 0 != strcmp(szValueBuf, ""))	||
            (0 == strcmp(name, "Device.X_CTC_IPTV.ServiceInfo.BroadbandAccount") && 0 != strcmp(szValueBuf, "")) ||
            (0 == strcmp(name, "Device.X_CTC_IPTV.ServiceInfo.BroadbandPassword") && 0 != strcmp(szValueBuf, "")) ||
            (0 == strcmp(name, "Device.X_CTC_IPTV.ServiceInfo.SSID") && 0 != strcmp(szValueBuf, "")))
        {
            zeroConfigFlag++;
            char lishi[1024] = {0};
            sprintf(lishi, "%s=%s;", name, value);
            strcat(sendZeroBuf, lishi);
            DPrint("hello test:%s\n", sendZeroBuf);
        }

        if (0 == strcmp(name, "Device.X_00E0FC.LogParaConfiguration.LogType") ||
            0 == strcmp(name, "Device.X_00E0FC.LogParaConfiguration.SyslogStartTime")||
            0 == strcmp(name, "Device.X_00E0FC.LogParaConfiguration.SyslogContinueTime")||
            0 == strcmp(name, "Device.X_00E0FC.LogParaConfiguration.LogLevel")||
            0 == strcmp(name, "Device.X_00E0FC.LogParaConfiguration.LogOutPutType")||
            0 == strcmp(name, "Device.X_00E0FC.LogParaConfiguration.SyslogServer"))
        {
        	int logcmd = -1;
			int j; 
        	for(j = 0; j<gSyCmdStrListLen && gSyCmdStrList[j].cmd!=-1; j++)
		    {
		        if(!strcmp(gSyCmdStrList[j].cmdStr, name))
		        {
					logcmd = gSyCmdStrList[j].cmd;
					break;
		        }
		        
		    }
			QOSMSG pMsg;
			pMsg.cmd = logcmd;   //log中cmd的值决定log类型
			pMsg.len = sizeof(pMsg);
			sprintf(pMsg.msg, "%s", value);
			DPrint("LogcatCaptrue cmd=%d, msg=%s\n", pMsg.cmd, pMsg.msg);
			runLogcat(&pMsg);

        }
		if (0 == strcmp(name, "Device.X_00E0FC.PlayDiagnostics.PlayURL") ||
		    /*0 == strcmp(name, "Device.X_00E0FC.PlayDiagnostics.PlayState") ||*/
            0 == strcmp(name, "Device.X_00E0FC.PlayDiagnostics.DiagnosticsState"))
        {
            SetValueToIptv(name, value);
        }

        if (0 == strcmp(name, "Device.ManagementServer.PeriodicInformInterval"))
        {
            if (atoi(value) > 0)
            {
                gSyHeartInterval = atoi(value);
                gSyIsFirstHeartBeat = 1;
            }
        }
        else if (0 == strcmp(name, "Device.X_00E0FC.DebugInfo.Action"))
        {
            if (atoi(value) == 0)
            {
                debugInfoCollectStop = 1;
            }
            else
            {
                /*如果已经在开始抓包了，不管是一键信息收集
                还是开机信息收集，在本次信息收集完成之前
                都会拒绝后续的一键信息收集请求，开机信息
                收集因为会在用户下次开机才启动所以不受影响*/
                if (0 != DebugInfoFlag || 2 == StartupInfoFlag)
                {
                    sprintf(FaultCodeStr, "%d", SY_CPE_REQUEST_DENIED);
                    sprintf(FaultString, "%s, %s", name, SyGetFaultString(SY_CPE_REQUEST_DENIED));
                    gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);
                    gSyCwmpFault->FaultString = Strdup(soap, FaultString);
                    return SOAP_CWMP_FAULT;
                }
                else
                {
                    DebugInfoFlag = 1;
                }
            }
        }
        else if (0 == strcmp(name, "Device.X_00E0FC.StartupInfo.Action"))
        {
            if (atoi(value) == 0)
            {
                //如果开机信息收集由于某些意外情况导致失败
                //这样会把状态切换到上传成功状态
                if (0 == StartupInfoFlag)
                {
                    SySetNodeValue("Device.X_00E0FC.StartupInfo.State", "3");
                }
                else
                {
                    startupInfoCollectStop = 1;
                }
            }
            else
            {
                if (2 == StartupInfoFlag || 0 != DebugInfoFlag)
                {
                    sprintf(FaultCodeStr, "%d", SY_CPE_REQUEST_DENIED);
                    sprintf(FaultString, "%s, %s", name, SyGetFaultString(SY_CPE_REQUEST_DENIED));
                    gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);
                    gSyCwmpFault->FaultString = Strdup(soap, FaultString);
                    return SOAP_CWMP_FAULT;
                }
                else
                {
                    if (1 == StartupInfoFlag)
                    {
                        StartupInfoFlag = 3;
                    }
                    else
                    {
                        StartupInfoFlag = 1;
                    }
                }
            }
        }
        else if (0 == strcmp(name, "Device.ManagementServer.ConnectionRequestPassword"))
        {
            if (NULL != gsyAcsCpeParamStru.ConnectionRequestPassword)
            {
                free(gsyAcsCpeParamStru.ConnectionRequestPassword);
            }
            gsyAcsCpeParamStru.ConnectionRequestPassword = strdup(value);
        }
        else if (0 == strcmp(name, "Device.ManagementServer.ConnectionRequestUsername"))
        {
            if (NULL != gsyAcsCpeParamStru.ConnectionRequestUsername)
            {
                free(gsyAcsCpeParamStru.ConnectionRequestUsername);
            }
            gsyAcsCpeParamStru.ConnectionRequestUsername = strdup(value);
        }
        else if (0 == strcmp(name, "Device.ManagementServer.Username"))
        {

            if ((0 != strcmp(value, "STBAdmin"))
                    && (0 != strcmp(value, gSyServiceInfoStu.STBID)))
            {
                sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_VALUE);
                sprintf(FaultString, "%s, %s", name, SyGetFaultString(SY_CPE_INVALID_PARAM_VALUE));
                gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
                gSyCwmpFault->FaultString = Strdup(soap, FaultString);
                return SOAP_CWMP_FAULT;
            }

            if (NULL != gsyAcsCpeParamStru.Username)
            {
                free(gsyAcsCpeParamStru.Username);
            }
            gsyAcsCpeParamStru.Username = strdup(value);

        }
        else if (0 == strcmp(name, "Device.ManagementServer.Password"))
        {

            if ((0 != strcmp(value, "STBAdmin"))
                    && (0 != strcmp(value, "111111")))
            {
                sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_VALUE);
                sprintf(FaultString, "%s, %s", name, SyGetFaultString(SY_CPE_INVALID_PARAM_VALUE));
                gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);
                gSyCwmpFault->FaultString = Strdup(soap, FaultString);
                return SOAP_CWMP_FAULT;
            }

            if (NULL != gsyAcsCpeParamStru.Password)
            {
                free(gsyAcsCpeParamStru.Password);
            }
            gsyAcsCpeParamStru.Password = strdup(value);


        }
        else if (0 == strcmp(name, "Device.ManagementServer.URL"))
        {
            if( NULL == gsyAcsCpeParamStru.URL )
            {
                gsyAcsCpeParamStru.URL = strdup(value);
            }
            else if( 0 != strcmp(gsyAcsCpeParamStru.URL, value) )
            {
                DPrint("TMC hand up URL:%s\n", value);
                if (NULL != gsyAcsCpeParamStru.URL)
                {
                    free(gsyAcsCpeParamStru.URL);
                }
                SySetNodeValue(name, value);
                gsyAcsCpeParamStru.URL = strdup(value);
                gSyChangeURL = 1;
            }
        }
        else if (0 == strcmp(name, "Device.LAN.IPAddress"))
        {

            sprintf(tmpBuf, "http://%s:%d", value, SY_CWMP_CLIENT_PORT);
            if (gSyManagementServerStu.ConnectionRequestURL != NULL)
            {
                free(gSyManagementServerStu.ConnectionRequestURL);
            }
            gSyManagementServerStu.ConnectionRequestURL = strdup(tmpBuf);

            if (0 == gSyChangeNetMode)
            {
                gSyChangeNetMode = 1;
            }
        }
        else if (0 == strcmp(name, "Device.X_CTC_IPTV.LogMsg.Enable"))
        {
            gSyEnableMsgLog = atoi(value);
        }
        else if (0 == strcmp(name, "Device.X_CTC_IPTV.LogMsg.MsgOrFile"))
        {
            gSyIsMsg = atoi(value);
        }
        else if (0 == strcmp(name, "Device.X_CTC_IPTV.LogMsg.Duration"))
        {
            gSyMsgLogDuration = atoi(value);
            if (gSyMsgLogDuration <= 0)
            {
                gSyMsgLogDuration = 30;
            }
        }
        else if (0 == strcmp(name, "Device.X_00E0FC.ErrorCodeSwitch"))
        {
            DPrint("Device.X_00E0FC.ErrorCodeSwitch be set to (%s)\n", value);
            gSyErrorCodeSwitch = atoi(value);

        }
        else if (0 == strcmp(name, "Device.X_00E0FC.ErrorCodeInterval"))
        {
            if (0 != atoi(value))
            {
                DPrint("Device.X_00E0FC.ErrorCodeInterval be set to (%s)\n", value);
                gSyErrorCodeInterval = atoi(value);
            }
        }

        else if (0 == strcmp(name, "Device.ManagementServer.STUNServerAddress"))
        {
            DPrint("Startup STUN service\n");
            gSyNatStart += 1;
        }

        if (1 == gSyNatStart)
        {
            usleep(100);
			DPrint("traverseNAT\n");
            traverseNAT();
            gSyNatStart += 1;
        }

        /*****************************************************************************/
        if (0 == strcmp(name, "Device.LAN.IPPingDiagnostics.DiagnosticsState"))
        {
            if (0 == strcasecmp(value, "Requested"))
            {
                if (0 == gSyIpPingTesting)
                {
                    gSyIpPing = 1;
                    gSyIpPingTesting = 1;
                }
                else
                {
                    WPrint("IPPING Testing... ['Requeste' be diacard]");
                }
            }
            else
            {
                sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_VALUE);
                sprintf(FaultString, "%s, %s", name, SyGetFaultString(SY_CPE_INVALID_PARAM_VALUE));
                gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);
                gSyCwmpFault->FaultString = Strdup(soap, FaultString);
                return SOAP_CWMP_FAULT;
            }
        }

        if (0 == strcmp(name, "Device.LAN.TraceRouteDiagnostics.DiagnosticsState"))
        {
            if (0 == strcasecmp(value, "Requested"))
            {
                if (0 == gSyTraceRouteTesting)
                {
                    gSyTraceRoute = 1;
                    gSyTraceRouteTesting = 1;
                }
                else
                {
                    WPrint("TraceRoute Testing... ['Requeste' be diacard]");
                }
            }
            else
            {
                sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_VALUE);
                sprintf(FaultString, "%s, %s", name, SyGetFaultString(SY_CPE_INVALID_PARAM_VALUE));
                gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);
                gSyCwmpFault->FaultString = Strdup(soap, FaultString);
                return SOAP_CWMP_FAULT;
            }
        }

        if (0 == strcmp(name, "Device.X_00E0FC.BandwidthDiagnostics.DiagnosticsState"))
        {
            if (0 == strcasecmp(value, "2"))
            {
                if (0 == gSyBandwidthDiagnosticsTesting)
                {
                    gSyBandwidthDiagnostics = 1;
                    gSyBandwidthDiagnosticsTesting = 1;
                }
                else
                {
                    WPrint("BandwidthDiagnostics Testing... ['Requeste' be diacard]");
                }
            }
            else
            {
                sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_VALUE);
                sprintf(FaultString, "%s, %s", name, SyGetFaultString(SY_CPE_INVALID_PARAM_VALUE));
                gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);
                gSyCwmpFault->FaultString = Strdup(soap, FaultString);
                return SOAP_CWMP_FAULT;
            }
        }

        if (0 == strcmp(name, "Device.X_00E0FC.PacketCapture.State"))
        {
            if (0 == strcasecmp(value, "2"))
            {
                memset(tmpBuf, 0x00, sizeof(tmpBuf));
                SyGetNodeValue("Device.X_00E0FC.PacketCapture.State", tmpBuf) ;
                if (0 == strcmp(tmpBuf, "3"))
                {
                	DPrint("PacketCapture.State value is %s\n", tmpBuf);
                    gSyBreakLoop = 1;
                    gSyDeleteCaputureFile = 1;
                    sleep(1);
                }
                gSyPacketCapture = 1;
                gSyPacketCapturing = 1;

            }
            else
            {
                sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_VALUE);
                sprintf(FaultString, "%s, %s", name, SyGetFaultString(SY_CPE_INVALID_PARAM_VALUE));
                gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);
                gSyCwmpFault->FaultString = Strdup(soap, FaultString);
                return SOAP_CWMP_FAULT;
            }
        }

        if (SY_SUCCESS == SySetNodeValue(name, value))
        {

            {
                res->Status = _cwmp__SetParaValRes_Status__0;

            }

            SyGetNodeAttr(name, szNotify);
            DPrint("szNotify:%s\n", szNotify);
            if (0 == strcmp(szNotify, "2"))
            {
                sprintf(szValueChangeBuf, "%s=%s\r\n", name, value);
                memcpy(szValueChangeBufs+strlen(szValueChangeBufs),
                       szValueChangeBuf, strlen(szValueChangeBuf));
            }
        }
        else
        {
            //res->Status = _cwmp__SetParaValRes_Status__1;
            invalidArgument = SY_TRUE;
            break;
        }
    }

    DPrint("szValueChangeBufs:%s\n", szValueChangeBufs);
    if (0 != strlen(szValueChangeBufs))
    {
        pFile = fopen(SY_VALUE_CHANGE_INFORM_FLAG_0, "wb");
        if (NULL != pFile)
        {
            fwrite(szValueChangeBufs, 1, strlen(szValueChangeBufs), pFile);
            fclose(pFile);
        }
        else
        {
            WPrint("open \"%s\" failed. [dont report value changed].", SY_VALUE_CHANGE_INFORM_FLAG_0);
        }
    }

    if (SY_TRUE == invalidArgument)
    {
        size = 1;
        DPrint("SetParameterValuesFault = %p", gSyCwmpFault->SetParameterValuesFault);
        if (NULL == gSyCwmpFault->SetParameterValuesFault)
        {
            gSyCwmpFault->__sizeSetParameterValuesFault = size;
            gSyCwmpFault->SetParameterValuesFault = (struct _cwmp__Fault_SetParameterValuesFault *)
                                                    Malloc(soap, size * sizeof(struct _cwmp__Fault_SetParameterValuesFault));
        }
        for(i = 0; i < size; i++)
        {
            DPrint("[i]:%d\n", i);
            gSyCwmpFault->SetParameterValuesFault[i].ParameterName = Strdup(soap, ParameterList->__ptrParameterValueStruct[i]->Name);
            sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_VALUE);
            sprintf(FaultString, "%s, %s", ParameterList->__ptrParameterValueStruct[i]->Name, SyGetFaultString(SY_CPE_INVALID_PARAM_VALUE));
            gSyCwmpFault->SetParameterValuesFault[i].FaultCode = Strdup(soap, FaultCodeStr);
            gSyCwmpFault->SetParameterValuesFault[i].FaultString = Strdup(soap, FaultString);
        }
        sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_ARGUMENTS);
        gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
        gSyCwmpFault->FaultString = Strdup(soap, (char*)SyGetFaultString(SY_CPE_INVALID_ARGUMENTS));
        return SOAP_CWMP_FAULT;
    }
    else
    {
        if (0 == gSyMsgLogPeriodRunning)
        {
            if ((1 == gSyEnableMsgLog) && (1 == gSyIsMsg))
            {
                gSyMsgLogPeriodRunning = 1;
                sprintf(sockMsg.msg, "PeriodMsgLog");
                sockMsg.len = strlen(sockMsg.msg);
                strcpy(sockMsg.user, "tr069");
                Send2CmdProcThd(&sockMsg);
            }
        }
        else
        {
            DPrint("MsgLog Running...\n");
        }

        if (1 == DebugInfoFlag)
        {
            DebugInfoFlag = 2;
            sprintf(sockMsg.msg, "debugInfo");
            sockMsg.len = strlen(sockMsg.msg);
            strcpy(sockMsg.user, "tr069");
            Send2CmdProcThd(&sockMsg);
        }
        if (1 == StartupInfoFlag)
        {
            //StartupInfoFlag += 1;
            FILE *fp = fopen(SY_START_UP_INFO_FLAG, "w");
            if (NULL != fp)
            {
                DPrint("create file %s success\n", SY_START_UP_INFO_FLAG);
                fclose(fp);
            }
            else
            {
                DPrint("create file %s failed\n", SY_START_UP_INFO_FLAG);
            }
        }
        if (1 == gSyIpPing)
        {
            gSyIpPing = 0;
            sprintf(sockMsg.msg, "IPPingDiagnostics");
            sockMsg.len = strlen(sockMsg.msg);
            strcpy(sockMsg.user, "tr069");
            Send2CmdProcThd(&sockMsg);
            // TODO:返回给服务器以内部错误
        }
        if (1 == gSyTraceRoute)
        {
            gSyTraceRoute = 0;
            sprintf(sockMsg.msg, "TraceRouteDiagnostics");
            sockMsg.len = strlen(sockMsg.msg);
            strcpy(sockMsg.user, "tr069");
            Send2CmdProcThd(&sockMsg);
        }

        if (1 == gSyBandwidthDiagnostics)
        {
            gSyBandwidthDiagnostics = 0;
            sprintf(sockMsg.msg, "BandwidthDiagnostics");
            sockMsg.len = strlen(sockMsg.msg);
            strcpy(sockMsg.user, "tr069");
            Send2CmdProcThd(&sockMsg);
        }
        if (1 == gSyPacketCapture)
        {
            gSyPacketCapture = 0;
            sprintf(sockMsg.msg, "PacketCapture");
            sockMsg.len = strlen(sockMsg.msg);
            strcpy(sockMsg.user, "tr069");
            Send2CmdProcThd(&sockMsg);
        }

        if (1 == gSyErrorCodeManage)
        {
        	DPrint("get is ErrorCodeManage\n");
            //gSyErrorCodeManage = 0;
            sprintf(sockMsg.msg, "ErrorCodeManage");
            sockMsg.len = strlen(sockMsg.msg);
            strcpy(sockMsg.user, "tr069");
            Send2CmdProcThd(&sockMsg);
        }

        if (1 == gSyChangeURL)
        {
            gSyChangeURL = 0;
            sprintf(sockMsg.msg, "TMCChanggedURL");
            sockMsg.len = strlen(sockMsg.msg);
            strcpy(sockMsg.user, "tr069");
            Send2CmdProcThd(&sockMsg);
        }



    }

    DONE;

    return SY_SUCCESS;
}

int __GetParaNames(struct soap *soap, char **ParameterPath, char *NextLevel,
                            struct cwmp__GetParameterNamesResponse *res)
{
    DO;
    int i = 0;
    int f = 1;
    int szNum = 0;
    int tmpSize = 0;
    int szOffset = 0;
    char szWritable[8];
    char szParamPath[256];
    char szName[256];
    char szPath[256];
    char tmpBuf[2048];
    char FaultCodeStr[32] = {0};
    char FaultString[512] = {0};
    char* ptr = NULL;
    char *name = NULL;

    memset(tmpBuf, 0x00, sizeof(tmpBuf));
    memset(szParamPath, 0, sizeof(szParamPath));
    memset(gSyPathList, 0x00, sizeof(gSyPathList));
    memcpy(gSyPathList + strlen(gSyPathList), "&", 1);

    struct ParameterInfoList* InfoList  = (struct ParameterInfoList*)soap_malloc(soap, sizeof(struct ParameterInfoList));
    if(strcmp("true", NextLevel) == 0)
    {
        szNum = SyGetNameList(ParameterPath[0], tmpBuf, NextLevel);
        DPrint("szNum:%d, tmpBuf:%s\n", szNum, tmpBuf);
        if ((0 == strcmp(ParameterPath[0], "."))
                || (0 == strcmp(ParameterPath[0], "")))
        {
            ptr = strstr(tmpBuf, "&");
            memcpy(szParamPath, tmpBuf, ptr - tmpBuf);
            szOffset = strlen(szParamPath) + 1;
        }
        else
        {
            szOffset = 0;
            memcpy(szParamPath, ParameterPath[0], strlen(ParameterPath[0]));
        }

        InfoList->__size = szNum;
        InfoList->__ptrParameterInfoStruct = (struct cwmp__ParameterInfoStruct**)soap_malloc(soap,
                                             szNum * sizeof(struct cwmp__ParameterInfoStruct*));
        while (i < szNum)
        {
            i++;
            memset(szName, 0, sizeof(szName));
            if(SyGetSubString(tmpBuf + szOffset, i, "+", szName) != 0)
            {
                continue;
            }
            memset(szPath, 0, sizeof(szPath));
            memset(szWritable, 0, sizeof(szWritable));
            sprintf(szPath, "%s%s", szParamPath, szName);
            DPrint("cPath2:%s, cName2:%s\n", szPath, szName);
            if (SY_SUCCESS != SyGetNodeRW(szPath, szWritable))
            {
                sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_NAME);
                sprintf(FaultString, "%s, %s", szPath, SyGetFaultString(SY_CPE_INVALID_PARAM_NAME));
                gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
                gSyCwmpFault->FaultString = Strdup(soap, FaultString);
                return SOAP_CWMP_FAULT;
            }
            DPrint("cWritable1:%s\n", szWritable);
            struct cwmp__ParameterInfoStruct  *Parametername = (struct cwmp__ParameterInfoStruct*)
                    soap_malloc(soap,sizeof(struct cwmp__ParameterInfoStruct));
            name = soap_strdup(soap, szPath);
            Parametername->Name = name;
            Parametername->Writable =  (*szWritable == 'W')? 1 : 0;;
            InfoList->__ptrParameterInfoStruct[i - 1] = Parametername;
        }
    }
    else
    {
        if ((0 == strcmp(ParameterPath[0], "."))
                || (0 == strcmp(ParameterPath[0], "")))
        {
            strcpy(szPath, "Device.");
        }
        else
        {
            strcpy(szPath, ParameterPath[0]);
        }

        SyCalcNodeNum(szPath);
        DPrint("gSyPathNumber:%d, len:%d, gSyPathList:%s\n",
               gSyPathNumber, strlen(gSyPathList), gSyPathList);

        InfoList->__size = gSyPathNumber;
        if (gSyPathNumber > 0)
        {
            InfoList->__ptrParameterInfoStruct = (struct cwmp__ParameterInfoStruct**)soap_malloc(soap,
                                                 gSyPathNumber * sizeof(struct cwmp__ParameterInfoStruct*));
            char** pathList = (char**)soap_malloc(soap, gSyPathNumber * sizeof(char*));
            tmpSize = SyDelimArgument2(gSyPathList, "&", pathList);
            szNum = tmpSize;
            DPrint("tmpSize:%d\n", tmpSize);
            for (i = 0; i < tmpSize; i++)
            {
                struct cwmp__ParameterInfoStruct  *Parametername = (struct cwmp__ParameterInfoStruct*)
                        soap_malloc(soap, sizeof(struct cwmp__ParameterInfoStruct));
                if (SY_SUCCESS != SyGetNodeRW(pathList[i], szWritable))
                {
                    sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_NAME);
                    sprintf(FaultString, "%s, %s", pathList[i], SyGetFaultString(SY_CPE_INVALID_PARAM_NAME));
                    gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
                    gSyCwmpFault->FaultString = Strdup(soap, FaultString);
                    return SOAP_CWMP_FAULT;
                }
                name = soap_strdup(soap, pathList[i]);
                Parametername->Name = name;
                Parametername->Writable =  (*szWritable == 'W')? 1 : 0;;
                InfoList->__ptrParameterInfoStruct[i] = Parametername;
            }

            for (i = 0; i < tmpSize; i++)
            {
                //DPrint("pathList[%d]:%s\n", i, pathList[i]);
                if (NULL != pathList[i])
                {
                    //DPrint("free......\n");
                    free(pathList[i]);
                    pathList[i] =  NULL;
                }
            }
        }
    }

    InfoList->__size = szNum;
    if (szNum > 0)
    {
        res->ParameterList = InfoList;
    }
    else
    {
        sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_NAME);
        gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
        gSyCwmpFault->FaultString = Strdup(soap, (char*)SyGetFaultString(SY_CPE_INVALID_PARAM_NAME));
        return SOAP_CWMP_FAULT;
    }
    DONE;
    return SY_SUCCESS;
}

int __SetParaAttr(struct soap *soap, struct SetParameterAttributesList *ParameterList,
                                 struct cwmp__SetParameterAttributesResponse *res)
{
    int i;
    int size = 0;
    int Notification = 0;
    int NotificationChange = 0;
    char szPath[256];
    char FaultCodeStr[32] = {0};
    char FaultString[512] = {0};
    struct cwmp__SetParameterAttributesStruct*  SetParameterAttributesStru = NULL;
    DO;
    if(NULL == ParameterList->__ptrSetParameterAttributesStruct[0])
    {
        DPrint("NULL ParameterValue\n");
        return SY_FAILED;
    }
    DPrint("size:%d\n", ParameterList->__size);
    size = ParameterList->__size;
    for(i = 0; i < size; i++)
    {
        SetParameterAttributesStru = ParameterList->__ptrSetParameterAttributesStruct[i];
        Notification = SetParameterAttributesStru->Notification;
        NotificationChange = SetParameterAttributesStru->NotificationChange;
        sprintf(szPath, "%s", SetParameterAttributesStru->Name[0]);
        DPrint("szPath:%s, Name:%s\n", szPath, SetParameterAttributesStru->Name[0]);
        if (SY_FAILED == SySetNodeAttr(szPath, NotificationChange, Notification))
        {
            sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_NAME);
            sprintf(FaultString, "%s, %s", szPath, SyGetFaultString(SY_CPE_INVALID_PARAM_NAME));
            gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
            gSyCwmpFault->FaultString = Strdup(soap, FaultString);
            return SOAP_CWMP_FAULT;
        }
    }
    DONE;
    return SY_SUCCESS;
}

int __GetParaAttr(struct soap *soap, struct ParameterNames *ParameterNames,
                                 struct cwmp__GetParameterAttributesResponse *res)
{
    int i;
    int szNum = 0;
    int szNotification = 0;
    char cNotification[8] = {0};
    char FaultCodeStr[32] = {0};
    char FaultString[512] = {0};
    struct ParameterAttributeList *AttributeList  = NULL;

    DO;
    szNum = ParameterNames->__size;

    AttributeList = (struct ParameterAttributeList*)soap_malloc(soap, sizeof(struct ParameterAttributeList));
    AttributeList->__size = szNum;
    AttributeList->__ptrParameterAttributeStruct = (struct cwmp__ParameterAttributeStruct**)
            soap_malloc(soap, szNum * sizeof(struct cwmp__ParameterAttributeStruct*));

    DPrint("szNum:%d\n", szNum);
    for(i = 0; i < szNum; i++)
    {
        if (SY_FAILED == SyGetNodeAttr(ParameterNames->__ptrstring[i], cNotification))
        {
            sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_NAME);
            sprintf(FaultString, "%s, %s", ParameterNames->__ptrstring[i], SyGetFaultString(SY_CPE_INVALID_PARAM_NAME));
            gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
            gSyCwmpFault->FaultString = Strdup(soap, FaultString);
            return SOAP_CWMP_FAULT;
        }
        else
        {
            struct cwmp__ParameterAttributeStruct  *ParameterAttr = (struct cwmp__ParameterAttributeStruct*)
                    soap_malloc(soap, sizeof(struct cwmp__ParameterAttributeStruct));
            struct AccessList *tmpAccessList = (struct AccessList *)soap_malloc(soap, sizeof(struct AccessList));
            memset(ParameterAttr, 0x00, sizeof(struct cwmp__ParameterAttributeStruct));
            memset(tmpAccessList, 0x00, sizeof(struct AccessList));

            szNotification = atoi(cNotification);

            AttributeList->__ptrParameterAttributeStruct[i] = ParameterAttr;
            ParameterAttr->Name = soap_strdup(soap, ParameterNames->__ptrstring[i]);
            ParameterAttr->Notification = (enum _cwmp__SetParaAttrStructRes_Notification)szNotification;
            ParameterAttr->AccessList = tmpAccessList;
            tmpAccessList->__size = 0;
            DPrint("szName:%s, szNotification:%d, \n", ParameterAttr->Name, szNotification);
        }
    }

    if(szNum > 0)
    {
        res->ParameterList = (struct ParameterAttributeList *)soap_malloc(soap, sizeof(struct ParameterAttributeList));
        res->ParameterList  = AttributeList;
    }
    else
    {
        sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_NAME);
        gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
        gSyCwmpFault->FaultString = Strdup(soap, (char*)SyGetFaultString(SY_CPE_INVALID_PARAM_NAME));
        return SOAP_CWMP_FAULT;
    }
    DONE;
    return SY_SUCCESS;
}

int __AddObject(struct soap *soap, char *ObjectName, char *ParameterKey,
                    struct cwmp__AddObjectResponse *res)
{
    int num = 0;
    int nodeNum = 0;
    char pNum[8] = {0};
    char tmpBuf[512] = {0};
    char *tmpPtr = NULL;
    char *tmpStr = NULL;
    char *objName = NULL;
    char FaultCodeStr[32] = {0};

    DO;

    DPrint("ObjectName:%s\n", ObjectName);
    DPrint("last:%c\n", (ObjectName[strlen(ObjectName) - 1]));
    if ('.' != (ObjectName[strlen(ObjectName) - 1]))
    {
        sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_NAME);
        gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
        gSyCwmpFault->FaultString = Strdup(soap, (char*)SyGetFaultString(SY_CPE_INVALID_PARAM_NAME));
        return SOAP_CWMP_FAULT;
    }

    tmpPtr = tmpBuf;
    sprintf(tmpPtr, "%s", ObjectName);
    tmpStr = strtok(tmpPtr, ".");
    while(tmpStr)
    {
        //DPrint("tmpStr:%s\n", tmpStr);
        if(0 == SyIsNumber(tmpStr))
        {
            nodeNum = atoi(tmpStr);
            break;
        }
        objName = tmpStr;
        tmpStr = strtok(NULL, ".");
    }

    DPrint("objName:%s, nodeNum:%d\n", objName, nodeNum);
    if ((strcmp(objName, "RouteList") != 0)
            && (strcmp(objName, "RouteHops") != 0)
            && (strcmp(objName, "DHCPOption") != 0))
    {
        sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_ARGUMENTS);
        gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
        gSyCwmpFault->FaultString = Strdup(soap,
                                    (char*)SyGetFaultString(SY_CPE_INVALID_ARGUMENTS));
        return SOAP_CWMP_FAULT;
    }

    num = SyAddInstanceObj(ObjectName);
    DPrint("num:%d\n", num);
    if (res != NULL)
    {
        if (num > 0)
        {
            res->InstanceNumber = num;
            res->Status = _cwmp__AddObjRes_Status__0;
            if (strcmp(objName, "RouteList") == 0)
            {
                sprintf(pNum, "%d", num);
                SySetNodeValue("Device.X_CTC_IPTV.ROUTETable.NumberOfRoute", pNum);
            }
            else if (strcmp(objName, "DHCPOption") == 0)
            {
                sprintf(pNum, "%d", num);
                SySetNodeValue("Device.LAN.DHCPOptionNumberOfEntries", pNum);
            }
            else if (strcmp(objName, "RouteHops") == 0)
            {
                sprintf(pNum, "%d", num);
                SySetNodeValue("Device.LAN.TraceRouteDiagnostics.NumberOfRouteHops", pNum);
            }
        }
        else if (-1 == num)
        {
            sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_NAME);
            gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
            gSyCwmpFault->FaultString = Strdup(soap, (char*)SyGetFaultString(SY_CPE_INVALID_PARAM_NAME));
            return SOAP_CWMP_FAULT;
        }
        else
        {
            sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_ARGUMENTS);
            gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
            gSyCwmpFault->FaultString = Strdup(soap, (char*)SyGetFaultString(SY_CPE_INVALID_ARGUMENTS));
            return SOAP_CWMP_FAULT;
        }
    }
    DONE;
    return SY_SUCCESS;
}

int __AddObjectIPTV(struct soap *soap, char *ObjectName, char *ParameterKey,
                        struct cwmp__AddObjectResponse *res)
{
    WPrint("Not implement!\n");
    return SY_SUCCESS;
}

int __DeleteObject(struct soap *soap, char *ObjectName, char *ParameterKey,
                       struct cwmp__DeleteObjectResponse *res)
{
    int nRet = SY_FAILED;
    char tmpBuf[512] = {0};
    char *tmpPtr = NULL;
    char *tmpStr = NULL;
    char *objName = NULL;
    char FaultCodeStr[32] = {0};
    DO;

    DPrint("last:%c\n", (ObjectName[strlen(ObjectName) - 1]));
    if ('.' != (ObjectName[strlen(ObjectName) - 1]))
    {
        sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_NAME);
        gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
        gSyCwmpFault->FaultString = Strdup(soap, (char*)SyGetFaultString(SY_CPE_INVALID_PARAM_NAME));
        return SOAP_CWMP_FAULT;
    }

    tmpPtr = tmpBuf;
    sprintf(tmpPtr, "%s", ObjectName);
    tmpStr = strtok(tmpPtr, ".");
    while(tmpStr)
    {
        DPrint("tmpStr:%s\n", tmpStr);
        if(0 == SyIsNumber(tmpStr))
        {
            break;
        }
        objName = tmpStr;
        tmpStr = strtok(NULL, ".");
    }

    DPrint("objName:%s\n", objName);
    if (strcmp(objName, "VendorConfigFile") != 0)
    {
        sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_ARGUMENTS);
        gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
        gSyCwmpFault->FaultString = Strdup(soap,
                                    (char*)SyGetFaultString(SY_CPE_INVALID_ARGUMENTS));
        return SOAP_CWMP_FAULT;
    }

    if(0 != SyIsNumber(tmpStr))
    {
        sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_ARGUMENTS);
        gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
        gSyCwmpFault->FaultString = Strdup(soap, (char*)SyGetFaultString(SY_CPE_INVALID_ARGUMENTS));
        return SOAP_CWMP_FAULT;
    }

    nRet = SyDelInstanceObj(ObjectName);
    if (SY_SUCCESS == nRet)
    {
        res->Status = _cwmp__DelObjRes_Status__0;
    }
    else
    {
        sprintf(FaultCodeStr, "%d", SY_CPE_INVALID_PARAM_NAME);
        gSyCwmpFault->FaultCode = Strdup(soap, FaultCodeStr);;
        gSyCwmpFault->FaultString = Strdup(soap, (char*)SyGetFaultString(SY_CPE_INVALID_PARAM_NAME));
        return SOAP_CWMP_FAULT;
    }

    DONE;
    return SY_SUCCESS;
}

int __Reboot(struct soap *soap, char *CommandKey, struct cwmp__RebootResponse *res)
{
    WPrint("Not implement!\n");

    return SY_SUCCESS;
}

int __Download(struct soap *soap,
               char *CommandKey,
               char *FileType,
               char *URL,
               char *Username,
               char *Password,
               int   FileSize,
               char *TargetFileName,
               int   DelaySeconds,
               char *SuccessURL,
               char *FailureURL,
               struct cwmp__DownloadResponse *res)
{
    char tmpUrl[256] = {0};
    unsigned char szFileType;
    syHostInfo    szHostInfo = {0};
    struct sockmsg sockMsg;

    DO;

    memset(tmpUrl, 0x00, sizeof(tmpUrl));
    memset(&sockMsg, 0x00, sizeof(sockMsg));

    strcpy(tmpUrl, URL);
    res->Status = _cwmp__DownloadRes_Status__1;

    szFileType = *FileType;
    DPrint("szFileType:%c\n", szFileType);
    switch(szFileType)
    {
    case DOWNLOAD_FIRMWARE_UPGRADE_IMAGE:
        sockMsg.len = sprintf(sockMsg.msg, "%s&%c&%s&%s&%s&%d&%s&%d&%s&%s&", SyMethodTypeToStr(SY_DOWNLOAD),
                szFileType, tmpUrl, Username, Password, FileSize, TargetFileName,
                DelaySeconds, SuccessURL, FailureURL);
        strcpy(sockMsg.user, "tr069");
        Send2CmdProcThd(&sockMsg);
        res->StartTime = SyGetCurrentTime();
        res->CompleteTime = res->StartTime + 10;
        break;

    case DOWNLOAD_VERDOR_CONFIGURATION_FILE:
        sockMsg.len = sprintf(sockMsg.msg, "%s&%c&%s&%s&%s&%d&%s&%d&%s&%s&", SyMethodTypeToStr(SY_DOWNLOAD),
                szFileType, tmpUrl, Username, Password, FileSize, TargetFileName,
                DelaySeconds, SuccessURL, FailureURL);
        strcpy(sockMsg.user, "tr069");
        Send2CmdProcThd(&sockMsg);
        res->StartTime = SyGetCurrentTime();
        res->CompleteTime = res->StartTime + 5;
        break;
    default:
        break;
    }

    DONE;
    return SY_SUCCESS;
}

int __Upload(struct soap *soap,
             char *CommandKey,
             char *FileType,
             char *URL,
             char *Username,
             char *Password,
             int   DelaySeconds,
             struct cwmp__UploadResponse *res)
{
    DO;
    char tmpUrl[256] = {0};
    unsigned char szFileType;
    struct sockmsg sockMsg;

    memset(tmpUrl, 0x00, sizeof(tmpUrl));
    memset(&sockMsg, 0x00, sizeof(sockMsg));

    strcpy(tmpUrl, URL);
    res->Status = _cwmp__UploadRes_Status__1;

    szFileType = *FileType;
    switch(szFileType)
    {
    case UPLOAD_VERDOR_CONFIGURATION_FILE:
        sockMsg.len = sprintf(sockMsg.msg, "%s&%c&%s&%s&%s&%d&", SyMethodTypeToStr(SY_UPLOAD),
                szFileType, tmpUrl, Username, Password, DelaySeconds);

        strcpy(sockMsg.user, "tr069");
        Send2CmdProcThd(&sockMsg);
        res->StartTime = SyGetCurrentTime();
        res->CompleteTime = res->StartTime + 10;
        break;

    case UPLOAD_VERDOR_LOG_FILE:
        sockMsg.len = sprintf(sockMsg.msg, "%s&%c&%s&%s&%s&%d&", SyMethodTypeToStr(SY_UPLOAD),
                szFileType, tmpUrl, Username, Password, DelaySeconds);

        strcpy(sockMsg.user, "tr069");
        Send2CmdProcThd(&sockMsg);
        res->StartTime = SyGetCurrentTime();
        res->CompleteTime = res->StartTime + 5;
        break;

    default:
        break;
    }

    DONE;
    return SY_SUCCESS;
}

int __FactoryReset(struct soap *soap, void *req,
                       struct cwmp__FactoryResetResponse *res)
{
    WPrint("Not implement!\n");

    return SY_SUCCESS;
}

int __ScheduleInform(struct soap *soap, int DelaySeconds, char *CommandKey,
                         struct cwmp__ScheduleInformResponse *res)
{

    WPrint("Not implement!\n");

    return SY_SUCCESS;
}

int __TransferComplete(struct soap *soap, char *CommandKey, struct cwmp__FaultStruct FaultStruct,
                           time_t StartTime, time_t CompleteTime, struct cwmp__TransferCompleteResponse *res)
{
    WPrint("Not implement!\n");
    return SY_SUCCESS;
}


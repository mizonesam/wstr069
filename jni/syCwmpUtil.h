/***********************************************************************
*
* syCwmpUtil.h
*
* Copyright (C) 2012 Lin jieling <mycourage0@126.com>
* Copyright (C) 2003-2012 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/
#ifndef SYCWMPUTIL_H
#define SYCWMPUTIL_H

typedef long long SY_INT64;
#define SY_MAX_HOP_COUNT 64
#define ARGUMENT_MAX 2048

#define SY_BUFFER_SIZE 1024
#define SY_FILE_NAME_MAX_SIZE 512

typedef struct
{
    long value[7];
} CPUUSAGE, *PCPUUSAGE;

typedef struct
{
    unsigned short wCmd;
    char szMsg[254];
} ItvSrvMsg_Out;

typedef struct
{
	short cmd;
	short len;
	char msg[1024];
} ItvSrvMsg_In;

struct _NOFITYMSG
{
    unsigned short wNotify;
    unsigned long lValue[10];
};

typedef struct _syHostInfo
{
    int     port;
    int     transferType;
    char*   path;
    char*   host;
} syHostInfo;

typedef struct _syTraceRouteResult
{
    char DiagnosticsState[256];
    unsigned int ResponseTime;
    unsigned int NumberOfRouteHops;
    char HopHost[64][256];
} syTraceRouteResult;

typedef struct _syIpPingResult
{
    char DiagnosticsState[256];
    unsigned int SuccessCount;
    unsigned int FailureCount;
    unsigned int AverageResponseTime;
    unsigned int MinimumResponseTime;
    unsigned int MaximumResponseTime;
} syIpPingResult;

typedef struct _syBandwidthTestResult
{
    char DiagnosticsState[256];
    char ErrorCode[128];
    unsigned int AvgSpeed;
    unsigned int MaxSpeed;
    unsigned int MinSpeed;
} syBandwidthTestResult;

typedef struct _syPacketCaptureResult
{
    char PacketCaptureState[256];
} syPacketCaptureResult;

typedef struct _syIpPingParam
{
    char Host[256];
    unsigned int NumberOfRepetitions;
    unsigned int Timeout;
    unsigned int DataBlockSize;
    unsigned int DSCP;
} syIpPingParam;

typedef struct _syTraceRouteParam
{
    char Host[256];
    unsigned int Timeout;
    unsigned int DataBlockSize;
    unsigned int MaxHopCount;
    unsigned int DSCP;
} syTraceRouteParam;

int AddRouteToEth(const char* pszHost);
int SendToIptvSer(const void* pszHost, char *routeRes, int cmd);

void* syStartupInfoThread(void *data);
void* syDebugInfoThread(void *data);
int SyGetCurrentTimeByType(int type, char *stime);
time_t SyGetCurrentTime(void);
char* GetCurrTime(void);
int SySetTimezone(unsigned char zone);
int SyGetMAC(int type, char* pszMAC);
int SyGetIPAddr(char* pszIPAddr);
int SyUrlParse(char* url, syHostInfo* hostInfo);
int SyDelimArgument(char *optarg , char *delim , char *argument[]);
int SyDelimArgument2(char *optarg , char *delim , char *argument[]);
void* syIpPingTestThread(void* data);
void* syTraceRouteTestThread(void* data);
void* syBandwidthTestThread(void* data);
void* syPacketCaptureThread(void* data);
void* syMsgLogPeriodThread(void* data);
void* ErrCodeManageThd(void* data);
int SyIpPingTest(syIpPingResult* pIpPingResult, char* Host, unsigned int NumberOfRepetitions,
                 unsigned int Timeout,unsigned int DataBlockSize,unsigned int DSCP);

int GetCPUUsageRate(CPUUSAGE *cpu, CPUUSAGE *cpu2);
int GetCPUUsage(CPUUSAGE *cpu);
int GetMemoryTotal();
int GetMemoryfree();

int  syIfProcessExist(char *processName);
void syKillExistProcess(char *processName);

int SyIsNumber(char *str);
int SyGetIfnameNetStats(const char* pszEth,
                        SY_INT64* rx_bytes, SY_INT64* rx_packets,
                        SY_INT64* tx_bytes, SY_INT64* tx_packets);
int syUpload(int type, const char* userPasswd, const char * remotepath,
             const char * localpath, long timeout, long tries);
int syDownload(int type, const char* userPasswd, const char* remotepath,
               const char* localpath, long timeout, long tries);
int sySftpUpload(char *hostUrl, int hostPort, int block, const char *username, char *password, char *localpath, char *sftppath);
int sySftpDownload(char *hostUrl, char *username, char *password, char *localpath, char *sftppath);
int SyGetPid(char *name);
int SyIsCorrectIp(const char *pIp);
uint memcpy_safe(void* dst, uint dst_len, void* src, uint src_len);

#endif

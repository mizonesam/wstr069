/***********************************************************************
*
* syCwmpCommon.h
*
* Copyright (C) 2012 Lin jieling <mycourage0@126.com>
* Copyright (C) 2003-2012 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/
#ifndef SYCWMPCOMMON_H
#define SYCWMPCOMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/system_properties.h>

#include "syCwmpUtil.h"
#include "stdsoap2.h"
#include "sylualib.h"

typedef unsigned char uchar,int8;
//typedef unsigned char int8;

typedef bool (*funPtrSetData)(const char*, char*);
typedef bool (*funPtrGetData)(const char*, char*, size_t);
typedef bool (*funPtrNotify)(int, char*);
typedef void* (*iFunction)(void *);


#define SY_TEST
#define SUPPORT_LUA
#define SY_GD_TR069

#define TAG "libcwmp"


#define SY_SYSLOG
#define SY_AUTO_ON_OFF
#define SY_PACKET_CAPTURE_SIZE  (64 * 1024)

#define SY_HAVE_LIBCURL

#define SY_GUANGDONG_WEISHENG

#define LOCAL static

#define SY_XML_ROOT_PATH                  "/data/data/"
#define SY_ROOT_PATH                      "/data/data/last_"

#define SY_SCREENON_FLAG                  SY_ROOT_PATH"syScreenOnInform"
#define SY_HEART_INFORM_FLAG              SY_ROOT_PATH"syHeartInform"
#define SY_IPPING_INFORM_FLAG             SY_ROOT_PATH"syIpPingInform"
#define SY_TRACEROUTE_INFORM_FLAG         SY_ROOT_PATH"syTraceRouteInform"

#define SY_ERRORCODEMANAGE_FLAG   	      SY_ROOT_PATH"syErrorCodeInform"
#define SY_ERRORCODETMP_FLAG  		      SY_ROOT_PATH"syErrorCodeTmp"
#define SY_ERRORCODEENCODETMP_FLAG  	  SY_ROOT_PATH"syErrorCodeEncodeTmp"

#define SY_BANDWIDTH_INFORM_FLAG          SY_ROOT_PATH"syTraceBandwidthInform"

#define SY_LOG_PERIODIC_INFORM_FLAG       SY_ROOT_PATH"syLogPeriodInform"
#define SY_TMP_LOG_PERIODIC_INFORM_FLAG   SY_ROOT_PATH"syTmpLogPeriodInform"

#define SY_DIAGNOSTICS_INFORM_FLAG        SY_ROOT_PATH"syDiagnosticsInform"
#define SY_VALUE_CHANGE_INFORM_FLAG       SY_ROOT_PATH"syValueChangeInform"
#define SY_VALUE_CHANGE_INFORM_FLAG_0     SY_ROOT_PATH"syValueChangeInform0"
#define SY_VALUE_CHANGE_INFORM_FLAG_1     SY_ROOT_PATH"syValueChangeInform1"
#define SY_VALUE_CHANGE_INFORM_FLAG_2     SY_ROOT_PATH"syValueChangeInform2"
#define SY_BOOT_FLAG                      SY_ROOT_PATH"syBoot"
#define SY_REBOOT_FLAG                    SY_ROOT_PATH"syReboot"
#define SY_SHUTDOWN_FLAG                  SY_ROOT_PATH"syShutdown"
#define SY_START_UP_INFO_FLAG		      SY_ROOT_PATH"startupInfoFlag"
#define SY_TS_INFO_FLAG				      SY_ROOT_PATH"tsInfoFlag"

#define DO   V("=>\n");
#define DONE V("<=\n");

#define SY_COMMON_XML               SY_ROOT_PATH"syXml.xml"

#define SY_ETH0                     "eth0"
#define SY_WLAN0                    "wlan0"

#define SY_REPORT_TIMES          3

#define SY_SUCCESS               0
#define SY_FAILED               -1
#define SY_IPTVSER_CMD_TRACERT	 24
#define SY_IPTVSER_CMD_LOGCAT	 18
#define SY_TRUE                  1
#define SY_FALSE                 0

#define SY_TYPE_TR069_SERVER     0
#define SY_TYPE_IPTV_CLIENT      1

#define SY_TYPE_FTP              0
#define SY_TYPE_HTTP             1
#define SY_TYPE_FTP_UPLOAD       0
#define SY_TYPE_HTTP_UPLOAD      1
#define SY_TYPE_FTP_DOWNLOAD     0
#define SY_TYPE_HTTP_DOWNLOAD    1

#define SY_COMMAND_LEN           1024
#define SY_USER_LEN              20
#define SY_MSG_LEN               4096

#define CMD_PROC_PORT       	 23416
#define SY_ZERO_CONFIG_PORT 	 28441
#define SY_ZERO_CONFIG_FLAG		 SY_ROOT_PATH"syZeroConfigFlag"


#define DOWNLOAD_FIRMWARE_UPGRADE_IMAGE 		   	'1'
#define DOWNLOAD_VERDOR_CONFIGURATION_FILE  		'3'
#define UPLOAD_VERDOR_CONFIGURATION_FILE            '1'
#define UPLOAD_VERDOR_LOG_FILE                      '2'

#define SY_CONFIG_XML_PATH      SY_XML_ROOT_PATH"syConfig.xml"

const char* getVer();
extern funPtrSetData g_setData;
extern funPtrGetData g_getData;
extern funPtrNotify g_notify;
extern int maxFuncNameLength;
extern int getMaxFuncNameLen(const char* funcName);

#include <android/log.h>
#define DEBUG_BUFFER_MAX_LENGTH 1023
#define DPrint(format, ...) \
		do{ \
			__android_log_print(ANDROID_LOG_DEBUG, TAG, \
					"%5d|%s|%*.*s| "format, gettid(), \
					getVer(), \
					maxFuncNameLength, \
					getMaxFuncNameLen(__FUNCTION__), \
					__FUNCTION__, ##__VA_ARGS__); \
		}while(0)
#define VPrint(format, ...) \
		do{ \
			__android_log_print(ANDROID_LOG_VERBOSE, TAG, \
					"%5d|%s|%*.*s| "format, gettid(), \
					getVer(), \
					maxFuncNameLength, \
					getMaxFuncNameLen(__FUNCTION__), \
					__FUNCTION__, ##__VA_ARGS__); \
		}while(0)
#define EPrint(format, ...) \
		do{ \
			__android_log_print(ANDROID_LOG_ERROR, TAG, \
					"%5d|%s|%*.*s| "format, gettid(), \
					getVer(), \
					maxFuncNameLength, \
					getMaxFuncNameLen(__FUNCTION__), \
					__FUNCTION__, ##__VA_ARGS__); \
		}while(0)
#define WPrint(format, ...) \
		do{ \
			__android_log_print(ANDROID_LOG_WARN, TAG, \
					"%5d|%s|%*.*s| "format, gettid(), \
					getVer(), \
					maxFuncNameLength, \
					getMaxFuncNameLen(__FUNCTION__), \
					__FUNCTION__, ##__VA_ARGS__); \
		}while(0)
#define PERROR(desc) \
            __android_log_print(ANDROID_LOG_ERROR, TAG,\
                    "%5d| "desc" %s(%d).\n",           \
                    gettid(),                          \
                    strerror(errno),                   \
                    errno)
#define SHOWMSG(showMsg) \
			DPrint("user:%s,\n cmd:%d,\n type:%d,\n msg:%s,\n len:%d",\
					(showMsg)->user,										      \
					(showMsg)->cmd, 										      \
					(showMsg)->type,										      \
					(showMsg)->msg, 										      \
					(showMsg)->len);


#define D(...) PrintLog(ANDROID_LOG_DEBUG,   TAG, __FUNCTION__, __VA_ARGS__)
#define E(...) PrintLog(ANDROID_LOG_ERROR,   TAG, __FUNCTION__, __VA_ARGS__)
#define V(...) PrintLog(ANDROID_LOG_VERBOSE, TAG, __FUNCTION__, __VA_ARGS__)
#define W(...) PrintLog(ANDROID_LOG_WARN,    TAG, __FUNCTION__, __VA_ARGS__)


#define SyCloseSocket(x) {shutdown(x, 2);close(x);}
#define CreateAndConn(p,t) CreateCliSocket(p,t,10)

typedef unsigned int uint;

typedef enum {
	GET_PARAM,//获取参数；
	SET_PARAM,//设置参数；
	SESSION_END//一个session的结束，在每个设置或者获取的一系列参数之后，加一个SESSION_NED,表示本次SEESION结束，可以做接下来的实现；
} _type;

typedef struct {
	int   cmd;
	_type type;
	int   len;
	char  msg[1024];
} Msg_APK_s;


typedef enum
{
    SY_OP_GET_PARAM,                  // TM ---> IPTV
    SY_OP_SET_PARAM,                  // TM ---> IPTV
    SY_OP_REBOOT,                     // TM ---> IPTV
    SY_OP_FACTORYRESET,               // TM ---> IPTV
    SY_OP_UPDATE,                     // TM ---> IPTV
    SY_OP_VALUE_CHANGE_REPORT,        // TM <--- IPTV
    SY_OP_START,                      // TM <--- IPTV
    SY_OP_HEART_BEAT,                 // TM <---> IPTV
    SY_OP_GET_PARAM_REPLY,            // TM <--- IPTV
    SY_OP_SET_PARAM_REPLY,            // TM <--- IPTV
    SY_OP_REBOOT_REPLY,               // TM <--- IPTV
    SY_OP_FACTORYRESET_REPLY,         // TM <--- IPTV
    SY_OP_UPDATE_REPLY,               // TM <--- IPTV
    SY_OP_VALUE_CHANGE_REPORT_REPLY,  // TM <--- IPTV
    SY_OP_START_FINISH,               // TM ---> IPTV
    SY_OP_HEAT_BEAT_REPLY,            // TM <---> IPTV
    SY_OP_LOG_CATCH,            	  // TM <--- IPTV

} syOpYype;

typedef struct sockmsg
{
    char        user[SY_USER_LEN];        //User  IPTV,user=IPTV; TR069,user=manager;
    int         cmd;                      //Command Value
    syOpYype    type;                     //Operation type
    char        msg[SY_MSG_LEN];          //Message
    int         len;                      //Message length
} Msg_TR069_s;

enum sy_method_type
{
    SY_UNKNOWN_METHOD,
    SY_GET_RPC_METHODS,
    SY_SET_PARAMETER_VALUES,
    SY_GET_PARAMETER_VALUES,
    SY_GET_PARAMETER_NAMES,
    SY_SET_PARAMETER_ATTRIBUTES,
    SY_GET_PARAMETER_ATTRIBUTES,
    SY_ADD_OBJECT,
    SY_DELETE_OBJECT,
    SY_REBOOT,
    SY_DOWNLOAD,
    SY_UPLOAD,
    SY_FACTORY_RESET,
    SY_GET_QUEUED_TRANSFERS,
    SY_GET_ALL_QUEUED_TRANSFERS,
    SY_SCHEDULE_INFORM,
    SY_SET_VOUCHERS,
    SY_GET_OPTIONS,
    SY_INFORM,
    SY_TRANSFER_COMPLETE,
    SY_AUTONOMOUS_TRANSFER_COMPLETE,
    SY_REQUEST_DOWNLOAD,
    SY_KICKED
};

typedef struct
{
    int    cmd;
    char*  cmdStr;
} syCmdStrStu;

typedef struct _syErrorCodeStu
{
    char  errorCodeTime[32];
    char  errorCodeValue[32];
} syErrorCodeStu;

#if 0
typedef struct
{
    int    type;
    int    socketFd;
} sy_type_sockfd_t;
#endif

pthread_t gSyErrorCodeManageTid;

int Send2CmdProcThd(struct sockmsg *msg);

bool SyInitConfigXml();
const char *SyMethodTypeToStr(enum sy_method_type type);
bool SyGetNodeValue(const char* path, char* value);
int SySetNodeValue(const char* path, char* value);
int SyGetNodeAttr(const char* path, char* value);
int SySetNodeAttr(const char* path, int notifyChange, int notify);
int SyGetNodeType(const char* path, char* type, const char*);
int SyGetNodeLevel(const char* path, char* level);
int SyGetNodeRW(const char* path, char* rw);
int SyGetNodeMaxLen(const char* path, char* maxLen);
int SyGetNodeMax(const char* path, char* max);
int SyGetNodeMin(const char* path, char* min);
int SyAddInstanceObj(char *path);
int SyDelInstanceObj(char *path);
int SyCalcNodeNum(char *path);
int SyGetNameList(char* path , char* nameList, char* nextLevel);
int SyGetSubString(char* str, int number, char* token, char* value);
int getprop(const char* name, char* value, const char* defaultValue);
void get_debug_switch();
void PrintLog(int proi, const char* tag, const char *function, const char *format, ...);

int CreateCliSocket(int port, int type, int timeout);
int readUptime();

int SetValueToIptv(const char* path, char* value);
int GetValueToIptv(const char* path, char* value); // 通过socket从so拿数据

bool SetValue(const char* path, char* value);
bool GetValue(const char* path, char* value, size_t); // 通过jni从apk拿数据

#endif


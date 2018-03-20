/***********************************************************************
*
* syCwmpSocket.h
*
* Copyright (C) 2013 Lin jieling <mycourage0@126.com>
* Copyright (C) 2003-2013 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/
#ifndef SYCWMPSOCKET_H
#define SYCWMPSOCKET_H

#include "syCwmpCommon.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#define MAX_CLIENT                             10
#define SY_TIMEOUT                             3
#define SY_TRY_TIMES                           3

#define SY_INTENT_TYPE_UPGRADE_CMD             1
#define SY_INTENT_TYPE_UPGRADE_RESULT          11

#define SY_RESULT_NEEDNOT_UPGRADE              0  //不需要升级
#define SY_RESULT_SUCCESS_UPGRADE              1  //升级成功无需重启
#define SY_RESULT_REBOOT                       2  //升级后第一次重启
#define SY_RESULT_FAIL_UPGRADE                 3  //升级失败无需重启
#define SY_RESULT_FAIL_UPGRADE_REBOOT          4  //升级后重启失败

//added by hitler 20130924
#define SY_IPTV_FACTORYRESET                   "1"
#define SY_SYSTEM_FACTORYRESET                 "2"
#define SY_ALL_FACTORYRESET                    "3"


#define SY_UPGRADE_SERVER_PORT                 23417
#define SY_TR069_SERVER_PORT                   23418
#define SY_TR069_ONEKEYCK_SERVER_PORT          23500 //一键检测服务端口

#define SY_WRITE_END         -1

#define QU "\""
#define BR "{"
#define RB "}"


typedef enum
{
    /*Network*/
    CONNECT_TYPE = 1,             //网络连接方式
    DHCP_USER = 2,                //DHCP用户名
    DHCP_PASSWD = 3,              //DHCP密码
    PPPOE_USER = 4,               //PPPOE用户名
    PPPOE_PASSWD = 5,             //PPPOE密码
    IPADDR = 6,	                //IP地址
    MASK = 7,                     //子网掩码
    DEFGATEWAY = 8,               //默认网关
    DNSMAIN = 9,                  //DNS1
    DNSBAK = 10,                  //DNS2
    MODELNAME = 11,
    Manufacturer = 12,
    ProductClass = 13,
    MAC = 21,                     //MAC地址

    /*Time*/
    TIMEZONE = 22,                //时区
    NTPMAIN = 23,                 //NTP地址1
    NTPBAK = 24,                  //NTP地址2

    /*Upgrade*/
    UPGRADE_URL = 25,             //升级地址
    UPGRADE_USER = 26,            //升级服务器用户名
    UPGRADE_PASSWD = 27,           //升级服务器密码
    STARTUPGRADE = 28,			//升级操作
    FORCEUPGRADE = 29, 			//强制升级
    UPGRADERESULT = 30,			//升级结果

    /*Basic Info*/
    MANUFACTORY = 32,              //生产厂商
    SN = 33,                       //序列号
    STBID = 34,                    //STBID
    HARDWARE_VER = 35,             //硬件版本号
    SOFTWARE_VER = 36,             //软件版本号
    LANGUAGE = 37,                 //语言

    /*TR069 Setting*/
    MANAGESERVER_URL = 38,         //网管认证地址
    MANAGESERVER_USER = 39,        //网管用户名
    MANAGESERVER_PASSWD = 40,      //网管密码
    HEARTBEAT = 41,                //心跳开关
    HEARTBEAT_INTERVAL = 42,       //心跳周期
    CPEUSER = 43,                  //CPEUSER
    CPEPASSWD = 44,                //CPEPASSWD

    /*Authentication*/
    AUTH_MAINURL = 45,             //业务主认证地址
    AUTH_BAKURL = 46,              //业务备认证地址
    IPTVUSER = 47,                 //业务用户名
    IPTVPASSWD = 48,	             //业务密码

    /*STB Reboot*/
    REBOOT = 49,

    /*Log Report*/
    LOGSERVERURL = 55, 		    //FTP address
    LOGSERVERUSER = 56,			//FTP username
    LOGSERVERPASSWD  = 57,		//FTP Password
    LOGDURATION = 58,				//log time
    LOGMSGORFILE = 59, 			//log or file
    IGMPINFO = 60, 				//IGMP info
    HTTPINFO = 61,
    RTSPINFO = 62,
    PKGTOTALONESEC = 63,			//PkgTotalOneSec
    BYTETOTALONESEC = 64,
    PKGLOSTRATE = 65,
    AVARAGERATE = 66,
    BUFFER = 67,
    ERROR = 68,
    LOGSWITCH = 69,				//log upload switcher

    SUPPORTPROTOCOLS = 90,        //StreamingControlProtocols
    TRANSPORTPROTOCOLS,           //StreamingTransportProtocols
    TRANSPORTCTLPROTOCOLS,	    //StreamingTransportControlProtocols
    PLEXTYPE,	                    //MultiplexTypes
    DEJITTERBUFFERSIZE,	        //MaxDejitteringBufferSize
    AUDIOSTANDARDS,	            //AudioStandards
    VIDEOSTANDARDS = 96,          //VideoStandards
    QOSSERVERURL = 97,            //QosServerUrl
    QOSUPLOADINTERVAL = 98,
    RECORDINTERVAL = 99,          //LogRecordInterval
    MONITORINGINTERVAL = 100,     //MonitoringInterval
    VIDEOLOGINFO = 101,	        //视频性能日志文件
    DIAGNOSTICSSTATE,             //DiagnosticsState
    PLAYURL,                      //PlayURL
    PLAYSTATE,                    //PlayState


#ifdef SY_SYSLOG
    LOGTYPE = 199,
    LOGLEVEL,
    LOGOUTPUTTYPE,
    SYSLOGSERVER,
    SYSLOGSTARTTIME,
    SYSLOGCONTINUETIME,
    SYSLOGTIMER,
    SYSLOGFTPSERVER,
#endif

    CHANGED = 501,
    SCREENOFF	= 502,
    RESTOREFACTORY = 503,
    FULLSCREENSTATE = 504,
    SWITCHCHANNEL = 506,
    WORKTIME = 507,
    REALBITRATE = 508,
    ORIGINBITRATE = 509,
    FINALFAILAUTHURL = 510,
    ISPLUGCABLE = 511,    // Is plug network cable in statistical preiod or not
    UNPLUGTOTALTIME = 512, //Total time of Unpluged network cable
    SCREENON = 520,

#ifdef SY_AUTO_ON_OFF
    //AutoOnOffConfiguration
    IS_AUTO_POWERON = 522,
    AUTO_POWERON_TIME = 523,
    AUTO_SHUTDOWN_TIME = 524,
#endif

    REPORT_IPADDR = 600,                   //上报IP地址

    GETORSETDATA = 1001,
    JNIDETACH = 1002,
    REALEND = 1003,
    FACTORYRESET = 1004,
    EPGSERVERURL = 1005,
    SQMSERVERURL = 1006,//SQM server URL

    ADD_ROUTE = 10007,
    ERROR_INFO = 10009,
    EPG_MODIFY = 10010,
    REMOTEDUG = 13011,
} syIptvConfigParam;




void* CmdProcThread(void* data);
//void* syClientSocketRevCmdHandle(void *data);
int RevCmdStart();
int SendDataToIptv(struct sockmsg *pMsg, short port);
int Send2zeroApk(int flag);
int startLogcat(char *cmd);
void HandleConnectRequest();
void HandleDownload(char *pos);
void HandleUpload(char* pos);
void HandleUtil(pthread_t *threadid, iFunction func, const char *key);

#endif

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

#define SY_RESULT_NEEDNOT_UPGRADE              0  //����Ҫ����
#define SY_RESULT_SUCCESS_UPGRADE              1  //�����ɹ���������
#define SY_RESULT_REBOOT                       2  //�������һ������
#define SY_RESULT_FAIL_UPGRADE                 3  //����ʧ����������
#define SY_RESULT_FAIL_UPGRADE_REBOOT          4  //����������ʧ��

//added by hitler 20130924
#define SY_IPTV_FACTORYRESET                   "1"
#define SY_SYSTEM_FACTORYRESET                 "2"
#define SY_ALL_FACTORYRESET                    "3"


#define SY_UPGRADE_SERVER_PORT                 23417
#define SY_TR069_SERVER_PORT                   23418
#define SY_TR069_ONEKEYCK_SERVER_PORT          23500 //һ��������˿�

#define SY_WRITE_END         -1

#define QU "\""
#define BR "{"
#define RB "}"


typedef enum
{
    /*Network*/
    CONNECT_TYPE = 1,             //�������ӷ�ʽ
    DHCP_USER = 2,                //DHCP�û���
    DHCP_PASSWD = 3,              //DHCP����
    PPPOE_USER = 4,               //PPPOE�û���
    PPPOE_PASSWD = 5,             //PPPOE����
    IPADDR = 6,	                //IP��ַ
    MASK = 7,                     //��������
    DEFGATEWAY = 8,               //Ĭ������
    DNSMAIN = 9,                  //DNS1
    DNSBAK = 10,                  //DNS2
    MODELNAME = 11,
    Manufacturer = 12,
    ProductClass = 13,
    MAC = 21,                     //MAC��ַ

    /*Time*/
    TIMEZONE = 22,                //ʱ��
    NTPMAIN = 23,                 //NTP��ַ1
    NTPBAK = 24,                  //NTP��ַ2

    /*Upgrade*/
    UPGRADE_URL = 25,             //������ַ
    UPGRADE_USER = 26,            //�����������û���
    UPGRADE_PASSWD = 27,           //��������������
    STARTUPGRADE = 28,			//��������
    FORCEUPGRADE = 29, 			//ǿ������
    UPGRADERESULT = 30,			//�������

    /*Basic Info*/
    MANUFACTORY = 32,              //��������
    SN = 33,                       //���к�
    STBID = 34,                    //STBID
    HARDWARE_VER = 35,             //Ӳ���汾��
    SOFTWARE_VER = 36,             //����汾��
    LANGUAGE = 37,                 //����

    /*TR069 Setting*/
    MANAGESERVER_URL = 38,         //������֤��ַ
    MANAGESERVER_USER = 39,        //�����û���
    MANAGESERVER_PASSWD = 40,      //��������
    HEARTBEAT = 41,                //��������
    HEARTBEAT_INTERVAL = 42,       //��������
    CPEUSER = 43,                  //CPEUSER
    CPEPASSWD = 44,                //CPEPASSWD

    /*Authentication*/
    AUTH_MAINURL = 45,             //ҵ������֤��ַ
    AUTH_BAKURL = 46,              //ҵ����֤��ַ
    IPTVUSER = 47,                 //ҵ���û���
    IPTVPASSWD = 48,	             //ҵ������

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
    VIDEOLOGINFO = 101,	        //��Ƶ������־�ļ�
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

    REPORT_IPADDR = 600,                   //�ϱ�IP��ַ

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

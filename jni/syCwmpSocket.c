/***********************************************************************
*
* syCwmpSocket.c
*
* Copyright (C) 2012 Lin jieling <mycourage0@126.com>
* Copyright (C) 2003-2012 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <openssl/des.h>
#include <sys/stat.h>
#include <netdb.h>

#include "syCwmpCommon.h"
#include "syCwmpSocket.h"
#include "syCwmpCrypto.h"
#include "syCwmpLog.h"
#include "syCwmpManagement.h"
#include "cJSON.h"
#include "cJSON_Utils.h"
#include "syCwmpTaskQueue.h"
#include "paramParse.h"

#define AV_SERVER     "125.88.69.132"	//guangdong

extern char sendZeroBuf[];
extern int gSyIsCPEStart;
extern int syNATCmd;
extern int LogTypeCmd;

extern int debugTsApkCollectstop;
extern syManagementServerStruct gSyManagementServerStu;

int zeroConfigFlag			= 0;
int gExit                   = 0;
int gSyIsTmStart            = 0;
int gSyTR069Sockfd          = -1;
int gSyGetValueSockfd       = -1;
int g_msg2cmdProc_fd        = -1;
int gSyUpgradeClientsockfd  = -1;
int gSyIsShutdownFlag  = 0;  //0:Not Shutdown 1:Shutdown
int gSyIsUpdateReply = 0;
int gSyUpdateSuccess = 0;
static int drawSocket = -1;
static int onekeySocket = -1;

/* 调试APK设置的默认数据抓取时间 */
int gInfoTime = 0;

int gNetChangFlag = 0;

/* 调试APK发送过来的U盘地址 */
char gMountPath[512] = {0};

pthread_mutex_t gMutexLock = PTHREAD_MUTEX_INITIALIZER;

int syCwmpSession = 1;
struct sockmsg gSyRecvSockmsg;
struct sockmsg gSySendSockmsg;
struct sockmsg gSySendSockmsgToOneKeyCK;

pthread_t gSyIpPingTestTid;
pthread_t gSyTraceRouteTestTid;
int syEPGmodifySupported;
extern int gSyErrorCodeNum;
extern syErrorCodeStu gSyErrorCodeArr[20];
pthread_t gSyBandwidthDiagnosticsTid;
pthread_t gSyPacketCaptureTid;

pthread_t gSyDebugInfoTid;
pthread_t gSyStartupInfoTid;
pthread_t gSyTsAPKDebugThreadId;
pthread_t gSyTsAPKStartupThreadId;

pthread_t gSyPeriodMsgLogTid;

#define WRITE 1
#define READ  0
//add by andy.zhao 2014-3-3 17:42:27
int gSyNatSession = SY_FALSE;

typedef struct {
    int    cmd;
    int    attr;         //read/write
    char*  key;
} data_key_path_t;

static data_key_path_t g_keyList[] = {

    {CONNECT_TYPE,   				WRITE, "ConnectMode"},
    {DHCP_USER,      				WRITE, "DHCPUserName"},
    {DHCP_PASSWD,    				WRITE, "DHCPPassword"},
    {PPPOE_USER,     				WRITE, "PPPOEUserName"},
    {PPPOE_PASSWD,   				WRITE, "PPPOEPassword"},
    {IPADDR,         				WRITE, "IpAddress"}, 
    {MASK,           				WRITE, "NetMask"},
    {DEFGATEWAY,     				WRITE, "DefaultGate"},
    {DNSMAIN,        				WRITE, "DNS"},
    {DNSBAK,         				WRITE, "SecondNDS"},
    {MAC,            				READ,  "Mac"},
    {MODELNAME,            		    READ,  "ModelName"},
    {Manufacturer,            	    READ,  "Manufacturer"},
    {ProductClass,            	    READ,  "ProductClass"},

	{PLAYURL, 					    WRITE, "PlayUrl"},
    {REMOTEDUG, 				    WRITE, "RemoteControlEnable"},
    {PLAYSTATE,					    READ,  "PlayUrl"},

    {TIMEZONE,       				WRITE, "TimeZone"},
    {NTPMAIN,        				WRITE, "ServerNTPUrl"},  
    {NTPBAK,         				WRITE, "ServerNTPBackupUrl"},
                                       				                        				
    //{UPGRADE_URL,    				WRITE, "UpGradeUrl"},
    //{UPGRADE_USER,   				WRITE, "config_Upgrade_UserName"},
    //{UPGRADE_PASSWD, 				WRITE, "config_Upgrade_Password"},
	{FORCEUPGRADE, 			        WRITE, "ForceUpgrade"},
	//{UPGRADERESULT,               WRITE, "UpgradeResult"},
                   				
    {AUTH_MAINURL,   				WRITE, "IPTVauthURL"},
    {AUTH_BAKURL,    				WRITE, "SecondIPTVauthURL"},
    {IPTVUSER,       				WRITE, "IPTVaccount"},
    //{IPTVUSER,       				WRITE, "IPTVaccount"},
    {IPTVPASSWD,     				WRITE, "IPTVpassword"},

    {MANUFACTORY,    				READ,  "Manufacturer"}, 
    //{SN,             				READ,  "SerialNumber"},  
    {STBID,          				READ,  "STBID"},
    {HARDWARE_VER,   				READ,  "HardwareVersion"},
    {SOFTWARE_VER,   				READ,  "SoftwareVersion"},
    {LANGUAGE,       				WRITE, "ChooseLanguage"},
	{SQMSERVERURL,				    WRITE, "MQMCURL"},
	{EPGSERVERURL,                  READ,  "ServerBaseUrl"},
	//{REMOTESTATE,					WRITE, "RemoteDebugState"},

    //{MANAGESERVER_ENABLE,         WRITE, "Config_WebMaster"},
    {MANAGESERVER_URL,   		    WRITE, "config_WebmasterUrl"},
    {MANAGESERVER_USER,  		    WRITE, "config_UserName"},
    {MANAGESERVER_PASSWD,		    WRITE, "config_Password"},

    {HEARTBEAT,          		    WRITE, "Congfig_HeartBear"},
    {HEARTBEAT_INTERVAL, 	        WRITE, "config_HeartBeat"},
    {CPEUSER,            		    WRITE, "config_CPEUser"},
    {CPEPASSWD,          		    WRITE, "config_CPEPassword"},

    {SUPPORTPROTOCOLS,            READ,  "config_SupportProtocols"},
    {TRANSPORTPROTOCOLS,          READ,  "config_TransportProtocols"},
    {TRANSPORTCTLPROTOCOLS,       READ,  "config_TransportCTLProtocols"},
    {PLEXTYPE,                    READ,  "config_Plextype"},
    {DEJITTERBUFFERSIZE,          READ,  "config_DejitterBufferSize"},
    //{IPTV_CONFIG_AUDIOSTANDARDS,                    READ,  "config_AudioStandards"},
    {VIDEOSTANDARDS,              READ,  "config_VideoStandards"},
    {QOSSERVERURL,                WRITE, "config_QosServerUrl"},
    {QOSUPLOADINTERVAL,           WRITE, "config_LoginTerval"},
    {RECORDINTERVAL,              WRITE, "config_RecordInterval"},
    {MONITORINGINTERVAL,          WRITE, "config_MonitoringInterval"},

    {LOGSWITCH,                   WRITE, "LogSwitch"},
	{LOGSERVERURL,                WRITE, "config_LogServerUrl"},
	{LOGSERVERUSER,               WRITE, "LogserverUser"},
	{LOGSERVERPASSWD,             WRITE, "LogserverPassword"},
	{LOGDURATION,				  WRITE, "Logduration"},
	{LOGMSGORFILE,                WRITE, "LogMsgOrFile"},

    {LOGTYPE,                     WRITE, "LogType"},
    {LOGLEVEL,                    WRITE, "LogLevel"},
    {LOGOUTPUTTYPE,               WRITE, "LogOutPutType"},
    {SYSLOGSERVER,                WRITE, "SyslogServer"},
    {SYSLOGSTARTTIME,             WRITE, "SyslogStartTime"},
    {SYSLOGCONTINUETIME,          WRITE, "SyslogContinueTime"},
    {SYSLOGTIMER,                 WRITE, "LogTimer"},
    {SYSLOGTIMER,                 WRITE, "LogFtpServer"},

    {IS_AUTO_POWERON,             WRITE, "IsAutoPowerOn"},
    {AUTO_POWERON_TIME,           WRITE, "AutoPowerOnTime"},
    {AUTO_SHUTDOWN_TIME,          WRITE, "AutoShutdownTime"},

	//{EPGIP,						READ,  "ServerBaseUrl"},
};

syCmdStrStu gSyCmdStrList[] =
{
    {ADD_ROUTE, 		"Device.AddRoute"},
    {REMOTEDUG,		"Device.X_00E0FC.RemoteControlEnable"},

    {MODELNAME, 		"Device.DeviceInfo.ModelName"},
    {Manufacturer, 	"Device.DeviceInfo.Manufacturer"},
    {ProductClass, 	"Device.DeviceInfo.ProductClass"},

    {EPG_MODIFY, 		"Device.ManagementServer.URLModifyFlag"},

    //PlayDiagnostics
    {DIAGNOSTICSSTATE, "Device.X_00E0FC.PlayDiagnostics.DiagnosticsState"},
    {PLAYURL, 		"Device.X_00E0FC.PlayDiagnostics.PlayURL"},
    {PLAYSTATE, 		"Device.X_00E0FC.PlayDiagnostics.PlayState"},

    //syslog
    {LOGTYPE, 		"Device.X_00E0FC.LogParaConfiguration.LogType"},
    {LOGLEVEL, 		"Device.X_00E0FC.LogParaConfiguration.LogLevel"},
    {LOGOUTPUTTYPE, 	"Device.X_00E0FC.LogParaConfiguration.LogOutPutType"},
    {SYSLOGSERVER, 	"Device.X_00E0FC.LogParaConfiguration.SyslogServer"},
    {SYSLOGSTARTTIME, "Device.X_00E0FC.LogParaConfiguration.SyslogStartTime"},
    {SYSLOGCONTINUETIME, "Device.X_00E0FC.LogParaConfiguration.SyslogContinueTime"},
    {SYSLOGTIMER, 	"Device.X_00E0FC.LogParaConfiguration.LogTimer"},
    {SYSLOGFTPSERVER, "Device.X_00E0FC.LogParaConfiguration.LogFtpServer"},

    //Network
    {CONNECT_TYPE, 	"Device.LAN.AddressingType"},
    {DHCP_USER, 		"Device.X_00E0FC.ServiceInfo.DHCPID"},
    {DHCP_PASSWD, 	"Device.X_00E0FC.ServiceInfo.DHCPPassword"},
    {PPPOE_USER, 		"Device.X_00E0FC.ServiceInfo.PPPoEID"},
    {PPPOE_PASSWD, 	"Device.X_00E0FC.ServiceInfo.PPPoEPassword"},

    {IPADDR, 			"Device.LAN.IPAddress"},
    {MASK, 			"Device.LAN.SubnetMask"},
    {DEFGATEWAY, 		"Device.LAN.DefaultGateway"},
    {DNSMAIN, 		"Device.LAN.DNSServers"},
    {DNSBAK, 			"Device.LAN.DNSServers2"},
    {MAC, 			"Device.LAN.MACAddress"},

    //Time
    {TIMEZONE, 		"Device.Time.LocalTimeZone"},
    {NTPMAIN, 		"Device.Time.NTPServer1"},
    {NTPBAK, 			"Device.Time.NTPServer2"},

    //Authentication
    {AUTH_MAINURL, 	"Device.X_00E0FC.ServiceInfo.AuthURL"},
    {AUTH_BAKURL, 	"Device.X_00E0FC.ServiceInfo.AuthURLBackup"},
    {IPTVUSER, 		"Device.X_00E0FC.ServiceInfo.UserID"},
    {IPTVPASSWD, 		"Device.X_00E0FC.ServiceInfo.UserIDPassword"},


    //Basic
    //{MANUFACTORY, "Device.DeviceInfo.Manufacturer"},
    {SN, 				"Device.DeviceInfo.SerialNumber"},
    {STBID, 			"Device.X_00E0FC.STBID"},
    {HARDWARE_VER, 	"Device.DeviceInfo.HardwareVersion"},
    {SOFTWARE_VER, 	"Device.DeviceInfo.SoftwareVersion"},
    //{LANGUAGE, "Device.UserInterface.CurrentLanguage"},

    //TR069
    {MANAGESERVER_URL, "Device.ManagementServer.URL"},
    //{MANAGESERVER_USER, "Device.ManagementServer.Username"},
    //{MANAGESERVER_PASSWD, "Device.ManagementServer.Password"},
    {HEARTBEAT, 		"Device.ManagementServer.PeriodicInformEnable"},
    {HEARTBEAT_INTERVAL, "Device.ManagementServer.PeriodicInformInterval"},
    //{CPEUSER, "Device.ManagementServer.ConnectionRequestUsername"},
    //{CPEPASSWD, "Device.ManagementServer.ConnectionRequestPassword"},


    {QOSSERVERURL, 	"Device.X_00E0FC.StatisticConfiguration.LogServerUrl"},
    {QOSUPLOADINTERVAL, "Device.X_00E0FC.StatisticConfiguration.LogUploadInterval"},
    {RECORDINTERVAL,  "Device.X_00E0FC.StatisticConfiguration.LogRecordInterval"},
    {MONITORINGINTERVAL, "Device.X_00E0FC.StatisticConfiguration.StatInterval"},
   
    //Log Report Config
    {LOGMSGORFILE, "Device.X_CTC_IPTV.LogMsg.MsgOrFile"},
    {LOGSWITCH, 	 "Device.X_CTC_IPTV.LogMsg.Enable"},
    {LOGSERVERURL, "Device.X_CTC_IPTV.LogMsg.LogFtpServer"},
    {LOGSERVERUSER, "Device.X_CTC_IPTV.LogMsg.LogFtpUser"},
    {LOGSERVERPASSWD, "Device.X_CTC_IPTV.LogMsg.LogFtpPassword"},
    {LOGDURATION, "Device.X_CTC_IPTV.LogMsg.Duration"},


    {IS_AUTO_POWERON, 	"Device.X_00E0FC.AutoOnOffConfiguration.IsAutoPowerOn"},
    {AUTO_POWERON_TIME, 	"Device.X_00E0FC.AutoOnOffConfiguration.AutoPowerOnTime"},
    {AUTO_SHUTDOWN_TIME,  "Device.X_00E0FC.AutoOnOffConfiguration.AutoShutdownTime"},

    //Write End Flag
    {SY_WRITE_END, ""},

    //{LANGUAGE, "Device.UserInterface.CurrentLanguage"},
};

syCmdStrStu gSyAutoReportList[] =
{
    {NTPMAIN, 		"Device.Time.NTPServer1"},
    {MANAGESERVER_URL,"Device.ManagementServer.URL"},
    {SOFTWARE_VER, 	"Device.DeviceInfo.SoftwareVersion"},

    {TIMEZONE, 		"Device.Time.LocalTimeZone"},

    {LANGUAGE, 		"Device.UserInterface.CurrentLanguage"},

    {IPADDR, 			"Device.LAN.IPAddress"},
    {CONNECT_TYPE, 	"Device.LAN.AddressingType"},
    {MAC, 			"Device.LAN.MACAddress"},

    {PPPOE_USER, 		"Device.X_00E0FC.ServiceInfo.PPPoEID"},
    {PPPOE_PASSWD, 	"Device.X_00E0FC.ServiceInfo.PPPoEPassword"},
    {DHCP_USER, 		"Device.X_00E0FC.ServiceInfo.DHCPID"},
    {DHCP_PASSWD, 	"Device.X_00E0FC.ServiceInfo.DHCPPassword"},
    {AUTH_MAINURL, 	"Device.X_00E0FC.ServiceInfo.AuthURL"},
    {AUTH_BAKURL, 	"Device.X_00E0FC.ServiceInfo.AuthURLBackup"},
    {IPTVUSER, 		"Device.X_00E0FC.ServiceInfo.UserID"},
    {IPTVPASSWD, 		"Device.X_00E0FC.ServiceInfo.UserIDPassword"},

};

syCmdStrStu gSyMsgLogList[] =
{
    {IGMPINFO, 		"Device.X_CTC_IPTV.LogMsg.IGMPInfo"},
    {HTTPINFO, 		"Device.X_CTC_IPTV.LogMsg.HTTPInfo"},
    {RTSPINFO, 		"Device.X_CTC_IPTV.LogMsg.RTSPInfo"},
    {PKGTOTALONESEC,  "Device.X_CTC_IPTV.LogMsg.PkgTotalOneSec"},
    {BYTETOTALONESEC, "Device.X_CTC_IPTV.LogMsg.ByteTotalOneSec"},
    {PKGLOSTRATE, 	"Device.X_CTC_IPTV.LogMsg.PkgLostRate"},
    {AVARAGERATE, 	"Device.X_CTC_IPTV.LogMsg.AvarageRate"},
    {BUFFER, 			"Device.X_CTC_IPTV.LogMsg.BUFFER"},
    {ERROR, 			"Device.X_CTC_IPTV.LogMsg.ERROR"},
};
int gSyMsgLogListLen = sizeof(gSyMsgLogList)/sizeof(syCmdStrStu);
int gSyCmdStrListLen = sizeof(gSyCmdStrList)/sizeof(syCmdStrStu);
int gSyAutoReportListLen = sizeof(gSyAutoReportList)/sizeof(syCmdStrStu);

extern syLANStruct gSyLANStu;
extern syTimeStruct gSyTimeStu;
extern syGlobalParmStruct  gsyGlobalParmStru;
extern syServiceInfoStruct gSyServiceInfoStu;
extern syAcsCpeParamStruct gsyAcsCpeParamStru;

extern void* syTSApkStartupInfoThread(void *data);
extern void* syTSApkDebugInfoThread(void *data);
LOCAL int DispatchCommand(const char* pCommand, int commandLen);
LOCAL int DispatchOneKeyCK(const char* pCommand, int commandLen);
LOCAL int CreateSrvSocket(unsigned short port);
LOCAL void* RevCmdHandle(void *data);
LOCAL int Send2UpgradeCli(const char* msg, int msgLen);
void HandleIptvReq(struct sockmsg *pMsg);
LOCAL int HandleZeroApkReq(void);
LOCAL int HandleJson(int fd, char *str);
void AssemblyJSON(int fd, char *state, char *tsTime, char *tsErrno, char *showstream, char *showott);
char* ParseJSON(cJSON *json, char *dstName, char *result);


void* CmdProcThread(void* data)
{
    struct timeval timeout;
	
	DO;

	int i = 0;
	int b_val = 1;
	int nRet = -1;
	int fdmax = -1;
	int length = 0;
	int serverfd = -1;
	int clientfd[MAX_CLIENT] = {0};
	fd_set readfds;
	struct sockaddr_in serverAddr;
	
	memset(clientfd, 0xFF, sizeof(clientfd));
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(CMD_PROC_PORT);
	
	do {
		serverfd = socket(PF_INET, SOCK_STREAM, 0);
		setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, (char *)&b_val, sizeof(b_val));
		nRet = bind(serverfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
		DPrint("serverfd:%d, bind result:%d\n", serverfd, nRet);
		if (nRet < 0) {
			DPrint("bind failed.\n");
			SyCloseSocket(serverfd);
			usleep(1000 * 1000);
			continue;
		}
		nRet = listen(serverfd, MAX_CLIENT);
		DPrint("listen result:%d\n", nRet);
		if (nRet < 0) {
			DPrint("listen failed.\n");
			SyCloseSocket(serverfd);
			usleep(1000 * 1000);
			continue;
		}
		
		fdmax = serverfd;
		
		while (0 == gExit)
		{
			FD_ZERO(&readfds);
			FD_SET(serverfd, &readfds);
			for(i = 0; i < MAX_CLIENT; i++)
			{
				if (clientfd[i] != -1)
				{
					if (clientfd[i] > fdmax)
						fdmax = clientfd[i];
					FD_SET(clientfd[i], &readfds);
				}
			}
			timeout.tv_sec = 3;
			timeout.tv_usec = 0;
			nRet = select(fdmax + 1, &readfds, NULL, NULL, (struct timeval *)&timeout);
			if (nRet <= 0) {
				if(-1 == nRet) PERROR("select");
				continue;
			}
			
			if (FD_ISSET(serverfd, &readfds) > 0) {
				struct sockaddr_in RecvAddr;
				int nLen = sizeof(RecvAddr);
				memset(&RecvAddr, 0, sizeof(RecvAddr));
				int newfd = accept(serverfd, (struct sockaddr *)&RecvAddr, &nLen);
	
				struct in_addr peerAddr;
				peerAddr.s_addr = 0;
				peerAddr.s_addr = RecvAddr.sin_addr.s_addr;
				DPrint("Peer ip %s:%d, Net-Order(BE) port=%d\n",
						inet_ntoa(peerAddr), ntohs(RecvAddr.sin_port), RecvAddr.sin_port);
	
				gSyTR069Sockfd = newfd;
				for(i = 0; i < MAX_CLIENT; i++) {
					if (clientfd[i] == -1) {
						clientfd[i] = newfd;
						DPrint("New connection coming.[%d]\n", newfd);
						break;
					}
				}
				
				continue;
			}
	
			for(i = 0; i < MAX_CLIENT; i++) {
				if (-1 == clientfd[i]) {
					continue;
				}
	
				if (!(FD_ISSET(clientfd[i], &readfds) > 0)) {
					continue;
				}
	
				memset(&gSyRecvSockmsg, 0, sizeof(gSyRecvSockmsg));
				length = recv(clientfd[i], &gSyRecvSockmsg, sizeof(gSyRecvSockmsg), 0);
	
				switch(length) {
				case -1:
	
					sleep(1);
					DPrint("Recv():%s", strerror(errno));
					break;
				case 0:
				
					if (clientfd[i] == gSyTR069Sockfd)
					{
						gSyTR069Sockfd = -1;
					}
					SyCloseSocket(clientfd[i]);
					clientfd[i] = -1;
					DPrint("Closed by peer.");
					break;
				case sizeof(struct sockmsg):
	
					if(strcmp(gSyRecvSockmsg.user, "tr069") == 0)
					{
						DispatchCommand(gSyRecvSockmsg.msg, gSyRecvSockmsg.len);
					}
					else if (strcmp(gSyRecvSockmsg.user, "onekeyCK") == 0)
					{
						DispatchOneKeyCK(gSyRecvSockmsg.msg, gSyRecvSockmsg.len);
						if((send(clientfd[i], &gSySendSockmsgToOneKeyCK, sizeof(gSySendSockmsgToOneKeyCK), 0)) < 0)
						{
							DPrint("send() failed\n");
						}
					}
					else if (strcmp(gSyRecvSockmsg.user, "IPTV") == 0)
					{
						//运作iptv的请求
						HandleIptvReq(&gSyRecvSockmsg);
						if ((0 == gSyIsTmStart) && (SY_OP_START != gSyRecvSockmsg.type)) {
							DPrint("No Start..\n");
							continue;
						}
						if(-1 == send(clientfd[i], &gSySendSockmsg, sizeof(gSySendSockmsg), MSG_NOSIGNAL)) {
	
							DPrint("Send Error\n");
						}
						//DPrint("gSyRecvSockmsg.type:%d\n", gSyRecvSockmsg.type);
						
					}
					break;
				default: {
					
					char jsonBuf[2048] = {0};
					strncpy(jsonBuf, (char *)&gSyRecvSockmsg, sizeof(jsonBuf));
					/* json 数据*/
					if (NULL != strstr(jsonBuf, "{"))
					{
						HandleJson(clientfd[i], jsonBuf);
					}
					/* 零配置APK发送过来的零配置消息 */
					else
					{
						char recvBuf[64] = {0};
						sscanf(((char*)&gSyRecvSockmsg), "%[^:]:", recvBuf);
						if (0 == strcmp(recvBuf, "zeroSet"))
						{
							if((send(clientfd[i], "ok", 2, 0)) < 0)
							{
								DPrint("send data to zero apk failed.\n");
							}
							HandleZeroApkReq();
						}
						else
						{
							DPrint("malformed! recv.msg:%s\n", gSyRecvSockmsg.msg);
						}
					}
					break;
				}
				}
			}
		}
	}while (0 == gExit);

		DONE;
		return NULL;
	}



int CreateCliSocket(int port, int type, int timeout)
{
    int nRet = 0;
    int socketFd = -1;
    struct sockaddr_in clientAddr;

    DO;

    memset(&clientAddr, 0, sizeof(clientAddr));
    socketFd = socket(PF_INET, type, 0); // SOCK_STREAM
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    clientAddr.sin_port = htons(port);

    struct timeval tv;
    tv.tv_usec = timeout%1000;
    tv.tv_sec = timeout/1000;
    setsockopt(socketFd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv));
    nRet = connect(socketFd, (struct sockaddr *)&clientAddr, sizeof(clientAddr));

    if (nRet < 0)
    {
        EPrint("socket %d. Failed to connect() :%s\n",
               socketFd, strerror(errno));
        SyCloseSocket(socketFd);
        return nRet;
    }

    struct in_addr localAddr;
	struct sockaddr_in RecvAddr;
	localAddr.s_addr = 0;
	int nLen = sizeof(RecvAddr);
	memset(&RecvAddr, 0, sizeof(RecvAddr));
	RecvAddr.sin_family = AF_INET;
	if (getsockname(socketFd, (struct sockaddr *)&RecvAddr, &nLen) == 0)
	{
		localAddr.s_addr = RecvAddr.sin_addr.s_addr;
		DPrint("local ip %s:%d, Net-Order(BE) port=%d\n",
				inet_ntoa(localAddr), ntohs(RecvAddr.sin_port), RecvAddr.sin_port);
	}
    DPrint("socket %d, connect %s\n",
           socketFd, 0==nRet?"success":"failure");

    DONE;

    return socketFd;
}


LOCAL int CreateSrvSocket(unsigned short port)
{
    int regSocket = -1;
    struct sockaddr_in sock_addr;

    DO;

    regSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (regSocket == -1)
    {
        DPrint("Create Socket Fail\n");
        return SY_FAILED;
    }

    if (port != 0)
    {
        bzero((char *)(&sock_addr), sizeof(sock_addr));
        sock_addr.sin_family = AF_INET;
        sock_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        sock_addr.sin_port = htons(port);
        if (bind(regSocket, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) != 0)
        {
            DPrint("bind error\n");
            close(regSocket);
            regSocket = -1;
            return SY_FAILED;
        }
    }
    DONE;
    return regSocket;
}



LOCAL void* RevCmdHandle(void *data)
{
    int nResult = -1;
    int selectresult = -1;
    char* pPos = NULL;
    char aType[32] = {0};
    char aMsg[512] = {0};
    char aResult[16] = {0};
    char cmdBuf[2048] = {0};
    fd_set readfds;
    struct sockaddr_in addr;

    memset(&addr, 0,sizeof(addr));
    socklen_t len = sizeof(addr);

    while(1)
    {
        int	revSize = 0;
        FD_ZERO(&readfds);
        /* onekeySocket 始终等于 -1, 这句一旦执行就会 crash.
         * man manuals 中有这么一句:
         * An  fd_set is a fixed size buffer.  Executing FD_CLR() or FD_SET() with a value of fd
         * that is negative or is equal to or larger than FD_SETSIZE will result in undefined behavior.
         */
        // TODO: 这是现网代码,不然这么危险的一句不能活到现在.
        FD_SET(onekeySocket, &readfds);
        memset(cmdBuf, 0, sizeof(cmdBuf));

        selectresult = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
        if (selectresult <= 0)
        {
            DPrint("select error\n");
            usleep(50 * 1000);
            continue;
        }

        revSize = recvfrom(onekeySocket, cmdBuf, sizeof(cmdBuf), 0,(struct sockaddr*)&addr, &len);
        if (revSize <= 0)
        {
            DPrint("control recv error \n");
            usleep(50 * 1000);
            continue;
        }
        else
        {
            DPrint("cmdBuf:%s\n", cmdBuf);
            if (1 == gSyIsUpdateReply)
            {
                DPrint("Update Reply already\n");
                usleep(50 * 1000);
                memset(cmdBuf, 0x00, sizeof(cmdBuf));
                sprintf(cmdBuf, "%s", "200");
                //接受到想数据之后又发送过去cmdbuf????
                sendto(drawSocket, cmdBuf, strlen(cmdBuf), 0,(struct sockaddr*)&addr, sizeof(addr));
                continue;
            }
            pPos = strstr(cmdBuf, "UPGRADE");
            DPrint("pPos:%p\n", pPos);
            if (pPos != NULL)
            {
                sscanf(cmdBuf, "%*[^type]type\":%[^,],", aType);
                DPrint("aType:%s\n", aType);
                if (SY_INTENT_TYPE_UPGRADE_RESULT == atoi(aType))
                {
                    sscanf(cmdBuf, "%*[^msg]msg\":\"%[^\"]\"", aMsg);
                    DPrint("aMsg:%s\n", aMsg);
                    pPos = strstr(aMsg, "+");
                    DPrint("pPos:%p\n", pPos);
                    if (pPos != NULL)
                    {
                        strcpy(aResult, pPos + 1);
                        DPrint("aResult:%s\n", aResult);
                        if ((0 == strcmp(aResult, "1")) || (0 == strcmp(aResult, "2"))
                                || (0 == strcmp(aResult, "4")))
                        {
                            gSyUpdateSuccess = 1;
                            FILE *pFile = NULL;
                            pFile = fopen(SY_BOOT_FLAG, "wb");
                            if (NULL != pFile)
                            {
                                fwrite("3", 1, strlen("3"), pFile);
                                fclose(pFile);
                            }
                            else
                            {

                                DPrint("%s open failed 1\n",SY_BOOT_FLAG);

                            }
                        }
                        gSyIsUpdateReply = 1;
                    }
                }
            }
        }
        memset(cmdBuf, 0x00, sizeof(cmdBuf));
        sprintf(cmdBuf, "%s", "200");
        sendto(drawSocket, cmdBuf, strlen(cmdBuf), 0,(struct sockaddr*)&addr, sizeof(addr));
    }

    return NULL;
}

int RevCmdStart()
{
    int ret = 0;
    pthread_t rev_id = 0;

    DO;

    drawSocket = CreateSrvSocket(SY_TR069_SERVER_PORT);
    if (drawSocket == -1)
        return SY_FAILED;

    ret = pthread_create(&(rev_id), NULL, (void*)RevCmdHandle, NULL);
    if (ret != 0)
    {
        DPrint("Failed to create rev cmd thread\n");
        return SY_FAILED;
    }
    else
    {
        pthread_detach(rev_id);
    }

    return SY_SUCCESS;
}

int Send2CmdProcThd(Msg_TR069_s *msg)
{
    int ret = -1;
    DO;
    
    if (NULL == msg)
    {
        DPrint("Invalid arguments.\n");
        return false;
    }

    DPrint("user:%s, cmd:%d, type:%d, msg:%s, len:%d\n",
           msg->user, msg->cmd, msg->type, msg->msg, msg->len);

    if (-1 == g_msg2cmdProc_fd)
    {
        g_msg2cmdProc_fd = CreateAndConn(CMD_PROC_PORT, SOCK_STREAM);

        if(g_msg2cmdProc_fd == -1)
        {
            // DPrint("Send Fail!\n");
            return false;
        }
    }
    ret = send(g_msg2cmdProc_fd, msg, sizeof(Msg_TR069_s), MSG_NOSIGNAL);
    if (ret <= 0)
    {
        DPrint("g_msg2cmdProc_fd:%d, ret:%d\n", g_msg2cmdProc_fd, ret);
        DPrint("Send failure. (%s)\n", strerror(errno));
        SyCloseSocket(g_msg2cmdProc_fd);
        g_msg2cmdProc_fd = -1;
        return false;
    }

    return true;
}

LOCAL int Send2UpgradeCli(const char* msg, int msgLen)
{
    int ret = -1;
    struct sockaddr_in addr;

    DO;
    if ((NULL == msg) || (strlen(msg) == 0))
    {
        DPrint("command is NULL or length is 0\n");
        return SY_FAILED;
    }
    DPrint("command:%s, len:%d\n", msg, msgLen);

    if (-1 == gSyUpgradeClientsockfd)
    {
        gSyUpgradeClientsockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (gSyUpgradeClientsockfd < 0)
        {
            DPrint("Create Socket Fail\n");
            return SY_FAILED;
        }
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(SY_UPGRADE_SERVER_PORT);
	DPrint("send to upgrade");
    ret = sendto(gSyUpgradeClientsockfd, msg, msgLen, 0, (struct sockaddr*)&addr, sizeof(addr));
    if(ret <= 0)
    {
        DPrint("Send Msg Fail\n");
        SyCloseSocket(gSyUpgradeClientsockfd);
        gSyUpgradeClientsockfd = -1;
        return SY_FAILED;
    }

    DONE;
    return SY_SUCCESS;
}

LOCAL int DispatchOneKeyCK(const char* pCommand, int commandLen)
{
    DPrint("pCommand = %s\n",pCommand);
    if( strcmp(pCommand, "pingAVserver") == 0)
    {
        FILE *ping_fp;
        char pingcmd[256];
        char pingbuf[1024];
        char buf[1024];
        sprintf(pingcmd, "ping -c 3 -w 10 %s",AV_SERVER);
        if ((ping_fp = popen(pingcmd, "r")) != NULL)
        {
            while (fgets(pingbuf, 1024, ping_fp))
            {
                DPrint("%s", pingbuf);
                strcat(buf, pingbuf);
            }

            DPrint("buf = %s", buf);
        }
        else
        {
            DPrint("ping popen error!\n");
            return 0;
        }
        memset(&gSySendSockmsgToOneKeyCK, 0x00, sizeof(gSySendSockmsgToOneKeyCK));
        strcpy(gSySendSockmsgToOneKeyCK.user, "manager");
        gSySendSockmsgToOneKeyCK.type = SY_OP_SET_PARAM;
        strcpy(gSySendSockmsgToOneKeyCK.msg, buf);
        gSySendSockmsgToOneKeyCK.len = strlen(buf);
    }
    return SY_SUCCESS;
}

typedef struct _jsonNode {
    int size;
    struct _jsonNode* next;
} jnode;

LOCAL int DispatchCommand(const char* pCommand, int commandLen)
{
    int i;
    int nRet = -1;
    int nLen = 0;
    int usedNum = 0;
    char* pBuf = NULL;
    char* pos = NULL;
    char* delim = "";
    char funcStr[64] = {0};
    char* argument[10]= {};
    char localpath[128] = {0};
#ifdef SY_HAVE_LIBCURL
    char userPasswd[256] = {0};
    char remotepath[256] = {0};
#endif
    char tmpBuf[512] = {0};
    char urlBuf[512] = {0};
    char msgBuf[1024] = {0};
    char cmdBuf[2048] = {0};
    char command[1024] = {0};
    syHostInfo  szHostInfo;
    struct sockmsg msg;

    DO;
    if ((NULL == pCommand) || (0 == commandLen))
    {
        DPrint("Invalid arguments.\n");
        return SY_FAILED;
    }
    memset(&msg, 0x00, sizeof(msg));
    memset(tmpBuf, 0x00, sizeof(tmpBuf));
    memset(cmdBuf, 0x00, sizeof(cmdBuf));
    memset(msgBuf, 0x00, sizeof(msgBuf));
    memset(command, 0x00, sizeof(command));
    memcpy(command, pCommand, sizeof(command));
    DPrint("command:%s, commandLen:%d\n", command, commandLen);

    pos = strstr((char*)command, "&");
    memcpy(funcStr, command, pos?(pos - command):commandLen);

    DPrint("funcStr:%s\n", funcStr);
    if (0 == strcmp(funcStr, SyMethodTypeToStr(SY_REBOOT)))
    {
        FILE *pFile = fopen(SY_REBOOT_FLAG, "wb");
        if (NULL != pFile) {
            fwrite("1", 1, strlen("1"), pFile);
            fclose(pFile);
        }
        else {
            PERROR(SY_REBOOT_FLAG" fopen:");
        }
        sleep(1);
        g_notify(REBOOT, "");

    }
    else if (0 == strcmp(funcStr, SyMethodTypeToStr(SY_FACTORY_RESET)))
    {
        remove(SY_BOOT_FLAG);
        g_notify(FACTORYRESET, "");
        
    }
    else if (0 == strcmp(funcStr, "SY_OP_CONNECTION_REQUEST")){
        DPrint("Connection request is coming.\ncwmp session = %d",syCwmpSession);
        while( 0 == syCwmpSession )
        {
            usleep(50*1000);
        }
        syCwmpSession = 0;

        gsyGlobalParmStru.SessionState |= SY_SESSION_IDLE;
        gsyGlobalParmStru.SessionEvent |= SY_EVENT_START;
        gsyGlobalParmStru.SessionTrigger |= SY_TRIGGER_CONNECTION_REQUEST;
        gsyGlobalParmStru.SessionFlags |= SY_STATE_NONE;

        gSyNatSession = SY_TRUE;
        if (0 == gSyIsCPEStart)
        {
            gSyIsCPEStart = 1;
        }
        syNATCmd = 1;
    }
    else if (0 == strcmp(funcStr, SyMethodTypeToStr(SY_DOWNLOAD)))
    {
        usedNum = SyDelimArgument2(pos, delim="&", argument);
        DPrint("argument[1]:%s\n", argument[1]);
        pBuf = tmpBuf;

        memcpy(pBuf, argument[1], strlen(argument[1]));


        DPrint("pBuf:%s\n", pBuf);

        if (SY_FAILED == SyUrlParse(pBuf, &szHostInfo))
        {
            DPrint("Parse URL failed\n");
            return SY_FAILED;
        }

        pos = strrchr(pBuf, '/');
        nLen = strlen(pBuf);
        memset(localpath, 0x00, sizeof(localpath));
        memset(urlBuf, 0x00, sizeof(urlBuf));
        if (NULL != pos)
        {
            memcpy(localpath, pos + 1, pBuf + nLen - pos);
            memcpy(urlBuf, pBuf, pos - pBuf);
        }
        else
        {
            memcpy(localpath, pBuf, strlen(pBuf));
        }
        DPrint("localpath:%s\n", localpath);

        DPrint("Download type:%c\n", argument[0][0]);
        if (DOWNLOAD_FIRMWARE_UPGRADE_IMAGE == argument[0][0])
        {
            DPrint("1111\n");
            sprintf(cmdBuf, "{\"user\":\"TM\",\"type\":%d,\"msg\":", SY_INTENT_TYPE_UPGRADE_CMD);
            sprintf(msgBuf, "\"UpgradeServerUrl+%s|UpgradeFileName+%s|FileSize+%s\"",
                urlBuf, localpath, argument[4]);
            sprintf(cmdBuf + strlen(cmdBuf), "%s,\"len\":%d}\n", msgBuf, strlen(msgBuf));
            Send2UpgradeCli(cmdBuf, strlen(cmdBuf));
			DPrint("end to Send2UpgradeCli");
        }
        else if (DOWNLOAD_VERDOR_CONFIGURATION_FILE == argument[0][0])
        {
            DPrint("2222\n");
#ifdef SY_HAVE_LIBCURL
#ifdef SY_SH_TR069
            sscanf(pBuf, "%*[^//]//%[^@]", userPasswd);
            sscanf(pBuf, "%*[^@]@%[^]", remotepath);
            DPrint("userPasswd:%s, remotepath:%s\n",
                   userPasswd, remotepath);
            syDownload(szHostInfo.transferType, userPasswd, remotepath,
                       (const char*)localpath, SY_TIMEOUT, SY_TRY_TIMES);
#endif
#endif
        }
        for (i = 0; i < usedNum; i++)
        {
            DPrint("argument[%d]:%s\n", i, argument[i]);
            if (NULL != argument[i])
            {
                //DPrint("free argument[%d]\n", i);
                free(argument[i]);
                argument[i] =  NULL;
            }
        }

        //sleep(10);
    }
    else if (0 == strcmp(funcStr, SyMethodTypeToStr(SY_UPLOAD)))
    {
        usedNum = SyDelimArgument2(pos, delim="&", argument);
        DPrint("argument[1]:%s\n", argument[1]);
        pBuf = tmpBuf;

        memcpy(pBuf, argument[1], strlen(argument[1]));


        DPrint("pBuf:%s\n", pBuf);
        if (SY_FAILED == SyUrlParse(pBuf, &szHostInfo))
        {
            DPrint("Parse URL failed\n");
            return SY_FAILED;
        }


        DPrint("Upload type:%c\n", argument[0][0]);

        if (UPLOAD_VERDOR_CONFIGURATION_FILE == argument[0][0])
        {
            DPrint("1111\n");

        }
        else if (UPLOAD_VERDOR_LOG_FILE == argument[0][0])
        {
            DPrint("2222\n");
        }

        for (i = 0; i < usedNum; i++)
        {
            DPrint("argument[%d]:%s\n", i, argument[i]);
            if (NULL != argument[i])
            {
                free(argument[i]);
                argument[i] =  NULL;
            }
        }
        sleep(2);
    }
    else if (0 == strcmp(funcStr, "syCwmpInit"))
    {
        if (!CwmpInit())
        {
            DPrint("Init Cmwp Error\n");
        }
    }
    else if (0 == strcmp(funcStr, "IPPingDiagnostics"))
    {
        if (0 == gSyIpPingTestTid)
        {
            pthread_create(&gSyIpPingTestTid, NULL, syIpPingTestThread, NULL);
        }
        else
        {
            DPrint("syIpPingTestThread is running\n");
        }
    }
    else if (0 == strcmp(funcStr, "TraceRouteDiagnostics"))
    {
        if (0 == gSyTraceRouteTestTid)
        {
            pthread_create(&gSyTraceRouteTestTid, NULL, syTraceRouteTestThread, NULL);
        }
        else
        {
            DPrint("syTraceRouteTestThread is running\n");
        }
    }
    else if (0 == strcmp(funcStr, "BandwidthDiagnostics"))
    {
        if (0 == gSyBandwidthDiagnosticsTid)
        {
            pthread_create(&gSyBandwidthDiagnosticsTid, NULL, syBandwidthTestThread, NULL);
        }
        else
        {
            DPrint("syBandwidthTestThread is running\n");
        }
    }
    else if (0 == strcmp(funcStr, "PacketCapture"))
    {
        SySetNodeValue("Device.X_00E0FC.PacketCapture.State", "3");
        if (0 == gSyPacketCaptureTid)
        {
            pthread_create(&gSyPacketCaptureTid, NULL, syPacketCaptureThread, NULL);
        }
        else
        {
            DPrint("syPacketCaptureThread is running\n");
        }
    }
    else if (0 == strcmp(funcStr, "TMCChanggedURL")) {
        DPrint("TMC hand out a new URL ,now restart \n");
        remove(SY_BOOT_FLAG);
        exit(1);
    }
    else if (0 == strcmp(funcStr, "debugInfo"))
    {
        DPrint("handle debug info .\n");
        if (0 == gSyDebugInfoTid)
        {
            pthread_create(&gSyDebugInfoTid, NULL, syDebugInfoThread, NULL);
        }
    }
    else if (0 == strcmp(funcStr, "startupInfo"))
    {
        DPrint("handle startup info .\n");
        if (0 == gSyStartupInfoTid)
        {
            pthread_create(&gSyStartupInfoTid, NULL, syStartupInfoThread, NULL);
        }
    }
    else if (0 == strcmp(funcStr, "tsAPKStartupInfo"))
    {
        DPrint("handle ts apk startup info .\n");
        if (0 == gSyTsAPKStartupThreadId)
        {
            if (pthread_create(&gSyTsAPKStartupThreadId, NULL, syTSApkStartupInfoThread, NULL) < 0)
            {
                DPrint("apk start to collect info failed!\n");
            }
            else
            {
                pthread_detach(gSyTsAPKStartupThreadId);
            }
        }
    }
    else if (0 == strcmp(funcStr, "tsAPKDebugInfo"))
    {
        DPrint("handle ts apk startup info .\n");
        if (0 == gSyTsAPKDebugThreadId)
        {
            if (pthread_create(&gSyTsAPKDebugThreadId, NULL, syTSApkDebugInfoThread, NULL) < 0)
            {
                DPrint("apk start to collect info failed!\n");
            }
            else
            {
                pthread_detach(gSyTsAPKDebugThreadId);
            }
        }
    }
    else if (0 == strcmp(funcStr, "PeriodMsgLog"))
    {
        if (0 == gSyPeriodMsgLogTid)
        {
            pthread_create(&gSyPeriodMsgLogTid, NULL, syMsgLogPeriodThread, NULL);
        }
    }

    DONE;
    return SY_SUCCESS;
}

//处理iptv请求
void HandleIptvReq(struct sockmsg *pMsg)
{
    int i;
    int ret;
    int pid;
    int tmpErrorNum = 0;
    char ValueBuf[512] = {0};
    size_t sizeOfBuf = sizeof(ValueBuf);
    char tmpBuf[512] = {0};
    char cValueChangeBuf[2024] = {0};
    FILE *pFile = NULL;

    struct sockmsg msg;

    pthread_t logcatThreadId = -1;
    memset(&gSySendSockmsg, 0, sizeof(gSySendSockmsg));
    memset(&msg, 0x00, sizeof(struct sockmsg));
    memcpy(&msg, pMsg, sizeof(struct sockmsg));

    switch(msg.type)
    {
    case SY_OP_LOG_CATCH:
        //ret = pthread_create(&logcatThreadId, 0, startLogcat, (void*)msg.msg);
        if(SY_SUCCESS != startLogcat(msg.msg))
        {
            DPrint("log catch err.");
        }
        if(17 == msg.cmd)
            remove("/data/data/com.android.smart.terminal.iptv/logtest.log");
        DPrint("remove /data/data/com.android.smart.terminal.iptv/logtest.log \n");
        break;
    case SY_OP_START:
        
        if (0 == gSyIsTmStart){
            struct sockmsg sendMsg;
            memset(&sendMsg, 0x00, sizeof(struct sockmsg));
            sprintf(sendMsg.msg, "syCwmpInit");
            sendMsg.len = strlen(sendMsg.msg);
            strcpy(sendMsg.user, "tr069");
            Send2CmdProcThd(&sendMsg);
            gSyIsTmStart = 1;
        }
        strcpy(gSySendSockmsg.user, "manager");
        gSySendSockmsg.type = SY_OP_START_FINISH;
        break;
    case SY_OP_HEART_BEAT:
        strcpy(gSySendSockmsg.user, "manager");
        gSySendSockmsg.type = SY_OP_HEAT_BEAT_REPLY;
        break;
    case SY_OP_VALUE_CHANGE_REPORT: {

        DPrint("user:%s, cmd:%d, type:%d, msg:%s, len:%d\n",
               msg.user, msg.cmd, msg.type, msg.msg, msg.len);
        switch (msg.cmd)
        {

        //这里新定义了一个类型，iptv端有错误码传过来时msg.cmd = ERROR_INFO
        case ERROR_INFO:
            //将错误信息写到SY_ERRORCODETMP_FLAG
            DPrint("error code = %s\n",msg.msg);
            pFile = fopen(SY_ERRORCODETMP_FLAG, "a+");
            if (NULL != pFile)
            {
                memset(tmpBuf, 0x00, sizeof(tmpBuf));
                memset(ValueBuf, 0x00, sizeof(ValueBuf));
                sscanf(msg.msg, "%[^|]", ValueBuf);
                sprintf(tmpBuf, "Device.X_00E0FC.ErrorCode.{i}.ErrorCodeTime=%s\n", GetCurrTime());
                fwrite(tmpBuf, 1, strlen(tmpBuf), pFile);

                memset(tmpBuf, 0x00, sizeof(tmpBuf));
                memset(ValueBuf, 0x00, sizeof(ValueBuf));
                sscanf(msg.msg, "%*[^|]|%s", ValueBuf);
                sprintf(tmpBuf, "Device.X_00E0FC.ErrorCode.{i}.ErrorCodeValue=%s\n", ValueBuf);
                fwrite(tmpBuf, 1, strlen(tmpBuf), pFile);
                fclose(pFile);
            }
            break;


            //这里新定义了一个类型，iptv拨测诊断后会将诊断状态DiagnosticsState，播放状态PlayState发送过来
            //播放某个节目时，也会将节目URL发送过来
        case PLAYURL:
        case PLAYSTATE:
        case MANAGESERVER_URL :        //网管认证地址
        case HEARTBEAT_INTERVAL :       //心跳周期
            SySetNodeValue(gSyCmdStrList[msg.cmd].cmdStr,msg.msg);
            break;

        case EPG_MODIFY:
            if ( 15 == syEPGmodifySupported )
            {
                DPrint("EPG hand up manage URL :%s\n",msg.msg);
                syEPGmodifySupported = 14;
                SySetNodeValue("Device.ManagementServer.URLModifyFlag" ,"14");
                //   SySetNodeValue("Device.ManagementServer.URL" ,msg.msg);
                remove(SY_BOOT_FLAG);
                exit(1);
            }
            break;

        case SCREENOFF:
            if (0 == gSyIsShutdownFlag)
            {
#if 0
                sySendInform(&soap, (void*)SY_EVENT_X_CTC_SHUT_DOWN);
                pFile = fopen(SY_SHUTDOWN_FLAG, "wb");
                DPrint("pFile:%p\n", pFile);
                if (NULL != pFile)
                {
                    fclose(pFile);
                }
#endif
                gSyIsShutdownFlag = 1;
            }
            break;
        case SCREENON:
            if (1 == gSyIsShutdownFlag)
            {
                pFile = fopen(SY_SCREENON_FLAG, "wb");
                DPrint("pFile:%p\n", pFile);
                if (NULL != pFile)
                {
                    fclose(pFile);
                }
                gSyIsShutdownFlag = 0;
            }
            break;
        case REPORT_IPADDR:
            DPrint("URL:%s\n", gsyAcsCpeParamStru.URL);
            sscanf(gsyAcsCpeParamStru.URL, "%*[^//]//%[^:]", tmpBuf);
            SetValue("Device.AddRoute", tmpBuf);
            pFile = fopen(SY_VALUE_CHANGE_INFORM_FLAG_1, "wb");
            if (NULL != pFile)
            {
                sprintf(cValueChangeBuf, "Device.LAN.IPAddress=%s\r\n", msg.msg);
                DPrint("cValueChangeBuf0:%s\n", cValueChangeBuf);
                fwrite(cValueChangeBuf, 1, strlen(cValueChangeBuf), pFile);
                fclose(pFile);
				addEvent(EVENT_VALUE_CHANGE);
            }
            break;
        case IPADDR:
            DPrint("URL:%s\n", gsyAcsCpeParamStru.URL);
            sscanf(gsyAcsCpeParamStru.URL, "%*[^//]//%[^:]", tmpBuf);
            SetValue("Device.AddRoute", tmpBuf);
            break;
        case SQMSERVERURL:
            DPrint("SQMSERVERURL:%s\n", msg.msg);
            SetValue("Device.AddRoute", msg.msg);
            break;
        case IPTVUSER:
            pFile = fopen(SY_VALUE_CHANGE_INFORM_FLAG_1, "wb");
            if (NULL != pFile) {
                for(i = 0; i < gSyAutoReportListLen; i++) {
                    if(gSyAutoReportList[i].cmd == msg.cmd) {
                        break;
                    }
                }
                if (i < gSyAutoReportListLen) {
                    if (GetValue(gSyAutoReportList[i].cmdStr, ValueBuf, sizeOfBuf)) {
                        sprintf(cValueChangeBuf, "%s=%s\r\n", gSyAutoReportList[i].cmdStr, ValueBuf);
                        DPrint("cValueChangeBuf:%s\n", cValueChangeBuf);
                        fwrite(cValueChangeBuf, 1, strlen(cValueChangeBuf), pFile);
                        fclose(pFile);
						addEvent(EVENT_VALUE_CHANGE);
                    }
                    else {
                        remove(SY_VALUE_CHANGE_INFORM_FLAG_1);
                        DPrint("open %s Error1...\n", SY_VALUE_CHANGE_INFORM_FLAG_1);
                    }
                }
                else {
                    remove(SY_VALUE_CHANGE_INFORM_FLAG_1);
                    DPrint("open %s Error2...\n", SY_VALUE_CHANGE_INFORM_FLAG_1);
                }
            }
            if (IPTVUSER == msg.cmd) {
                if (gSyServiceInfoStu.UserID != NULL) {
                    free(gSyServiceInfoStu.UserID);
                }
                gSyServiceInfoStu.UserID = strdup(msg.msg);
            }

            break;
        case NTPMAIN:
        case TIMEZONE:
        case AUTH_MAINURL:
            //case MANAGESERVER_URL:
        case PPPOE_USER:
        case DNSMAIN:
        case SOFTWARE_VER:
            pFile = fopen(SY_VALUE_CHANGE_INFORM_FLAG_2, "wb");
            if (NULL != pFile)
            {
                for(i = 0; i < gSyAutoReportListLen; i++)
                {
                    if(gSyAutoReportList[i].cmd == msg.cmd)
                    {
                        break;
                    }
                }
                if (i < gSyAutoReportListLen)
                {
                    if (GetValue(gSyAutoReportList[i].cmdStr, ValueBuf, sizeOfBuf))
                    {
                        sprintf(cValueChangeBuf, "%s=%s\r\n", gSyAutoReportList[i].cmdStr, ValueBuf);
                        DPrint("cValueChangeBuf:%s\n", cValueChangeBuf);
                        fwrite(cValueChangeBuf, 1, strlen(cValueChangeBuf), pFile);
                        fclose(pFile);
						addEvent(EVENT_VALUE_CHANGE);
                    }
                    else
                    {
                        remove(SY_VALUE_CHANGE_INFORM_FLAG_2);
                        DPrint("open %s Error1...\n", SY_VALUE_CHANGE_INFORM_FLAG_2);
                    }
                }
                else
                {
                    remove(SY_VALUE_CHANGE_INFORM_FLAG_2);
                    DPrint("open %s Error2...\n", SY_VALUE_CHANGE_INFORM_FLAG_2);
                }
            }

            if (NTPMAIN == msg.cmd)
            {
                if (gSyTimeStu.NTPServer1 != NULL)
                {
                    free(gSyTimeStu.NTPServer1);
                }
                gSyTimeStu.NTPServer1 = strdup(msg.msg);
            }
            else if (AUTH_MAINURL == msg.cmd)
            {
                if (gSyServiceInfoStu.AuthURL != NULL)
                {
                    free(gSyServiceInfoStu.AuthURL);
                }
                gSyServiceInfoStu.AuthURL = strdup(msg.msg);
            }
            else if (MANAGESERVER_URL == msg.cmd)
            {
#if 0
                if (gsyAcsCpeParamStru.URL != NULL)
                {
                    free(gsyAcsCpeParamStru.URL);
                }
                gsyAcsCpeParamStru.URL = strdup(msg.msg);
#endif
            }
#if 1
            else if (PPPOE_USER == msg.cmd)
            {
                if (gSyServiceInfoStu.PPPoEID != NULL)
                {
                    free(gSyServiceInfoStu.PPPoEID);
                }
                gSyServiceInfoStu.PPPoEID = strdup(msg.msg);
            }
            else if (DNSMAIN == msg.cmd)
            {
                if (gSyLANStu.DNSServers!= NULL)
                {
                    free(gSyLANStu.DNSServers);
                }
                gSyLANStu.DNSServers = strdup(msg.msg);
            }
#endif
            break;
#if 0
        case IPTVUSER:
            if (gSyServiceInfoStu.UserID != NULL)
            {
                free(gSyServiceInfoStu.UserID);
            }
            gSyServiceInfoStu.UserID = strdup(msg.msg);
            break;
        case PPPOE_USER:
            if (gSyServiceInfoStu.PPPoEID != NULL)
            {
                free(gSyServiceInfoStu.PPPoEID);
            }
            gSyServiceInfoStu.PPPoEID = strdup(msg.msg);
            break;
        case DNSMAIN:
            if (gSyLANStu.DNSServers!= NULL)
            {
                free(gSyLANStu.DNSServers);
            }
            gSyLANStu.DNSServers = strdup(msg.msg);
            break;
        case IPADDR:
            if (gSyLANStu.IPAddress!= NULL)
            {
                free(gSyLANStu.IPAddress);
            }
            gSyLANStu.IPAddress = strdup(msg.msg);
            break;
#endif
        case DIAGNOSTICSSTATE:
            SySetNodeValue(gSyCmdStrList[msg.cmd].cmdStr,msg.msg);
			#if 0
            pFile = fopen(SY_DIAGNOSTICS_INFORM_FLAG, "wb");
            if (NULL != pFile) {
                fclose(pFile);
            }
            else {
                PERROR(SY_DIAGNOSTICS_INFORM_FLAG" fopen:");
            }
			#endif
			addEvent(EVENT_DIAGNOSTICS);
            break;
        case IGMPINFO:
        case HTTPINFO:
        case RTSPINFO:
        case PKGTOTALONESEC:
        case BYTETOTALONESEC:
        case PKGLOSTRATE:
        case AVARAGERATE:
        case BUFFER:
        case ERROR:
            DPrint("cmd:%d, msg:%s\n", msg.cmd, msg.msg);
#if 0
            pFile = fopen(SY_TMP_LOG_PERIODIC_INFORM_FLAG, "wb");
            if (NULL != pFile)
            {
                for(i = 0; i < gSyMsgLogListLen; i++)
                {
                    if(gSyMsgLogList[i].cmd == msg.cmd)
                    {
                        break;
                    }
                }
                if (GetValue(gSyMsgLogList[i].cmdStr, ValueBuf, sizeOfBuf))
                {
                    sprintf(cValueChangeBuf, "%s=%s\r\n", gSyMsgLogList[i].cmdStr, ValueBuf);
                    DPrint("cValueChangeBuf:%s\n", cValueChangeBuf);
                    fwrite(cValueChangeBuf, 1, strlen(cValueChangeBuf), pFile);
                    fclose(pFile);
                }
                else
                {
                    fclose(pFile);
                    remove(SY_TMP_LOG_PERIODIC_INFORM_FLAG);
                }
            }
#endif
            break;
        case CHANGED:
        { 
			// 这对大括号一定要加不然会编译报错
            char* pos = NULL;
            char tmp_buf[512] = {0};
            if ((pos = strstr(pMsg->msg, "EPGDomain=")))
            {
                sscanf(pos, "%*[^0-9]%15[^:])", gSyServiceInfoStu.EpgIP);
                DPrint("EpgIP %s\n", gSyServiceInfoStu.EpgIP);
            }
			else if (strcmp(msg.msg, "networkstatuschange") == 0){
				gNetChangFlag = 1;
				UpdateNetPara();
				pFile = fopen(SY_VALUE_CHANGE_INFORM_FLAG_1, "wb");
				if (NULL == pFile){
					DPrint("open %s failed\n", SY_VALUE_CHANGE_INFORM_FLAG_1);
					break;
				}
                fclose(pFile);
				addEvent(EVENT_VALUE_CHANGE);
			}
            break;
        }
        default:
			DPrint("cmd %d is invaild!\n", msg.cmd);
            break;

        }
        break;
    }
    case SY_FACTORY_RESET:
        remove(SY_BOOT_FLAG);
        break;
    default:
        DPrint("valid type is %d\n", msg.type);
        break;
    }

    return;
}
bool GetValueToTM(const char* path, char* buffer, size_t sizeOfValue){
	if(path == NULL)
		return false;
	if (!g_getData(path, buffer, sizeOfValue))
    {
        EPrint("Fault, failed to get data.\n");
        return false;
    }
	DPrint("get buffer is %s\n", buffer);

	if (0 == strcmp(path, "ConnectMode"))
    {
        if (0 == strcmp(buffer, "0")){
            strcpy(buffer, "PPPoE");
        }
		else if (0 == strcmp(buffer, "1")){
			strcpy(buffer, "DHCP");
		}
        else if (0 == strcmp(buffer, "2")){
            strcpy(buffer, "Static");
        }
        else if (0 == strcmp(buffer, "3")){
            strcpy(buffer, "IPoE");
        }
		else if (0 == strcmp(buffer, "4")){
			strcpy(buffer, "Wifi");
		}
        else if (0 == strcmp(buffer, "-1")){
            strcpy(buffer, "None");
        }
    }
	return true;
}
bool getParamV(int cmd, char* buffer, size_t sizeOfValue)
{
	int  i;
    int  nRet = 0;
    int  length = sizeof(g_keyList)/sizeof(xml_key_path_t);
	char path[128] = {0};
	
    if(-1 == cmd)
    {
        EPrint("Invalid argument.\n");
        return false;
    }

    for(i = 0; i < length; i++)
    {
        if((g_keyList[i].cmd == cmd)
            && (NULL != g_keyList[i].key))
        {
            strncpy(path, g_keyList[i].key, sizeof(path));
            DPrint("Found <%s>.\n",  path);
            break;
        }
    }

    if(length == i)
    {
        EPrint("Not found\n");
        return false;
    }

    if (!g_getData(g_keyList[i].key, buffer, sizeOfValue))
    {
        EPrint("Fault, failed to get data.\n");
        return false;
    }
	DPrint("get buffer is %s\n", buffer);
	/*if (REMOTESTATE == recvmsg.cmd){
		if (!strcmp(buffer, "open")){
			strcpy(buffer, "1");
		} else {
			strcpy(buffer, "0");
		}
	}*/
    if (CONNECT_TYPE == cmd)
    {
        if (0 == strcmp(buffer, "0")){
            strcpy(buffer, "PPPoE");
        }
		else if (0 == strcmp(buffer, "1")){
			strcpy(buffer, "DHCP");
		}
        else if (0 == strcmp(buffer, "2")){
            strcpy(buffer, "Static");
        }
        else if (0 == strcmp(buffer, "3")){
            strcpy(buffer, "IPoE");
        }
		else if (0 == strcmp(buffer, "4")){
			strcpy(buffer, "Wifi");
		}
        else if (0 == strcmp(buffer, "-1")){
            strcpy(buffer, "None");
        }
    }

    return true;
}

bool setParamV(const char* path, struct sockmsg recvmsg)
{
#if 0
    int  i, nRet = 0;
    int  logSwitchVal = 0;
    int  length = sizeof(g_keyList)/sizeof(data_key_path_t);
	char path[128] =          {0};
    char connectType[2] =     {0};

    if((int)strlen(recvmsg.msg) != recvmsg.len)
    {
        E("length of msg wrong.\n");
        return false;
    }
    
    Msg_APK_s msg;
    memset(&msg, 0, sizeof(msg));

    D("cmd:%d, msg:%s, len:%d\n", recvmsg.cmd, recvmsg.msg, recvmsg.len);

    /*cmd是否合法,如果cmd不合法,返回内容cmd=-1,msg为空,len=-1*/
    for(i = 0; i < length; i++)
    {
        if((g_keyList[i].cmd == recvmsg.cmd)
            && (WRITE == g_keyList[i].attr)
            && (NULL != g_keyList[i].key))
        {
            strncpy(path, g_keyList[i].key, sizeof(path));
            D("Found %s.\n",  path);
            break;
        }
    }
    
    if(length == i) {
        E("Not found.\n");
        return false;
    }
#endif
    switch(recvmsg.cmd){
    case CONNECT_TYPE: 
        if (strlen(recvmsg.msg) > 15){
            E("Cmd:%d, wrong length\n", recvmsg.cmd);
            return false;
        }
        if (0 == strcmp(recvmsg.msg, "PPPoE")){
            strcpy(recvmsg.msg, "0");
		} else if (0 == strcmp(recvmsg.msg, "DHCP")){
            strcpy(recvmsg.msg, "1");
        } else if (0 == strcmp(recvmsg.msg, "Static")){
            strcpy(recvmsg.msg, "2");
        } else if (0 == strcmp(recvmsg.msg, "IPoE")){
            strcpy(recvmsg.msg, "3");
        } else if (0 == strcmp(recvmsg.msg, "Wifi")){
        	strcpy(recvmsg.msg, "4");
        } else {
            E("'%s', invalid value.\n", recvmsg.msg);
            return false;
        }
		break;
    case DHCP_USER:
    case PPPOE_USER:
    case IPTVUSER:
    case DHCP_PASSWD:
    case PPPOE_PASSWD:
    case IPTVPASSWD:
        if (recvmsg.len > 36){
            E("Cmd:%d, wrong length\n", recvmsg.cmd);
            return false;
        }
        break;
    case IPADDR:
    case MASK:
    case DEFGATEWAY:
    case DNSMAIN:
    case DNSBAK:
    case AUTH_MAINURL:
    case AUTH_BAKURL:
        if (recvmsg.len > 128){
            E("Cmd:%d, wrong length\n", recvmsg.cmd);
            return false;
        }                
        break;
    case NTPMAIN:
    case NTPBAK:
        if (recvmsg.len > 64){
            E("Cmd:%d, wrong length\n",  recvmsg.cmd);
            return false;
        }    
        break;
    case TIMEZONE:
    case MANAGESERVER_USER:
    case MANAGESERVER_PASSWD:
    case QOSSERVERURL:
        if (recvmsg.len > 256){
            E("Cmd:%d, wrong length\n",  recvmsg.cmd);
            return false;
        } 
        break;
    case MONITORINGINTERVAL:
        if(recvmsg.msg[1] == 'x')
            memmove(recvmsg.msg, recvmsg.msg+2, strlen(recvmsg.msg+2));
        for(int i = 0; i<(int)strlen(recvmsg.msg); i++) {
            if(!isdigit(recvmsg.msg[i])) {
                E("Cmd:%d, Value non-number.\n",  recvmsg.cmd);
                return false;
            }
        }
        break;
    case QOSUPLOADINTERVAL:
    case RECORDINTERVAL:
    case LOGDURATION:
    case LOGTYPE:
    case LOGLEVEL:
    case LOGOUTPUTTYPE:
    case SYSLOGCONTINUETIME:
    case SYSLOGTIMER:
        for(int i = 0; i < strlen(recvmsg.msg); i++){
            if (!isdigit(recvmsg.msg[i])) {
                E("Cmd:%d, Value non-number\n", recvmsg.cmd); 
                return false;
            }
        }
        break;
	case FORCEUPGRADE:
    case LANGUAGE:
        if ('0' != *recvmsg.msg && '1' != *recvmsg.msg)
        {
            E("Cmd:%d, Value non-number\n", recvmsg.cmd);
            return false;
        }
        break;
    case LOGSWITCH:
    case LOGMSGORFILE:
        if ((0 == strcasecmp(recvmsg.msg, "true"))
            || (0 == strcmp(recvmsg.msg, "1"))){
            strcpy(recvmsg.msg, "1");
        }
        else if (0 == strcasecmp(recvmsg.msg, "false")
            || (0 == strcmp(recvmsg.msg, "0"))){
            strcpy(recvmsg.msg, "0");
        }
        else {
            E("Cmd:%d, Value non-number\n", recvmsg.cmd);
            return false;
        }
        break;
//	case REMOTESTATE:
//		if (!strcmp(recvmsg.msg, "1")){
//			strcpy(recvmsg.msg, "open");
//		} else {
//			strcpy(recvmsg.msg, "close");
//		}
//		break;
    default:
        break;
    }
    return g_setData(path, recvmsg.msg);
}


bool SetValue(const char* path, char* value)
{
    int i = 0;
    int nCmd = -1;
    int revSize = 0;
    bool ret = false;
    struct sockmsg sendMsg;
    memset(&sendMsg, 0, sizeof(sendMsg));
    DPrint("set <%s> to %s.\n", path, value);
    memset(&sendMsg, 0x00, sizeof(sendMsg));
#if 0
    for(i = 0; i<gSyCmdStrListLen && gSyCmdStrList[i].cmd!=-1; i++)
    {
        if(!strcmp(gSyCmdStrList[i].cmdStr, path))
        {
            nCmd = gSyCmdStrList[i].cmd;
			break;
        }
        
    } 

	VPrint("index <%d>.\n", nCmd);
	
	 if (SY_WRITE_END != nCmd) {
		 sendMsg.cmd = nCmd;
		 sendMsg.len = strlen(strcpy(sendMsg.msg, value));
		 
		 ret = setParamV(sendMsg);
	 }
#else	
    int  length = sizeof(g_keyList)/sizeof(data_key_path_t);
	xml_key_path_t tData;
	if(getParamForNode(path, &tData)){
		for(i = 0; i < length; i++){
			if(!strcmp(g_keyList[i].key, tData.keyname)){
				nCmd = g_keyList[i].cmd;
				break;
			}
		}

		VPrint("index <%d>.\n", nCmd);
		sendMsg.cmd = nCmd;
		sendMsg.len = strlen(strcpy(sendMsg.msg, value));
		ret = setParamV(tData.keyname, sendMsg);
	}
#endif

	 

    DONE;

    return ret;
}

extern lua_State* luaVM;

/*
 * 从apk侧获取数据
 */
bool GetValue(const char* path, char* value, size_t sizeOfValue)
{
    int i = 0;
    int nCmd = -1;
    int revSize = 0;
    bool ret = false;
    struct sockmsg sendMsg;

    DPrint("get <%s>", path);

    memset(&sendMsg, 0x00, sizeof(sendMsg));

#ifdef SY_TEST
		xml_key_path_t tData;
	if(getParamForNode(path, &tData)){
		ret = GetValueToTM(tData.keyname, value, sizeOfValue);
	}
#else
    for(i = 0; i<gSyCmdStrListLen && gSyCmdStrList[i].cmd!=-1; i++)
    {
        if(!strcmp(gSyCmdStrList[i].cmdStr, path))
        {
            nCmd = gSyCmdStrList[i].cmd;
            break;
        }
        
    }

    VPrint("index <%d>\n", nCmd);
  
    if (SY_WRITE_END != nCmd) {
        if ((ret = getParamV(nCmd, value, sizeOfValue))) {
            EPrint("getParamV() failed.");
        }
    }
#endif

#ifdef SUPPORT_LUA
	callLuaFunc(luaVM, "getvalue", "ppi>", path, value, sizeOfValue);
#endif

    DONE;

    return ret;
}

int startLogcat(char* cmd)
{
    int trun = 0;
    int n = -1;
    FILE *fd;

    DO;

    if(!strncmp(cmd, "logcat", strlen("logcat")))
    {
        system("logcat -c");
        DPrint("system(logcat -c)");
    }
    //umask(S_IRUSR|S_IRUSR|S_IRGRP |S_IWGRP |S_IROTH |S_IWOTH);
    DPrint("cmd:%s", cmd);
    while(trun < 2)
    {
        if(trun == 0)
        {
            if ((fd = popen(cmd, "r")) != NULL)
            {
            	//DPrint("run logcat cmd is %s\n", cmd);
                DPrint("popen(%s) successs.", cmd);
                pclose(fd);
                break;
            }
        }
        else if(trun == 1)
        {
            n = system(cmd);
            DPrint("system(%s) return :%d", cmd, n);
            break;
        }
        else
        {
            return SY_FAILED;
        }
        trun++;
    }

    system("chmod 666 /data/data/com.android.smart.terminal.iptv/logtest.log");

    return SY_SUCCESS;

}
void HandleConnectRequest(){
	DPrint("Connection request is coming.\ncwmp session = %d",syCwmpSession);
    while( 0 == syCwmpSession )
    {
       usleep(50*1000);
    }
    syCwmpSession = 0;

    gsyGlobalParmStru.SessionState |= SY_SESSION_IDLE;
    gsyGlobalParmStru.SessionEvent |= SY_EVENT_START;
    gsyGlobalParmStru.SessionTrigger |= SY_TRIGGER_CONNECTION_REQUEST;
    gsyGlobalParmStru.SessionFlags |= SY_STATE_NONE;

    gSyNatSession = SY_TRUE;
    if (0 == gSyIsCPEStart)
    {
       gSyIsCPEStart = 1;
    }
    syNATCmd = 1;
}
void HandleDownload(char *pos){

	char* argument[10] = {0};
	char userPasswd[256] = {0};
    char remotepath[256] = {0};
	char msgBuf[1024] = {0};
	char cmdBuf[2048] = {0};
	char localPath[1024] = {0};
	syHostInfo  szHostInfo;

    int usedNum = SyDelimArgument2(pos, "&", argument);
    char  URL[512] = {0};
	char* urlPtr = URL;
    char* tempPtr = NULL;
    char* fileNamePtr = NULL;
    char* fileSizePtr = argument[4];
    char* commandKey = argument[usedNum-1];
	#if 0
    FILE* pFile = fopen(SY_UPGRADE_FLAG, "w+");
    if (!pFile) {
        PERROR(SY_UPGRADE_FLAG" fopen:");
        return ;
    }
    int ret = fwrite(commandKey, 1, strlen(commandKey), pFile);
    DPrint("written %d bytes.The command key is %s.\n", ret, commandKey);
    fclose(pFile);
	#endif
    DPrint("%d, url %s\n", usedNum, argument[1]);
    DPrint("Filesize %s\n", fileSizePtr);
    
    if ((tempPtr = strrchr(URL, '/')) != NULL) {
		memcpy(localPath, URL, tempPtr - URL);
        fileNamePtr = tempPtr;
    }
	else {
		memcpy(localPath, URL, strlen(URL));
		fileNamePtr = "";
	}
    DPrint("file name:%s.\n", fileNamePtr);
    DPrint("Download type:%c.\n", argument[0][0]);
    if (DOWNLOAD_FIRMWARE_UPGRADE_IMAGE == argument[0][0]) {
        int msgLen = snprintf(msgBuf, sizeof(msgBuf),
                             QU"UpgradeServerUrl+%s|"
                               "UpgradeFileName+%s|"
                               "FileSize+%s"QU,
                               URL,
                               fileNamePtr,
                               fileSizePtr);
        snprintf(cmdBuf, sizeof(cmdBuf), 
						BR"\"user\":\"TM\","
                         "\"type\":1,"
                         "\"msg\" :%s,"
                         "\"len\" :%d"RB,
                         msgBuf,
                         msgLen);
        Send2UpgradeCli(cmdBuf, strlen(cmdBuf));
    }
    else if (DOWNLOAD_VERDOR_CONFIGURATION_FILE == argument[0][0]) {
        if (SY_FAILED == SyUrlParse(URL, &szHostInfo)) {
            DPrint("Parse URL failed!\n");
            return ;
        }
#ifdef SY_HAVE_LIBCURL
#ifdef SY_SH_TR069
        sscanf(URL, "%*[^//]//%[^@]", userPasswd);
        sscanf(URL, "%*[^@]@%[^]", remotepath);
        DPrint("userPasswd:%s, remotepath:%s\n", userPasswd, remotepath);
        syDownload(szHostInfo.transferType, userPasswd, remotepath,
            localPath, SY_TIMEOUT, SY_TRY_TIMES);
#endif
#endif
    }
	
    for (int i = 0; i < usedNum; i++) {
        DPrint("argument[%d]:%s\n", i, argument[i]);
        if (NULL != argument[i]){
            free(argument[i]);
            argument[i] =  NULL;
        }
    }
}

void HandleUpload(char* pos){

	char* argument[10] = {0};
	char tmpBuf[1024] = {0};
	char userPasswd[128] = {0};
	char remotepath[512] = {0};
	syHostInfo  szHostInfo;
	
    int usedNum = SyDelimArgument2(pos, "&", argument);
    DPrint("argument[1]:%s\n", argument[1]);
    memcpy(tmpBuf, argument[1], strlen(argument[1]));
    DPrint("tmpBuf:%s\n", tmpBuf);
    if (SY_FAILED == SyUrlParse(tmpBuf, &szHostInfo)) {
        DPrint("Parse URL failed\n");
        return ;
    }

    DPrint("Upload type:%c.\n", argument[0][0]);
#ifdef SY_SH_TR069
    sscanf(tmpBuf, "%*[^//]//%[^@]", userPasswd);
    sscanf(tmpBuf, "%*[^@]@%[^]", remotepath);
    DPrint("userPasswd:%s, remotepath:%s.\n", userPasswd, remotepath);
#endif
    if (UPLOAD_VERDOR_CONFIGURATION_FILE == argument[0][0]) {
#ifdef SY_HAVE_LIBCURL
#ifdef SY_SH_TR069
        syUpload(szHostInfo.transferType, userPasswd, remotepath, "./route.xml", SY_TIMEOUT, SY_TRY_TIMES);
#endif
#endif
    } 
	else if (UPLOAD_VERDOR_LOG_FILE == argument[0][0]) {
#ifdef SY_HAVE_LIBCURL
#ifdef SY_SH_TR069
        syUpload(szHostInfo.transferType, userPasswd, remotepath, "./route.xml", SY_TIMEOUT, SY_TRY_TIMES);
#endif
#endif
    }

    for (int i = 0; i < usedNum; i++) {
        DPrint("argument[%d]:%s\n", i, argument[i]);
        if (NULL != argument[i]) {
            free(argument[i]);
            argument[i] =  NULL;
        }
    }
    sleep(2);
}
void HandleUtil(pthread_t *threadid, iFunction func, const char *key) {
	if (0 == *threadid) {
        pthread_create(threadid, NULL, func, NULL);
    }
    else {
        DPrint("%s thread is running!\n", key);
    }
}

int Send2zeroApk(int flag)
{
    DO;

    int i;
    int maxNum = 3;
    int num = -1;
    int zeroConfigFd = -1;
    char *buf[] = {"SUCCESS", "FAILED"};
    char sendBuf[1024] = {0};
    FILE *fp = NULL;

    DPrint("flag:%d\n", flag);
    num = flag > 0 ? 0 : 1;
    for (i=0; i<maxNum; i++)
    {
        zeroConfigFd = CreateAndConn(SY_ZERO_CONFIG_PORT, SOCK_STREAM);
        if(-1 == zeroConfigFd)
        {
            DPrint("create fd failed ...\n");
            usleep(500 * 500);
            continue;
        }

        memset(sendBuf, 0x00, sizeof(sendBuf));
        sprintf(sendBuf, "%s:%s", buf[num], sendZeroBuf);
        sendBuf[strlen(sendBuf) - 1] = 0;
        int ret = send(zeroConfigFd, sendBuf, strlen(sendBuf), MSG_NOSIGNAL);
        DPrint("zero config fd:%d, buf:%s\n", zeroConfigFd, sendBuf);
        if (ret <= 0)
        {
            close(zeroConfigFd);
            zeroConfigFd = -1;
            usleep(500 * 500);
            continue;
        }
        break;
    }

    if (NULL != (fp = fopen(SY_BOOT_FLAG, "wb")))
    {
        fclose(fp);
    }

    close(zeroConfigFd);
    remove(SY_ZERO_CONFIG_FLAG);
    DONE;
    return SY_SUCCESS;
}

/* 处理TS APK 发送过来的JSON数据 */
LOCAL int HandleJson(int fd, char *str)
{
    DPrint("---->\n");
    if (fd == -1 || NULL == str || 0 == strlen(str))
    {
        DPrint("fd is -1 or str is NULL!\n");
        return SY_FAILED;
    }

    int i = 0;
    int rebootFlag = 0;
    int mountPathAvailable = -1;
    cJSON *json = NULL;
    char *output = NULL;
    char action[128] = {0};
    char lMountPath[1024] = {0};
    char tmpBuf[1024] = {0};

    json = cJSON_Parse(str);

    /* 测试用，打印接收到的全部JSON数据 */
    output = cJSON_Print(cJSONUtils_GetPointer(json, ""));
    if (NULL != output)
    {
        DPrint("recv json:\n%s\n", output);
        free(output);
        output = NULL;
    }

    /* 解析抓数据的方式:1)实时数据抓取，2)开机信息抓取 */
    ParseJSON(json, "action", action);

    /* 解析U盘路径 */
    ParseJSON(json, "mountPath", lMountPath);

    /* 解析数据抓取的限定时长 */
    ParseJSON(json, "time", tmpBuf);
    //gInfoTime = atoi(tmpBuf);

    cJSON_Delete(json);
    json = NULL;

    char tsTime[128] = {0};
    char tsState[128] = {"true"};
    char tsErrno[128] = {0};
    char tsShowstream[1024] = {0};
    char tsShowott[1024] = {0};

    /* 启动调试信息收集，收集实时数据 */
    if (0 == strcmp(action, "gatherInfo"))
    {

        /* 判断U盘路径是否可以读写 */
        if (-1 == access(lMountPath, R_OK|W_OK))
        {
            D("%s cannot read or write!\n", lMountPath);
            strncpy(tsState, "false", sizeof(tsState));
            strncpy(tsErrno, "U盘路径错误", sizeof(tsErrno));
        }
        else
        {
            if (1 == debugTsApkCollectstop)
            {
                strcpy(gMountPath, lMountPath);

                pthread_mutex_lock(&gMutexLock);
                gInfoTime = atoi(tmpBuf) * 60;
                DPrint("gInforTime:%d\n", gInfoTime);
                pthread_mutex_unlock(&gMutexLock);

                struct sockmsg sockMsg;
                memset(&sockMsg, 0x00, sizeof(struct sockmsg));
                sprintf(sockMsg.msg, "tsAPKDebugInfo");
                sockMsg.len = strlen(sockMsg.msg);
                strcpy(sockMsg.user, "tr069");
                Send2CmdProcThd(&sockMsg);
            }
            /* 如果已经有在抓取，则本次调试APK请求的调试信息抓取不起作用 */
            else
            {
                DPrint("tsAPKDebugInfo thread is running!.\n");
            }
        }
    }
    /* 启动开机信息收集, 收集开机数据 */
    else if (0 == strcmp(action, "gatherInfoAndReboot"))
    {
        strncpy(tsState,  "false", sizeof(tsState));
        FILE *fp = fopen(SY_TS_INFO_FLAG, "wb");
        if (NULL != fp)
        {
            char wbuf[1024] = {0};
            snprintf(wbuf, sizeof(wbuf), "%s\r\n", tmpBuf);
            int ret = (int)fwrite(wbuf, 1, strlen(wbuf), fp);
            DPrint("fwrite byte:%d, tmpBuf:%s, wbuf:%s\n", ret, tmpBuf, wbuf);
            fflush(NULL);
            fclose(fp);
            fp = NULL;
            rebootFlag = 1;
        }
        else
        {
            DPrint("create %s failed!\n", SY_TS_INFO_FLAG);
        }
    }
    /* 停止数据收集 */
    else if (0 == strcmp(action, "stop"))
    {
        strncpy(tsState,  "false", sizeof(tsState));
        debugTsApkCollectstop = 1;
    }
    /* 调试APK获取信息收集状态 */
    else if (0 == strcmp(action, "getstate"))
    {
        strncpy(tsState, debugTsApkCollectstop ? "false" : "true", sizeof(tsState));

        pthread_mutex_lock(&gMutexLock);
        snprintf(tsTime, sizeof(tsTime), "%d", gInfoTime);
        DPrint("gInfoTime:%d\n", gInfoTime);
        pthread_mutex_unlock(&gMutexLock);
    }
    /* 调试APK开机以后主动发送挂载地址过来 */
    else if (0 == strcmp(action, "upathmount"))
    {
        strncpy(tsState,  "true", sizeof(tsState));
        strcpy(gMountPath, lMountPath);
    }

    DPrint("state:%s, errno:%s, showstream:%s, showott:%s\n", tsState, tsErrno, tsShowstream, tsShowott);
    AssemblyJSON(fd, tsState, tsTime, tsErrno, tsShowstream, tsShowott);
    DPrint("action:%s, mountPath:%s, tmpBuf:%s\n", action, lMountPath, tmpBuf);

    /* 开机信息数据保存成功并正常回复APK以后，开始重启机顶盒 */
    if (1 == rebootFlag)
    {
        fflush(NULL);
        sleep(3);
        fflush(NULL);
        char aaa[] = {"reboot"};
        DPrint("system(%s) result:%d\n", aaa, system(aaa));
    }

    return SY_SUCCESS;
}

/* 解析cJSON数据 */
char* ParseJSON(cJSON *json, char *dstName, char *result)
{
    if (NULL == json || NULL == dstName || NULL == result)
    {
        DPrint("json or dstName or result is NULL!\n");
        return NULL;
    }

    cJSON *test = cJSON_GetObjectItem(json, dstName);
    if (NULL != test)
    {
        if(test->type == cJSON_Number)
        {
            sprintf(result, "%d", test->valueint);
            DPrint("dstName:%s, int value:%d\n", dstName, test->valueint);
        }
        else if(test->type == cJSON_String)
        {
            sprintf(result, "%s", test->valuestring);
            DPrint("dstName:%s, string value:%s\n", dstName, test->valuestring);
        }
    }
    return NULL;
}

/* 组装JSON数据并发送到TS APK */
void AssemblyJSON(int fd, char *state, char *tsTime, char *tsErrno, char *showstream, char *showott)
{
    DPrint("---->\n");
    if (NULL == state || fd == -1)
    {
        DPrint("state is NULL! or fd is -1!\n");
        return ;
    }

    DPrint("state:%s, errno:%s, showstream:%s, showott:%s\n", state, tsErrno, showstream, showott);

    cJSON *json = cJSON_CreateObject();
    if (NULL != json)
    {
        cJSON_AddStringToObject(json, "state", state);
        cJSON_AddStringToObject(json, "time", tsTime ? tsTime : "");
        cJSON_AddStringToObject(json, "erro", tsErrno ? tsErrno : "");
        cJSON_AddStringToObject(json, "showstream", showstream ? showstream : "");
        cJSON_AddStringToObject(json, "showott", showott ? showott : "");
        char *output = cJSON_Print(cJSONUtils_GetPointer(json, ""));
        if (NULL != output)
        {
            DPrint("output:\n%s\n", output);

            /* 发送数据到TS APK */
            if (send(fd, output, strlen(output), MSG_NOSIGNAL) <= 0)
            {
                DPrint("send failed! fd:%d\n", fd);
            }

            /* 清理申请的空间 */
            if (NULL != output)
            {
                free(output);
                output = NULL;
            }
        }

        cJSON_Delete(json);
        json = NULL;
    }

    DPrint("<----\n");
}

LOCAL int HandleZeroApkReq(void)
{
    DO;

    remove(SY_BOOT_FLAG);
    FILE *fp = NULL;
    if (NULL != (fp = fopen(SY_ZERO_CONFIG_FLAG, "wb")))
    {
        DPrint("create %s success\n", SY_ZERO_CONFIG_FLAG);
        fclose(fp);
    }
    else
    {
        DPrint("create %s failed\n", SY_ZERO_CONFIG_FLAG);
    }

    struct sockmsg sendMsg;
    memset(&sendMsg, 0x00, sizeof(sendMsg));
    if (0 == gSyIsTmStart)
    {
        gSyIsTmStart = 1;
        sendMsg.len = sprintf(sendMsg.msg, "%s","syCwmpInit");
        strcpy(sendMsg.user, "tr069");
        Send2CmdProcThd(&sendMsg);
    }

    DONE;
    return SY_SUCCESS;
}
#define Connect2libTR069(x,y) OpenAndConn("127.0.0.1",(x),(y),0,100)

#define SY_IPTV_SOCKET_HANDLE_PORT   23415
#define SY_IPTV_SOCKET_HANDLE_PORTEX 23414
int OpenAndConn(char* ip, int port, int type, int sec, int msec)
{
    int fd = socket(PF_INET, type, 0);
	struct sockaddr_in clientAddr;
	
    memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr(ip);
    clientAddr.sin_port = htons(port);
    
    struct timeval tv = {sec, msec*1000};

    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv));

	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));
	
    int ret = connect(fd, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
    if (ret < 0){
        EPrint("socket %d. Failed to connect - %s.\n", fd, strerror(errno));
        SyCloseSocket(fd);
        return ret;
    }
    DPrint("socket %d, connect %s\n", fd, 0 == ret ? "success":"failure");
    return fd;
}

/*

int Send2libTR069(Msg_TR069_s *pMsg, Msg_TR069_s* outBuf)
{
	static int GetAndSetValueSockfd = -1;
    if (NULL == pMsg){
        EPrint("Invalid argument.\n");
        return -1;
    }
    
    if (-1 == GetAndSetValueSockfd){
		GetAndSetValueSockfd = Connect2libTR069(SY_IPTV_SOCKET_HANDLE_PORT, SOCK_STREAM);
    }
    if (-1 == GetAndSetValueSockfd){
		WPrint("connect to iptv failed!\n");
		GetAndSetValueSockfd = Connect2libTR069(SY_IPTV_SOCKET_HANDLE_PORTEX, SOCK_STREAM);
		if (-1 == GetAndSetValueSockfd){
			EPrint("connect to laucher failed!\n");
			return SY_FAILED;
		}
    }
    int ret = send(GetAndSetValueSockfd, pMsg, sizeof(struct sockmsg), MSG_NOSIGNAL);
    if (ret <= 0){
        EPrint("fault, send(): %s.\n", strerror(errno));
        SyCloseSocket(GetAndSetValueSockfd);
        GetAndSetValueSockfd = -1;
        return SY_FAILED;
    }
    
    DPrint("recv %d bytes.",
    	recv(GetAndSetValueSockfd, outBuf, sizeof(*outBuf), MSG_NOSIGNAL));

    return ret;
}
bool GetData(int cmd, )
{
	Msg_TR069_s msg, rmsg;
	memset(rmsg, 0, sizeof(rmsg));
	msg.cmd = cmd;
    msg.type = SET_PARAM;
    msg.len = strlen(strcpy(msg.msg, s.msg));
    Send2libTR069(&msg, &rmsg);
}
*/
	
pthread_mutex_t gDataLock = PTHREAD_MUTEX_INITIALIZER;

static int gSyGetAndSetValueSockfd = -1;
int sySendDataToIptv(struct sockmsg *pMsg)
{
    if (NULL == pMsg){
        EPrint("Invalid argument.\n");
        return SY_FAILED;
    }
    
    if (-1 == gSyGetAndSetValueSockfd){
		gSyGetAndSetValueSockfd = Connect2libTR069(SY_IPTV_SOCKET_HANDLE_PORT, SOCK_STREAM);
        
    }
    if (-1 == gSyGetAndSetValueSockfd){
		EPrint("connect to iptv failed!\n");
		gSyGetAndSetValueSockfd = Connect2libTR069(SY_IPTV_SOCKET_HANDLE_PORTEX, SOCK_STREAM);
		if (-1 == gSyGetAndSetValueSockfd){
			EPrint("connect to laucher failed!\n");
			return SY_FAILED;
		}
    }
    int ret = send(gSyGetAndSetValueSockfd, pMsg, sizeof(struct sockmsg), MSG_NOSIGNAL);
    if (ret <= 0){
        EPrint("fault, send(): %s.\n", strerror(errno));
        SyCloseSocket(gSyGetAndSetValueSockfd);
        return SY_FAILED;
    }
    return SY_SUCCESS;
}


int SetValueToIptv(const char* path, char* value)
{
    int i = 0;
    int nCmd = -1;
    struct sockmsg sendMsg;
    struct sockmsg recvmsg;

    memset(&sendMsg, 0x00, sizeof(sendMsg));
    memset(&recvmsg, 0x00, sizeof(recvmsg));

    for (i = 0; i < gSyCmdStrListLen; i++){
        if (0 == strcmp(gSyCmdStrList[i].cmdStr, path)){
            nCmd = gSyCmdStrList[i].cmd;
            break;
        }
        else {
            if (SY_WRITE_END == gSyCmdStrList[i].cmd) {
                break;
            }
        }
    }

	int ret = SY_FAILED;
    if (-1 != nCmd){
		pthread_mutex_lock(&gDataLock);
        sendMsg.cmd = nCmd;
        sendMsg.type = SY_OP_SET_PARAM;
        strcpy(sendMsg.msg, value);
        strcpy(sendMsg.user, "manager");
        sendMsg.len = strlen(sendMsg.msg);
        if (SY_FAILED == sySendDataToIptv(&sendMsg)){
			SHOWMSG(&sendMsg);
        }
		else {
			int revSize = recv(gSyGetAndSetValueSockfd, &recvmsg, sizeof(recvmsg), 0);
	        if (revSize <= 0){
	            SyCloseSocket(gSyGetAndSetValueSockfd);
	            EPrint("Connection closed Or error occurred!\n");
	        }
	        else if (revSize != sizeof(recvmsg)) {
	            WPrint("Validation failed.\n");
	        }
			else {
				ret = SY_SUCCESS;
			}
		}
		pthread_mutex_unlock(&gDataLock);
    } 
	else {
		EPrint("Not found (%s)\n", path);
    }
    return ret;
}


/* 
 * 从iptv/launcher侧获取数据
 */
int GetValueToIptv(const char* path, char* value)
{
    int i = 0;
    int nCmd = -1;
    struct sockmsg sendMsg;
    struct sockmsg recvmsg;

    memset(&sendMsg, 0x00, sizeof(sendMsg));
    memset(&recvmsg, 0x00, sizeof(recvmsg));

    for (i = 0; i < gSyCmdStrListLen; i++){
        if (0 == strcmp(gSyCmdStrList[i].cmdStr, path)){
            nCmd = gSyCmdStrList[i].cmd;
            break;
        }
    }

	int ret = SY_FAILED;
    if (-1 != nCmd) {
        sendMsg.cmd = nCmd;
        sendMsg.type = SY_OP_GET_PARAM;
        strcpy(sendMsg.user, "manager");
        sendMsg.len = 0;
        if (SY_FAILED == sySendDataToIptv(&sendMsg)) {
			SHOWMSG(&sendMsg);
        }
		else {
			int revSize = recv(gSyGetAndSetValueSockfd, &recvmsg, sizeof(recvmsg), 0);
	        if (revSize < 0){
	            PERROR("recv:");
	            SyCloseSocket(gSyGetAndSetValueSockfd);
	        }
	        else if (revSize = 0){
	            SyCloseSocket(gSyGetAndSetValueSockfd);
	            EPrint("Connection be closed by peer.\n");
	        }
	        else if (revSize != sizeof(recvmsg)){
	            SyCloseSocket(gSyGetAndSetValueSockfd);
	            EPrint("Validate failed.\n");
	        }
	        else {
	            if ((-1 != recvmsg.len) && (-1 != recvmsg.cmd) && (0 != recvmsg.len)) {
	                strcpy(value, recvmsg.msg);
					ret = SY_SUCCESS;
	            }
	            else {
					SHOWMSG(&recvmsg);
		            EPrint("Invalid result.\n");
	            }
	        }
		}
    }
    else {
        EPrint("Not found (%s).\n", path);
    }

    return ret;
}



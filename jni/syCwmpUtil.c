/***********************************************************************
*
* syCwmpUtil.c
*
* Copyright (C) 2012 Lin jieling <mycourage0@126.com>
* Copyright (C) 2003-2012 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/
#include <time.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <dirent.h>


#include "syCwmpCommon.h"
#ifdef SY_HAVE_LIBCURL
#include "./thirdLib/curl/curl.h"
#endif
#include "./thirdLib/pcap/pcap.h"
#include "./thirdLib/ssh/libssh2.h"
#include "./thirdLib/ssh/libssh2_sftp.h"

#include "syCwmpManagement.h"
#include "syCwmpTaskQueue.h"

#define SY_PATH_PROCNET_DEV "/proc/net/dev"

extern char gMountPath[];
extern int gInfoTime;
extern pthread_mutex_t gMutexLock;

extern int StartupInfoFlag;
extern int DebugInfoFlag;
extern int startupInfoCollectStop;
extern int debugInfoCollectStop;

extern int gSyIpPingTesting;
extern int gSyTraceRouteTesting;
extern int gSyErrorCodeSwitch ;
extern int gSyErrorCodeInterval;

pthread_t gSyDebugInfoTid;
pthread_t gSyStartupInfoTid;

/*
调试APK停止数据抓取标志
有2种状态:1)1停止或者未曾收集, 2)0正在收集数据中
*/
int debugTsApkCollectstop = 1;

/* 调试APK启动实时数据收集的线程号 */
extern pthread_t gSyTsAPKStartupThreadId;

/* 调试APK启动开机数据收集的线程号 */
extern pthread_t gSyTsAPKDebugThreadId;

extern pthread_t gSyIpPingTestTid;
extern pthread_t gSyTraceRouteTestTid;
long gSyStartCaptureTime = 0L;
int gSyCaptureDuration = 0;
int gSyDeleteCaputureFile = 0;
int gSyBreakLoop = 0;
int gSyCount = 0;
char gSypcapFile[256] = {0};
char gSyPcapSftpFile[128] = {0};
pcap_t* gSydevice = NULL;
pcap_dumper_t* gSyDump = NULL;
extern int gSyPacketCapturing;
extern int gSyBandwidthDiagnosticsTesting;
extern pthread_t gSyPacketCaptureTid;
extern pthread_t gSyBandwidthDiagnosticsTid;

extern pthread_t gSyPeriodMsgLogTid;
extern int gSyEnableMsgLog;
extern int gSyMsgLogDuration;
extern int gSyMsgLogListLen;
extern int gSyMsgLogPeriodRunning;
extern syCmdStrStu gSyMsgLogList[];

void runFileDel(char *localPath);
void runPacketCapture(int flag, char *localFilePath);
void* runPacketCaptureNew(void *data);
void* runCaptureThread(void *data);
void runSftpUpload(int flag, char *localPath, char *fileName);
void runShellCommand(int flag, char *localPath, char *arg[][2]);
void runShellCommand1(int flag, char *cmd, char *localPath);
void syMacToTypeMAC(int type, char *MAC);
void* syLOGCAT(void *data);
void *runCollectLOG(void *data);
void* runShellCommandNew(void *localPath);
void runLOGCAT(char *localPath);

int SyIpPingTest(syIpPingResult* pIpPingResult, char* Host, unsigned int NumberOfRepetitions,
                 unsigned int Timeout,unsigned int DataBlockSize,unsigned int DSCP);
int SyTraceRouteTest(syTraceRouteResult* pTraceRouteResult, char *Host, unsigned int Timeout, unsigned int DataBlockSize,
                     unsigned int MaxHopCount,unsigned int DSCP);
int SyBandwidthTest(syBandwidthTestResult* pBandwidthTestResult);
void* syPacketCaptureCountThread(void* data);
int SyPacketCapture(syPacketCaptureResult* pPacketCaptureResult);
void SyCapCallBack(u_char * userarg, const struct pcap_pkthdr * pkthdr, const u_char * packet);

#ifdef SY_HAVE_LIBCURL
size_t getcontentlengthfunc(void *ptr, size_t size, size_t nmemb, void *stream);
size_t discardfunc(void *ptr, size_t size, size_t nmemb, void *stream);
size_t writefunc(void *ptr, size_t size, size_t nmemb, void *stream);
size_t readfunc(void *ptr, size_t size, size_t nmemb, void *stream);
int SyCurlUpload(CURL *curlhandle, const char* userPasswd,
                 const char * remotepath, const char * localpath, long timeout, long tries);
int SyCurlDownload(CURL *curlhandle, const char* userPasswd,
                   const char * remotepath, const char * localpath, long timeout, long tries);
static size_t WriteCallback(void *ptr, size_t size, size_t nmemb, void *data);
int SyCheckBandwidth(syBandwidthTestResult* pBandwidthTestResult, const char* userPasswd, const char* url);
void syKillExistProcess(char *processName);
#endif

extern bool GetValue(const char* path, char* value, size_t);

typedef struct
{
    LIBSSH2_SFTP * sftp_session_t;
    LIBSSH2_SESSION * session_t;
    int sock_t;
} sftp_sesandssock;

/*******************************************************************************
函数说明:	利用sftp协议上传文件
参数说明:    	sftp_sesandssock_t	:	自定义的结构体，包含了socket和sftp_session 和session
				block_t				:	0：非阻塞，1：非阻塞
				block_times			：	当设置block_t为非阻塞(0)时，block_times表示重复select的次数
				local_path			:	在本地主机中上传文件的位置
				remote_path			:	在远程服务器中存放文件的位置
返  回   值: 	0	:	上传成功
				-1	:	上传失败
*******************************************************************************/
int uploadfile_sftp(sftp_sesandssock * sftp_sesandssock_t, unsigned int block_t, unsigned int block_times, const char *local_path, const char *remote_path);

/*******************************************************************************
函数说明:	利用sftp协议下载文件
参数说明:    	sftp_sesandssock_t	:	自定义的结构体，包含了socket和sftp_session 和session
				block_t				:	0：非阻塞，1：非阻塞
				block_times			：	当设置block_t为非阻塞(0)时，block_times表示重复select的次数
				local_path			:	在本地主机中上传文件的位置
				remote_path			:	在远程服务器中存放文件的位置
返  回   值: 	0	:	下载成功
				-1	:	下载失败
*******************************************************************************/
int downloadfile_sftp(sftp_sesandssock * sftp_sesandssock_t, unsigned int block_t,unsigned int block_times, const char *local_path, const char *remote_path);
/*******************************************************************************
函数说明:	初始化一个sftp会话,成功返回
参数说明:   	hostaddr_t	:	服务器主机地址
				port_t		:	服务器该设置的主机端口号
				block_t		:	0： 非阻塞； 1：阻塞
				block_times	：	block_t设置为非阻塞(0)时，block_times表示select的次数，block_t设置成阻塞(1)时无意义
				publicfile	:	:	公钥文件
				privatefile	:	私钥文件
				username_t	:	登陆服务器的用户名
				password+t	:	登陆服务器对于用户名的密码
返  回   值: 	成功返回一个 sftp_sesandssock 的数组指针
				失败返回NULL
*******************************************************************************/
sftp_sesandssock * init_sftpsession(unsigned long hostaddr_t, unsigned int port_t, unsigned int block_t, unsigned int block_times, const char *publicfile, const char *privatefile, const char *username_t, const char *password_t);
void exit_sftpsession(sftp_sesandssock * sftp_sesandssock_t);



int SyGetCurrentTimeByType(int type, char *stime)
{
    static const char* date[12] =
    {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    time_t now = 0L;
    struct tm *tm1;

    DO;

    now = time(NULL);
    tm1 = localtime(&now);

    switch(type)
    {
    case 1:
        sprintf(stime, "%d%02d%02d%02d%02d%02d", tm1->tm_year+1900, tm1->tm_mon+1,
                tm1->tm_mday, tm1->tm_hour, tm1->tm_min, tm1->tm_sec);
        break;
    case 2:
        sprintf(stime, "%s %02d %02d:%02d:%02d", date[tm1->tm_mon], tm1->tm_mday,
                tm1->tm_hour, tm1->tm_min, tm1->tm_sec);
        break;
    case 3:
        sprintf(stime, "%d-%02d-%02dT%02d:%02d:%02d", tm1->tm_year+1900, tm1->tm_mon+1,
                tm1->tm_mday, tm1->tm_hour, tm1->tm_min, tm1->tm_sec);
        break;
    default:
        sprintf(stime, "%d-%02d-%02d:%02d-%02d-%02d", tm1->tm_year+1900, tm1->tm_mon+1,
                tm1->tm_mday, tm1->tm_hour, tm1->tm_min, tm1->tm_sec);
        break;
    }
    return SY_SUCCESS;
}

time_t SyGetCurrentTime(void)
{
    time_t CurrentTime;
    CurrentTime = time(NULL);
    return CurrentTime;
}

char* GetCurrTime(void)
{
    static char CurrentTimeBuf[32] = {0};
    time_t CurrentTime;
    struct tm* TmpTm;

    CurrentTime = time(NULL);
    TmpTm = localtime(&CurrentTime);
    sprintf(CurrentTimeBuf, "%04d-%02d-%02dT%02d:%02d:%02d", TmpTm->tm_year + 1900, TmpTm->tm_mon + 1,
            TmpTm->tm_mday, TmpTm->tm_hour, TmpTm->tm_min, TmpTm->tm_sec);
	DPrint("CurrentTimeBuf is %s, len is %d", CurrentTimeBuf, strlen(CurrentTimeBuf));
    return CurrentTimeBuf;
}

int SyGetMAC(int type, char* pszMAC)
{
    int nSocket;
    struct ifreq struReq;
    DO;

    nSocket = socket(PF_INET, SOCK_STREAM, 0);
    memset(&struReq, 0, sizeof(struReq));
    strncpy(struReq.ifr_name, "wlan0", sizeof(struReq.ifr_name));
    if (ioctl(nSocket, SIOCGIFHWADDR, &struReq) < 0)
    {
        DPrint("1\n");
        memset(&struReq, 0, sizeof(struReq));
        strncpy(struReq.ifr_name, "eth0", sizeof(struReq.ifr_name));
        //strncpy(struReq.ifr_name, "rmnet0", sizeof(struReq.ifr_name));
        if (ioctl(nSocket, SIOCGIFHWADDR, &struReq) < 0)
        {
            DPrint("2\n");
            close(nSocket);
            return SY_FAILED;
        }
    }
    close(nSocket);
    unsigned char* p = (unsigned char*)struReq.ifr_hwaddr.sa_data;

    switch(type)
    {
    case 1:
        sprintf(pszMAC, "%02X%02X%02X%02X%02X%02X", p[0],
                p[1], p[2], p[3], p[4], p[5]);
        break;
    case 2:
        sprintf(pszMAC, "%02X-%02X-%02X-%02X-%02X-%02X", p[0],
                p[1], p[2], p[3], p[4], p[5]);
        break;
    default:
        sprintf(pszMAC, "%02X:%02X:%02X:%02X:%02X:%02X", p[0],
                p[1], p[2], p[3], p[4], p[5]);
        break;
    }

    return SY_SUCCESS;
}

int SyGetIPAddr(char* pszIPAddr)
{
    /*
     * TODO:我们应该通过路由表确定当前正在使用的接口.
     */
    int nSocket;
    struct ifreq struReq;
    DO;

    nSocket = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&struReq, 0, sizeof(struReq));
    strncpy(struReq.ifr_name, "wlan0", sizeof(struReq.ifr_name));
    if (ioctl(nSocket, SIOCGIFADDR, &struReq) < 0)
    {
        W("wlan0 haven't IP.");
        memset(&struReq, 0, sizeof(struReq));
        strncpy(struReq.ifr_name, "eth0", sizeof(struReq.ifr_name));
        if (ioctl(nSocket, SIOCGIFADDR, &struReq) < 0)
        {

            close(nSocket);
            return SY_FAILED;
        }
    }
    close(nSocket);
    char* bytes = (char*)&(((struct sockaddr_in*)&(struReq.ifr_addr))->sin_addr);
    sprintf(pszIPAddr, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
    DONE;
    return SY_SUCCESS;
}

int SySetTimezone(unsigned char zone)
{
    char cTimezone[16] = {0};

    sprintf(cTimezone, "UTC-%d", zone);
    setenv("ZT", cTimezone, 1);
    return SY_SUCCESS;
}

int SyUrlParse(char* url, syHostInfo* hostInfo)
{
    char* pos;
    char* tmpUrl;

    tmpUrl = (char *)strdup(url);
    DPrint("tmpUrl:%s\n", tmpUrl);
    if(NULL != strstr(tmpUrl, "http://"))
    {
        hostInfo->host = tmpUrl + 7;
        hostInfo->transferType = SY_TYPE_HTTP;
    }
    else if(NULL != strstr(tmpUrl, "ftp://"))
    {
        hostInfo->host = tmpUrl + 6;
        hostInfo->transferType = SY_TYPE_FTP;
    }
    else if(NULL != strstr(tmpUrl, "HTTP://"))
    {
        hostInfo->host = tmpUrl + 7;
        hostInfo->transferType = SY_TYPE_HTTP;
    }
    else if(NULL != strstr(tmpUrl, "FTP://"))
    {
        hostInfo->host = tmpUrl + 6;
        hostInfo->transferType = SY_TYPE_FTP;
    }
    else
    {
        free(tmpUrl);
        return SY_FAILED;
    }

    pos = strchr(hostInfo->host, '/');
    if(!pos)
    {
        hostInfo->path = "";
    }
    else if(*pos == '/')
    {
        *pos = '\0';
        hostInfo->path = pos + 1;
    }
    return SY_SUCCESS;
}

int SyDelimArgument(char *optarg , char *delim , char *argument[])
{
    int i = 1;
    char *p[ARGUMENT_MAX];

    //p[0]=&ch_flag;
    p[0]=strtok(optarg, delim);
    while((p[i]=strtok(NULL, delim)))
        i++;

    memcpy((void*)argument, (void*)p, i * sizeof(char*));
    //printf("argument_s_analyze=%s\n",buf[0]);

    return SY_SUCCESS;
}

/* return usedNum*/
int SyDelimArgument2(char *optarg , char *delim , char *argument[])
{
    int i = 0;
    char* pos = NULL;
    char* p[ARGUMENT_MAX];
    char tmpBuf[256] = {0};

    memset(tmpBuf, 0x00, sizeof(tmpBuf));
    //DPrint("optarg:%s\n", optarg);

    pos = optarg;
    while((p[i] = strstr(pos + 1, delim)))
    {
        memset(tmpBuf, 0x00, sizeof(tmpBuf));
        memcpy(tmpBuf, pos + 1, p[i] - (pos + 1));
        //DPrint("tmpBuf:%s\n", tmpBuf);
        pos = p[i];
        p[i] = strdup((const char*)tmpBuf);
        i++;
    }

    memcpy((void*)argument, (void*)p, i * sizeof(char*));
    return i;
}

void* syStartupInfoThread(void *data)
{
    char fileFormat[1024] = {0};
    char localPath[256] = {"/data/data/tmp/"};
    char uploadPath[256] = {"/data/data/"};

    char *arg[][2] =
    {
        {"busybox arp -a", 		"arp.txt"},
        {"ip route show", 		"route.txt"},
        {"busybox ifconfig -a", "networkConfiguration.txt"},
        {NULL, 					"syConfig.xml"},
        NULL,
    };

    DPrint("mkdir(%s) result:%d\n", localPath, mkdir(localPath, 0777));

    /*设置信息正在收集标识*/
    SySetNodeValue("Device.X_00E0FC.StartupInfo.State", "1");

    /*开机启动抓包*/
    char localFilePath[512] = {0};
    snprintf(localFilePath, sizeof(localFilePath), "%scapture.pcap", localPath);
    runLOGCAT(localPath);
    runPacketCapture(1, localFilePath);
    runShellCommand(1, localPath, arg);

    while (1)
    {
        if (1 == startupInfoCollectStop)
        {
            DPrint("startup data collect quit and begin upload data.\n");
            break;
        }
        usleep(500*1000);
    }

    /*标识正在上传文件*/
    SySetNodeValue("Device.X_00E0FC.StartupInfo.State", "2");

    /*打包收集的文件*/
    char packageName[512] = {0};
    char MAC[128] = {0};
    size_t sizeOfBuf = sizeof(MAC);
    char cTime[256] = {0};
    char tarBuf[1024] = {0};
    char tarFileName[1024] = {0};
    //SyGetMAC(1, MAC);
    if (!GetValue("Device.LAN.MACAddress", MAC, sizeOfBuf))
    {
        getprop("ro.mac", MAC, "noDefine");
    }
    syMacToTypeMAC(1, MAC);
    SyGetCurrentTimeByType(1, cTime);
    snprintf(packageName, sizeof(packageName), "STBStartupInfo%s%s.tar.gz", MAC, cTime);
    snprintf(tarFileName, sizeof(tarFileName), "/data/data/%s", packageName);
    snprintf(tarBuf, sizeof(tarBuf), "busybox tar czvf %s %s", tarFileName, localPath);
    DPrint("system(%s) result:%d\n", tarBuf, system(tarBuf));

    /*开始上传*/
    runSftpUpload(1, uploadPath, packageName);

    /*标识文件上传完成*/
    SySetNodeValue("Device.X_00E0FC.StartupInfo.State", "3");

    runFileDel(localPath);
    remove(tarFileName);
    remove(SY_START_UP_INFO_FLAG);
    startupInfoCollectStop = 0;
    StartupInfoFlag = 0;
    gSyStartupInfoTid = 0;
    return NULL;
}

void syMacToTypeMAC(int type, char *MAC)
{
    int i, j;
    char lMac[32] = {0};
    switch (type)
    {
    case 1:
        for (i=0,j=0; i<strlen(MAC); i++)
        {
            if (MAC[i] != ':')
            {
                if (MAC[i] >= 'a' && MAC[i] <= 'z')
                {
                    lMac[j++] = MAC[i] - 32;
                }
                else
                {
                    lMac[j++] = MAC[i];
                }
            }
        }
        break;
    default:
        break;
    }

    strcpy(MAC, lMac);
    return ;
}

void* syDebugInfoThread(void *data)
{
    char fileFormat[1024] = {0};
    char localPath[256] = {"/data/data/tmp/"};
    char uploadPath[256] = {"/data/data/"};

    char *arg[][2] =
    {
        {"busybox arp -a", 		"arp.txt"},
        {"ip route show", 		"route.txt"},
        {"busybox ifconfig -a", "networkConfiguration.txt"},
        NULL,
    };

    DPrint("mkdir(%s) result:%d\n", localPath, mkdir(localPath, 0777));

    /*设置信息正在收集标识*/
    SySetNodeValue("Device.X_00E0FC.DebugInfo.State", "1");

    /*获取MAC*/
    char packageName[512] = {0};
    char MAC[128] = {0};
    size_t sizeOfBuf = sizeof(MAC);
    //SyGetMAC(1, MAC);
    if (!GetValue("Device.LAN.MACAddress", MAC, sizeOfBuf))
    {
        getprop("ro.mac", MAC, "noDefine");
    }
    syMacToTypeMAC(1, MAC);

    /*获取TIME*/
    char cTime[256] = {0};
    SyGetCurrentTimeByType(1, cTime);
    snprintf(packageName, sizeof(packageName), "STBDebugInfo%s%s.tar.gz", MAC, cTime);

    /*启动抓包线程*/
    char localFilePath[512] = {0};
    snprintf(localFilePath, sizeof(localFilePath), "%scapture.pcap", localPath);
    runLOGCAT(localPath);
    runPacketCapture(0, localFilePath);
    runShellCommand(0, localPath, arg);

    /**/
    while (1)
    {
        if (1 == debugInfoCollectStop)
        {
            DPrint("startup data collect quit and begin upload data.\n");
            break;
        }
        usleep(500*1000);
    }

    /*标识正在上传文件*/
    SySetNodeValue("Device.X_00E0FC.DebugInfo.State", "2");

    /*打包文件*/
    char tarBuf[1024] = {0};
    char tarFileName[1024] = {0};
    snprintf(tarFileName, sizeof(tarFileName), "/data/data/%s", packageName);
    snprintf(tarBuf, sizeof(tarBuf), "busybox tar czvf %s %s", tarFileName, localPath);
    DPrint("system(%s) result:%d\n", tarBuf, system(tarBuf));

    /*上传文件*/
    runSftpUpload(0, uploadPath, packageName);

    /*标识文件上传完成*/
    SySetNodeValue("Device.X_00E0FC.DebugInfo.State", "3");

    runFileDel(localPath);
    remove(tarFileName);
    debugInfoCollectStop = 0;
    DebugInfoFlag = 0;
    gSyDebugInfoTid = 0;

    return NULL;
}

void runFileDel(char *localPath)
{
    DO;
    DIR *dir = NULL;
    struct dirent *dirent = NULL;

    if (NULL != (dir = opendir(localPath)))
    {
        while ((dirent = readdir(dir)))
        {
            if (!strcmp(dirent->d_name, ".") || !strcmp(dirent->d_name, ".."))
            {
                continue;
            }
            char fileName[128] = {0};
            snprintf(fileName, sizeof(fileName), "%s%s", localPath, dirent->d_name);
            DPrint("remove(%s) result:%d\n", fileName, remove(fileName));
        }
        closedir(dir);
    }

    DONE;
    return ;
}

void runSftpUpload(int flag, char *localPath, char *fileName)
{
    DO;

    char *URL = (char *)malloc(1024);
    char *username = (char *)malloc(1024);
    char *passwd = (char *)malloc(1024);
    if (NULL == URL || NULL == username || NULL == passwd)
    {
        free(URL);
        free(username);
        free(passwd);
        DPrint("malloc failed\n");
        return ;
    }

    memset(URL, 0x00, 1024);
    memset(username, 0x00, 1024);
    memset(passwd, 0x00, 1024);
    if (0 == flag)
    {
        SyGetNodeValue("Device.X_00E0FC.DebugInfo.UploadURL", URL);
        SyGetNodeValue("Device.X_00E0FC.DebugInfo.Username", username);
        SyGetNodeValue("Device.X_00E0FC.DebugInfo.Password", passwd);
    }
    else
    {
        SyGetNodeValue("Device.X_00E0FC.StartupInfo.UploadURL", URL);
        SyGetNodeValue("Device.X_00E0FC.StartupInfo.Username", username);
        SyGetNodeValue("Device.X_00E0FC.StartupInfo.Password", passwd);
    }
    DPrint("flag:%d, uploadURL:%s, username:%s, password:%s\n", flag, URL, username, passwd);

    int block = 0;
    int hostPort = 22;
    char hostIP[128] = {0};
    char ipPort[128] = {0};
    char buf[256] = {0};
    char sftpPath[256] = {0};
    char sftpFilePath[1024] = {0};
    char localFilePath[1024] = {0};

    char *pURL = strstr(URL, "sftp://");
    if (NULL != pURL)
    {
        pURL = pURL + 7;
        char *pIpPort = strstr(pURL, "/");
        /*从URL中提取IP地址和端口部分*/
        if (NULL != pIpPort)
        {
            memcpy(ipPort, pURL, (int)(pIpPort-pURL));
            memcpy(sftpPath, pIpPort, strlen(pIpPort));
            if (sftpPath[strlen(sftpPath) - 1] != '/')
            {
                sftpPath[strlen(sftpPath)] = '/';
            }
        }
        else
        {
            memcpy(ipPort, pURL, strlen(pURL));
            strcpy(sftpPath, "/");
        }
        /*分离IP和端口PORT*/
        char *pIP = strstr(ipPort, ":");
        if (NULL != pIP)
        {
            memcpy(hostIP, ipPort, (int)(pIP-ipPort));
            memcpy(buf, pIP+1, strlen(pIP+1));
            hostPort = atoi(buf);
        }
        else
        {
            memcpy(hostIP, ipPort, strlen(ipPort));
        }
    }
    snprintf(localFilePath, sizeof(localFilePath), "%s%s", localPath, fileName);
    snprintf(sftpFilePath, sizeof(sftpFilePath), "%s%s", sftpPath, fileName);
    FILE *fp = fopen(localFilePath, "r");
    if (NULL != fp)
    {
        fclose(fp);
        fp = NULL;
        DPrint("hostIP:%s, hostport:%d, block:%d, username:%s, password:%s, localpath:%s, sftppath:%s\n" ,
               hostIP, hostPort, block, username, passwd, localFilePath, sftpFilePath);
        sySftpUpload(hostIP, hostPort, block, username, passwd, localFilePath, sftpFilePath);
    }
    else
    {
        DPrint("%s is not exist or open failed,\n", localFilePath);
    }

    free(URL);
    free(username);
    free(passwd);
    DONE;
    return ;
}

void runPacketCapture(int flag, char *localFilePath)
{
    DO;
    pthread_t tid;
    int fileSize = 0;
    int maxSize = 0;
    char buf[512] = {0};
    char capCmd[1024] = {0};

    if (0 == flag)
    {
        SyGetNodeValue("Device.X_00E0FC.DebugInfo.CapCommand", buf);
        if (!strcmp(buf, ""))
        {
            DPrint("no capture command ... function return .\n");
            return ;
        }
        char *p = strstr(capCmd, "-w");
        if (NULL == p)
        {
            snprintf(capCmd, sizeof(capCmd), "%s -w %s -U", buf, localFilePath);
        }
        else
        {
            memcpy(capCmd, buf, strlen(buf));
        }
    }
    else if (1 == flag)
    {
        snprintf(capCmd, sizeof(capCmd), "/data/data/com.android.smart.terminal.iptv/bin/tcpdump -i eth0 -s 0 -w %s -U", localFilePath);
        memset(buf, 0x00, sizeof(buf));
        SyGetNodeValue("Device.X_00E0FC.StartupInfo.MaxSize", buf);
        maxSize = atoi(buf);
        DPrint("flag:%d, maxSize:%d\n", flag, maxSize);
    }
    else if (2 == flag)
    {
        snprintf(capCmd, sizeof(capCmd), "/data/data/com.android.smart.terminal.iptv/bin/tcpdump -s 0 -w %s -U", localFilePath);
        maxSize = 1;
    }

    if (pthread_create(&tid, NULL, runCaptureThread, (void*)capCmd) < 0)
    {
        DPrint("create thread failed\n");
    }
    else
    {
        pthread_detach(tid);
    }

    sleep(1);
    struct stat logFileName;
    while (1)
    {
        /* 网管平台启动的一键信息收集 */
        if (0 == flag)
        {
            if (1 == debugInfoCollectStop)
            {
                DPrint("kill tcpdump.\n");
                syKillExistProcess("tcpdump");
                break;
            }
        }
        /* 网管平台启动的开机信息收集 */
        else if (1 == flag)
        {
            if (1 == startupInfoCollectStop)
            {
                DPrint("kill tcpdump.\n");
                syKillExistProcess("tcpdump");
                break;
            }
            stat(localFilePath, &logFileName);
            fileSize = logFileName.st_size;
            if (fileSize >= maxSize * 1000000)
            {
                DPrint("kill tcpdump, maxSize:%d, size:%d, flagPath:%s\n",
                       maxSize, fileSize, localFilePath);
                syKillExistProcess("tcpdump");
                break;
            }
            DPrint("fileSize:%d\n", fileSize);
        }
        /* 调试APK启动的信息收集 */
        else if (2 == flag)
        {
            if (1 == debugTsApkCollectstop)
            {
                DPrint("kill tcpdump.\n");
                syKillExistProcess("tcpdump");
                break;
            }
            stat(localFilePath, &logFileName);
            fileSize = logFileName.st_size;

            /* 设置一个安全抓包大小,最大为1G */
            if (fileSize >= maxSize * 1000000000)
            {
                DPrint("kill tcpdump, maxSize:%d, size:%d, flagPath:%s\n",
                       maxSize, fileSize, localFilePath);
                syKillExistProcess("tcpdump");
                break;
            }
            DPrint("fileSize:%d\n", fileSize);
        }

        sleep(1);
    }

    DONE;
    return ;
}

void* runPacketCaptureNew(void *data)
{
    DPrint("---->\n");
    char pcapFilePath[1024] = {0};

    if (NULL == data)
    {
        DPrint("data is NULL.\n");
        return NULL;
    }

    memcpy(pcapFilePath, data, sizeof(pcapFilePath));
    runPacketCapture(2, pcapFilePath);

    return NULL;
}

void* runCaptureThread(void *data)
{
    DO;
    char cmd[1024] = {0};

    memcpy(cmd, data, strlen(data));
    DPrint("system(%s), result:%d\n", cmd, system(cmd));

    DONE;
    return NULL;
}

void* syLOGCAT(void *data)
{
    DO;
    char filePath[1024] = {0};
    char cmd[1024] = {0};

    memcpy(filePath, data, strlen((char*)data));
    snprintf(cmd, sizeof(cmd), "logcat -vtime > %slog.log", filePath);
    DPrint("system(%s) result:%d\n", cmd, system(cmd));

    return NULL;
}

void runLOGCAT(char *localPath)
{
    DO;
    pthread_t lTid;
    if (pthread_create(&lTid, NULL, runCollectLOG, (void*)localPath) < 0)
    {
        DPrint("run logcat failed\n");
    }
    else
    {
        pthread_detach(lTid);
    }

    sleep(1);
    return ;
}

void *runCollectLOG(void *data)
{
    DO;
    char filePath[1024] = {0};
    pthread_t lTid;

    /**/
    memcpy(filePath, data, strlen((char*)data));
    if (pthread_create(&lTid, NULL, syLOGCAT, (void*)filePath) < 0)
    {
        DPrint("create thread 1 failed\n");
    }
    else
    {
        pthread_detach(lTid);
    }

    while (1)
    {
        if (1 == startupInfoCollectStop || 1 == debugInfoCollectStop || 1 == debugTsApkCollectstop)
        {
            syKillExistProcess("logcat");
            break;
        }

        /*判断抓log线程是否还存在如果不存在就
        结束上报,这样是防止抓log线程意外结束了
        但是还没有达到下发的时间间隔的情况
        这样提前结束整个实时日志上传*/
        int ret = pthread_kill(lTid, 0);
        if ((ret == ESRCH) || (ret == EINVAL))
        {
            break;
        }
        usleep(500 * 1000);
    }

    DONE;
    return NULL;
}

void runShellCommand1(int flag, char *cmd, char *localPath)
{
    DPrint("---->\n");
    FILE *fp = NULL;
    char mCmd[1024] = {0};
    char buf[1024] = {0};
    char localFilePath[1024] = {0};
    char *ptr = NULL;

    switch (flag)
    {
    case 0:
        snprintf(mCmd, sizeof(mCmd), "cp /data/data/syConfig.xml %s", localPath);
        DPrint("system(%s) result:%d\n", mCmd, system(mCmd));
        break;
    case 1:
        snprintf(localFilePath, sizeof(localFilePath), "%sarp.txt", localPath);
        snprintf(mCmd, sizeof(mCmd), "%s>>%s", cmd, localFilePath);
        DPrint("system(%s) result:%d\n", mCmd, system(mCmd));
        ptr = localFilePath;
        break;
    case 2:
        snprintf(localFilePath, sizeof(localFilePath), "%sroute.txt", localPath);
        snprintf(mCmd, sizeof(mCmd), "%s>>%s", cmd, localFilePath);
        DPrint("system(%s) result:%d\n", mCmd, system(mCmd));
        ptr = localFilePath;
        break;
    case 3:
        snprintf(localFilePath, sizeof(localFilePath), "%snetConfig.txt", localPath);
        snprintf(mCmd, sizeof(mCmd), "%s>>%s", cmd, localFilePath);
        DPrint("system(%s) result:%d\n", mCmd, system(mCmd));
        ptr = localFilePath;
        break;
    default:
        /* 注意top之所以用popen而不像上面一样用system函数是因为，system不知道
        为什么在代码里面一直有问题，文件一直是空，所以采用popen */
        if ((fp = popen(cmd, "r")) != NULL)
        {
            snprintf(localFilePath, sizeof(localFilePath), "%stop.txt", localPath);
            ptr = localFilePath;

            /* 注意这个地方为了配合调试APK要求的5秒抓取一次数据，然后追加到末尾的方式开打 */
            FILE *fileFp = fopen(localFilePath, "a+");
            if (NULL != fileFp)
            {
                while (fgets(buf, sizeof(buf), fp))
                {
                    fwrite(buf, strlen(buf), 1, fileFp);
                }
                fclose(fileFp);
                fileFp = NULL;
            }
            pclose(fp);
            fp = NULL;
        }
        break;
    }

    /* 如果是五秒一次的刷新，则需要在每一部分数据之间添加一个分割线 */
    if (NULL != ptr)
    {
        char cTime[1024] = {0};
        char lBuf[1024] = {0};
        SyGetCurrentTimeByType(3, cTime);
        snprintf(lBuf, sizeof(lBuf), "\r\n*********************************** %s ********************************\r\n\r\n\r\n", cTime);
        fp = fopen(localFilePath, "a+");
        if (NULL != fp)
        {
            fwrite(lBuf, strlen(lBuf), 1, fp);
            fclose(fp);
            fp = NULL;
        }
    }

    return ;
}

void runShellCommand(int flag, char *localPath, char *arg[][2])
{
    DO;
    FILE *fp = NULL;
    FILE *fileFp = NULL;
    char aBuf[1024] = {0};
    char cmdBuf[128] = {0};
    char wBuf[1024] = {0};
    char buf[1024] = {0};
    int num = 0;

    snprintf(cmdBuf, sizeof(cmdBuf), "cp /data/data/syConfig.xml %s", localPath);
    DPrint("system(%s) result:%d\n", cmdBuf, system(cmdBuf));

    int i = 0;
    while (arg[i][1] != NULL)
    {
        if (arg[i][0] != NULL)
        {
            memset(buf, 0x00, sizeof(buf));
            snprintf(buf, sizeof(buf), "%s>>%s%s", arg[i][0], localPath, arg[i][1]);
            DPrint("system(%s) result:%d\n", buf, system(buf));
        }
        i++;
    }

    if ((fp = popen("busybox top -n 1", "r")) != NULL)
    {
        char filePath[1024] = {0};
        snprintf(filePath, sizeof(filePath), "%stop.txt", localPath);

        /* 注意这个地方为了配合调试APK要求的5秒抓取一次数据，然后追加到末尾的方式开打 */
        fileFp = fopen(filePath, "a+");
        if (NULL != fileFp)
        {
            while (fgets(wBuf, sizeof(wBuf), fp))
            {
                fwrite(wBuf, strlen(wBuf), 1, fileFp);
            }
        }
        fclose(fileFp);
        pclose(fp);
    }

    DONE;
    return ;
}

/* 调试APK启动当前数据收集 */
void* syTSApkDebugInfoThread(void *data)
{
    DPrint("---->\n");

    int timeCount = 0;
    char localPath[1024] = {0};
    char cTime[256] = {0};

    /*获取TIME*/
    SyGetCurrentTimeByType(1, cTime);
    DPrint("get local time:%s\n", cTime);

    /* 判断U盘路径是否可以读写 */
    if (0 != access(gMountPath, R_OK|W_OK))
    {
        DPrint("%s cannot read or write!\n", gMountPath);
        gSyTsAPKDebugThreadId = 0;
        return NULL;
    }

    if (gMountPath[strlen(gMountPath) - 1] == '/')
    {
        snprintf(localPath, sizeof(localPath), "%sTsFile%s/", gMountPath, cTime);
    }
    else
    {
        snprintf(localPath, sizeof(localPath), "%s/TsFile%s/", gMountPath, cTime);
    }
    DPrint("mkdir(%s) result:%d\n", localPath, mkdir(localPath, 0777));

    /* 先初始化stop的值 */
    debugTsApkCollectstop = 0;

    //saveSTBMessage();

    /* 启动抓log线程 */
    pthread_t threadLogTid;
    if (pthread_create(&threadLogTid, NULL, runCollectLOG, (void*)localPath) < 0)
    {
        DPrint("run logcat failed\n");
    }
    else
    {
        pthread_detach(threadLogTid);
    }

    /* 启动arp, route, top等数据抓取线程 */
    pthread_t threadDataTid;
    if (pthread_create(&threadDataTid, NULL, runShellCommandNew, (void*)localPath) < 0)
    {
        DPrint("run logcat failed\n");
    }
    else
    {
        pthread_detach(threadDataTid);
    }

    /* 启动抓包线程 */
    char pcapFilePath[128] = {0};
    snprintf(pcapFilePath, sizeof(pcapFilePath), "%spcap.pcap", localPath);
    pthread_t threadTid;
    if (pthread_create(&threadTid, NULL, runPacketCaptureNew, (void*)pcapFilePath) < 0)
    {
        DPrint("run logcat failed\n");
    }
    else
    {
        pthread_detach(threadTid);
    }
    //runPacketCapture(2, pcapFilePath);

    /* gInfoTime如果是0表示没有设定数据收集
    的时间，需要手动停止,抓包那里有做一个安全限制，如果抓取网络包
    大小超过1G还没有手动停止，抓包也会自动停止*/
    if (0 == gInfoTime)
    {
        while (0 == debugTsApkCollectstop)
        {
            DPrint("not set gInfoTime, wait for stop!\n");
            sleep(3);
        }
    }
    /* gInfoTime不为0表示设置了数据收集的时间，按照设定时长来抓取 */
    else
    {
        pthread_mutex_lock(&gMutexLock);
        timeCount = gInfoTime;
        DPrint("timeCount:%d\n", timeCount);
        pthread_mutex_unlock(&gMutexLock);
        while (timeCount-- > 0 && 0 == debugTsApkCollectstop)
        {
            sleep(1);

            pthread_mutex_lock(&gMutexLock);
            if (gInfoTime >= 1)
            {
                gInfoTime -= 1;
                DPrint("gInfoTime:%d\n", gInfoTime);
            }
            pthread_mutex_unlock(&gMutexLock);
        }

        pthread_mutex_lock(&gMutexLock);
        gInfoTime = 0;
        pthread_mutex_unlock(&gMutexLock);

        debugTsApkCollectstop = 1;
    }

    gSyTsAPKDebugThreadId = 0;
    D("<----\n");
    return NULL;
}

void* runShellCommandNew(void *localPath)
{
    DPrint("---->\n");
    if (NULL == localPath)
    {
        DPrint("localPath is NULL.\n");
        return NULL;
    }

    while (1)
    {
        if (1 == debugTsApkCollectstop)
        {
            DPrint("quit.\n");
            break;
        }
        runShellCommand1(0, NULL, localPath);
        runShellCommand1(1, "busybox arp -a", localPath);
        runShellCommand1(2, "ip route show", localPath);
        runShellCommand1(3, "busybox ifconfig -a", localPath);
        runShellCommand1(4, "busybox top -n 1", localPath);
        sleep(5);
    }

    return NULL;
}

/* 调试APK启动开机信息收集 */
void* syTSApkStartupInfoThread(void *data)
{
    DPrint("---->\n");

    int timeCount = 0;
    char localPath[1024] = {"/data/data/syTsTest/"};
    char cTime[256] = {0};
    char lInfoTime[128] = {0};

    FILE *fp = fopen(SY_TS_INFO_FLAG, "rb");
    if (NULL != fp)
    {
        int ret = (int)fread(lInfoTime, 1, sizeof(lInfoTime) - 1, fp);
        timeCount = atoi(lInfoTime) * 60;
        if (timeCount <= 0 || timeCount >= 1200)
        {
            timeCount = 180;
        }

        pthread_mutex_lock(&gMutexLock);
        gInfoTime = timeCount;
        DPrint("fread byte:%d, lInfoTime :%s, gInfoTime:%d, timeCount:%d\n", ret, lInfoTime, gInfoTime, timeCount);
        pthread_mutex_unlock(&gMutexLock);

        fclose(fp);
        fp = NULL;

        remove(SY_TS_INFO_FLAG);
    }
    else
    {
        DPrint("open %s failed!\n", SY_TS_INFO_FLAG);
        timeCount = 180;
    }

    /* 创建目录 */
    DPrint("mkdir(%s) result:%d\n", localPath, mkdir(localPath, 0777));

    /* 判断路径是否可以读写 */
    if (0 != access(localPath, R_OK|W_OK))
    {
        DPrint("%s cannot read or write!\n", localPath);
        gSyTsAPKStartupThreadId = 0;
        return NULL;
    }

    /* 清空缓存到机顶盒信息抓取目录中的所有文件，注意是机顶盒，
    如果路径是U盘要慎重 */
    char removeBuf[128] = {0};
    snprintf(removeBuf, sizeof(removeBuf), "rm %s*", localPath);
    DPrint("system(%s) result:%d\n", removeBuf, system(removeBuf));

    /* 先初始化stop的值 */
    debugTsApkCollectstop = 0;

    //saveSTBMessage();

    /* 启动抓log线程 */
    pthread_t threadLogTid;
    if (pthread_create(&threadLogTid, NULL, runCollectLOG, (void*)localPath) < 0)
    {
        DPrint("run thread to handle runCollectLOG failed\n");
    }
    else
    {
        pthread_detach(threadLogTid);
    }

    /* 启动arp, route, top等数据抓取线程 */
    pthread_t threadDataTid;
    if (pthread_create(&threadDataTid, NULL, runShellCommandNew, (void*)localPath) < 0)
    {
        DPrint("run thread to handle runShellCommandNew failed\n");
    }
    else
    {
        pthread_detach(threadDataTid);
    }

    /* 启动抓包线程 */
    char pcapFilePath[128] = {0};
    snprintf(pcapFilePath, sizeof(pcapFilePath), "%spcap.pcap", localPath);
    pthread_t threadTid;
    if (pthread_create(&threadTid, NULL, runPacketCaptureNew, (void*)pcapFilePath) < 0)
    {
        DPrint("run thread to handle runPacketCaptureNew failed\n");
    }
    else
    {
        pthread_detach(threadTid);
    }

    /* gInfoTime 为0表示没有设置信息抓取的时间，需要手动停止 */
    if (0 == gInfoTime)
    {
        while (0 == debugTsApkCollectstop)
        {
            DPrint("wait for stop!\n");
            sleep(1);
        }
    }
    /* gInfoTime 有值说明设置了预期的抓取时间 */
    else
    {
        DPrint("timeCount:%d\n", timeCount);

        /* 0 == debugTsApkCollectstop 加上这个条件是为了防止设置了数据收集时间的情况
        下又出现手动停止的状况*/
        while (timeCount-- > 0 && 0 == debugTsApkCollectstop)
        {
            DPrint("the stop time will be coming after %d seconds\n", gInfoTime);
            sleep(1);

            pthread_mutex_lock(&gMutexLock);
            if (gInfoTime >= 1)
            {
                gInfoTime -= 1;
            }
            pthread_mutex_unlock(&gMutexLock);
        }

        pthread_mutex_lock(&gMutexLock);
        gInfoTime = 0;
        pthread_mutex_unlock(&gMutexLock);

        debugTsApkCollectstop = 1;
    }

    /* 抓取完成以后从机顶盒中将数据拷贝到U盘中 */
    while (1)
    {
        /* 判断是否发送挂载路径过来 */
        if (0 == strlen(gMountPath))
        {
            DPrint("wait for u mount path!\n");
            sleep(1);
            continue;
        }

        /* 判断U盘路径是否可以读写 */
        if (0 == access(gMountPath, R_OK|W_OK))
        {
            DPrint("uMountPath is ready now !\n");
            char uPath[512] = {0};
            char cmd[512] = {0};

            /*获取TIME*/
            SyGetCurrentTimeByType(1, cTime);
            DPrint("get local time:%s\n", cTime);

            if (gMountPath[strlen(gMountPath) - 1] == '/')
            {
                snprintf(uPath, sizeof(uPath), "%sTsFile%s", gMountPath, cTime);
            }
            else
            {
                snprintf(uPath, sizeof(uPath), "%s/TsFile%s", gMountPath, cTime);
            }

            /* 创建目录 */
            DPrint("mkdir(%s) result:%d\n", uPath, mkdir(uPath, 0777));

            /* 判断U盘路径是否可以读写 */
            if (0 == access(uPath, R_OK|W_OK))
            {
                snprintf(cmd, sizeof(cmd), "cp %s* %s", localPath, uPath);
                DPrint("system(%s) result:%d\n", cmd, system(cmd));
            }
            else
            {
                DPrint("%s cannot read or write!\n", uPath);
            }
        }
        else
        {
            DPrint("%s cannot read or write!\n", gMountPath);
        }
        break;
    }

    gSyTsAPKStartupThreadId = 0;
    DPrint("<----\n");
    return NULL;
}

int SyIpPingTest(syIpPingResult* pIpPingResult, char* Host, unsigned int NumberOfRepetitions,
                 unsigned int Timeout,unsigned int DataBlockSize,unsigned int DSCP)
{
    int i = 0;
    int l = 0;
    int n = 0;
    unsigned int a[20];
    char ost[256];
    char buf[1024];
    char buf1[1024];
    char buf2[20480];
    char pingbuf[1024];
    char pingcmd[1024];
    char *str = buf;
    char *s =(char *)malloc(strlen(buf));
    FILE *ping_fp;
    syIpPingParam pings;

    DO;
    memset(ost, 0x00, sizeof(ost));
    memset(buf, 0x00, sizeof(buf));
    memset(buf1, 0x00, sizeof(buf1));
    memset(buf2, 0x00, sizeof(buf2));
    memset(pingbuf, 0x00, sizeof(pingbuf));

    strcpy(pings.Host, Host);
    pings.NumberOfRepetitions = NumberOfRepetitions;
    pings.Timeout = Timeout;
    pings.DataBlockSize = DataBlockSize;
    pings.DSCP = DSCP;

#ifdef SY_GUANGDONG_WEISHENG
    sprintf(pingcmd, "ping -c %d -i %s -s %d %s",
            pings.NumberOfRepetitions, "0.3" , pings.DataBlockSize,
            pings.Host); //wxb modify 160219 for ping -w
    /*
        sprintf(pingcmd, "ping -c %d -w %d -s %d %s",
            pings.NumberOfRepetitions, pings.Timeout, pings.DataBlockSize,
            pings.Host);
    */
#else
    sprintf(pingcmd, "ping -c %d -w %d -s %d -Q %d %s",
            pings.NumberOfRepetitions, pings.Timeout, pings.DataBlockSize,
            pings.DSCP, pings.Host);
#endif
	//sprintf(pingcmd, "ping -c 5 -i 0.3 -s 32 119.75.217.109");
    DPrint("pingcmd:%s", pingcmd);

    if ((ping_fp = popen(pingcmd, "r")) != NULL)
    {
		DPrint("SyIpPingTest ping sleep\n");
        while (fgets(pingbuf, 1024, ping_fp)!=NULL)
        {
            DPrint("ping result is:%s", pingbuf);
            if(strstr(pingbuf, "packets"))
                strcat(buf, pingbuf);
            else if(strstr(pingbuf, "min/avg/max"))
                strcat(buf, pingbuf);
            else
                strcat(buf2, pingbuf);
        }
    }
    else
    {
        DPrint("ping popen error!\n");
        return SY_FAILED;
    }

    for(n = 0; n < strlen(buf2); n++)
        buf1[n] = tolower(buf2[n]);

    while (*str)
    {
        if((*str >= '0' && *str <= '9') || *str == '.')
        {
            *s = *str;
            ++s, ++l;
        }
        else if(l > 0)
        {
            *s = '\0';
            s -= l;
            a[i++] = atoi(s);
            l = 0;
            continue;
        }
        ++str;
    }

#ifdef SY_GUANGDONG_WEISHENG
    if (0 == a[1])
    {
        a[3] = 0;
        a[4] = 0;
        a[5] = 0;
    }
#else
    if (0 == a[1])
    {
        a[4] = 0;
        a[5] = 0;
        a[6] = 0;
    }
#endif

    pIpPingResult->SuccessCount = a[1];
    pIpPingResult->FailureCount = (a[0] - a[1]);
    if((pIpPingResult->SuccessCount > NumberOfRepetitions) ||
            (pIpPingResult->SuccessCount <= 0))
    {
        EPrint("!!! ERROR !!!, immediate exit.");
        // TODO:这里是退出还是返回呢 需要斟酌下
        //exit(-1);
		//return -1;
	}

    pIpPingResult->AverageResponseTime = a[4];
    pIpPingResult->MinimumResponseTime = a[5];
    pIpPingResult->MaximumResponseTime = a[6];


    DPrint("a0 1 2 3 4 5 6 :%d %d %d %d %d %d %d", a[0],a[1],a[2],a[3],a[4],a[5],a[6]);
    if(strstr(buf1, "no answer") && (pIpPingResult->SuccessCount == 0))
        strcpy(pIpPingResult->DiagnosticsState, "None");
    else if((strstr(buf1, "no answer")&&(pIpPingResult->SuccessCount > 0))||strstr(buf1, "request timed out"))
        strcpy(pIpPingResult->DiagnosticsState, "Requested");
    else if((strlen(buf1) == 0) || strstr(buf1, "bad ip address"))
        strcpy(pIpPingResult->DiagnosticsState, "Error_CannotResolveHostName");
    else if(strstr(buf1, "destination host unreachable"))
        strcpy(pIpPingResult->DiagnosticsState, "Error_Internal");
    else if(pIpPingResult->SuccessCount == NumberOfRepetitions)
        strcpy(pIpPingResult->DiagnosticsState, "Complete");
    else if((0 < pIpPingResult->SuccessCount) && (pIpPingResult->SuccessCount < NumberOfRepetitions))
        strcpy(pIpPingResult->DiagnosticsState, "Part Complete");
    else
        strcpy(pIpPingResult->DiagnosticsState, "Error_Others");

    DPrint("ping test result:\n"
           "DiagnosticsState:%s\n"
           "SuccessCount:%d\n"
           "FailureCount:%d\n"
           "AverageResponseTime:%d\n"
           "MinimumResponseTime:%d\n"
           "MaximumResponseTime:%d\n",
           pIpPingResult->DiagnosticsState,
           pIpPingResult->SuccessCount,
           pIpPingResult->FailureCount,
           pIpPingResult->AverageResponseTime,
           pIpPingResult->MinimumResponseTime,
           pIpPingResult->MaximumResponseTime);
    free(s);
    pclose(ping_fp);

    DONE;

    return SY_SUCCESS;
}


int SyTraceRouteTest(syTraceRouteResult* pTraceRouteResult, char *Host, unsigned int Timeout, unsigned int DataBlockSize,
                     unsigned int MaxHopCount,unsigned int DSCP)
{
    int i = 0;
    int num = 0;
    unsigned int ResponseTime;
    unsigned int NumberOfRouteHops;
    int fd = -1;
    char tmpBuf[1024];
    char msgBuf[8096];
    char HopHost[1024];
    char traceroutebuf[1024];
    char traceroutecmd[1024];
    char *pos1;
    char *pos2;
    FILE *traceroute_fp;
    syTraceRouteParam traceroutes;

    DO;
    memset(tmpBuf, 0x00, sizeof(tmpBuf));
    memset(msgBuf, 0x00, sizeof(msgBuf));
    memset(traceroutebuf, 0x00, sizeof(traceroutebuf));
    memset(traceroutecmd, 0x00, sizeof(traceroutecmd));

    memset(&traceroutes, 0x00, sizeof(traceroutes));
    strcpy(traceroutes.Host,Host);
    traceroutes.Timeout = (0==(Timeout/1000) ? 5 : (Timeout/1000));
    traceroutes.DataBlockSize = DataBlockSize;
    traceroutes.MaxHopCount = MaxHopCount;
    traceroutes.DSCP = DSCP;

	
    if(0 == access("/data/data/com.android.smart.terminal.iptv/bin/sy_tracert", F_OK))
    {
        sprintf(traceroutecmd, "/data/data/com.android.smart.terminal.iptv/bin/sy_tracert -I ICMP -w %d -m %d -t %d %s %d",
                traceroutes.Timeout,
                traceroutes.MaxHopCount,
                traceroutes.DSCP,
                traceroutes.Host,
                traceroutes.DataBlockSize);

    }
    else
    {
        sprintf(traceroutecmd, "sy_tracert -I ICMP -w %d -m %d -t %d %s %d",
                traceroutes.Timeout, traceroutes.MaxHopCount, traceroutes.DSCP,
                traceroutes.Host, traceroutes.DataBlockSize);
    }
	DPrint("traceroutecmd:%s", traceroutecmd);

	

    if(MaxHopCount <= SY_MAX_HOP_COUNT)
    {
		SendToIptvSer(Host, NULL, SY_IPTVSER_CMD_TRACERT);
		if((traceroute_fp= fopen("/tmp/tracetTmp", "r")) != NULL)
        {
			fseek(traceroute_fp,0L,SEEK_END);
			int tracesize=ftell(traceroute_fp);			
			fseek(traceroute_fp,0L,SEEK_SET);
            while(fgets(traceroutebuf, 1024, traceroute_fp))
            {
                DPrint("traceroutebuf:%s, len=%d\n", traceroutebuf, strlen(traceroutebuf));
                //strcat(buf, traceroutebuf);
                if(strstr(traceroutebuf,"  * * *") || strstr(traceroutebuf,"  !")
                        || strstr(traceroutebuf," !H") || strstr(traceroutebuf," !N")
                        || strstr(traceroutebuf," !P") || strstr(traceroutebuf," !S")
                        || strstr(traceroutebuf," !F")||strlen(traceroutebuf) <= 0)
                {
                    strcat(msgBuf, traceroutebuf);
                }
                else
                {
                    strcat(pTraceRouteResult->HopHost[num], traceroutebuf);
                    DPrint("HopHost:%s\n", pTraceRouteResult->HopHost[num]);
                    num++;
                }
            }
			//DPrint("traceroute over, stracert num=%d\n", num);
        }
		else
		{
			DPrint("cannot run traceroute\n");
		}
    }
    else
    {
        DPrint("traceroute popen error!\n");
        pTraceRouteResult->ResponseTime = 0;
        pTraceRouteResult->NumberOfRouteHops = 0;
        strcpy(pTraceRouteResult->DiagnosticsState, "Error_MaxHopCountExceeded");
        return SY_FAILED;
    }

    if (0 != num)
    {
        DPrint("HopHost:%s", pTraceRouteResult->HopHost[num - 1]);
        pos1 = strstr(pTraceRouteResult->HopHost[num - 1], ")  ");
        pos2 = strstr(pTraceRouteResult->HopHost[num - 1], " ms");
        DPrint("pos1:%p, pos2:%p", pos1, pos1);
        if ((NULL != pos1) && (NULL != pos2))
        {
            memcpy(tmpBuf, pos1 + strlen(")  "), pos2 - (pos1 + strlen(")  ")));
            DPrint("tmpBuf:%s", tmpBuf);
            ResponseTime = atoi(tmpBuf);
        }
        else
        {
            ResponseTime = 0;
        }
    }
    else
    {
        ResponseTime = 0;
    }

    NumberOfRouteHops = num;
    pTraceRouteResult->ResponseTime = ResponseTime;
    pTraceRouteResult->NumberOfRouteHops = NumberOfRouteHops;

    DPrint("msgBuf:%s", msgBuf);
    if(strstr(msgBuf, "  * * *") && (pTraceRouteResult->NumberOfRouteHops == 0))
        strcpy(pTraceRouteResult->DiagnosticsState, "None");
    //else if(strstr(msgBuf, "  * * *") || strstr(msgBuf, "  !") || strstr(msgBuf," !H")
    //        || strstr(msgBuf, "  !P") || strstr(msgBuf, " !F") && (pTraceRouteResult->NumberOfRouteHops != 0))
    //	strcpy(pTraceRouteResult->DiagnosticsState, "Requested");
    else if((strlen(msgBuf) == 0) || (strstr(msgBuf," !S")) || (strstr(msgBuf," !N")))
        strcpy(pTraceRouteResult->DiagnosticsState, "Error_CannotResolveHostName");
    else if((pTraceRouteResult->NumberOfRouteHops) > MaxHopCount)
        strcpy(pTraceRouteResult->DiagnosticsState, "Error_MaxHopCountExceeded");
    else
        strcpy(pTraceRouteResult->DiagnosticsState, "Complete");

    DPrint("traceroute test result:\nDiagnosticsState:%s\nResponseTime:%d\nNumberOfRouteHops:%d\nHopHost:%p\n",
           pTraceRouteResult->DiagnosticsState,
           pTraceRouteResult->ResponseTime,
           pTraceRouteResult->NumberOfRouteHops,
           pTraceRouteResult->HopHost);

    if (NumberOfRouteHops < SY_MAX_HOP_COUNT)
    {
        DPrint("HopHost:\n");
        for(i = 0; i < NumberOfRouteHops; i++)
        {
            DPrint("%d) %s", i, pTraceRouteResult->HopHost[i]);
            sscanf(pTraceRouteResult->HopHost[i], "%*[^(](%[^)])", msgBuf);
            sprintf(tmpBuf, "Device.LAN.TraceRouteDiagnostics.RouteHops.%d.HopHost", i + 1);
            DPrint("path:%s, value:%s\n", tmpBuf, msgBuf);
            SySetNodeValue(tmpBuf, msgBuf);
        }
    }
    else
    {
        for(i = 0; i < SY_MAX_HOP_COUNT; i++)
        {
            DPrint("%d) %s", i, pTraceRouteResult->HopHost[i]);
            memset(msgBuf, 0x00, sizeof(msgBuf));
            sscanf(pTraceRouteResult->HopHost[i], "%*[^(](%[^)])", msgBuf);
            sprintf(tmpBuf, "Device.LAN.TraceRouteDiagnostics.RouteHops.%d.HopHost", i + 1);
            DPrint("path:%s, value:%s\n", tmpBuf, msgBuf);
            SySetNodeValue(tmpBuf, msgBuf);
        }
    }

    pclose(traceroute_fp);

    DONE;
    return SY_SUCCESS;
}

int SyBandwidthTest(syBandwidthTestResult* pBandwidthTestResult)
{
    char DownloadURL[512] = {0};
    char DownloadURLtmp[512] = {0};
    char temp[64] = {0};
    DO;
    SyGetNodeValue("Device.X_00E0FC.BandwidthDiagnostics.DownloadURL", DownloadURLtmp) ;

    memcpy(DownloadURL,"ftp://",6);
    SyGetNodeValue("Device.X_00E0FC.BandwidthDiagnostics.Username", temp) ;
    strcat(DownloadURL, temp);
    strcat(DownloadURL, ":");
    memset(temp, 0, sizeof(temp));
    SyGetNodeValue("Device.X_00E0FC.BandwidthDiagnostics.Password", temp) ;
    strcat(DownloadURL, temp);
    strcat(DownloadURL, "@");
    strcat(DownloadURL, DownloadURLtmp+6);

	//memset(DownloadURL, 0, 512);
	//strcpy(DownloadURL, "http://qunying.jb51.net:81/201303/tools/PCHunter_jb51.net.rar");
    DPrint("DownloadURL = %s\n",DownloadURL);

    SyCheckBandwidth(pBandwidthTestResult, NULL, DownloadURL);
    //for test
    //  SyCheckBandwidth(pBandwidthTestResult, NULL, "ftp://admin:admin@192.168.3.180/CuteFTP9.zip");
    //SyCheckBandwidth(pBandwidthTestResult, NULL, "ftp://stbspeed:D1y4Hl66@121.8.251.132/Serv-U");

    DONE;
    return SY_SUCCESS;
}

void* syPacketCaptureCountThread(void* data)
{
    long tmpTime = 0L;
    long continueTime = 0L;

    tmpTime = time(NULL);
    continueTime = tmpTime - gSyStartCaptureTime;

    while (gSyCaptureDuration - continueTime >= 0)
    {
        tmpTime = time(NULL);
        continueTime = tmpTime - gSyStartCaptureTime;

        DPrint("continueTime:%ld, gSyCaptureDuration:%d\n", continueTime, gSyCaptureDuration);

        usleep(500 * 1000);
        if (1 == gSyBreakLoop)
            break;
    }
#if 1
   	//system("pkill -f sy_capture");
    syKillExistProcess("tcpdump");
#else
    pcap_breakloop(gSydevice);
#endif

    DONE;
    return NULL;
}


void SyCapCallBack(u_char * userarg, const struct pcap_pkthdr * pkthdr, const u_char * packet)
{
    //gnCount++;
    //printf("%d:%d\n", gnCount, pkthdr->len);

    if (pcap_dump_ftell(gSyDump) > SY_PACKET_CAPTURE_SIZE)
    {
        DPrint("cap loop break...\n");
        pcap_breakloop(gSydevice);
        gSyBreakLoop = 1;
        return;
    }
    pcap_dump((u_char *)gSyDump, pkthdr, packet);
    pcap_dump_flush(gSyDump);
}

typedef struct _CMD2{
	unsigned int ip;
	unsigned short port;
	int cmd;
	int len;
	char szCmd[240];//256-14-2
}CMD2, *PCMD2;


int SyPacketCapture(syPacketCaptureResult* pPacketCaptureResult)
{
    int i = 0;
    int tmp = 0;
    u_int netmask;
    bpf_u_int32 net;
    char* szIfname = NULL;
    char cIP[32] = {0};
    char cPort[16] = {0};
    char cDuration[16] = {0};
    char packetFilter[128] = {0};
    char tmpBuf[128] = {0};
    char mac[32] = {0};
    char date[64] = {0};
    char errBuf[PCAP_ERRBUF_SIZE] = {0};
    struct bpf_program fcode;	// used in pcap_compile()
    pthread_t tid;

    DO;
    memset(packetFilter, 0x00, sizeof(packetFilter));
    SyGetNodeValue("Device.X_00E0FC.PacketCapture.IP", cIP) ;
    SyGetNodeValue("Device.X_00E0FC.PacketCapture.Port", cPort) ;
    SyGetNodeValue("Device.X_00E0FC.PacketCapture.Duration", cDuration) ;
#if 0
    if (0 != strlen(cIP))
    {
        if (SY_FALSE == SyIsCorrectIp(cIP))
        {
            DPrint("IP Format Error...\n");
            return SY_FAILED;
        }
    }

    if (0 != strlen(cPort))
    {
        if ((0 == atoi(cPort)) || (atoi(cPort) > 65536))
        {
            memset(cPort, 0x00, sizeof(cPort));
        }
    }
#endif
    tmp = atoi(cDuration);
    if (0 != tmp)
    {
        gSyCaptureDuration = tmp;
    }
    else
    {
        gSyCaptureDuration = 60;
    }
    gSyStartCaptureTime = time(NULL);
    DPrint("gSyStartCaptureTime:%ld, gSyCaptureDuration:%d\n",
           gSyStartCaptureTime, gSyCaptureDuration);
//sprintf(gSyLANStu.MACAddress,"",mac);
    char tmp_mac[20]= {0},MACAddress[20]= {0};
    extern syLANStruct gSyLANStu;
    strcpy(MACAddress,gSyLANStu.MACAddress);
    DPrint("MACAddress:%s\n",MACAddress);
    int n = 6, wi = 0;
    while(n--)
    {
        sscanf(MACAddress + wi,"%[^:]",tmp_mac);
        wi += 3;
        /*mac和tmp_mac都是数组不存在mac==null,就算mac里面没有值用strcat也能连接.*/
        strncat(mac, tmp_mac, strlen(tmp_mac));
    }
    DPrint("mac no : is %s\n",mac);
    //SyGetMAC(1, mac);


    SyGetCurrentTimeByType(1, date);
    for (i = 0; i < (int)strlen(mac); i++)
    {
        if ((mac[i] >= 'A') && (mac[i] <= 'Z'))
        {
            tmpBuf[i] = mac[i] + ('a' - 'A');
        }
        else
        {
            tmpBuf[i] = mac[i];
        }
    }
    sprintf(gSypcapFile, "/cache/%s_%s.pcap", tmpBuf, date);
    sprintf(gSyPcapSftpFile, "/%s_%s.pcap", tmpBuf, date);
    DPrint("pcapFile:%s,sftppath:%s\n", gSypcapFile,gSyPcapSftpFile);

    DPrint("portLen:%d, IpLen:%d\n", strlen(cPort), strlen(cIP));
    //"dst port %s and dst host %s"
    if ((0 != strlen(cPort)) && (0 != strlen(cIP)))
    {
        sprintf(packetFilter, "dst port %s and host %s", cPort, cIP);
    }
    else if ((0 != strlen(cPort)) && (0 == strlen(cIP)))
    {
        sprintf(packetFilter, "dst port %s", cPort);
    }
    else if ((0 == strlen(cPort)) && (0 != strlen(cIP)))
    {
        sprintf(packetFilter, "dst host %s", cIP);
    }
    DPrint("packetFilter:%s\n", packetFilter);

#if 1
    gSyBreakLoop = 0;
    pthread_create(&tid, NULL, syPacketCaptureCountThread, NULL);
    FILE *cmd_fp;
    char cmdBuf[256] = {0};
    char msgbug[1024] = {0};

    /*if (0 == strlen(packetFilter))
    {
        sprintf(cmdBuf, "/data/data/com.android.smart.terminal.iptv/bin/tcpdump -s 0 -w %s", gSypcapFile);//sy_capture
    }
    else
    {
        sprintf(cmdBuf, "/data/data/com.android.smart.terminal.iptv/bin/tcpdump -s 0 -w %s %s", gSypcapFile, packetFilter);
    }*/

    DPrint("system(%s)\n.", cmdBuf);
	sprintf(gSypcapFile, "/tmp/test.pcap");
	
	memset(cmdBuf, 0, sizeof(cmdBuf));
	sprintf(cmdBuf, "Compile:net(%s) tcp dst port 50,time:%s,bytes:10000000,file:/tmp/test.pcap", cIP, cDuration);

	CMD2 capCmd;
	capCmd.len = strlen(cmdBuf);
	//capCmd.ip = 1000;
	//capCmd.port = 0;
    strncpy(capCmd.szCmd, cmdBuf, strlen(cmdBuf));
	SendToIptvSer(&capCmd, NULL, SY_IPTVSER_CMD_LOGCAT);
#else
    szIfname = pcap_lookupdev(errBuf);
    if (szIfname)
    {
        DPrint("success: device: %s\n", szIfname);
    }
    else
    {
        DPrint("error: %s\n", errBuf);
        return SY_FAILED;
    }

    netmask = 0xffffff; /* 255.255.255.0 */
    if(pcap_lookupnet(szIfname, &net, &netmask, errBuf) == -1)
    {
        DPrint("Can't get netmask for device %s\n", szIfname);
        netmask = 0;
    }
    DPrint("netmask:0x%x\n", netmask);

    gSydevice = pcap_open_live(szIfname, 65535, 1, 0, errBuf);
    if (NULL == gSydevice)
    {
        DPrint("error:%s\n", errBuf);
        return SY_FAILED;
    }

    if (0 != strlen(packetFilter))
    {
        /* compile the filter */
        if (pcap_compile(gSydevice, &fcode, packetFilter, 1, netmask) < 0)
        {
            DPrint("Error1\n");
            pcap_close(gSydevice);
            return SY_FAILED;
        }

        /* set the filter */
        if (pcap_setfilter(gSydevice, &fcode) < 0)
        {
            DPrint("Error2\n");
            pcap_close(gSydevice);
            return SY_FAILED;
        }
    }

    gSyDump = pcap_dump_open(gSydevice, gSypcapFile);
    gSyBreakLoop = 0;
    pthread_create(&tid, NULL, syPacketCaptureCountThread, NULL);
    pcap_loop(gSydevice, -1, SyCapCallBack, 0);
    pcap_close(gSydevice);
#endif

    DONE;
    return SY_SUCCESS;
}

void* syIpPingTestThread(void* data)
{
    int nDSCP = 0;
    int nTimeOut = 0;
    int nDataBlockSize = 0;
    int nNumberOfRepetitions = 0;
    char Host[256] = {0};
    char DSCP[10] = {0};
    char Timeout[10] = {0};
    char DataBlockSize[10] = {0};
    char NumberOfRepetitions[10] = {0};
    char tmpBuf[10] = {0};
    syIpPingResult IpPingResult;
    DO;
	DPrint("enter syTraceRouteTestThread\n"); 
    memset(&IpPingResult, 0x00, sizeof(IpPingResult));

    SyGetNodeValue("Device.LAN.IPPingDiagnostics.Host", Host) ;
    SyGetNodeValue("Device.LAN.IPPingDiagnostics.NumberOfRepetitions", NumberOfRepetitions) ;
    SyGetNodeValue("Device.LAN.IPPingDiagnostics.Timeout", Timeout) ;
    SyGetNodeValue("Device.LAN.IPPingDiagnostics.DataBlockSize", DataBlockSize) ;
    SyGetNodeValue("Device.LAN.IPPingDiagnostics.DSCP", DSCP) ;

    if (0 == strlen(Timeout))
    {
        nTimeOut = 10;
    }
    else
    {
        nTimeOut = atoi(Timeout)/1000;
    }

    if (0 == strlen(NumberOfRepetitions))
    {
        nNumberOfRepetitions = 3;
    }
    else
    {
        nNumberOfRepetitions = atoi(NumberOfRepetitions);
    }

    if (0 == strlen(DataBlockSize))
    {
        nDataBlockSize = 100;
    }
    else
    {
        nDataBlockSize = atoi(DataBlockSize);
    }

    if (0 == strlen(DSCP))
    {
        nDSCP = 0;
    }
    else
    {
        nDSCP = atoi(DSCP);
    }

    SyIpPingTest(&IpPingResult, Host, nNumberOfRepetitions, nTimeOut, nDataBlockSize, nDSCP);

    DPrint("DiagnosticsState:%s, SuccessCount:%d, FailureCount:%d, AverageResponseTime:%d, MinimumResponseTime:%d, MaximumResponseTime:%d\n",
           IpPingResult.DiagnosticsState,
           IpPingResult.SuccessCount,
           IpPingResult.FailureCount,
           IpPingResult.AverageResponseTime,
           IpPingResult.MinimumResponseTime,
           IpPingResult.MaximumResponseTime);


    SySetNodeValue("Device.LAN.IPPingDiagnostics.DiagnosticsState", IpPingResult.DiagnosticsState);
    sprintf(tmpBuf, "%d", IpPingResult.SuccessCount);
    SySetNodeValue("Device.LAN.IPPingDiagnostics.SuccessCount", tmpBuf);
    sprintf(tmpBuf, "%d", IpPingResult.FailureCount);
    SySetNodeValue("Device.LAN.IPPingDiagnostics.FailureCount", tmpBuf);
    sprintf(tmpBuf, "%d", IpPingResult.AverageResponseTime);
    SySetNodeValue("Device.LAN.IPPingDiagnostics.AverageResponseTime", tmpBuf);
    sprintf(tmpBuf, "%d", IpPingResult.MinimumResponseTime);
    SySetNodeValue("Device.LAN.IPPingDiagnostics.MinimumResponseTime", tmpBuf);
    sprintf(tmpBuf, "%d", IpPingResult.MaximumResponseTime);
    SySetNodeValue("Device.LAN.IPPingDiagnostics.MaximumResponseTime", tmpBuf);
#if 0
    FILE *pFile = NULL;
    pFile = fopen(SY_IPPING_INFORM_FLAG, "wb");
    if (NULL != pFile)
    {
        fwrite("1", 1, strlen("1"), pFile);
        fclose(pFile);
    }
#endif
    gSyIpPingTesting = 0;
    gSyIpPingTestTid = 0;
	addEvent(EVENT_PING);
    DONE;
    return NULL;
}

void* syTraceRouteTestThread(void* data)
{
    int nDSCP = 0;
    int nTimeOut = 0;
    int nDataBlockSize = 0;
    int nMaxHopCount = 0;
    char tmpBuf[10] = {0};
    char DSCP[10] = {0};
    char Host[256] = {0};
    char Timeout[10] = {0};
    char MaxHopCount[10] = {0};
    char DataBlockSize[10] = {0};
    syTraceRouteResult TraceRouteResult;
    DO;

    memset(&TraceRouteResult, 0x00, sizeof(TraceRouteResult));

    SyGetNodeValue("Device.LAN.TraceRouteDiagnostics.Host", Host) ;
    SyGetNodeValue("Device.LAN.TraceRouteDiagnostics.Timeout", Timeout) ;
    SyGetNodeValue("Device.LAN.TraceRouteDiagnostics.DataBlockSize", DataBlockSize) ;
    SyGetNodeValue("Device.LAN.TraceRouteDiagnostics.DSCP", DSCP) ;
    SyGetNodeValue("Device.LAN.TraceRouteDiagnostics.MaxHopCount", MaxHopCount) ;

    if (0 == strlen(Timeout))
    {
        nTimeOut = 10;
    }
    else
    {
        nTimeOut = atoi(Timeout);
    }

    if (0 == strlen(MaxHopCount))
    {
        nMaxHopCount = 32;
    }
    else
    {
        nMaxHopCount = atoi(MaxHopCount);
    }

    if (0 == strlen(DataBlockSize))
    {
        nDataBlockSize = 100;
    }
    else
    {
        nDataBlockSize = atoi(DataBlockSize);
    }

    if (0 == strlen(DSCP))
    {
        nDSCP = 0;
    }
    else
    {
        nDSCP = atoi(DSCP);
    }

    SyTraceRouteTest(&TraceRouteResult, Host, nTimeOut, nDataBlockSize, nMaxHopCount, nDSCP);
#ifdef SY_TEST
    DPrint("DiagnosticsState:%s, ResponseTime:%d, NumberOfRouteHops:%d\n",
           TraceRouteResult.DiagnosticsState,
           TraceRouteResult.ResponseTime,
           TraceRouteResult.NumberOfRouteHops);
#endif
    SySetNodeValue("Device.LAN.TraceRouteDiagnostics.DiagnosticsState", TraceRouteResult.DiagnosticsState);
    sprintf(tmpBuf, "%d", TraceRouteResult.ResponseTime);
    SySetNodeValue("Device.LAN.TraceRouteDiagnostics.ResponseTime", tmpBuf);
    sprintf(tmpBuf, "%d", TraceRouteResult.NumberOfRouteHops);
    SySetNodeValue("Device.LAN.TraceRouteDiagnostics.NumberOfRouteHops", tmpBuf);
#if 0
    FILE *pFile = NULL;
    pFile = fopen(SY_TRACEROUTE_INFORM_FLAG, "wb");
    DPrint("pFile:%p\n", pFile);
    if (NULL != pFile)
    {
        fwrite("1", 1, strlen("1"), pFile);
        fclose(pFile);
    }
#endif
    gSyTraceRouteTesting = 0;
    gSyTraceRouteTestTid = 0;
	addEvent(EVENT_TRACEROUTE);
    DONE;
    return NULL;
}

void* syBandwidthTestThread(void* data)
{
    char tmpBuf[32] = {0};
    syBandwidthTestResult BandwidthTestResult;

    DO;
    memset(&BandwidthTestResult, 0x00, sizeof(BandwidthTestResult));

    SyBandwidthTest(&BandwidthTestResult);

    SySetNodeValue("Device.X_00E0FC.BandwidthDiagnostics.DiagnosticsState", BandwidthTestResult.DiagnosticsState);
    SySetNodeValue("Device.X_00E0FC.BandwidthDiagnostics.ErrorCode", BandwidthTestResult.ErrorCode);
    sprintf(tmpBuf, "%d", BandwidthTestResult.AvgSpeed);
    SySetNodeValue("Device.X_00E0FC.BandwidthDiagnostics.AvgSpeed", tmpBuf);
    sprintf(tmpBuf, "%d", BandwidthTestResult.MaxSpeed);
    SySetNodeValue("Device.X_00E0FC.BandwidthDiagnostics.MaxSpeed", tmpBuf);
    sprintf(tmpBuf, "%d", BandwidthTestResult.MinSpeed);
    SySetNodeValue("Device.X_00E0FC.BandwidthDiagnostics.MinSpeed", tmpBuf);
#if 0
    FILE *pFile = NULL;
    pFile = fopen(SY_BANDWIDTH_INFORM_FLAG, "wb");
    DPrint("pFile:%p\n", pFile);
    if (NULL != pFile)
    {
        fwrite("1", 1, strlen("1"), pFile);
        fclose(pFile);
    }
#endif
    gSyBandwidthDiagnosticsTesting = 0;
    gSyBandwidthDiagnosticsTid = 0;
	addEvent(EVENT_BANDWIDTH);
    DONE;
    return NULL;
}

/*	add by andy.zhao 2014-2-26 17:44:13
一旦有错误码则马上生成一个临时文件(如:./ErrorCodeTmpfile)，并写入错误码个数
同时将错误信息写到xml文件相应节点中，时间到了就去检查是否有那个
临时文件存在，存在的话就读文件里面的数，写到另一个文件(该文件可以
引起上报事件的发生)，并且删除临时文件，清空xml中节点信息

*/
void* ErrCodeManageThd(void* data)
{

    int i;
    int fnet = 0;
    int sleepCount = 0;
    FILE * pFile = NULL;
    FILE * tmpFile = NULL;
    FILE * flagFile = NULL;
    char tmpbuf[512] = {0};
    char tmpbuf1[512] = {0};
    char tmpbuf2[512] = {0};
    char errorCodeNum[16] = {0};

    DO;
    DPrint("gSyErrorCodeSwitch = %d, gSyErrorCodeInterval = %d\n",gSyErrorCodeSwitch,gSyErrorCodeInterval);

    do
    {
        sleep(5);
		//DPrint("ErrCodeManageThd after sleep \n");
        sleepCount++;
        if (sleepCount >= gSyErrorCodeInterval/5)
        {
            //DPrint("ErrorCodeInterval time's up and create inform flag\n");
            sleepCount = 0;
            pFile = fopen(SY_ERRORCODETMP_FLAG, "r");
            if ( pFile != NULL )
            {
                DPrint("%s open success\n",SY_ERRORCODETMP_FLAG);
            }
            else {
                // PERROR(SY_ERRORCODETMP_FLAG" fopen:");
                continue;
            }
            tmpFile	= fopen(SY_ERRORCODEENCODETMP_FLAG, "wt+");
            if (!pFile) {
                PERROR(SY_ERRORCODEENCODETMP_FLAG" fopen:");
                gSyErrorCodeManageTid = 0;
                return NULL;
            }

            //定位文件到最后20条消息
            fnet =  fseek(pFile, -116*20, SEEK_END);
            //如果没有20条消息，则从文件开头开始读起
            if ( fnet != 0)
            {
                fseek(pFile, 0, SEEK_SET);
            }

            //将20条消息编号，并保存到另外一个文件
            for (i=2; i<=41; i++)
            {
                memset(tmpbuf, 0 , sizeof(tmpbuf));
                memset(tmpbuf1, 0 , sizeof(tmpbuf1));
                memset(tmpbuf2, 0 , sizeof(tmpbuf2));


                DPrint("%d\n", i);
                fgets(tmpbuf, 512, pFile);
                if ( strcmp(tmpbuf, "") == 0 )
                {
                    DPrint("total %d errorCodes\n", (i-1)/2);
                    break;
                }
                sscanf(tmpbuf, "%26s", tmpbuf1);
                sscanf(tmpbuf, "%*[^}]%s", tmpbuf2);
                memset(tmpbuf, 0x0, sizeof(tmpbuf));
                sprintf(tmpbuf, "%s%d%s\r\n", tmpbuf1, i/2, tmpbuf2+1);
                DPrint("tmpbuf = %s\n", tmpbuf);
                fwrite(tmpbuf, 1, strlen(tmpbuf), tmpFile);
            }

            flagFile = fopen(SY_ERRORCODEMANAGE_FLAG, "wb");
            if ( flagFile != NULL )
            {
                DPrint("flagFile:%p\n", pFile);
                fclose(flagFile);
            }

            fclose(pFile);
            fclose(tmpFile);
            //删除未编码的错误代码文件
            remove(SY_ERRORCODETMP_FLAG);
            DPrint("gSyErrorCodeSwitch = %d, gSyErrorCodeInterval = %d end\n",
                   gSyErrorCodeSwitch, gSyErrorCodeInterval);
        }
    }
    while(gSyErrorCodeSwitch == 1);

    gSyErrorCodeManageTid = 0;
    DONE;
    return NULL;
}


void* syPacketCaptureThread(void* data)
{
    int nRet = SY_SUCCESS;
    char tmpBuf[32] = {0};
    char tmpBuf_1[32] = {0};
    char name[32] = {0};
    char passwd[32] = {0};
    char sftpurl[64] = {0};
    syPacketCaptureResult PacketCaptureResult;

    DO;

    SyGetNodeValue("Device.X_00E0FC.PacketCapture.Username", name);
    SyGetNodeValue("Device.X_00E0FC.PacketCapture.Password", passwd);
    SyGetNodeValue("Device.X_00E0FC.PacketCapture.UploadURL", tmpBuf);
    //memcpy(sftpurl, "125.88.86.54",13);
    strcpy(tmpBuf_1,&tmpBuf[7]);//去掉 sftp://

    if(strstr(tmpBuf_1,":"))//如果后面带有端口号，去掉
        strncpy(sftpurl,tmpBuf_1,(strlen(tmpBuf_1) - strlen(strstr(tmpBuf_1,":"))));
    else
        strcpy(sftpurl,tmpBuf_1);

    //memcpy(sftpurl, "125.88.86.16",13);
    //sftpurl[12] = '\0';

    while(1)
    {
        if (1 == gSyPacketCapturing)
        {
            gSyPacketCapturing = 0;
            memset(&PacketCaptureResult, 0x00, sizeof(PacketCaptureResult));
            DPrint("gSypcapFile:%s\n", gSypcapFile);
            if (NULL != strstr(gSypcapFile, ".pcap"))
            {
                DPrint("rm %s\n", gSypcapFile);
                remove(gSypcapFile);
                memset(gSypcapFile, 0x00, sizeof(gSypcapFile));
                memset(gSyPcapSftpFile, 0x00, sizeof(gSyPcapSftpFile));
            }

			if(access("/tmp/test.pcap", F_OK) == 0)
			{
				remove("/tmp/test.pcap");
			}
				
            nRet = SyPacketCapture(&PacketCaptureResult);

            DPrint("gSyDeleteCaputureFile:%d\n", gSyDeleteCaputureFile);
			gSyDeleteCaputureFile = 0;
            if (0 == gSyDeleteCaputureFile)
            {
                if (SY_FAILED == nRet)
                {
                    SySetNodeValue("Device.X_00E0FC.PacketCapture.State", "4");
                }
                else
                {
                    nRet = sySftpUpload(sftpurl, 37028, 0, name, passwd, gSypcapFile, gSyPcapSftpFile);
                    SySetNodeValue("Device.X_00E0FC.PacketCapture.State", "5");
                    remove(gSypcapFile);
                    memset(gSypcapFile, 0x00, sizeof(gSypcapFile));
                    memset(gSyPcapSftpFile, 0x00, sizeof(gSyPcapSftpFile));
                    if (SY_SUCCESS == nRet)
                    {
                        SySetNodeValue("Device.X_00E0FC.PacketCapture.State", "6");
                    }
                    else
                    {
                        SySetNodeValue("Device.X_00E0FC.PacketCapture.State", "7");
                    }
                }
            }
            else
            {
                gSyDeleteCaputureFile = 0;
            }
        }
        else
        {
            sleep(1);
        }
    }


    gSyPacketCaptureTid = 0;

    DONE;

    return NULL;
}

void* syMsgLogPeriodThread(void* data)
{
    int i = 0;
    int times = 0;
    int sleepTime = 0;
    int sleepTimeArray[] = {2, 4, 6};
    char tmpBuf[1024] = {0};
    char MsgLogBuf[9096] = {0};
    char tempStr[2048] = {0};
    size_t sizeOfBuf = sizeof(tempStr);
    FILE *pFile = NULL;

    DO;

    for (times = 0; times < SY_REPORT_TIMES; times++)
    {
        memset(MsgLogBuf, 0x00, sizeof(MsgLogBuf));
        for (i = 0; i < gSyMsgLogListLen; i++)
        {
            if (GetValue(gSyMsgLogList[i].cmdStr, tempStr, sizeOfBuf))
            {
                sprintf(tmpBuf, "%s=%s\r\n", gSyMsgLogList[i].cmdStr, tempStr);
            }
            else
            {
                sprintf(tmpBuf, "%s=%s\r\n", gSyMsgLogList[i].cmdStr, "0");
            }

            DPrint("tmpBuf:%s\n", tmpBuf);

            memcpy(MsgLogBuf+strlen(MsgLogBuf), tmpBuf, strlen(tmpBuf));
        }

        DPrint("MsgLogBuf:%s\n", MsgLogBuf);

        if (0 != strlen(MsgLogBuf))
        {
            pFile = fopen(SY_LOG_PERIODIC_INFORM_FLAG, "wb");
            if (NULL != pFile)
            {
                fwrite(MsgLogBuf, 1, strlen(MsgLogBuf), pFile);
                fclose(pFile);
            }
        }

        DPrint("sleepTimeArray[%d]:%d\n", times, sleepTimeArray[times]);
        sleepTime = gSyMsgLogDuration/sleepTimeArray[times];
        DPrint("gSyMsgLogDuration:%d, sleepTime:%d\n",
               gSyMsgLogDuration, sleepTime);
        if (times != SY_REPORT_TIMES)
        {
            sleep(sleepTime);
        }
    }

    gSyEnableMsgLog = 0;
    gSyPeriodMsgLogTid = 0;
    gSyMsgLogPeriodRunning = 0;

    DONE;

    return NULL;
}
int SyIsNumber(char *str)
{
    char *ptr = NULL;
    ptr = str;
    while(*ptr)
    {
        if(*ptr < 0x30 || *ptr > 0x39)
            return 1;
        ptr++;
    }
    return 0 ;
}

int SyGetIfnameNetStats(const char* pszEth,
                        SY_INT64* rx_bytes, SY_INT64* rx_packets,
                        SY_INT64* tx_bytes, SY_INT64* tx_packets)
{
    int ver = 1;
    int iRet = 0;
    char buf[1024];
    char ifname[16];
    //DO;

    FILE* fp = fopen(SY_PATH_PROCNET_DEV, "r");
    if (fp == NULL)
    {
        DPrint("open file faild:%s\n", SY_PATH_PROCNET_DEV);
        return SY_FAILED;
    }

    //读出前两行
    fgets(buf, sizeof(buf), fp);
    fgets(buf, sizeof(buf), fp);
    //查看版本
    if (strstr(buf, "compressed") != NULL)
        ver = 3;
    else if(strstr(buf, "bytes") != NULL)
        ver = 2;
#ifdef SY_GD_TR069
    ver = 3;
#endif
    while(fgets(buf, sizeof(buf), fp))
    {
        memset(ifname, 0x00, sizeof(ifname));
        sscanf(buf, "%*[^a-z]%[^:]", ifname);
        if (strcmp(ifname, pszEth) != 0)
            continue;

        DPrint("buf = %s\n", buf);
        if (ver == 3)
        {
            sscanf(buf, "%*[^:]:%llu %llu %*u %*u %*d %*u %*u %*u %llu %llu",
                   rx_bytes, rx_packets, tx_bytes, tx_packets);
            DPrint("ver=3 rx_bytes %lld rx_packets %lld tx_bytes %lld tx_packets %lld\n", *rx_bytes,*rx_packets,*tx_bytes,*tx_packets);
        }
        else if(ver == 2)
        {
            sscanf(buf, "%*[^:]:%llu %llu %*u %*u %*d %*u %llu %llu",
                   rx_bytes, rx_packets, tx_bytes, tx_packets);
            DPrint("ver=2 rx_bytes %lld rx_packets %lld tx_bytes %lld tx_packets %lld\n", *rx_bytes,*rx_packets,*tx_bytes,*tx_packets);
        }
        else
        {
            sscanf(buf, "%*[^:]:%llu %*u %*u %*u %*d %llu", rx_packets, tx_packets);
        }
        iRet = SY_SUCCESS;
    }

    fclose(fp);
    return iRet;
}

#ifdef SY_HAVE_LIBCURL
size_t getcontentlengthfunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
    int r;
    long len = 0;

    r = sscanf((const char*)ptr, "Content-Length: %ld\n", &len);
    if (r)
        *((long *) stream) = len;
    return size * nmemb;
}

size_t discardfunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
    return size * nmemb;
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
    return fwrite(ptr, size, nmemb, (FILE*)stream);
}

size_t readfunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
    FILE *f = (FILE*)stream;
    size_t n;
    if (ferror(f))
        return CURL_READFUNC_ABORT;
    n = fread(ptr, size, nmemb, f) * size;
    return n;
}

int SyCurlUpload(CURL *curlhandle, const char* userPasswd,
                 const char * remotepath, const char * localpath, long timeout, long tries)
{
    int c;
    FILE *f;
    long uploaded_len = 0;
    CURLcode r = CURLE_GOT_NOTHING;

    f = fopen(localpath, "rb");
    if (f == NULL)
    {
        DPrint("Open File Fail\n");
        return SY_FAILED;
    }
    curl_easy_setopt(curlhandle, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curlhandle, CURLOPT_URL, remotepath);
    curl_easy_setopt(curlhandle, CURLOPT_USERPWD, userPasswd);
    if (timeout)
        curl_easy_setopt(curlhandle, CURLOPT_FTP_RESPONSE_TIMEOUT, timeout);
    curl_easy_setopt(curlhandle, CURLOPT_HEADERFUNCTION, getcontentlengthfunc);
    curl_easy_setopt(curlhandle, CURLOPT_HEADERDATA, &uploaded_len);
    curl_easy_setopt(curlhandle, CURLOPT_WRITEFUNCTION, discardfunc);
    curl_easy_setopt(curlhandle, CURLOPT_READFUNCTION, readfunc);
    curl_easy_setopt(curlhandle, CURLOPT_READDATA, f);
    curl_easy_setopt(curlhandle, CURLOPT_FTPPORT, "-");
    curl_easy_setopt(curlhandle, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);
    curl_easy_setopt(curlhandle, CURLOPT_VERBOSE, 1L);

    for (c = 0; (r != CURLE_OK) && (c < tries); c++)
    {
        if (c)
        {
            curl_easy_setopt(curlhandle, CURLOPT_NOBODY, 1L);
            curl_easy_setopt(curlhandle, CURLOPT_HEADER, 1L);
            r = curl_easy_perform(curlhandle);
            if (r != CURLE_OK)
                continue;
            curl_easy_setopt(curlhandle, CURLOPT_NOBODY, 0L);
            curl_easy_setopt(curlhandle, CURLOPT_HEADER, 0L);
            fseek(f, uploaded_len, SEEK_SET);
            curl_easy_setopt(curlhandle, CURLOPT_APPEND, 1L);
        }
        else
        {
            curl_easy_setopt(curlhandle, CURLOPT_APPEND, 0L);
        }
        r = curl_easy_perform(curlhandle);
    }
    fclose(f);
    if (r == CURLE_OK)
    {
        return SY_SUCCESS;
    }
    else
    {
        DPrint("%s\n", curl_easy_strerror(r));
        return SY_FAILED;
    }
}

int SyCurlDownload(CURL *curlhandle, const char* userPasswd,
                   const char * remotepath, const char * localpath, long timeout, long tries)
{
    FILE *f;
    long filesize =0 ;
    int use_resume = 0;
    curl_off_t local_file_len = -1;
    CURLcode r = CURLE_GOT_NOTHING;
    struct stat file_info;
    if(stat(localpath, &file_info) == 0)
    {
        local_file_len = file_info.st_size;
        use_resume = 1;
    }
    f = fopen(localpath, "ab+");
    if (NULL == f)
    {
        DPrint("Open File Fail\n");
        return SY_FAILED;
    }

    curl_easy_setopt(curlhandle, CURLOPT_URL, remotepath);
    curl_easy_setopt(curlhandle, CURLOPT_USERPWD, userPasswd);
    curl_easy_setopt(curlhandle, CURLOPT_CONNECTTIMEOUT, timeout);
    curl_easy_setopt(curlhandle, CURLOPT_HEADERFUNCTION, getcontentlengthfunc);
    curl_easy_setopt(curlhandle, CURLOPT_HEADERDATA, &filesize);
    //curl_easy_setopt(curlhandle, CURLOPT_RESUME_FROM_LARGE, use_resume?local_file_len:0);
    curl_easy_setopt(curlhandle, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curlhandle, CURLOPT_WRITEDATA, f);
    curl_easy_setopt(curlhandle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curlhandle, CURLOPT_VERBOSE, 1L);
    r = curl_easy_perform(curlhandle);
    fclose(f);
    if (r == CURLE_OK)
    {
        return SY_SUCCESS;
    }
    else
    {
        DPrint("%s\n", curl_easy_strerror(r));
        return SY_FAILED;
    }
}

int syUpload(int type, const char* userPasswd, const char * remotepath,
             const char * localpath, long timeout, long tries)
{
    int nRet = SY_SUCCESS;
    char remoteURL[512] = {0};
    CURL *curlftpup;
    CURL *curlhttpup;

    curl_global_init(CURL_GLOBAL_ALL);
    DPrint("type:%d\n", type);
    switch(type)
    {
    case SY_TYPE_FTP_UPLOAD:
        curlftpup = curl_easy_init();
        sprintf(remoteURL, "ftp://%s", remotepath);
        DPrint("remoteURL:%s\n", remoteURL);
        nRet = SyCurlUpload(curlftpup, userPasswd, remoteURL,
                            localpath, timeout, tries);
        curl_easy_cleanup(curlftpup);
        break;
    case SY_TYPE_HTTP_UPLOAD:
        curlhttpup = curl_easy_init();
        sprintf(remoteURL, "http://%s", remotepath);
        DPrint("remoteURL:%s\n", remoteURL);
        nRet = SyCurlUpload(curlhttpup, userPasswd, remoteURL,
                            localpath, timeout, tries);
        curl_easy_cleanup(curlhttpup);
        break;
    default:
        nRet = SY_FAILED;
        break;
    }
    curl_global_cleanup();
    return nRet;
}

int syDownload(int type, const char* userPasswd, const char* remotepath,
               const char* localpath, long timeout, long tries)
{
    int nRet = SY_SUCCESS;
    char remoteURL[512] = {0};
    CURL *curlftpdown;
    CURL *curlhttpdown;

    curl_global_init(CURL_GLOBAL_ALL);

    DPrint("type:%d\n", type);
    switch(type)
    {
    case SY_TYPE_FTP_DOWNLOAD:
        curlftpdown = curl_easy_init();
        sprintf(remoteURL, "ftp://%s", remotepath);
        DPrint("remoteURL:%s\n", remoteURL);
        nRet = SyCurlDownload(curlftpdown, userPasswd, remoteURL,
                              localpath, timeout, tries);
        curl_easy_cleanup(curlftpdown);
        break;
    case SY_TYPE_HTTP_DOWNLOAD:
        curlhttpdown = curl_easy_init();
        sprintf(remoteURL, "http://%s", remotepath);
        DPrint("remoteURL:%s\n", remoteURL);
        nRet = SyCurlDownload(curlhttpdown, userPasswd, remoteURL,
                              localpath, timeout, tries);
        curl_easy_cleanup(curlhttpdown);
        break;
    default:
        nRet = SY_FAILED;
        break;
    }
    curl_global_cleanup();
    return nRet;
}

static size_t WriteCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    /* we are not interested in the downloaded bytes itself,
       so we only return the size we would have saved ... */
    (void)ptr;  /* unused */
    (void)data; /* unused */
    return (size_t)(size * nmemb);
}

int SyCheckBandwidth(syBandwidthTestResult* pBandwidthTestResult,
                     const char* userPasswd, const char* url)
{
    int AvgSpeed = 0;
    int MaxSpeed = 0;
    int MinSpeed = 0;
    //double downloadedBytes = 0;
    //double totalDownloadTime = 0;
    CURL *curl_handle;
    CURLcode res;
    DO;
    time_t t = time(NULL);
    DPrint("Localtime: %s", ctime(&t));
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 1200);
    curl_easy_setopt(curl_handle, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_ANY);
    //curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
    res = curl_easy_perform(curl_handle);
    if(CURLE_OK == res)
    {
        double val;
#if 0
        /* check for bytes downloaded */
        res = curl_easy_getinfo(curl_handle, CURLINFO_SIZE_DOWNLOAD, &val);
        if((CURLE_OK == res) && (val>0))
        {
            DPrint("Data downloaded: %0.0f bytes.\n", val);
            downloadedBytes = val;
        }
        /* check for total download time */
        res = curl_easy_getinfo(curl_handle, CURLINFO_TOTAL_TIME, &val);
        if((CURLE_OK == res) && (val>0))
        {
            DPrint("Total download time: %0.3f sec.\n", val);
            totalDownloadTime = val;
        }
#endif
        /* check for average download speed */
        res = curl_easy_getinfo(curl_handle, CURLINFO_SPEED_DOWNLOAD, &val);
        if((CURLE_OK == res) && (val>0))
        {
            DPrint("Average download speed: %0.3f kbyte/sec.\n", val / 1024);
            MinSpeed = (int)(val / 1024);
            AvgSpeed = (int)(val / 1000);
            MaxSpeed = 2 * AvgSpeed - MinSpeed + 1;
        }
    }
    else
    {
        DPrint("Error while fetching '%s' : %s\n",
               url, curl_easy_strerror(res));
    }
    DPrint("AvgSpeed:%d, MinSpeed:%d, MaxSpeed:%d\n",
           AvgSpeed, MinSpeed, MaxSpeed);
    pBandwidthTestResult->AvgSpeed = AvgSpeed;
    pBandwidthTestResult->MinSpeed = MinSpeed;
    pBandwidthTestResult->MaxSpeed = MaxSpeed;
    switch (res)
    {
    case CURLE_OK:
        strcpy(pBandwidthTestResult->ErrorCode, "");
        strcpy(pBandwidthTestResult->DiagnosticsState, "3");
        break;
    case CURLE_OPERATION_TIMEDOUT:
        strcpy(pBandwidthTestResult->ErrorCode, "102001");
        strcpy(pBandwidthTestResult->DiagnosticsState, "4");

        break;
    case CURLE_REMOTE_FILE_NOT_FOUND:
        strcpy(pBandwidthTestResult->ErrorCode, "102050");
        strcpy(pBandwidthTestResult->DiagnosticsState, "4");
        break;
    case CURLE_COULDNT_CONNECT:
        strcpy(pBandwidthTestResult->ErrorCode, "102051");
        strcpy(pBandwidthTestResult->DiagnosticsState, "4");
        break;
    case CURLE_PARTIAL_FILE:
        strcpy(pBandwidthTestResult->ErrorCode, "102052");
        strcpy(pBandwidthTestResult->DiagnosticsState, "4");
        break;
    default:
        strcpy(pBandwidthTestResult->DiagnosticsState, "4");
        break;
    }
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
    DONE;
    return SY_SUCCESS;
}
#endif

#if 1
const char *keyfile1="id_rsa.pub";
const char *keyfile2="id_rsa";

//键盘输入密码模块
static void kbd_callback(const char *name, int name_len,
                         const char *instruction, int instruction_len, int num_prompts,
                         const LIBSSH2_USERAUTH_KBDINT_PROMPT *prompts,
                         LIBSSH2_USERAUTH_KBDINT_RESPONSE *responses,
                         void **abstract)
{
    int i;
    size_t n;
    char buf[1024];
    (void)abstract;

    //执行键盘交互验证
    DPrint("Performing keyboard-interactive authentication.\n");
    DPrint("Authentication name: '%s'\n", name);
    DPrint("Authentication instruction: '%s'\n", instruction);

    //输出相关密码的提示性语言
    DPrint("Number of prompts: %d\n\n", num_prompts);

    for (i = 0; i < num_prompts; i++)
    {
        DPrint("Prompt %d from server: %s'\n", i, prompts[i].text);
        DPrint("Please type response: ");
        //等待你用户输入密码
        fgets(buf, sizeof(buf), stdin);
        n = strlen(buf);
        //将输入的最后一个回车符换为字符串结束符
        while (n > 0 && strchr("\r\n", buf[n - 1]))
            n--;
        buf[n] = 0;

        responses[i].text = strdup(buf);
        responses[i].length = n;

        DPrint("Response %d from user is '%s'\n\n", i, responses[i].text);
    }
    DPrint("Done. Sending keyboard-interactive responses to server now.\n");
}

//非阻塞情况下的等待传输
static int waitsocket(int socket_fd, LIBSSH2_SESSION *session)
{
    struct timeval timeout;
    int rc;
    fd_set fd;
    fd_set *writefd = NULL;
    fd_set *readfd = NULL;
    int dir;


    DO;
    timeout.tv_sec = 2;     //设置等待的秒数
    timeout.tv_usec = 0;

    FD_ZERO(&fd);

    FD_SET(socket_fd, &fd);

    /* now make sure we wait in the correct direction */
    dir = libssh2_session_block_directions(session);

    if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        readfd = &fd;

    if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        writefd = &fd;

    rc = select(socket_fd + 1, readfd, writefd, NULL, &timeout);
    DONE;
    return rc;
}

/*******************************************************************************
函数说明:   初始化一个sftp会话,成功返回
参数说明:       hostaddr_t  :   服务器主机地址
                port_t      :   服务器该设置的主机端口号
                block_t     :   0： 非阻塞； 1：阻塞
                block_times ：  block_t设置为非阻塞(0)时，block_times表示select的次数，block_t设置成阻塞(1)时无意义
                publicfile  :   :   公钥文件
                privatefile :   私钥文件
                username_t  :   登陆服务器的用户名
                password+t  :   登陆服务器对于用户名的密码
返  回   值:    成功返回一个 sftp_sesandssock 的数组指针
                失败返回NULL
*******************************************************************************/
sftp_sesandssock * init_sftpsession(unsigned long hostaddr_t, unsigned int port_t, unsigned int block_t, unsigned int block_times, const char *publicfile, const char *privatefile, const char *username_t, const char *password_t)
{


    int sock, i,auth_pw_t = 0;
    int rc;
    int block_count;
    struct sockaddr_in sin;
    const char *fingerprint;
    char *userauthlist;
    LIBSSH2_SESSION *session;
    FILE *local;
    LIBSSH2_SFTP *sftp_session;
    LIBSSH2_SFTP_HANDLE *sftp_handle;
    sftp_sesandssock *  sftp_sesandssock_t = (sftp_sesandssock *)malloc(sizeof(sftp_sesandssock));

    rc = libssh2_init (0);
    if (rc != 0)
    {
        DPrint ("libssh2 initialization failed (%d)\n",rc);
        return NULL;
    }


    /*
     * The application code is responsible for creating the socket
     * and establishing the connection
     建立连接
     */
    sock = socket(AF_INET, SOCK_STREAM, 0);

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port_t);
    sin.sin_addr.s_addr = hostaddr_t;
    if (connect(sock, (struct sockaddr*)(&sin),
                sizeof(struct sockaddr_in)) != 0)
    {
        DPrint("failed to connect!\n");
        return NULL;
    }

    /* Create a session instance
    创建一个会话实例，成功返回一个LIBSSH2_SESSION实例，失败返回NULL
     */
    session = libssh2_session_init();

    if(!session)
        return NULL;

    /* Since we have set non-blocking, tell libssh2 we are blocking
    设置为阻塞，如果是参数是0的话就可以设置成非阻塞
    */
    if( 0 == block_t)
        libssh2_session_set_blocking(session, 0);
    else
        libssh2_session_set_blocking(session, 1);
    /* ... start it up. This will trade welcome banners, exchange keys,
     * and setup crypto, compression, and MAC layers
     开始传输层谈判和连接主机，返回0成功，非0失败
     */
    if( 1 == block_t )
        rc = libssh2_session_handshake(session, sock);
    else
    {
        while ((rc = libssh2_session_handshake(session, sock)) == LIBSSH2_ERROR_EAGAIN);
    }
    if(rc)
    {
        DPrint("Failure establishing SSH session: %d\n", rc);
        return NULL;
    }

    /* At this point we havn't yet authenticated.  The first thing to do
     * is check the hostkey's fingerprint against our known hosts Your app
     * may have it hard coded, may go to a file, may present it to the
     * user, that's your call
     去获取主机指纹，返回获取到的指纹
     */
    fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);

    DPrint("Fingerprint: ");
    //打印出获得的密钥
    for(i = 0; i < 20; i++)
    {
        DPrint("%02X ",(unsigned char)fingerprint[i]);
    }
    DPrint("\n");

    /* check what authentication methods are available
    检查何种验证方法是可行的，成功返回一个以逗号分隔的验证方案的列表
    */
    if( 1 == block_t )
    {
        userauthlist = libssh2_userauth_list(session, username_t, strlen(username_t));

        DPrint("Authentication methods: %s\n",userauthlist);
        //如果是用户名和密码登陆的情况auth_pw_t |= 1
        if (strstr(userauthlist, "password") != NULL)
        {
            auth_pw_t |= 1;
        }
        if (strstr(userauthlist, "keyboard-interactive") != NULL)
        {
            auth_pw_t |= 2;
        }
        //如果是公共密钥
        if (strstr(userauthlist, "publickey") != NULL)
        {
            auth_pw_t |= 4;
        }

        /* if we got an 4. argument we set this option if supported */


        if (auth_pw_t & 1)
        {
            /* We could authenticate via password_t
            在已知密码下的身份验证
            */
            if (libssh2_userauth_password(session, username_t, password_t))
            {

                DPrint("Authentication by password_t failed.\n");
                libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
                libssh2_session_free(session);
            }

        }
        else if (auth_pw_t & 2)
        {
            /* Or via keyboard-interactive
            在用键盘密码交互下的身份验证
            */
            if (libssh2_userauth_keyboard_interactive(session, username_t, &kbd_callback) )
            {

                DPrint("\tAuthentication by keyboard-interactive failed!\n");
                libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
                libssh2_session_free(session);
            }
            else
            {
                DPrint("\tAuthentication by keyboard-interactive succeeded.\n");
            }

        }
        else if (auth_pw_t & 4)
        {
            /* Or by public key
            密钥对情况下的身份验证
            */
            if ((rc = libssh2_userauth_publickey_fromfile(session, username_t, publicfile, privatefile, password_t)) != 0)
            {

                DPrint("\tAuthentication by public key failed! = %d\n",rc);
                libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
                libssh2_session_free(session);
            }
            else
            {
                DPrint("\tAuthentication by public key succeeded.\n");
            }
        }
        else
        {
            DPrint("No supported authentication methods found!\n");
            libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
            libssh2_session_free(session);
        }
    }
    else
    {
        auth_pw_t = 1;
        if (auth_pw_t)
        {
            /* We could authenticate via password */
            while ((rc = libssh2_userauth_password(session, username_t, password_t))
                    == LIBSSH2_ERROR_EAGAIN);
            if (rc)
            {
                DPrint("Authentication by password failed.\n");
                libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
                libssh2_session_free(session);
            }
        }
        else
        {
            /* Or by public key */
            while ((rc =
                        libssh2_userauth_publickey_fromfile(session, username_t, publicfile, privatefile, password_t)) ==
                    LIBSSH2_ERROR_EAGAIN);
            if (rc)
            {
                DPrint("\tAuthentication by public key failed\n");
                libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
                libssh2_session_free(session);
            }
        }
    }

    DPrint("libssh2_sftp_init()!\n");

    //打开通道，初始化sftp子系统。
    if( 1 == block_t )
    {
        sftp_session = libssh2_sftp_init(session);
        if (!sftp_session)
        {
            DPrint("Unable to init SFTP session\n");
            libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
            libssh2_session_free(session);
        }
    }
    else
    {
        block_count = 0;
        do
        {
            sftp_session = libssh2_sftp_init(session);
            if(!sftp_session)
            {
                if(libssh2_session_last_errno(session) ==
                        LIBSSH2_ERROR_EAGAIN)
                {
                    DPrint(".");
                    waitsocket(sock, session);
                    block_count++;
                    if(block_count >= block_times)
                    {
                        DPrint("\nnon-blocking init faild!!! try increase block_times value\n");
                        return NULL;
                    }
                }
                else
                {
                    DPrint("Unable to init SFTP session\n");
                    libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
                    libssh2_session_free(session);
                }
            }
        }
        while (!sftp_session);
    }
    sftp_sesandssock_t->sock_t = sock;
    sftp_sesandssock_t->sftp_session_t = sftp_session;
    sftp_sesandssock_t->session_t = session;

    return sftp_sesandssock_t;
}


void exit_sftpsession(sftp_sesandssock * sftp_sesandssock_t)
{
    libssh2_sftp_shutdown(sftp_sesandssock_t->sftp_session_t);
    libssh2_exit();
}

/*******************************************************************************
函数说明:   利用sftp协议上传文件
参数说明:       sftp_sesandssock_t  :   自定义的结构体，包含了socket和sftp_session 和session
                block_t             :   0：非阻塞，1：阻塞
                block_times         ：  当设置block_t为非阻塞(0)时，block_times表示重复select的次数
                local_path          :   在本地主机中上传文件的位置
                remote_path         :   在远程服务器中存放文件的位置
返  回   值:    0   :   上传成功
                -1  :   上传失败
*******************************************************************************/
int uploadfile_sftp(sftp_sesandssock *sftp_sesandssock_t, unsigned int block_t, unsigned block_times, const char *local_path, const char *remote_path)
{
    size_t nread;
    int rc;
    FILE *local;
    char mem[SY_BUFFER_SIZE];
    char *ptr = NULL;
    struct timeval timeout;
    fd_set fd;
    time_t start;
    long total = 0;
    int duration;
    int block_count = 0;
    LIBSSH2_SFTP_HANDLE *sftp_handle = NULL;

    //打开本地文件
    local = fopen(local_path, "rb");

    if (!local)
    {
        DPrint("Can't open local file %s\n",local_path);
        libssh2_sftp_close(sftp_handle);
        return SY_FAILED;
    }

    //阻塞情况下
    if( 1 == block_t )
    {
        sftp_handle = libssh2_sftp_open(sftp_sesandssock_t->sftp_session_t, remote_path, LIBSSH2_FXF_WRITE|LIBSSH2_FXF_CREAT|LIBSSH2_FXF_TRUNC,
                                        LIBSSH2_SFTP_S_IRUSR|LIBSSH2_SFTP_S_IWUSR|
                                        LIBSSH2_SFTP_S_IRGRP|LIBSSH2_SFTP_S_IROTH);

        if (!sftp_handle)
        {
            DPrint("Unable to open file with SFTP: %ld\n",
                   libssh2_sftp_last_error(sftp_sesandssock_t->sftp_session_t));
            return SY_FAILED;
        }

        DPrint("libssh2_sftp_open() is done, now uploading data!\n");

        start = time(NULL);

        do
        {
            nread = fread(mem, 1, sizeof(mem), local);
            if (nread <= 0)
            {
                /* end of file */
                break;
            }
            ptr = mem;

            total += nread;

            do
            {
                /* write data in a loop until we block */
                rc = libssh2_sftp_write(sftp_handle, ptr, nread);
                if(rc < 0)
                    break;
                ptr += rc;
                nread -= rc;
            }
            while (nread);

        }
        while (rc > 0);

        duration = (int)(time(NULL)-start);

        DPrint("%ld bytes in %d seconds makes %.1f bytes/sec\n",
               total, duration, total/(double)duration);

    }
    //非阻塞情况下
    else
    {
        block_count = 0;
        do
        {
            sftp_handle = libssh2_sftp_open(sftp_sesandssock_t->sftp_session_t, remote_path,
                                            LIBSSH2_FXF_WRITE|LIBSSH2_FXF_CREAT,
                                            LIBSSH2_FXF_TRUNC);//LIBSSH2_SFTP_S_IWUSR);
            /*
                     libssh2_sftp_open(sftp_sesandssock_t->sftp_session_t, remote_path,
                           LIBSSH2_FXF_WRITE|LIBSSH2_FXF_CREAT|LIBSSH2_FXF_TRUNC,
                           LIBSSH2_SFTP_S_IRUSR|LIBSSH2_SFTP_S_IWUSR|
                           LIBSSH2_SFTP_S_IRGRP|LIBSSH2_SFTP_S_IROTH);
                         */
            if (sftp_handle == NULL)
            {
                DPrint("sftp_handle is null\n,");
                DPrint("libssh2_sftp_open error:%d\n,",libssh2_session_last_errno(sftp_sesandssock_t->session_t));
            }
            if (!sftp_handle && (libssh2_session_last_errno(sftp_sesandssock_t->session_t) != LIBSSH2_ERROR_EAGAIN))
            {
                DPrint("Unable to open file with SFTP\n,");
                return SY_FAILED;
            }
            else
            {
                waitsocket(sftp_sesandssock_t->sock_t, sftp_sesandssock_t->session_t); /* now we wait */
                DPrint(".\n");
                block_count++;
                if( block_count >= block_times )
                {
                    DPrint("on-blocking open faild!!! try increase block_times value\n");
                    return SY_FAILED;
                }
            }
        }
        while (!sftp_handle);

        DPrint("libssh2_sftp_open() is done, now send data!, sftpfile:%s\n",remote_path);

        start = time(NULL);
        do
        {
            nread = fread(mem, 1, sizeof(mem), local);
            if (nread <= 0)
            {
                /* end of file */
                break;
            }

            ptr = mem;
            total += nread;
            block_count = 0;

            do
            {
                block_count = 0;
                /* write data in a loop until we block */
                while ((rc = libssh2_sftp_write(sftp_handle, ptr, nread)) ==
                        LIBSSH2_ERROR_EAGAIN)
                {
                    waitsocket(sftp_sesandssock_t->sock_t, sftp_sesandssock_t->session_t);
                    block_count++;
                    if( block_count >= block_times+3 )
                    {
                        DPrint("\non-blocking write faild!!! try increase block_times value\n");
                        return SY_FAILED;
                    }
                }
                if(rc < 0)
                {
                    DPrint("write failed %d\n",rc);
                    break;
                }
                DPrint("write successed %d\n",rc);
                ptr += rc;
                nread -= rc;

            }
            while (nread);
        }
        while (rc > 0);

        duration = (int)(time(NULL)-start);
        if(0 == duration)
        {
            sleep(1);
            duration = (int)(time(NULL)-start);
        }
        //统计传输文件大小和传输速度
        DPrint("%ld bytes in %d seconds makes %.1f bytes/sec\n",
               total, duration, total/(double)duration);
    }

    DPrint("SFTP upload done!\n");

    libssh2_sftp_close(sftp_handle);
    close(sftp_sesandssock_t->sock_t);
    fclose(local);

    return SY_SUCCESS;
}


/*******************************************************************************
函数说明:   利用sftp协议下载文件
参数说明:       sftp_sesandssock_t  :   自定义的结构体，包含了socket和sftp_session 和session
                block_t             :   0：非阻塞，1：非阻塞
                block_times         ：  当设置block_t为非阻塞(0)时，block_times表示重复select的次数
                local_path          :   在本地主机中上传文件的位置
                remote_path         :   在远程服务器中存放文件的位置
返  回   值:    0   :   下载成功
                -1  :   下载失败
*******************************************************************************/
int downloadfile_sftp(sftp_sesandssock * sftp_sesandssock_t, unsigned int block_t, unsigned int block_times, const char *local_path, const char *remote_path)
{
    size_t nread;
    int rc;
    int block_count;
    FILE *local;
    char mem[SY_BUFFER_SIZE];
    char *ptr = NULL;
    struct timeval timeout;
    fd_set fd;
    time_t start;
    long total = 0;
    int duration;
    LIBSSH2_SFTP_HANDLE *sftp_handle = NULL;


    local = fopen(local_path, "wb");
    if (!local)
    {
        DPrint("Can't open local file %s\n", local_path);
        libssh2_sftp_close(sftp_handle);
        return SY_FAILED;
    }

    //阻塞情况下
    if( 1 == block_t )
    {
        sftp_handle = libssh2_sftp_open(sftp_sesandssock_t->sftp_session_t, remote_path,LIBSSH2_FXF_READ, 0);

        if (!sftp_handle)
        {
            DPrint("Unable to open file with SFTP: %lu\n", libssh2_sftp_last_error(sftp_sesandssock_t->sftp_session_t));
            return SY_FAILED;
        }

        DPrint("libssh2_sftp_open() is done, now downloading data!\n");
        DPrint("libssh2_sftp_read()!\n");

        start = time(NULL);

        do
        {
            /* loop until we fail */
            //fprintf(stderr, "libssh2_sftp_read()!\n");
            rc = libssh2_sftp_read(sftp_handle, mem, sizeof(mem));
            total += rc;
            if (rc > 0)
            {
                //打印到标准输出
                //  write(1, mem, rc);
                fwrite(mem, rc, 1, local);
            }
            else
            {
                break;
            }
        }
        while (1);

        duration = (int)(time(NULL)-start);

        //统计传输文件大小和传输速度
        DPrint("%ld bytes in %d seconds makes %.1f bytes/sec\n",
               total, duration, total/(double)duration);

    }
    //非阻塞情况下
    else
    {
        block_count = 0;
        do
        {
            sftp_handle = libssh2_sftp_open(sftp_sesandssock_t->sftp_session_t, remote_path,
                                            LIBSSH2_FXF_READ, 0);

            if (!sftp_handle && (libssh2_session_last_errno(sftp_sesandssock_t->session_t) != LIBSSH2_ERROR_EAGAIN))
            {
                DPrint("Unable to open file with SFTP\n");
                return SY_FAILED;
            }
            else
            {
                waitsocket(sftp_sesandssock_t->sock_t, sftp_sesandssock_t->session_t);
                DPrint(".");
                block_count++;
                if( block_count >= block_times+3 )
                {
                    DPrint("\non-blocking open faild!!! try increase block_times value\n");
                    return SY_FAILED;
                }
            }
        }
        while (!sftp_handle);

        DPrint("\nlibssh2_sftp_open() is done, now receive data!\n");

        start = time(NULL);
        block_count = 0;

        do
        {
            /* loop until we fail */
            block_count = 0;
            while ((rc = libssh2_sftp_read(sftp_handle, mem,
                                           sizeof(mem))) == LIBSSH2_ERROR_EAGAIN)
            {
                waitsocket(sftp_sesandssock_t->sock_t, sftp_sesandssock_t->session_t);
                block_count++;
                if( block_count >= block_times+3 )
                {
                    DPrint("\non-blocking read faild!!! try increase block_times value\n");
                    return SY_FAILED;
                }
            }
            if (rc > 0)
            {
                total += rc;
                fwrite(mem, rc, 1, local);
            }
            else
            {
                break;
            }
        }
        while (1);

        duration = (int)(time(NULL)-start);

        //统计传输文件大小和传输速度
        DPrint("%ld bytes in %d seconds makes %.1f bytes/sec\n",
               total, duration, total/(double)duration);

    }

    libssh2_sftp_close(sftp_handle);
    close(sftp_sesandssock_t->sock_t);
    fclose(local);
    return SY_SUCCESS;
}

int sySftpUpload(char *hostUrl, int hostPort, int block, const char *username, char *password, char *localpath, char *sftppath)
{
    int nRet = SY_FAILED;
    sftp_sesandssock *sftp_session;
    unsigned long hostaddr = inet_addr(hostUrl);
    //unsigned int hostport = 37028;

    DPrint ("hostaddr:%s . username:%s .password:%s \n",hostUrl,username,password);
    sftp_session = init_sftpsession(hostaddr, hostPort, block, 20, keyfile1, keyfile2, username, password);
    if(sftp_session == NULL)
    {
        DPrint ("init_sftpsession failed.\n");
        return SY_FAILED;
    }
    else
        DPrint ("init_sftpsession success.\n");
    nRet = uploadfile_sftp(sftp_session, block, 20, localpath, sftppath);

    if(nRet == SY_FAILED)
    {
        DPrint ("upload file_sftp failed.\n");
    }
    else
        DPrint ("upload file_sftp succes.\n");

    exit_sftpsession(sftp_session);

    DPrint("all done\n");

    return nRet;
}

int sySftpDownload(char *hostUrl, char *username, char *password, char *localpath, char *sftppath)
{
    int nRet = SY_FAILED;
    sftp_sesandssock *sftp_session;
    unsigned long hostaddr = inet_addr(hostUrl);
    unsigned int hostport = 37028;

    sftp_session = init_sftpsession(hostaddr, hostport, 0, 10, keyfile1, keyfile2, username, password);
    if(sftp_session == NULL)
    {
        DPrint ("init_sftpsession failed \n");
        return SY_FAILED;
    }

    nRet = downloadfile_sftp(sftp_session, 0, 10, localpath, sftppath);

    exit_sftpsession(sftp_session);

    DPrint("all done\n");

    return nRet;
}
#endif


int SyIsCorrectIp(const char *pIp)
{
    int i, len;
    int sum = 0;
    int nCounts = 0;
    int dotCounts = 0;
    char ip[128] = {0};

    strcpy(ip, pIp);
    len = strlen(ip);

    if((NULL == pIp))
    {
        return SY_FALSE;
    }

    if(ip[0] == '.' || ip[len-1] == '.')
    {
        return SY_FALSE;
    }

    for(i = 0; i < len; i++)
    {
        if(ip[i] >= '0' && ip[i] <= '9')
        {
            sum = sum * 10 + (ip[i] - '0');
            nCounts++;
            continue;
        }
        else if(ip[i] == '.')
        {
            if(ip[i + 1] == '.')
            {
                return SY_FALSE;
            }
            dotCounts++;
            if(!((nCounts >= 0) && (nCounts <= 3)) || !((sum >= 0) && (sum <= 255)))
            {
                return -1;
            }
            sum = 0;
            nCounts = 0;
            continue;
        }
        else
        {
            return SY_FALSE;
        }
    }

    if(!((nCounts >= 0) && (nCounts <= 3)) || !((sum >= 0) && (sum <= 255)))
    {
        return SY_FALSE;
    }

    if (3 == dotCounts)
    {
        return SY_TRUE;
    }

    return SY_FALSE;
}

int SyGetPid(char *name)
{
    int pid = -1;
    FILE *fp;
    char* cRet = NULL;
    char buf[1024], cmd[200];
    DO;

    memset(buf, 0x00, sizeof(buf));
    sprintf(cmd, "ps | grep %s$", name);
    DPrint("cmd:%s\n", cmd);
    if ((fp = popen(cmd, "r")) == NULL)
        return -1;

    DPrint("fp:%p\n", fp);
#if 0
    while(fgets(buf, sizeof(buf), fp))
    {
        DPrint("cmd:%s\n", buf);
        if (sscanf(buf, "%*s %d", &pid) == 1)
            break;
    }
#else
    int nRet = -1;
    DPrint("before fread\n");
    nRet = fread(buf, 1, sizeof(buf), fp);
    DPrint("nRet:%d, buf:%s\n", nRet, buf);
    if(0 != nRet)
    {
        //sscanf(buf, "%*s %d", &pid);
        pid = -1;
        DPrint("pid:%d\n", pid);
    }
#endif
    pclose(fp);
    DONE;
    return pid;
}

int  syIfProcessExist(char *processName)
{
    char cmd[64] = {0};
    char *pingbuf = (char *)malloc(1024);
    FILE *fd;
    int pid;
    sprintf(cmd, "ps |grep %s",processName);
    if ((fd = popen(cmd, "r")) != NULL)
    {
        fgets(pingbuf, 1024, fd);
        if( 0 == strlen(pingbuf))
        {
            pclose(fd);
            return 0;
        }
        else
        {
            pclose(fd);
            return 1;
        }
    }
    return SY_TRUE;
}


void syKillExistProcess(char *processName)
{
    char cmd[64] = {0};
    char *pingbuf = (char *)malloc(1024);
    FILE *fd;
    int pid;
    sprintf(cmd, "ps |grep %s",processName);
	DPrint("kill tcpdump\n");
	if ((fd = popen(cmd, "r")) != NULL)
    {
        while (fgets(pingbuf, 1024, fd))
        {
            DPrint("%s", pingbuf);
            while( *pingbuf > '9' || *pingbuf < '0')
            {
                pingbuf++;
            }
            pid = atoi(pingbuf);
            DPrint("pid = %d", pid);
            kill(pid, SIGTERM);
        }
    }
    pclose(fd);
}

int GetCPUUsageRate(CPUUSAGE *cpu, CPUUSAGE *cpu2)
{
    /*cpu  55027 327 38497 2075733 396 0 533 0 0 0
    user (55027) 从系统启动开始累计到当前时刻，用户态的CPU时间（单位：jiffies） ，不包含 nice值为负进程。1jiffies=0.01秒
    nice (327) 从系统启动开始累计到当前时刻，nice值为负的进程所占用的CPU时间（单位：jiffies）
    注意：用户态的真正时间应该包括：user时间+nice时间 即：55027 + 327
    system (38497) 从系统启动开始累计到当前时刻，核心时间（单位：jiffies）
    idle (2075733) 从系统启动开始累计到当前时刻，除硬盘IO等待时间以外其它等待时间（单位：jiffies）
    iowait (396) 从系统启动开始累计到当前时刻，硬盘IO等待时间（单位：jiffies）
    irq (0) 从系统启动开始累计到当前时刻，硬中断时间（单位：jiffies）
    softirq (533) 从系统启动开始累计到当前时刻，软中断时间（单位：jiffies）
    cpu利用率=(user_pass + system_pass + irq_pass)*100%/(user_pass +irq_pass+ system_pass + idle_pass)
    */
    long lUser = cpu2->value[0] - cpu->value[0];
    long lSystem = cpu2->value[2] - cpu->value[2];
    long lIrq = cpu2->value[5] - cpu->value[5];
    long lIdle = cpu2->value[3] - cpu->value[3];
    long x = (lUser + lSystem + lIrq);
    long y = (lUser + lSystem + lIrq + lIdle);
    long lRate = x * 1000 / y;
    if (lRate < 0)
    {
        DPrint("%ld,%ld,%ld,%ld,%ld,%ld\n", lUser, lSystem, lIrq, lIdle, x, y);
    }
    return lRate;
}
int GetCPUUsage(CPUUSAGE *cpu)
{
    int iRet = -1;
    int fd = open("/proc/stat", O_RDONLY, 0444);
    if (fd == -1)
    {
        DPrint("open /proc/stat error:%d,%s\n", errno, strerror(errno));
        return -1;
    }
    char buf[512];
    if (read(fd, buf, sizeof(buf)) >= 0)
    {
        printf("%s\n", buf);
        sscanf(buf, "cpu %ld %ld %ld %ld %ld %ld %ld", &cpu->value[0], &cpu->value[1],
               &cpu->value[2], &cpu->value[3], &cpu->value[4], &cpu->value[5], &cpu->value[6]);
        iRet = 1;
        DPrint("get one is %ld  \n", cpu->value[0]);
    }
    close(fd);
    return iRet;
}

int GetMemoryTotal()
{
    int nTotal = 100;
    int nFree = 1;
    int fd = open("/proc/meminfo", O_RDONLY, 0444);
    if (fd != -1)
    {
        char buffer[1024];
        char szValue[64];
        memset(buffer, 0, sizeof(buffer));
        read(fd, buffer, sizeof(buffer));
        char* p = strstr(buffer, "MemTotal:");
        if (p != NULL)
        {
            sscanf(p, "MemTotal:%s[^kK]", szValue);
            nTotal = atoi(szValue);
        }
        close(fd);
    }
    if (nTotal > 0)
        return nTotal;
    return 10;
}

int GetMemoryfree()
{
    int nTotal = 100;
    int nFree = 1;
    int fd = open("/proc/meminfo", O_RDONLY, 0444);
    if (fd != -1)
    {
        char buffer[1024];
        char szValue[64];
        memset(buffer, 0, sizeof(buffer));
        read(fd, buffer, sizeof(buffer));
        char* p = strstr(buffer, "MemFree:");
        if (p != NULL)
        {
            sscanf(p, "MemFree:%s[^kK]", szValue);
            nTotal = atoi(szValue);
        }
        close(fd);
    }
    if (nTotal > 0)
        return nTotal;
    return 10;
}

void SyGetModelName(char * name)
{
    char cmd[64] = {0};
    char *pingbuf = (char *)malloc(1024);
    FILE *fd;
    int pid;

    sprintf(cmd, "getprop ro.product.model");
    if ((fd = popen(cmd, "r")) != NULL)
    {
        while (fgets(pingbuf, 1024, fd))
        {
            DPrint("%s", pingbuf);
            if( strlen(pingbuf) != 0 )
                strcpy(name, pingbuf);
            else
                strcpy(name, "PTV-8098");
        }
    }
    free(pingbuf);
    pclose(fd);
    return ;
}

/**/
int InitCmdSocket(const char *strIP, unsigned short port)
{
    int fd = -1;

    if (-1 == (fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))) {
        DPrint("create socket failed!\n");
        return -1;
    }

    struct sockaddr_in peerAddr;
    memset(&peerAddr, 0, sizeof(peerAddr));
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_addr.s_addr = inet_addr(strIP);
    peerAddr.sin_port = htons(port);

    struct timeval timeout = {0, 100*1000};

    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));

    if (connect(fd, (struct sockaddr*)&peerAddr, sizeof(peerAddr)) == -1)
    {
    	DPrint("socket connect failed!\n");
        close(fd);
        return -1;
    }

    return fd;
}


int SendToIptvSer(const void* pszHost, char *routeRes, int cmd)
{
	ItvSrvMsg_In rMsg;
	memset(&rMsg, 0, sizeof(rMsg));
    int sockfd = InitCmdSocket("127.0.0.1", 12978);
    if (sockfd != -1)
    {
        ItvSrvMsg_Out sendMsg;
        sendMsg.wCmd = cmd;		
		
		memcpy(sendMsg.szMsg, pszHost, sizeof(sendMsg));
		CMD2 *t = (CMD2 *)sendMsg.szMsg;
		DPrint("sendMsg is %s", t->szCmd);
		
		DPrint("send to IptvServ cmd=%d, msg=%s\n", sendMsg.wCmd, sendMsg.szMsg);
		int ret = -1;
        
        if ((ret=send(sockfd, &sendMsg, sizeof(sendMsg), 0)) <= 0)
        {
            DPrint("send failed! cmd:%d, msg:%s\n, ret = %d", sendMsg.wCmd, sendMsg.szMsg, ret);
            return -1;
        }
        DPrint("send cmd:%d, msg:%s, ret = %d\n", sendMsg.wCmd, sendMsg.szMsg, ret);        

		if(cmd == 24)
		{
			ret = recv(sockfd, (char*)&rMsg, sizeof(rMsg), 0);
			FILE *routefp;
			if((routefp = fopen("/tmp/tracetTmp", "wb+")) == NULL)
			{
				DPrint("open /tmp/data.log error\n");
			}
			fwrite(rMsg.msg, 1, sizeof(rMsg.msg), routefp);
			fclose(routefp);
		}
	        
		DPrint("recv ret = %d\n", ret);
        //DPrint("recv cmd:%d, len:%d, msg:%s\n",rMsg.cmd, rMsg.len, rMsg.msg);
		
    }

    return 0;
}


int AddRouteToEth(const char* pszHost)
{
    if (pszHost == NULL || strlen(pszHost) == 0)
    {
        DPrint("pszHost is NULL or empty!\n");
        return 0;
    }
    DPrint("pszHost:%s\n", pszHost);

    char strIP[256] = {0};
    sscanf(pszHost, "%*[^//]//%[^:]", strIP);
	ItvSrvMsg_In rMsg;
	memset(&rMsg, 0, sizeof(rMsg));
    int sockfd = InitCmdSocket("127.0.0.1", 12978);
    if (sockfd != -1)
    {
        ItvSrvMsg_Out sendMsg;
        sendMsg.wCmd = 24;
		sendMsg.szMsg[250] = 0;
        strncpy(sendMsg.szMsg, strIP, sizeof(sendMsg.szMsg));
		int ret = -1;
        //
        if ((ret=send(sockfd, &sendMsg, sizeof(sendMsg), 0)) <= 0)
        {
            DPrint("send failed! cmd:%d, msg:%s\n, ret = %d", sendMsg.wCmd, sendMsg.szMsg, ret);
            return -1;
        }
        DPrint("send cmd:%d, msg:%s, ret = %d\n", sendMsg.wCmd, sendMsg.szMsg, ret);

        ret = recv(sockfd, (char*)&rMsg, sizeof(rMsg), 0);
		DPrint("recv ret = %d\n", ret);
        DPrint("recv cmd:%d, len:%d, msg:%s\n",rMsg.cmd, rMsg.len, rMsg.msg);
    }

    return 0;
}


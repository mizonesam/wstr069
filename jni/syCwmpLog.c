#include "syCwmpLog.h"

int gnLogcatPid = 1;
int ContinueTime =0;//持续抓日质奔渚
char LogLevel[10];
char Level[10]={0};
char StartTime[32] ={0};//抓日志开始时间
char Time[32] ={0};
int cTime =0;
int option = 0;

char gnSzModelName[64] = {0};
char gnSzSoftwareVersion[64] = {0};


int gnMsgLogIsCreat = 0;
int gnMsgLogIsUpload = 0;
int gnMsgLogDuration = 300;
int gnMsgLogIsMsg = 1;
int gnMsgLogIsTimeout = 0;
int gnMsgLogSize = 0;
int gnFirstMsgLog = 1;
int gnMsgLogQKey = -1;
int gnMsgLogRecordIsEnd = 1;
time_t gnBefore = 0L;
pthread_t gnMsgLogRecordTid = 0;
pthread_t gnMsgLogUploadTid = 0;
pthread_t gnMsgLogTimeoutDealTid = 0;
pthread_t gnMsgLogcatUploadTid = 0;//dfm
pthread_t gnMsgLogcatid = 0;//dfm
pthread_t gnMsgContinueLogcatid = 0;//dfm
pthread_t gnMsgStartLogcat = 0;//dfm
pthread_t gnMsgStartTime = 0;//dfm
pthread_t gnMsgGetFileSizeid = 0;

int ContinueFlag = 0;//dfm日志持续时间下发标志
char gnMsgLogcatFilePath[256] = {0};//dfm
char gnMsgLogFilePath[256] = {0};
char gnMsgLogText[128*1024] = {0};
char gnMsgLogTmpBuf[10*1024] = {0};
long int SleepTime = 0;//dfm当前时间到开始抓日志之间的等待时间

int gnSysLogType = 0;
int gnSysLogLevel = 0;
int gSyTmsSockfd = -1;
int gnSysLogOutputType = 0;
char gnSyslogServer[64] = {0};

setIptvData gsetData = NULL;
int runLogcat(QOSMSG* pMsg){
    DPrint("[%s] --->\n", __FUNCTION__);
    DPrint("[%s] cmd :%d  msg: %s \n", __FUNCTION__,pMsg->cmd, pMsg->msg);
	int Iset = -1;
	int n=0;

	//if (gsetData != NULL){
	if (pMsg->cmd == 200){
		n = atoi(pMsg->msg);     //cmd类型决定log类型
		if(n == 0){
			strcpy(LogLevel,"*:V");
		} else if(n == 3){
			strcpy(LogLevel,"*:E");
		} else if(n == 6){
			strcpy(LogLevel,"*:I");
		} else if(n == 7){
			strcpy(LogLevel,"*:D");
		}
		DPrint("[%s]--n=%d,option:%d,LogLevel: %s---\n", __FUNCTION__,n,option, LogLevel);
	}
	
	if (pMsg->cmd == 203){
		strcpy(StartTime,pMsg->msg);
		
		DPrint("[%s]--option:%d,StartTime: %s---\n", __FUNCTION__,option, StartTime);
		if(gnMsgStartTime==0)
				pthread_create(&gnMsgStartTime, NULL, MsgStartTimeThread, NULL);
	}
	if (pMsg->cmd ==204){
		ContinueTime = atoi(pMsg->msg);
		ContinueFlag = 1;
		DPrint("[%s]--option:%d,ContinueTime: %d---\n", __FUNCTION__,option, ContinueTime);
	}
	if ((strcmp(LogLevel,Level)==0)&&(strcmp(StartTime,Time)==0)&&(ContinueTime==cTime)){
		option = 0;
	} else {	
		option =1;
	}
    if (pMsg->cmd == 201){
		n = atoi(pMsg->msg);
		char name[64] ;
		char ncmd[256];
		DPrint("[%s]--test n= %d---\n", __FUNCTION__, n);
		sprintf(gnMsgLogcatFilePath, "%slogtest.log", LOG_FILE_PATH);
		if (n != 0){
			FILE *logfile = fopen(gnMsgLogcatFilePath, "wb+");		//先创建文件
			fclose(logfile);
			if(gnMsgStartLogcat==0)
			{
				DPrint("start thread MsgStartLogcatThread\n");
				pthread_create(&gnMsgStartLogcat, NULL, MsgStartLogcatThread, NULL);
			}
				
		} else if(n == 0){
			char cmd[64] = "busybox pkill logcat";
			char *pingbuf = (char *)malloc(128);
			FILE *fd;
			if ((fd = popen(cmd, "r")) != NULL){
				pclose(fd);	
			}
			system("rm -r /data/logtest.log");
			DPrint("[%s]--rm logtestfile --\n", __FUNCTION__);
		}
	}
	DPrint("[%s]--test3 --\n", __FUNCTION__);
	return 0;
	
    DPrint("[%s] runLogcat failed \n", __FUNCTION__);
	return -1;
}

static void* MsgContinueLogcatThread(void *data){
	DPrint("[%s]ContinueTime=%d--->\n", __FUNCTION__,ContinueTime*60);
	sleep(SleepTime + ContinueTime*60);
	char cmd[64] = "busybox pkill logcat";
	DPrint("[%s]cmd=%s--->\n", __FUNCTION__,cmd);
	FILE *fd;
	if ((fd = popen(cmd, "r")) != NULL) {
       
		pclose(fd);
	}
	gnMsgContinueLogcatid = 0;
	return NULL;
}


static void* MsgStartTimeThread(void *data){
	DPrint("[%s]--->\n", __FUNCTION__);
	char stime[32] = {0};
	char Cyear[10] = {0};
	char Cmon[10] = {0};
	char Cday[10] = {0};
	char Chour[10] = {0};
	char Cmin[10] = {0};
	char Csec[10] = {0};
	int Iyear,Imon,Iday,Ihour,Imin,Isec;
	time_t now = 0L;
	struct tm *tm1;
	now = time(NULL);
	tm1 = localtime(&now);
	sprintf(stime, "%d-%02d-%02dT%02d:%02d:%02d", tm1->tm_year+1900, tm1->tm_mon+1,
             tm1->tm_mday, tm1->tm_hour, tm1->tm_min, tm1->tm_sec);
	memcpy(Cyear, StartTime, 4);
	Iyear=atoi(Cyear);
	DPrint("test1 Cyear:%s,Iyear:%d----\n",Cyear,Iyear);
	memcpy(Cmon, StartTime+5, 2);
	Imon=atoi(Cmon);
	DPrint("test2 Cmon:%s,Imon:%d----\n",Cmon,Imon);
	memcpy(Cday, StartTime+8, 2);
	Iday=atoi(Cday);
	DPrint("test2 Cday:%s,Iday:%d----\n",Cday,Iday);
	memcpy(Chour, StartTime+11, 2);
	Ihour=atoi(Chour);
	DPrint("test2 Cday:%s,Ihour:%d----\n",Chour,Ihour);
	memcpy(Cmin, StartTime+14, 2);
	Imin=atoi(Cmin);
	DPrint("test2 Cmin:%s,Imin:%d----\n",Cmin,Imin);
	memcpy(Csec, StartTime+17, 2);
	Isec=atoi(Csec);
	DPrint("test2 Csec:%s,Isec:%d----\n",Csec,Isec);
	SleepTime=(Iyear-(tm1->tm_year+1900))*365*12*3600*24+(Imon-(tm1->tm_mon+1))*12*24*3600+(Iday-tm1->tm_mday)*3600*24+(Ihour-tm1->tm_hour)*3600+(Imin-tm1->tm_min)*60+(Isec-tm1->tm_sec);
	DPrint("test2 SleepTimec:%ld----\n",SleepTime);
	gnMsgStartTime = 0;
	return NULL;
}//dfm

static void* MsgGetFileSizeThread(void *data){
	DPrint("[%s]--->\n", __FUNCTION__);
	unsigned long filesize = 0;
	char cmd[64] = "busybox pkill logcat";
	DPrint("[%s]cmd=%s--->\n", __FUNCTION__,cmd);
	sleep(2);
	struct stat buf;
	while(filesize<1024*1024*20){
		sleep(2);
		stat(gnMsgLogcatFilePath,&buf);
		filesize = buf.st_size;
		//DPrint("get logfile size=%lu\n", filesize);
	}
	DPrint("[%s]Gettest filesize=%lu--->\n", __FUNCTION__, filesize);
	FILE *fd;
	if ((fd = popen(cmd, "r")) != NULL) {
       
		pclose(fd);
	}
	DPrint("<---[%s]\n", __FUNCTION__);
	gnMsgGetFileSizeid = 0;
	return NULL;
}



static void* MsgStartLogcatThread(void *data){
	
	FILE *fd;
	char cmd[64] = "busybox pkill logcat";
	DPrint("[%s]cmd=%s--->\n", __FUNCTION__,cmd);
	while(option==0){
		sleep(1);
	}
	DPrint("[%s]--option2:%d ---\n", __FUNCTION__,option);
	strcpy(Level,LogLevel);
	strcpy(Time,StartTime);
	cTime = ContinueTime;
	if ((fd = popen(cmd, "r")) != NULL) {
       
		pclose(fd);
	}
	DPrint("[%s]-- ContinueFlag1:%d--\n", __FUNCTION__,ContinueFlag);
	while(ContinueFlag==0){
		sleep(1);
	}
	DPrint("[%s]-- ContinueFlag2:%d--\n", __FUNCTION__,ContinueFlag);
	ContinueFlag = 0;
	DPrint("[%s]-- SleepTime1:%ld--\n", __FUNCTION__, SleepTime);
	if(SleepTime>=0){
		sleep(SleepTime);
		SleepTime = 0;
	}
	else
	{
		DPrint("[%s]-- SleepTime2:%ld--\n", __FUNCTION__, SleepTime);
		int t = 5; 
		if(t>=0)
			sleep(t);
		if(SleepTime + ContinueTime*60 <= 0)
		{
			return NULL;
		}
	}
	if(gnMsgLogcatid == 0){
		pthread_create(&gnMsgLogcatid, NULL, MsgLogcatThread, NULL);
		DPrint("[%s]--test1 MsgLogcatThread--\n", __FUNCTION__);
	}
	gnMsgStartLogcat = 0;	
	return NULL;
}

static void* MsgLogcatThread(void *data){
	DPrint("[%s]--->\n", __FUNCTION__);
	char ncmd[256];
	int n=0;
	int pid=0;
	char *pingbuf = (char *)malloc(128);
	//n=system(ncmd);
	system("logcat -c");
	sprintf(ncmd, "logcat %s -v time >%s &",LogLevel,gnMsgLogcatFilePath);
	DPrint("[%s]--test szCmd= %s\n", __FUNCTION__, ncmd);
	pthread_create(&gnMsgLogcatUploadTid, NULL, MsgLogcatUploadThread, NULL);
	DPrint("[%s]uptest--->\n", __FUNCTION__);
	pthread_create(&gnMsgContinueLogcatid, NULL, MsgContinueLogcatThread, NULL);
	pthread_create(&gnMsgGetFileSizeid, NULL, MsgGetFileSizeThread, NULL);
	DPrint("[%s]uptest2--->\n", __FUNCTION__);
	n=system(ncmd);
	DPrint("[%s]uptest3 n=%d--->\n", __FUNCTION__,n);
	
	gnMsgLogcatid = 0;
	return NULL;
}



static void* MsgLogcatUploadThread(void *data){
	FILE*msgLogFp=NULL; 
	char sysLogcatBuf[1024]={0};
	char sysLogBuf[1024]={0};
	int n =1;
	//dfm
	char szMac[20] = {0};
    char szTime[32] = {0};
	//char *p;
	DO;
	//dfm
	sleep(5);
	DPrint("[%s] gnMsgLogcatFilePath=%s--->\n", __FUNCTION__,gnMsgLogcatFilePath);
	msgLogFp = fopen(gnMsgLogcatFilePath, "r");
	if(msgLogFp==NULL)
		return NULL;
	while(fgets(sysLogcatBuf, 1024, msgLogFp)){
		
		if(gnMsgContinueLogcatid > 0)
		{
			GetMAC(szMac, ':');
			GetCurrentTime(2, szTime);
			sprintf(sysLogBuf, "<%d>%s %s %s%s\r\n%s", ((gnSysLogLevel<<3)+gnSysLogType),
					    szTime, szMac, gnSzModelName, gnSzSoftwareVersion, sysLogcatBuf);
			sySendMsgToTms(sysLogBuf, strlen(sysLogBuf));
			memset(sysLogcatBuf, 0, sizeof(sysLogcatBuf));
			memset(sysLogBuf, 0, sizeof(sysLogBuf));
			sleep(3); 		//需要停顿一下 
			if(access(gnMsgLogcatFilePath, F_OK) < 0)	 //判断Log文件是否存在
			{
				DPrint("logfile is not exist\n");
				break;
			}
		}	
	}
	lseek(fileno(msgLogFp), 0, SEEK_SET);
	fclose(msgLogFp);
	gnMsgLogcatUploadTid = 0;
	DONE;
	return NULL;
}

int GetMAC(char* pszMAC, char mark){
	DPrint("[%s] --->\n", __FUNCTION__);
	
	int nSocket;
	struct ifreq struReq;
	nSocket = socket(PF_INET, SOCK_STREAM, 0);
	memset(&struReq, 0, sizeof(struReq));
	strncpy(struReq.ifr_name, "eth0", sizeof(struReq.ifr_name));
	ioctl(nSocket, SIOCGIFHWADDR, &struReq);
	close(nSocket);
	unsigned char* p = (unsigned char*)struReq.ifr_hwaddr.sa_data;
	sprintf(pszMAC, "%02X%c%02X%c%02X%c%02X%c%02X%c%02X", p[0], mark,
		p[1], mark, p[2], mark, p[3], mark, p[4], mark, p[5]);
	
	return 1;
}


static int GetCurrentTime(int type, char *stime){
    static const char* date[12] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    time_t now = 0L;
    struct tm *tm1;

	//DO;
    now = time(NULL);
    tm1 = localtime(&now);

    switch(type){
        case 1:
            sprintf(stime, "%d%02d%02d%02d%02d%02d", tm1->tm_year+1900, tm1->tm_mon+1,
                tm1->tm_mday, tm1->tm_hour, tm1->tm_min, tm1->tm_sec); 
        break;
        case 2:
            sprintf(stime, "%s %02d %02d:%02d:%02d", date[tm1->tm_mon], tm1->tm_mday, 
                tm1->tm_hour, tm1->tm_min, tm1->tm_sec); 
        break;
        default:
            sprintf(stime, "%d-%02d-%02d:%02d-%02d-%02d", tm1->tm_year+1900, tm1->tm_mon+1,
                tm1->tm_mday, tm1->tm_hour, tm1->tm_min, tm1->tm_sec);     
        break;
    }
	//DONE;
    return SUCCESS;
}

int sySendMsgToTms(const char* msg, int msgLen){
    int ret = -1;
    int port = -1;
    char* pos = NULL;
    char IpAddr[32] = {0};
    char PortStr[16] = {0};
	struct sockaddr_in addr;
	DO;
   // DPrint("[%s] --->\n", __FUNCTION__);
    
    if ((NULL == msg) || (strlen(msg) == 0)){
        DPrint("[%s] command is NULL or length is 0\n", __FUNCTION__);
        return FAILURE;
    }
    //DPrint("[%s] command:%s, len:%d\n", __FUNCTION__, msg, msgLen);

    if (-1 == gSyTmsSockfd){
        gSyTmsSockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (gSyTmsSockfd < 0){
            DPrint("[%s] Create Socket Fail\n", __FUNCTION__);      
            return FAILURE;
        }
    }

    if (0 == strlen(gnSyslogServer)) {
		
		if(!GetValue("Device.X_00E0FC.LogParaConfiguration.SyslogServer", gnSyslogServer, sizeof(gnSyslogServer)))
		{
			SyGetNodeValue("Device.X_00E0FC.LogParaConfiguration.SyslogServer", gnSyslogServer);
		}
	
        //GetXmlValue("Service/ServiceInfo/SyslogServer", gnSyslogServer, sizeof(gnSyslogServer));
    }/************************/
	
   // DPrint("[%s] SyslogServer : %s\n", __FUNCTION__, gnSyslogServer);
    if (0 != strlen(gnSyslogServer)){
        pos = strstr(gnSyslogServer, ":"); 
        if (NULL != pos) {
            memcpy(IpAddr, gnSyslogServer, pos - gnSyslogServer);
            memcpy(PortStr, pos + 1, gnSyslogServer + strlen(gnSyslogServer) - pos);
            port = atoi(PortStr);
        } else {
            memcpy(IpAddr, gnSyslogServer, strlen(gnSyslogServer));
            port = 514;
        }
        
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(IpAddr);
        addr.sin_port = htons(port);
        
        ret = sendto(gSyTmsSockfd, msg, msgLen, 0, (struct sockaddr*)&addr, sizeof(addr));
        if(ret <= 0){
            DPrint("[%s] Send Msg Fail\n", __FUNCTION__);      
            SyCloseSocket(gSyTmsSockfd);
            gSyTmsSockfd = -1;
            return FAILURE;
        }    
       // DPrint("[%s] <---\n", __FUNCTION__);
    }
	else {
        DPrint("[%s] Wrong SyslogServer\n", __FUNCTION__);
        return FAILURE;
    }
    DONE;
    return SUCCESS;
}





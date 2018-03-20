#ifndef SYCWMPTASKQUEUE_H
#define SYCWMPTASKQUEUE_H
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdlib.h>  
#include <assert.h>

#define addEvent(id)	AddEvent(id,NULL)
#define SY_VECTOR
//恢复出厂设置
#define EVENT_FACTORY_RESET		0
//重启
#define EVENT_REBOOT			1
//下载
#define EVENT_DOWNLOAD			2
//上传
#define EVENT_UPLOAD			3
//启动
#define EVENT_INITIALIZE		4
//零配置
#define EVENT_ZEROCONFIG		5
//播控诊断
#define EVENT_DIAGNOSTICS		6
//ping
#define EVENT_PING				7
//Traceroute
#define EVENT_TRACEROUTE		8
//Bandwidth speed test
#define EVENT_BANDWIDTH		9
//抓包
#define EVENT_CAPPACKET		10

//待机
#define EVENT_SHUT_DOWN		11
//配置更改
#define EVENT_VALUE_CHANGE		12
//错误码
#define EVENT_ERRORCODEMANAGE	13
//心跳
#define EVENT_PERIODIC			14
//SYSLOG
#define EVENT_LOGPERIODIC		15
//上报
#define EVENT_INFORM			16

//ACS连接CPE
#define EVENT_CONNECT_REQUEST	17

#define EVENT_STARTUPINFO		18	//开机信息收集
#define EVENT_DEBUGINFO			19	//调试信息
#define EVENT_TSAPK_STARTUPINFO	20	//调试apk启动开机信息收集
#define EVENT_TSAPK_DEBUGINFO	21	//调试apk启动调试信息
#define EVENT_TMCCHANGEDURL		22

typedef struct _TASKMSG{
	int cmd; //标识任务id
	char* msg; //message
	int len;	//message len
}TASKMSG;

typedef struct _Vector{
	int size;
	int maxsize;
	TASKMSG* TaskMsg;
}*vector;


static pthread_mutex_t	mtx;
static pthread_mutex_t	lock;
static pthread_cond_t	cond;
static pthread_t gThread;
static int gisSignal = 0;
static int gStop = 0;
vector g_vecEvent;

#ifdef SY_VECTOR
vector vector_new();
int push_back(vector v, TASKMSG t);
TASKMSG pop_back(vector v, int flag);
void removeAt(vector v, int flag);
int vector_size(vector v);
void vector_free(vector v);
#endif
void WaitEvent(pthread_cond_t* pCond, pthread_mutex_t* pMtx, int* pisSignal, int timeout);
void AddEvent(int nEventID, const char* msg);
void Start();
void* RunThread(void* lParam);

#endif

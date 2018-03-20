#ifndef SYCWMPTASKQUEUE_H
#define SYCWMPTASKQUEUE_H
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdlib.h>  
#include <assert.h>

#define addEvent(id)	AddEvent(id,NULL)
#define SY_VECTOR
//�ָ���������
#define EVENT_FACTORY_RESET		0
//����
#define EVENT_REBOOT			1
//����
#define EVENT_DOWNLOAD			2
//�ϴ�
#define EVENT_UPLOAD			3
//����
#define EVENT_INITIALIZE		4
//������
#define EVENT_ZEROCONFIG		5
//�������
#define EVENT_DIAGNOSTICS		6
//ping
#define EVENT_PING				7
//Traceroute
#define EVENT_TRACEROUTE		8
//Bandwidth speed test
#define EVENT_BANDWIDTH		9
//ץ��
#define EVENT_CAPPACKET		10

//����
#define EVENT_SHUT_DOWN		11
//���ø���
#define EVENT_VALUE_CHANGE		12
//������
#define EVENT_ERRORCODEMANAGE	13
//����
#define EVENT_PERIODIC			14
//SYSLOG
#define EVENT_LOGPERIODIC		15
//�ϱ�
#define EVENT_INFORM			16

//ACS����CPE
#define EVENT_CONNECT_REQUEST	17

#define EVENT_STARTUPINFO		18	//������Ϣ�ռ�
#define EVENT_DEBUGINFO			19	//������Ϣ
#define EVENT_TSAPK_STARTUPINFO	20	//����apk����������Ϣ�ռ�
#define EVENT_TSAPK_DEBUGINFO	21	//����apk����������Ϣ
#define EVENT_TMCCHANGEDURL		22

typedef struct _TASKMSG{
	int cmd; //��ʶ����id
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

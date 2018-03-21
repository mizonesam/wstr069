#include "syCwmpTaskQueue.h"
#include "syCwmpCommon.h"
#include "syCwmpSocket.h"
#include "syCwmpManagement.h"
#include "syCwmpUtil.h"

extern int gSyIsZeroConfig;
extern pthread_t gSyBandwidthDiagnosticsTid;
extern pthread_t gSyPacketCaptureTid;
extern pthread_t gSyIpPingTestTid;
extern pthread_t gSyTraceRouteTestTid;
extern pthread_t gSyDebugInfoTid;
extern pthread_t gSyStartupInfoTid;
extern pthread_t gSyTsAPKDebugThreadId;
extern pthread_t gSyTsAPKStartupThreadId;

extern void* syTSApkStartupInfoThread(void *data);
extern void* syTSApkDebugInfoThread(void *data);

#ifdef SY_VECTOR
vector vector_new()
{
	vector v = (vector)malloc(sizeof(struct _Vector));
	v->size = 0;
	v->maxsize = 20;
	v->TaskMsg = (TASKMSG*)malloc(sizeof(TASKMSG)* v->maxsize);
	assert(v->TaskMsg != NULL);
	return v;
}
int push_back(vector v, TASKMSG t)
{
	assert(v != NULL);
	if(v->size >= v->maxsize){
		DPrint("Join a task error\n");
	}
	else{
		v->TaskMsg[v->size++] = t;
	}
	return v->size;
}
TASKMSG pop_back(vector v, int flag)
{
	assert(v != NULL);
	return v->TaskMsg[flag];
}
void removeAt(vector v, int flag){
	int i;
	assert(v != NULL || flag <= v->size);
	if(flag < v->size){
		for(i = flag; i < v->size; i++)
			v->TaskMsg[i] = v->TaskMsg[i+1];
		v->size--;
	}
	else
		v->size--;
}
int vector_size(vector v){
	assert(v != NULL);
	return v->size;
}
void vector_free(vector v){
	assert(v != NULL && v->TaskMsg != NULL);
	free(v->TaskMsg);
	v->TaskMsg = NULL;
	free(v);
	v = NULL;
}
#endif


void WaitEvent(pthread_cond_t* pCond, pthread_mutex_t* pMtx, int* pisSignal, int timeout)
{
	int		retval = 0;
	struct	timeval delta;
	struct	timespec abstime;
	if (timeout != -1)
	{
		gettimeofday(&delta, NULL);
		abstime.tv_sec = delta.tv_sec + (timeout/1000);
		abstime.tv_nsec = (delta.tv_usec + (timeout%1000) * 1000) * 1000;
		if ( abstime.tv_nsec > 1000000000 ) 
		{
			abstime.tv_sec += 1;
			abstime.tv_nsec -= 1000000000;
		}
	}	
	pthread_mutex_lock(pMtx);
	while(*pisSignal == 0)
	{
		if (timeout != -1)
			retval = pthread_cond_timedwait(pCond, pMtx, &abstime);
		else
			retval = pthread_cond_wait(pCond, pMtx);
		if (retval == EINTR)
			continue;
		if (retval == ETIMEDOUT)
			retval = 258;//WAIT_TIMEOUT
		else
			retval = 0;
		break;
	}
	*pisSignal = 0;
	pthread_mutex_unlock(pMtx);
}
void AddEvent(int nEventID,const char* msg){
	assert(g_vecEvent != NULL);	
	DPrint("[%s]--->\n",__FUNCTION__);
	pthread_mutex_lock(&lock);
	TASKMSG taskmsg;
	taskmsg.cmd = nEventID;
	if(msg != NULL){
		taskmsg.msg = (char*)malloc(strlen(msg)+1);
		strcpy(taskmsg.msg, msg);
		taskmsg.len = strlen(msg);
	}
	else{
		taskmsg.msg = NULL;
		taskmsg.len = 0;
	}
	int nIndex = push_back(g_vecEvent, taskmsg);
	int nCnt = vector_size(g_vecEvent);
	//清除相同的指令
	DPrint("nIndex:%d,nCnt:%d.\n",nIndex, nCnt);
	for(int i = 1; i <= nCnt; i++){
		if(nIndex == i)
			continue;
		if(nEventID == pop_back(g_vecEvent, i).cmd){
			removeAt(g_vecEvent, i);
			nCnt--;
			break;
		}
	}
	DPrint("size:%d\n", vector_size(g_vecEvent));
	pthread_mutex_unlock(&lock);
	
	pthread_mutex_lock(&mtx);
	gisSignal = 1;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock( &mtx );
}
void Start(){
	DPrint("task queue init: %d\n", gThread);
	if(gThread == 0){
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, 0);
		pthread_mutex_init(&mtx, &attr);
		pthread_mutex_init(&lock, NULL);
		pthread_cond_init(&cond, NULL);
		g_vecEvent = vector_new();
		int ret = pthread_create(&gThread, NULL, RunThread, NULL);
		if(ret != 0){
			DPrint("Failed to create task queue thread\n");
			return;
		}
		else{
			pthread_detach(gThread);
		}
	}
}

void* RunThread(void * lParam){
	DPrint("[%s]--->\n",__FUNCTION__);
	while(0 == gStop){
		int nCnt = 0;
		{
			pthread_mutex_lock(&lock);
			nCnt = vector_size(g_vecEvent);
			pthread_mutex_unlock(&lock);
		}
		DPrint("nCnt:%d\n",nCnt);
		while(nCnt > 0 && 0 == gStop){
			TASKMSG taskmsg;
			taskmsg = pop_back(g_vecEvent, 0);
			DPrint("task queue id: %d\n", taskmsg.cmd);
			switch(taskmsg.cmd){
				case EVENT_REBOOT:
					//保存重启标记到数据中心
					g_notify(REBOOT, "");
					//gStop = 1;
					break;
				case EVENT_FACTORY_RESET:
					g_notify(FACTORYRESET, "");
					//gStop = 1;
					break;
				case EVENT_DOWNLOAD:
					if(0 != taskmsg.len)
						HandleDownload(taskmsg.msg);
					break;
				case EVENT_UPLOAD:
					if(0 != taskmsg.msg);
						HandleUpload(taskmsg.msg);
					break;
				case EVENT_ZEROCONFIG:
					gSyIsZeroConfig = 1; //零配置标识
				case EVENT_INITIALIZE:
					if(!CwmpInit()){
						DPrint("Init Cmwp Error\n");
					}
					break;
				case EVENT_DIAGNOSTICS:
					//if(SY_SUCCESS == HandleCreateInform((void*)SY_EVENT_DIAGNOSTICS_COMPLETE, NULL))
					{
						HandleInform((void*)SY_EVENT_DIAGNOSTICS_COMPLETE, 0);
					}
					break;
				case EVENT_PING:
					if(taskmsg.len != 0){
						HandleUtil(&gSyIpPingTestTid, syIpPingTestThread, "IPPingDiagnostics");
					}
					else{
						HandleInform((void*)SY_EVENT_DIAGNOSTICS_COMPLETE, 0);
					}
					break;
				case EVENT_TRACEROUTE:
					if(taskmsg.len != 0){
						HandleUtil(&gSyTraceRouteTestTid, syTraceRouteTestThread, "TraceRouteDiagnostics");
					}
					else{
						HandleInform((void*)SY_EVENT_DIAGNOSTICS_COMPLETE, 0);
					}
					break;
				case EVENT_BANDWIDTH:
					if(taskmsg.len != 0){
						HandleUtil(&gSyBandwidthDiagnosticsTid, syBandwidthTestThread, "BandwidthDiagnostics");
					}
					else{
						HandleInform((void*)SY_EVENT_DIAGNOSTICS_COMPLETE, 0);
					}
					break;
				case EVENT_CAPPACKET:
					SySetNodeValue("Device.X_00E0FC.PacketCapture.State", "3");
					HandleUtil(&gSyPacketCaptureTid, syPacketCaptureThread, "PacketCapture");
					break;
				case EVENT_SHUT_DOWN:
					HandleInform((void*)SY_EVENT_X_CTC_SHUT_DOWN, 0);
					break;
				case EVENT_VALUE_CHANGE:
					//taskmsg.msg值:  key=value;
					//if(SY_SUCCESS == HandleCreateInform((void*)SY_EVENT_VALUE_CHANGE, taskmsg.msg))
					{
						HandleInform((void*)SY_EVENT_VALUE_CHANGE, 0);
					}
					break;
				case EVENT_ERRORCODEMANAGE:
					HandleInform((void*)SY_EVENT_X_00E0FC_ErrorCode, 0);
					break;
				case EVENT_PERIODIC:
					HandleInform((void*)SY_EVENT_PERIODIC, 0);
					break;
				case EVENT_LOGPERIODIC:
					HandleInform((void*)SY_EVENT_CTC_LOG_PERIODIC, 0);
					break;
				case EVENT_INFORM:
					HandleInform((void*)taskmsg.msg, taskmsg.len);
					break;
				case EVENT_CONNECT_REQUEST:
					HandleConnectRequest();
					break;
				case EVENT_STARTUPINFO:
					HandleUtil(&gSyStartupInfoTid, syStartupInfoThread, "startupInfo");
					break;
				case EVENT_DEBUGINFO:
					HandleUtil(&gSyDebugInfoTid, syDebugInfoThread, "debugInfo");
					break;
				case EVENT_TSAPK_STARTUPINFO:
					HandleUtil(&gSyTsAPKStartupThreadId, syTSApkStartupInfoThread, "tsAPKStartupInfo");
					break;
				case EVENT_TSAPK_DEBUGINFO:
					HandleUtil(&gSyTsAPKDebugThreadId, syTSApkDebugInfoThread, "tsAPKDebugInfo");
					break;
				case EVENT_TMCCHANGEDURL:
					DPrint("TMC hand out a new URL ,now restart \n");
					remove(SY_BOOT_FLAG);
					exit(1);
					break;
				default:
					break;
			}
			{
				pthread_mutex_lock(&lock);
				if(taskmsg.len != 0)
					free(taskmsg.msg);
				removeAt(g_vecEvent, 0);
				nCnt = vector_size(g_vecEvent);
				DPrint("run cmd after:%d\n", nCnt);
				pthread_mutex_unlock(&lock);
			}
		}	
		//if(gStop)
		//	vector_free(g_vecEvent);
		WaitEvent(&cond, &mtx, &gisSignal, -1);
	}
	
	gThread = 0;
	pthread_mutex_destroy(&mtx);
	pthread_cond_destroy(&cond);
	DPrint("[%s]<---\n",__FUNCTION__);
}


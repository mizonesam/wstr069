/*
 ============================================================================
 Name        : syNATClient.c
 Author      : yxy
 Version     :
 Copyright   : Your copyright notice
 Description : realize NAT traversal interface
 ============================================================================
 */

#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#include "syNATUdp.h"
#include "syNATStun.h"
#include "syNATClient.h"
#include "syCwmpCommon.h"

#define DISCOVERYMODE  1
#define MONITORMODE    2
 
static StunAddress4 gDstServer;
static unsigned int STUNMaxiKeepAlive;
static int STUNMiniKeepAlive;
static int gFuzzy;
extern int gPriFd;
extern int gNetChangFlag;
extern StunAtrString gStunUserName;
extern StunAtrString gStunPassword;
static pthread_t gLocalIpThread = 0;
static pthread_mutex_t gSendMutex=PTHREAD_MUTEX_INITIALIZER;
UInt32 gNotificationLimit = 0;
int gDiscoveryPhase = SY_FALSE;
int gModeState = MONITORMODE;
static int gLocalIpChanged = SY_FALSE;
static int gFirstBindChange = SY_FALSE;
int NATDetected = -1;
int gNATFlag = -1;


static void
probePeriod()
{
	VPrint("NAT detected false,so probe period\n");
	int ret = SY_FALSE;

	while (SY_TRUE != ret)
	{
		sleep(60 * 10);
		ret = sendBindReqPeriod(0);
	}
}

static int 
binarySearch(UInt32 *timeout, UInt32 *min, UInt32 *max)
{
	assert(*max > *min);

	VPrint("binary search min:%d max:%d\n", *min, *max);

	UInt32 mid = (*min + *max) / 2;
	int isBind = SY_FALSE;
	gFirstBindChange = SY_FALSE;
	int ret = 0;
	int count = 3;

	sleep(mid);

	while (count > 0)
	{
		count -= 1;

		ret = stunTimeoutDisc(&gDstServer, &isBind);	
		if (SY_FAILED == ret)
		{
			return SY_FAILED;
		}

		if (SY_TRUE == isBind)
			break;
		else
			continue;
	}

	VPrint("isBind == %d\n",isBind);

	if (isBind == SY_TRUE)
	{
		*min = mid;
	}
	else
	{
		*max = mid;
	}

#if 0
	if (*max - *min == 1)
	{
		*max = *min;
		*timeout = *max;
	}
#endif
	if (*max - *min <= gFuzzy)
	{
		*max = *min;
		*timeout = *max;
		gFirstBindChange = SY_TRUE;
	}

	return SY_SUCCESS;
}

//发现最小保活间隔时间
static void
discoveryTimeout(UInt32 *timeout, UInt32 *min, UInt32 *max)
{
//	char minimum[16] = {0};
//	char maximum[32] = {0};
//
//	SyGetNodeValue("Device.ManagementServer.STUNMaximumKeepAlivePeriod", maximum);
//	SyGetNodeValue("Device.ManagementServer.STUNMinimumKeepAlivePeriod", minimum);
//	*min = atoi(minimum);
//	*max = atoi(maximum);
//	STUNMaxiKeepAlive = *max;
//	STUNMiniKeepAlive = *min;
	VPrint("-->\n");
	VPrint("min = %d max = %d\n",*min, *max);
	
	if (*min == *max) 
	{
		*timeout = *max;
	}
	else
	{
		binarySearch(timeout, min, max);
	}
	VPrint("<--\n");
}

static void*
localIpChanged(void *args)
{ 
	while(NATDetected)
	{
		sleep(60 * 2);
		UInt32 ip = 0;
		int ret = getLocalIp(&ip);
		pthread_mutex_lock(&gSendMutex);
		if (SY_SUCCESS == ret && ip != gLocalIp)
		{
			VPrint("local ip changed,send bingding changed\n");			
			stunSaveValue(SY_TRUE, SY_TRUE);
			gBindingChanged = SY_TRUE;
			gLocalIpChanged = SY_TRUE;
			if (gStunUserName.sizeofValue > 0 && gStunPassword.sizeofValue > 0)
			{
				stunNatType(&gDstServer, SY_NAT_PRIMARY_PORT, gBindingChanged);		
			}
			else
			{
				stunActiveNofify();
			}
		}
		else
		{
			gBindingChanged = SY_FALSE;
		}
		pthread_mutex_unlock(&gSendMutex);
	}
	return NULL;
}

static void
createLocalIpThread()
{
	int ret = pthread_create(&gLocalIpThread, NULL, localIpChanged, NULL);
	if (SY_SUCCESS != ret)
	{
		EPrint("thread create failed\n");
	} else {
		pthread_detach(gLocalIpThread);
	}
}

static void
updateLocalIp()
{
	DO;

	if (gLocalIpThread == 0)
	{
		DPrint("gLocalIpThread == 0 create thread\n");
		createLocalIpThread();
		return;
	}

	//以下代码仅仅判断线程是否还存在，并不是去杀掉线程
	int kill_rc = pthread_kill(gLocalIpThread, 0);
	if(kill_rc == ESRCH)
	{
		DPrint("the specified thread did not exists or already quit\n");
		createLocalIpThread();	
	}
	else if(kill_rc == EINVAL)
	{
		EPrint("signal is invalid\n");
	}
	else
	{
		DPrint("the specified thread is alive\n");
	}

}

static void*
maintainBind(void *args)
{ 
	UInt32 timeout = 0;
	UInt32 min = STUNMiniKeepAlive;//10
	UInt32 max = STUNMaxiKeepAlive;//180
	/*
	UInt32 Rmin = min;
	UInt32 Rmax = max;
	
	UInt32 Count = 0;
	UInt32 minCount = 0;
	*/
	int ret = 0;

	while (1)
	{
		switch (gModeState)
		{
			case DISCOVERYMODE:
				
				discoveryTimeout(&timeout, &min, &max);
				
				DPrint("discovery timeout time %d\n",timeout);

				if ((timeout > 0) | (min != max))
				{
					gModeState = MONITORMODE;
				}
				break;
			//开始走这里
			case MONITORMODE:
				if (STUNMiniKeepAlive == STUNMaxiKeepAlive)
					timeout = STUNMiniKeepAlive;
				//timeout = Rmin;//wxb 20150812
				pthread_mutex_lock(&gSendMutex);
				//只有在本地ip改变或者映射的公网ip改变时， SY_TRUE == gBindingChanged
				if (STUNMiniKeepAlive != STUNMaxiKeepAlive && SY_TRUE == gBindingChanged)
				{
					//VPrint("gBindingChanged is true\n");
					if (SY_TRUE == gLocalIpChanged || (timeout != 0 && !gFirstBindChange))
					{
						VPrint("Time out or IP be changed.\n");
						timeout = 0;
						min = STUNMiniKeepAlive;
						max = STUNMaxiKeepAlive;
						gModeState = DISCOVERYMODE;
						gBindingChanged = SY_FALSE;
						gLocalIpChanged = SY_FALSE;
						break;
					}
					else if (SY_TRUE == gFirstBindChange)
					{
						VPrint("gFirstBindChange is true\n");
						gFirstBindChange = SY_FALSE;
					}
					gBindingChanged = SY_FALSE;
				}
				DPrint("send binding request interval: %d,"
						"previous NATDetected: %d\n",
						timeout, NATDetected);

				ret = sendBindReqPeriod(timeout);
				pthread_mutex_unlock(&gSendMutex);

				DPrint("is NAT network: %d", ret);
				if (SY_TRUE == ret)
				{
					NATDetected = SY_TRUE;
					gNATFlag = SY_TRUE;
					updateLocalIp();

					if (0 == timeout)
					{
						gModeState = DISCOVERYMODE;
					}
				}
				else
				{
					NATDetected = SY_FALSE;
					timeout = 0;
					min = STUNMiniKeepAlive;
					max = STUNMaxiKeepAlive;
					if (gNATFlag == -1 || gNATFlag == SY_TRUE){
						VPrint("stunActiveNofify");
						gNATFlag = SY_FALSE;
						stunSaveValue(SY_FALSE, SY_FALSE);
						stunActiveNofify();
					}
				}
				break;

			default:
				EPrint("invalid mode state\n");
				break;
		}
	}
	return NULL;
}

static int
initGlobalParam()
{
	gPriFd = openPort(SY_NAT_PRIMARY_PORT, 0);
	gSecFd = openPort(SY_NAT_SECOND_PORT, 0);
	gLocalMsgFd = INVALID_SOCKET;

	char minimum[16] = {0};
	char maximum[32] = {0};
	SyGetNodeValue("Device.ManagementServer.STUNMaximumKeepAlivePeriod", maximum);
	SyGetNodeValue("Device.ManagementServer.STUNMinimumKeepAlivePeriod", minimum);
	STUNMiniKeepAlive = atoi(minimum);
	STUNMaxiKeepAlive = atoi(maximum);
	gFuzzy = (STUNMaxiKeepAlive - STUNMiniKeepAlive <= 50) ? 10 : 30;

	char limit[16] = {0};
	SyGetNodeValue("Device.ManagementServer.UDPConnectionRequestAddressNotificationLimit", limit);
	gNotificationLimit = atoi(limit);

	getLocalIp(&gLocalIp);
	VPrint("gLocalIp = %u\n",gLocalIp);
	NATDetected = -1;

	gStunUserName.sizeofValue = 0;
	gStunPassword.sizeofValue = 0;
	return 0;
}

int
isBehindNAT()
{
	assert( sizeof(UInt8) == 1);
	assert( sizeof(UInt16) == 2);
	assert( sizeof(UInt32) == 4);
 
	int stunEnable = SY_FALSE;
	char enable[8] = {0};

	SyGetNodeValue("Device.ManagementServer.STUNEnable", enable);
	DPrint("STUNEnable value is: %s\n", enable);
	if (0 != strlen(enable))
	{
		if (0 == strcmp(enable, "true"))
		{
			stunEnable = SY_TRUE;
		}
		else if (1 == strlen(enable))
		{
			stunEnable = atoi(enable);
		}
		else
		{
			stunSaveValue(SY_FALSE, SY_FALSE);
			stunEnable = SY_FALSE;
		}
	}

	if (SY_TRUE == stunEnable)
	{
		char stunUrl[256] = {0};
		char port[16] = {0};
		UInt32 stunPort = STUN_DEFAULT_PORT;

		SyGetNodeValue("Device.ManagementServer.STUNServerAddress", stunUrl);
		SyGetNodeValue("Device.ManagementServer.STUNServerPort", port);
		if (0 == strlen(stunUrl))
		{
			SyGetNodeValue("Device.ManagementServer.URL", stunUrl);
		}

		if (0 != strlen(port))
		{
			stunPort = atoi(port);
		}
		return discoveryNAT(stunUrl, stunPort);
	}

	return SY_FALSE;
}

int
discoveryNAT(char* stunUrl, int stunPort)
{
	assert(strlen(stunUrl) != 0);
	assert(stunPort != 0);

	StunAddress4 stunServerAddr;
	stunServerAddr.addr = 0;
	struct in_addr sin_addr;

	memset(&sin_addr, 0, sizeof(sin_addr));  

	//if (0 == inet_aton(stunUrl, &sin_addr))
	//{
	int ret = stunParseSrvName(stunUrl, &stunServerAddr, stunPort);
	if (SY_SUCCESS != ret)
	{
		EPrint("server host is invalid");
		return SY_FAILED;
	}
	//}
	//else
	//{
	//	VPrint("inet_aton transform url success\n");
	//	stunServerAddr.addr = ntohl(sin_addr.s_addr);
	//	stunServerAddr.port = stunPort;
	//}

	gDstServer = stunServerAddr;

//	if (0 == srcPort)
//	{
//		srcPort = stunRandomPort();
//	}

	VPrint("stun server addr:%x  port:%d\n", stunServerAddr.addr, stunServerAddr.port);
	return stunBindDisc(&stunServerAddr);
}

int
sendBindReqPeriod(unsigned int timeout)
{
	int ret = isBehindNAT();

	if (0 != timeout)
	{
		gNetChangFlag = 0;
		gDiscoveryPhase = SY_FALSE;
		int sleepCount = 600;
		while (sleepCount-- > 0 && gNetChangFlag == 0){
			usleep(100 * 1000);
		}
		gNetChangFlag = 0;
		//sleep(60);
	}
	else {
		VPrint("gDiscoveryPhase = true\n");
		gDiscoveryPhase = SY_TRUE;
	}
	return ret;
}

void
traverseNAT()
{ 
	pthread_t maintain;
	DO;
	EPrint("enter traverseNAT\n");
	initGlobalParam();

	// 接收acs发过来的信息
	stunRecvMsgThd();

	// 保活线程
	int ret = pthread_create(&maintain, NULL, maintainBind, NULL);
	if (SY_SUCCESS != ret)
	{
  		EPrint("thread create failed\n");
  	} else {
  		pthread_detach(maintain);
	}
	DONE;
}





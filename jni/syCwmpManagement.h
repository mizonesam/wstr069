/***********************************************************************
*
* syCwmpManagement.h
*
* Copyright (C) 2012 Lin jieling <mycourage0@126.com>
* Copyright (C) 2003-2012 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/
#ifndef SYCWMPMANAGEMENT_H
#define SYCWMPMANAGEMENT_H

//#define SY_CWMP_CLIENT_PORT         22344
#define SY_CWMP_CLIENT_PORT          6547


#define SY_CPE_REQUEST_ID_LENGTH    16
#define SY_BUFFER_LENGTH            512

#define SY_MAX_ENVELOPES            1

#define SY_ACS_NO_CONTACTED         0
#define SY_ACS_CONTACTED            1
#define SY_ACS_VALUE_CHANGED        2
#define SY_ACS_UPDATE_SUCCESS       3
#define SY_ACS_UPDATE_FAIL          4

#define SY_PARAM_INFORM             0
#define SY_PARAM_DIAGNOSTICS        1
#define SY_PARAM_PERIODIC           2
#define SY_PARAM_VALUE_CHANGED      3
#define SY_PARAM_ERROR_CODE      	4


#define SY_CPE_NOT_INITIALIZED      0
#define SY_CPE_INITIALIZED          1

enum syInformParam
{
    SY_INFORM_DEVICE_SUMMARY = 0,             //Device.DeviceSummary
    SY_INFORM_MODE_NAME,                      //Device.DeviceInfo.ModelName
    SY_INFORM_DESCRIPTION,                    //Device.DeviceInfo.Description
    SY_INFORM_UPTIME,                         //Device.DeviceInfo.UpTime
    SY_INFORM_FIRST_USEDATE,                  //Device.DeviceInfo.FirstUseDate
    SY_INFORM_HARDWARE_VERSION,               //Device.DeviceInfo.HardwareVersion
    SY_INFORM_SOFTWARE_VERSION,               //Device.DeviceInfo.SoftwareVersion
    SY_INFORM_ADDHARDWARE_VERSION,            //Device.DeviceInfo.AdditionalHardwareVersion
    SY_INFORM_ADDSOFTWARE_VERSION,            //Device.DeviceInfo.AdditionalSoftwareVersion
    SY_INFORM_CONNECTION_REQUEST_URL,		  //Device.ManagementServer.ConnectionRequestURL
    SY_INFORM_PARAMETER_KEY,				  //Device.ManagementServer.ParameterKey
    SY_INFORM_IPADDRESS,					  //Device.LAN.IPAddress
    SY_INFORM_MACAddress,					  //Device.LAN.MACAddress
    SY_INFORM_STBID,
    SY_INFORM_USER_ID,						  //Device.X_CTC_IPTV.ServiceInfo.UserID
    SY_INFORM_AUTHURL,                        //Device.X_CTC_IPTV.ServiceInfo.AuthURL
    SY_INFORM_PPPOE_ID,
    SY_INFORM_ISENCRYMARK,

};

typedef enum _sySessionTriggerType
{
    SY_TRIGGER_NONE                         = 0x0000,
    SY_TRIGGER_BOOTSTRAP                    = 0x0001,
    SY_TRIGGER_BOOT                         = 0x0002,
    SY_TRIGGER_PERIODIC                     = 0x0004,
    SY_TRIGGER_VALUE_CHANGE                 = 0x0008,
    SY_TRIGGER_CONNECTION_REQUEST           = 0x0010,
    SY_TRIGGER_TRANSFER_COMPLETE            = 0x0020,
    SY_TRIGGER_DIAGNOSTICS_COMPLETE         = 0x0040,
    SY_TRIGGER_REQUEST_DOWNLOAD             = 0x0080,
    SY_TRIGGER_AUTONOMOUS_TRANSFER_COMPLETE = 0x0100,
    SY_TRIGGER_REBOOT                       = 0x0200,
    SY_TRIGGER_DOWNLOAD                     = 0x0400,
    SY_TRIGGER_UPLOAD                       = 0x0800,
    SY_TRIGGER_X_CTC_SHUT_DOWN              = 0x1000,

} sySessionTriggerType;

typedef enum
{
    SY_EVENT_BOOTSTRAP,
    SY_EVENT_BOOT,
    SY_EVENT_PERIODIC,
    SY_EVENT_VALUE_CHANGE,
    SY_EVENT_CONNECTION_REQUEST,
    SY_EVENT_TRANSFER_COMPLETE,
    SY_EVENT_DIAGNOSTICS_COMPLETE,
    SY_EVENT_REQUEST_DOWNLOAD,
    SY_EVENT_AUTONOMOUS_TRANSFER_COMPLETE,
    SY_EVENT_REBOOT,
    SY_EVENT_DOWNLOAD,
    SY_EVENT_UPLOAD,
    SY_EVENT_X_CTC_SHUT_DOWN,
    SY_EVENT_CTC_LOG_PERIODIC,
    SY_EVENT_X_00E0FC_ErrorCode,

} syInformEventType;

typedef struct _Event {
	syInformEventType num;
	char EventStr[64];
	char CommandKey[64];
} EVENT_MOD_t;

/*State Between CPE and ACS*/
typedef enum _sySessionState
{
    SY_SESSION_IDLE,
    SY_SESSION_AWAITED,
    SY_SESSION_UP
} sySessionState;

typedef enum _syCpeAcsEvent
{
    SY_EVENT_NONE                   = 0x00,
    SY_EVENT_START                  = 0x01,
    SY_EVENT_CONFIGURED_ACS_URL     = 0x02,
    SY_EVENT_RECV_INFORM_RESPONSE   = 0x04,
    SY_EVENT_M_REBOOT               = 0x08,
    SY_EVENT_VALUSE_CHANGED         = 0x10,
    SY_EVENT_DOWNLOAD_COMPLETED     = 0x20,
    SY_EVENT_SESSION_TIMEOUT        = 0x40,
} syCpeAcsEvent;

typedef enum _syCpeStateFlag
{
    SY_STATE_NONE,

    SY_STATE_BOOT,
    SY_STATE_BOOTSTRAP,
    SY_STATE_FACTORYRESET,
    SY_STATE_VALUE_CHANGED,
    SY_STATE_PERIODIC,

    SY_STATE_HOLD_REQUEST,
    SY_STATE_NO_MORE_REQUEST,

    SY_STATE_REBOOT_PENDING,
    SY_STATE_STOP_PENDING,

    SY_STATE_SESSION_INITIATE,
    SY_STATE_FIRST_SESSION,
    SY_STATE_SESSION_TEARDOWN,

    SY_STATE_RESET_TRIGGERSET,

    SY_STATE_DOWNLOAD_PENDING,
    SY_STATE_DOWNLOAD_CONFIGURATION_COMPLETE,
    SY_STATE_DOWNLOAD_FIRMWARE_COMPLETE,
    SY_STATE_RESET_VALID_DOWNLOAD,

    SY_STATE_UPLOAD_PENDING,
    SY_STATE_UPLOAD_COMPLETE,
    SY_STATE_RESET_VALID_UPLOAD,

    SY_STATE_CONNECTREQURL,

} syCpeStateFlag;

typedef struct _syDeviceInfoStruct
{
    char* DeviceSummary;               //Device.DeviceSummary

    char* ModelName;                   //Device.DeviceInfo.ModelName
    char* Description;                 //Device.DeviceInfo.Description
    char* HardwareVersion;             //Device.DeviceInfo.HardwareVersion
    char* SoftwareVersion;             //Device.DeviceInfo.SoftwareVersion
    char* AdditionalHardwareVersion;   //Device.DeviceInfo.AdditionalHardwareVersion
    char* AdditionalSoftwareVersion;   //Device.DeviceInfo.AdditionalSoftwareVersion
    char* ProvisioningCode;            //Device.DeviceInfo.ProvisioningCode
    char* UpTime;                      //Device.DeviceInfo.UpTime
    char* FirstUseDate;                //Device.DeviceInfo.FirstUseDate
} syDeviceInfoStruct;

typedef struct _syManagementServerStruct
{
    char* URL;                          //Device.ManagementServer.URL
    char* ParameterKey;                 //Device.ManagementServer.ParameterKey
    char* ConnectionRequestURL;         //Device.ManagementServer.ConnectionRequestURL
} syManagementServerStruct;

typedef struct _syLANStruct
{
    char* AddressingType;               //Device.LAN.AddressingType
    char* IPAddress;                    //Device.LAN.IPAddress
    char* MACAddress;                   //Device.LAN.MACAddress
    char* DNSServers;                   //Device.LAN.DNSServers
} syLANStruct;

typedef struct _syTimeStruct
{
    char* NTPServer1;                    //Device.Time.NTPServer1
    char* LocalTimeZone;                 //Device.Time.LocalTimeZone
} syTimeStruct;

typedef struct _syServiceInfoStruct
{
    char* STBID;                         //Device.X_CTC_IPTV.STBID
    char* PPPoEID;                       //Device.X_CTC_IPTV.ServiceInfo.PPPoEID
    char* UserID;                        //Device.X_CTC_IPTV.ServiceInfo.UserID
    char* AuthURL;                       //Device.X_CTC_IPTV.ServiceInfo.AuthURL
    char* EpgIP;
} syServiceInfoStruct;

typedef struct _syGlobalParmStruct
{
    int             IsCPEInitialized;
    int             ConnectReqUrlUpdateFlag;
    uint            CurrentRequestId;
    uint            SessionFlags;
    uint            SessionEvent;
    uint            SessionTrigger;
    uint            SessionState;
    char            URL[SY_BUFFER_LENGTH];
    char            Username[SY_BUFFER_LENGTH];
    char            Password[SY_BUFFER_LENGTH];
} syGlobalParmStruct;

typedef struct _syAcsCpeParamStruct
{
    char*       AuthType;
    char*       URL;
    char*       URLBackup;
    char*       Username;
    char*       Password;
    /* ConnectionRequestURL根本没有什么卵用 */
    char*       ConnectionRequestURL;
    char*       ConnectionRequestUsername;
    char*       ConnectionRequestPassword;

    char*       ParameterKey;
    char*       NewParamterKey;
    char*       RebootCommandKey;
    char*       UploadCommandKey;
    char*       DownloadCommandKey;
    char*       ProvisioningCode;
    int         UpgradesManaged;

    int         InformEnable;
    int         InformInterval;
    int         MaxEnvelopes;
    int         RetryCount;
    time_t      NextAcsInformTime;

    int         HoldRequests;
    int         NoMoreRequests;
    int         NoConnectionReqAuth;

    int         FaultCode;
    int         DownloadState;
    char*       DownloadFaultMessage;

    int         UploadState;
    char*       UploadFaultMessage;

    time_t      StartTime;
    time_t      CompleteTime;

    char*       Realm;
    int         IsAcsContacted; //决定了cpe和acs连接的状态
    int         HaveReboot;
} syAcsCpeParamStruct;


/* 初始化soap结构并启动心跳线程和接收线程(ACS发来的http请求) */
bool 
CwmpInit();

/* 初始化xml并启动一个socket server 用于接收外部或者内部的命令(CwmpInit就是从这个server开始的) */
bool 
CwmpMain();

/* 调用soap接口来发送Inform到ACS */
int 
SendInform(struct soap* soap, void* handle);

/* 带soap的strdup */
char *
Strdup(struct soap *soap, char *s);

/* 带soap的malloc */
void *
Malloc(struct soap *soap, int size);

/*  */
LOCAL int 
FreeParaList(void);

/*  */
LOCAL int 
MallocParaList(struct soap* soap, int type, const char* flagFile1, const char* flagFile2, const char* flagFile3);

/*  */
LOCAL int 
ValueChanged(struct soap* soap, void* handle);

/*  */
LOCAL int 
Periodic(struct soap* soap, void* handle);

/*  */
LOCAL int 
LogMsgPeriod(struct soap* soap, void* handle);

/*  */
LOCAL int 
Diagnostics(struct soap* soap, void* handle);

/*  */
LOCAL int 
Inform(struct soap* soap, void* handle);

/*  */
LOCAL char* 
GetNewReqId(struct soap* soap);

/* 填充Event结构体 这个结构体在报文中体现为Event元素.见文件最下方的图表 */
LOCAL void 
CreateInformEvt(struct soap* soap,  void* handle);

/* 填充ParaList结构体 这文件结构体在报文中体现为ParameterList元素.见文件最下方图表 */
LOCAL void 
CreateIfmPara(struct soap* soap,   void* handle);

/* 其实只是调用CreateInformEvt和CreateIfmPara而已.(⊙o⊙)… */
LOCAL void 
CreateInform(struct soap* soap, void* handle);

int HandleCreateInform(void* handle, char* msg);

/* 上报inform并启动心跳线程 */
LOCAL void* 
ClientThread(void* data);

/* 接收(ACS发来的http请求)线程 */
LOCAL void* 
ServerThread(void* data);

/* 获取每次开机只需要读取一次的值 */
LOCAL bool 
GetDeviceInfo();

/* 其实只是调用GetDeviceInfo而已.(⊙o⊙)… */
LOCAL bool 
GlobalParamInit();

/* 获取一些需要经常更新的值的值 */
LOCAL bool 
PramUpdate();

/* 没卵用 */
LOCAL bool 
Initialize(struct soap* soap);

/* send inform 之后需要接受ACS返回的数据
 * 可以把SendInform理解为SendRPC.
 * 在tr069协议中CPE可以调用ACS的方法,反之也行.
 * 但是一般情况下CPE只调用ACS众多RPC方法中的Inform方法
 * 而ACS几乎会调用CPE的所有方法
 * 所以这里用RecvRPC来接受所有可能被调用的方法的请求
 */
LOCAL int 
RecvRPC(struct soap* soap);

/* ProcThread线程中用来处理各种请求的方法 (CwmpInit就是其中之一) */
LOCAL int 
DispatchReq(struct soap* soap);

/* 发起一个EVENT为 6 CONNECTION 的 Inform到ACS
 * ACS回复InformResponse之后,再发送一个空包给ACS
 * ACS就掌握了主动权,然后ACS想download就download
 * 想Setparameter就Setparameter
 */
LOCAL int 
StartSession(struct soap* soap);

/* 发送一个空包到ACS,使ACS获得主动权 */
LOCAL int 
SendEmptyPost(struct soap* soap);

/* 发送心跳Inform到ACS */
LOCAL void 
SendHeartIfm(struct soap* soap);

/* 没卵用 */
LOCAL void 
Run(struct soap *soap);

/* 没卵用 */
LOCAL void 
InitAuthProto(struct soap *soap);

/* 循环检测一些标志文件,当检测变化时上报对应的Inform */
LOCAL void *
ProcIfmThread(void* data);
#if 0
/* 实际执行检测结果的函数 */
LOCAL void 
HandleInform(struct soap* soap , char* inform);
#else
void 
HandleInform(void* inform, int len);
#endif
/* 没卵用 */
LOCAL bool 
AcsConnInit(struct soap *soap);

/*  */
LOCAL int 
GetInformNum(const char * FilePath);

/*  */
LOCAL int 
AddIfmParam(struct soap* soap, const char* flagFile);

/* 当CPE的RPC被调用失败时执行该函数以回复ACS调用结果 */
LOCAL int 
SendFaultResp(struct soap *soap);

/*  */
LOCAL int 
FreeFault(void);

/* 零配置处理函数 */
LOCAL int 
HandleZeroCfg(struct soap *soap);

/* 这是一个完整的cpe(盒子)报文结构
 * <?xml version="1.0" encoding="UTF-8"?>  // xml header
 * Envelope                                // 封包
 *     Header                              // header
 *     Body                                // 实体
 *         Inform                          // inform结构 <-- SendInform 函数 主要功能就是把这个结构打包发送给ACS(服务端)
 *             DeviceId                    // SendInform 函数参数之一
 *             Event                       // Event结构 <-- CreateInformEvt 函数主要功能就是填充这个结构
 *             MaxEnvelopes                // SendInform 函数参数之一
 *             CurrentTime                 // SendInform 函数参数之一
 *             RetryCount                  // SendInform 函数参数之一
 *             ParameterList               // ParameterList结构 <-- CreateIfmPara 函数主要功能就是填充这个结构
 */

int UpdateNetPara(void);

#endif

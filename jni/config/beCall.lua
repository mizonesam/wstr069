package.path = "/data/?.lua;/data/jit/?.lua"..package.path
local ffi = require("ffi")
ffi.cdef[[
typedef struct 
{
	int a;
	char* s; 
}stru1;

typedef struct 
{
	int a;
	stru1* s; 
}stru2;

typedef enum {
    SY_EVENT_BOOTSTRAP = 0,					
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
    SY_EVENT_X_CT_COM_BIND					
} syInformEventType;

typedef struct cwmp__EventStruct
{
	char *EventCode;	
	char *CommandKey;	
} cwmp_ES_t;

typedef struct _EventStruct
{
	struct cwmp__EventStruct *__ptrEventStruct;
	int __size;
}_EventStruct;

typedef struct {
    char* AddressingType;              
    char* IPAddress;                   
    char* MACAddress;                  
    char* DNSServers;                  
} syLANStruct;

typedef struct cwmp__ParameterValueStruct
{
	char *Name;	/* required element of type xsd:string */
	char *Value;	/* required element of type xsd:string */
	char *Type;	/* required element of type xsd:string */
} cwmp_PVS_t;

struct _ParameterValueStruct
{
	struct cwmp__ParameterValueStruct *__ptrParameterValueStruct;
	int __size;
};

typedef struct _syManagementServerStruct {
    char* URL;                          
    char* ParameterKey;                 
    char* ConnectionRequestURL;         
} syManagementServerStruct;

typedef struct _Event {
	syInformEventType num;
	char EventStr[64];
	char CommandKey[64];
} EVENT_MOD_t;

typedef struct _syServiceInfoStruct {
    char* STBID;                         //Device.X_CTC_IPTV.STBID
    char* PPPoEID;                       //Device.X_CTC_IPTV.ServiceInfo.PPPoEID
    char* UserID;                        //Device.X_CTC_IPTV.ServiceInfo.UserID
    char* AuthURL;                       //Device.X_CTC_IPTV.ServiceInfo.AuthURL
} syServiceInfoStruct;

typedef struct _syBMinputParamStruct {
	char* UserID;
	char* UserName;
} syBMinputParamStruct;

typedef struct {
    int    cmd;
    int    attr;         //0/1
    char*  key;
} xml_key_path_t;

typedef struct _syAcsCpeParamStruct {
    char*       AuthType;
    char*       URL;
    char*       URLBackup;
    char*       Username;
    char*       Password;
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
    long int    NextAcsInformTime;
    int         HoldRequests;
    int         NoMoreRequests;
    int         NoConnectionReqAuth;
    int         FaultCode;
    int         DownloadState;
    char*       DownloadFaultMessage;
    int         UploadState;
    char*       UploadFaultMessage;
    long int    StartTime;
    long int    CompleteTime;
    char*       Realm;
    int         IsAcsContacted; 
    int         HaveReboot;
} syAcsCpeParamStruct;

enum syInformParam {
    SY_INFORM_DEVICE_SUMMARY = 0,
    SY_INFORM_MODE_NAME,                     
    SY_INFORM_DESCRIPTION,                   
    SY_INFORM_UPTIME,                        
    SY_INFORM_FIRST_USEDATE,                 
    SY_INFORM_HARDWARE_VERSION,              
    SY_INFORM_SOFTWARE_VERSION,              
    SY_INFORM_ADDHARDWARE_VERSION,           
    SY_INFORM_ADDSOFTWARE_VERSION,           
	SY_INFORM_CONNECTION_REQUEST_URL,		 
	SY_INFORM_PARAMETER_KEY,				 
	SY_INFORM_IPADDRESS,					 
	SY_INFORM_MACAddress,					 
	SY_INFORM_STBID,   
	SY_INFORM_USER_ID,						 
	SY_INFORM_AUTHURL,                       
	SY_INFORM_PPPOE_ID,   
	SY_INFORM_AddressingType,
    SY_INFORM_ISENCRYMARK,
    // SY_INFORM_CODE,
    // SY_INFORM_NTPSERVER,
    // SY_INFORM_DNSSERVER,
};

typedef enum
{
    /*Network*/
    CONNECT_TYPE = 1,             //网络连接方式
    DHCP_USER = 2,                //DHCP用户名
    DHCP_PASSWD = 3,              //DHCP密码
    PPPOE_USER = 4,               //PPPOE用户名
    PPPOE_PASSWD = 5,             //PPPOE密码
    IPADDR = 6,	                  //IP地址
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


    LOGTYPE = 199,
    LOGLEVEL,
    LOGOUTPUTTYPE,
    SYSLOGSERVER,
    SYSLOGSTARTTIME,
    SYSLOGCONTINUETIME,
    SYSLOGTIMER,
    SYSLOGFTPSERVER,

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

    //AutoOnOffConfiguration
    IS_AUTO_POWERON = 522,
    AUTO_POWERON_TIME = 523,
    AUTO_SHUTDOWN_TIME = 524,

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

typedef struct _syDeviceInfoStruct {
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

typedef struct
{
    int    cmd;
    char*  cmdStr;
} syCmdStrStu;

void DPrintL(char* log);
bool GetValue(const char* path, char* value, size_t sizeOfValue);
void free(void *ptr);
]]

tr069_so = ffi.load("/system/lib/libcwmp.so")

--以下代码仅为简化函数的书写
fs = ffi.string
fn = ffi.new
fc = ffi.copy
fC = ffi.C


if jit then
    print('close jit')
    jit.off()
    jit.flush()
end
function mallocStr(str)
    s = fn('char[?]', #fs(str)+1, fs(str))
    return s
end

function toChar(str)
    s = ffi.cast('char*', fs(str))
    return s
end

function DPrint(s)
    d = debug.getinfo(2)
    s = string.format("[%s]%s", d.name, s)
    tr069_so.DPrintL(ffi.cast('char*', s))
end


--要额外添加的节点，加到下面
ParamList = {
    "Device.ManagementServer.URL",
    "Device.X_00E0FC.STBID",
    "Device.LAN.MACAddress",
}

--事件类型和个数
EventCode = {
    ['0'] = {'0 BOOTSTRAP'},
    ['1'] = {'1 BOOT', '4 VALUE CHANGE'},
    ['2'] = {'2 PERIODIC'}
}

--盒子自身获取值（getValue）
Node_Value = {
    ["Device.ManagementServer.URL"]     =   "http://125.88.87.18:37021/acs",
    ["Device.X_00E0FC.STBID"]           =   "00100399010004800026A8BD3A5B258F",
    ["Device.LAN.MACAddress"]           =   "00:19:F3:FF:F0:F1",
}

--服务器设置值
SetparamMap = {
    ["Device.ManagementServer.URL"]     =   "http://125.88.87.18:37021/acs",
    ["Device.X_00E0FC.STBID"]           =   "00100399010004800026A8BD3A5B258F",
    ["Device.LAN.MACAddress"]           =   "00:19:F3:FF:F0:F1",
}

function updateParam(gSyParamList1)
    DPrint('call Param')
    local gSyParamList = ffi.cast('struct _ParameterValueStruct*', gSyParamList1)
    local ParamListSize = #ParamList
    local OldSize = gSyParamList.__size

    if ParamListSize ~= 0 then
        DPrint('ParamListSize is NULL')
        local ParamValueTmp = fn('struct cwmp__ParameterValueStruct[?]', gSyParamList.__size + ParamListSize)
        
        DPrint('deep copy size is '..gSyParamList.__size + ParamListSize)
        --因为参数gSyParamList1是指针，需要深度复制
        for i=0,gSyParamList.__size-1 do
            ParamValueTmp[i].Name = toChar(gSyParamList.__ptrParameterValueStruct[i].Name)
            --DPrint('copy '..fs(gSyParamList.__ptrParameterValueStruct[i].Name))
            ParamValueTmp[i].Value = toChar(gSyParamList.__ptrParameterValueStruct[i].Value)
            --DPrint('->'..fs(gSyParamList.__ptrParameterValueStruct[i].Value))
        end
        DPrint('copy new param')
        for k,v in pairs(ParamList) do
            ParamValueTmp[k+OldSize-1].Name = toChar(v)
            NewValue = fn('char[?]', 256)   --
            tr069_so.GetValue(toChar(v), NewValue, 256)
            --DPrint(fs(NewValue))
            ParamValueTmp[k+OldSize-1].Value = NewValue  
        end
        DPrint('copy __ptrParameterValueStruct')
        ffi.copy(gSyParamList.__ptrParameterValueStruct, ParamValueTmp, ffi.sizeof(ParamValueTmp))
    end
    gSyParamList.__size = gSyParamList.__size + ParamListSize
    ffi.copy(gSyParamList1, gSyParamList, ffi.sizeof(gSyParamList))
end

function updateEvent(nType, gSyEvent1, gSyLANStu1, gSyManagementServerStu1, gsyAcsCpeParamStru1)
    DPrint('call updateEvent nType is '..nType)
    local gSyEvent = ffi.cast('struct _EventStruct*', gSyEvent1)
    local eventId = tostring(nType)
    DPrint('eventid is '..eventId)
    local eventArray = EventCode[eventId]
    DPrint(eventArray)
    if(eventArray ~= nil) then
        eventNum = #eventArray
        DPrint('eventArray is '..eventNum)
        if(eventNum ~= gSyEvent.__size) then
            ffi.C.free(gSyEvent.__ptrEventStruct)
            gSyEvent.__ptrEventStruct = ffi.new("struct cwmp__EventStruct[?]", eventNum)
            DPrint('new __ptrEventStruct')
            for i=0,eventNum-1 do
                gSyEvent.__ptrEventStruct[i].EventCode = toChar(EventCode[eventId][i+1])
                gSyEvent.__ptrEventStruct[i].CommandKey = toChar('')
            end
            DPrint('copy ok')
            gSyEvent.__size = eventNum
        end
    end
    ffi.copy(gSyEvent1, gSyEvent, ffi.sizeof(gSyEvent))
end

function getvalue(path, value, sizeOfValue)
    local key = fs(path)
    DPrint('call getvalue path is '..key)
    local v1 = fs(value)
    local v = Node_Value[key]
    if(v == nil) then
        DPrint('map is nil')
        return
    end
    
    if(v ~= v1) then
        ffi.copy(value, v)
    end
end

function setParam()
    
end



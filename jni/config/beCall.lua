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
const int SY_WRITE_END;
bool GetDataFromJava(const char* pszKey, char* pszValue, size_t sizeOfValue);
bool SetDataToJava(const char* pszKey, char* pszValue);
]]
tr069_so = ffi.load("/system/lib/libcwmp.so")
g_getData = tr069_so.GetDataFromJava
g_setData = tr069_so.SetDataToJava
fs = ffi.string
fn = ffi.new
fc = ffi.copy
fca = ffi.cast
fC = ffi.C
function mallocStr(str)
    s = fn('char[?]', #fs(str)+1, fs(str))
    return s
end
function toChar(str)
    s = ffi.cast('char*', fs(str))
    return s
end
g_keyList = ffi.new("xml_key_path_t[?]", 66, {
    {fC.CONNECT_TYPE,   				1,  toChar("ConnectMode")},
    {fC.DHCP_USER,      				1,  toChar("DHCPUserName")},
    {fC.DHCP_PASSWD,    				1,  toChar("DHCPPassword")},
    {fC.PPPOE_USER,     				1,  toChar("PPPOEUserName")},
    {fC.PPPOE_PASSWD,   				1,  toChar("PPPOEPassword")},
    {fC.IPADDR,         				1,  toChar("IpAddress")}, 
    {fC.MASK,           				1,  toChar("NetMask")},
    {fC.DEFGATEWAY,     				1,  toChar("DefaultGate")},
    {fC.DNSMAIN,        				1,  toChar("DNS")},
    {fC.DNSBAK,         				1,  toChar("SecondNDS")},
    {fC.MAC,            				0,  toChar("Mac")},
    {fC.MODELNAME,            		    0,  toChar("ModelName")},
    {fC.Manufacturer,            	    0,  toChar("Manufacturer")},
    {fC.ProductClass,            	    0,  toChar("ProductClass")},
	{fC.PLAYURL, 					    1,  toChar("PlayUrl")},
    {fC.REMOTEDUG, 				        1,  toChar("RemoteControlEnable")},
    {fC.PLAYSTATE,					    0,  toChar("PlayUrl")},
    {fC.TIMEZONE,       				1,  toChar("TimeZone")},
    {fC.NTPMAIN,        				1,  toChar("ServerNTPUrl")},  
    {fC.NTPBAK,         				1,  toChar("ServerNTPBackupUrl")},
	{fC.FORCEUPGRADE, 			        1,  toChar("ForceUpgrade")},               				
    {fC.AUTH_MAINURL,   				1,  toChar("IPTVauthURL")},
    {fC.AUTH_BAKURL,    				1,  toChar("SecondIPTVauthURL")},
    {fC.IPTVUSER,       				1,  toChar("IPTVaccount")},
    {fC.IPTVPASSWD,     				1,  toChar("IPTVpassword")},
    {fC.MANUFACTORY,    				0,  toChar("Manufacturer")},   
    {fC.STBID,          				0,  toChar("STBID")},
    {fC.HARDWARE_VER,   				0,  toChar("HardwareVersion")},
    {fC.SOFTWARE_VER,   				0,  toChar("SoftwareVersion")},
    {fC.LANGUAGE,       				1,  toChar("ChooseLanguage")},
	{fC.SQMSERVERURL,				    1,  toChar("MQMCURL")},
	{fC.EPGSERVERURL,                   0,  toChar("ServerBaseUrl")},
    {fC.MANAGESERVER_URL,   		    1,  toChar("config_WebmasterUrl")},
    {fC.MANAGESERVER_USER,  		    1,  toChar("config_UserName")},
    {fC.MANAGESERVER_PASSWD,		    1,  toChar("config_Password")},
    {fC.HEARTBEAT,          		    1,  toChar("Congfig_HeartBear")},
    {fC.HEARTBEAT_INTERVAL, 	        1,  toChar("config_HeartBeat")},
    {fC.CPEUSER,            		    1,  toChar("config_CPEUser")},
    {fC.CPEPASSWD,          		    1,  toChar("config_CPEPassword")},
    {fC.SUPPORTPROTOCOLS,               0,  toChar("config_SupportProtocols")},
    {fC.TRANSPORTPROTOCOLS,             0,  toChar("config_TransportProtocols")},
    {fC.TRANSPORTCTLPROTOCOLS,          0,  toChar("config_TransportCTLProtocols")},
    {fC.PLEXTYPE,                       0,  toChar("config_Plextype")},
    {fC.DEJITTERBUFFERSIZE,             0,  toChar("config_DejitterBufferSize")},
    {fC.VIDEOSTANDARDS,                 0,  toChar("config_VideoStandards")},
    {fC.QOSSERVERURL,                   1,  toChar("config_QosServerUrl")},
    {fC.QOSUPLOADINTERVAL,              1,  toChar("config_LoginTerval")},
    {fC.RECORDINTERVAL,                 1,  toChar("config_RecordInterval")},
    {fC.MONITORINGINTERVAL,             1,  toChar("config_MonitoringInterval")},                                      
    {fC.LOGSWITCH,                      1,  toChar("LogSwitch")},
	{fC.LOGSERVERURL,                   1,  toChar("config_LogServerUrl")},
	{fC.LOGSERVERUSER,                  1,  toChar("LogserverUser")},
	{fC.LOGSERVERPASSWD,                1,  toChar("LogserverPassword")},
	{fC.LOGDURATION,				    1,  toChar("Logduration")},
	{fC.LOGMSGORFILE,                   1,  toChar("LogMsgOrFile")},
    {fC.LOGTYPE,                        1,  toChar("LogType")},
    {fC.LOGLEVEL,                       1,  toChar("LogLevel")},
    {fC.LOGOUTPUTTYPE,                  1,  toChar("LogOutPutType")},
    {fC.SYSLOGSERVER,                   1,  toChar("SyslogServer")},
    {fC.SYSLOGSTARTTIME,                1,  toChar("SyslogStartTime")},
    {fC.SYSLOGCONTINUETIME,             1,  toChar("SyslogContinueTime")},
    {fC.SYSLOGTIMER,                    1,  toChar("LogTimer")},
    {fC.SYSLOGTIMER,                    1,  toChar("LogFtpServer")},
    {fC.IS_AUTO_POWERON,                1,  toChar("IsAutoPowerOn")},
    {fC.AUTO_POWERON_TIME,              1,  toChar("AutoPowerOnTime")},
    {fC.AUTO_SHUTDOWN_TIME,             1,  toChar("AutoShutdownTime")},
})


gSendEvent = ffi.new("EVENT_MOD_t[?]", 16, {
    {ffi.C.SY_EVENT_BOOTSTRAP,							"0 BOOTSTRAP",							""},
    {ffi.C.SY_EVENT_BOOT,								"1 BOOT", 								""},
    {ffi.C.SY_EVENT_PERIODIC,							"2 PERIODIC",							""},
    {ffi.C.SY_EVENT_VALUE_CHANGE,						"4 VALUE CHANGE",						""},
    {ffi.C.SY_EVENT_CONNECTION_REQUEST,					"6 CONNECTION REQUEST",					""},
    {ffi.C.SY_EVENT_TRANSFER_COMPLETE,					"7 TRANSFER COMPLETE",					""},
    {ffi.C.SY_EVENT_DIAGNOSTICS_COMPLETE,				"8 DIAGNOSTICS COMPLETE",				""},
    {ffi.C.SY_EVENT_REQUEST_DOWNLOAD,					"9 REQUEST DOWNLOAD",					""},
    {ffi.C.SY_EVENT_AUTONOMOUS_TRANSFER_COMPLETE,		"10 AUTONOMOUS TRANSFER COMPLETE",		""},
    {ffi.C.SY_EVENT_REBOOT,								"M Reboot",					            ""},
    {ffi.C.SY_EVENT_DOWNLOAD,							"M Download",							""},
    {ffi.C.SY_EVENT_UPLOAD,								"M Upload",								""},
    {ffi.C.SY_EVENT_X_CTC_SHUT_DOWN,					"M X_CTC_SHUT_DOWN",					""},
    {ffi.C.SY_EVENT_CTC_LOG_PERIODIC,					"M CTC LOG_PERIODIC",					""},
    {ffi.C.SY_EVENT_X_00E0FC_ErrorCode,					"X CTC ErrorCode",						""},
    {ffi.C.SY_EVENT_X_CT_COM_BIND,						"X CT-COM BIND",						""}
    })
    
gSyCmdStrList = ffi.new("syCmdStrStu[?]", 56, {
    {fC.ADD_ROUTE, 	                toChar("Device.AddRoute")},
    {fC.REMOTEDUG,	                toChar("Device.X_00E0FC.RemoteControlEnable")},
    {fC.MODELNAME, 	                toChar("Device.DeviceInfo.ModelName")},
    {fC.Manufacturer, 	            toChar("Device.DeviceInfo.Manufacturer")},
    {fC.ProductClass, 	            toChar("Device.DeviceInfo.ProductClass")},
    {fC.EPG_MODIFY, 		        toChar("Device.ManagementServer.URLModifyFlag")},  
    {fC.DIAGNOSTICSSTATE,           toChar("Device.X_00E0FC.PlayDiagnostics.DiagnosticsState")},
    {fC.PLAYURL, 		            toChar("Device.X_00E0FC.PlayDiagnostics.PlayURL")},
    {fC.PLAYSTATE, 		            toChar("Device.X_00E0FC.PlayDiagnostics.PlayState")},
    {fC.LOGTYPE, 		            toChar("Device.X_00E0FC.LogParaConfiguration.LogType")},
    {fC.LOGLEVEL, 		            toChar("Device.X_00E0FC.LogParaConfiguration.LogLevel")},
    {fC.LOGOUTPUTTYPE, 	            toChar("Device.X_00E0FC.LogParaConfiguration.LogOutPutType")},
    {fC.SYSLOGSERVER, 	            toChar("Device.X_00E0FC.LogParaConfiguration.SyslogServer")},
    {fC.SYSLOGSTARTTIME,            toChar("Device.X_00E0FC.LogParaConfiguration.SyslogStartTime")},
    {fC.SYSLOGCONTINUETIME,         toChar("Device.X_00E0FC.LogParaConfiguration.SyslogContinueTime")},
    {fC.SYSLOGTIMER, 	            toChar("Device.X_00E0FC.LogParaConfiguration.LogTimer")},
    {fC.SYSLOGFTPSERVER,            toChar("Device.X_00E0FC.LogParaConfiguration.LogFtpServer")},
    {fC.CONNECT_TYPE, 	            toChar("Device.LAN.AddressingType")},
    {fC.DHCP_USER, 		            toChar("Device.X_00E0FC.ServiceInfo.DHCPID")},
    {fC.DHCP_PASSWD, 	            toChar("Device.X_00E0FC.ServiceInfo.DHCPPassword")},
    {fC.PPPOE_USER, 	            toChar("Device.X_00E0FC.ServiceInfo.PPPoEID")},
    {fC.PPPOE_PASSWD, 	            toChar("Device.X_00E0FC.ServiceInfo.PPPoEPassword")},
    {fC.IPADDR, 		            toChar("Device.LAN.IPAddress")},
    {fC.MASK, 			            toChar("Device.LAN.SubnetMask")},
    {fC.DEFGATEWAY, 	            toChar("Device.LAN.DefaultGateway")},
    {fC.DNSMAIN, 		            toChar("Device.LAN.DNSServers")},
    {fC.DNSBAK, 		            toChar("Device.LAN.DNSServers2")},
    {fC.MAC, 			            toChar("Device.LAN.MACAddress")},
    {fC.TIMEZONE, 		            toChar("Device.Time.LocalTimeZone")},
    {fC.NTPMAIN, 		            toChar("Device.Time.NTPServer1")},
    {fC.NTPBAK, 		            toChar("Device.Time.NTPServer2")},
    {fC.AUTH_MAINURL, 	            toChar("Device.X_00E0FC.ServiceInfo.AuthURL")},
    {fC.AUTH_BAKURL, 	            toChar("Device.X_00E0FC.ServiceInfo.AuthURLBackup")},
    {fC.IPTVUSER, 		            toChar("Device.X_00E0FC.ServiceInfo.UserID")},
    {fC.IPTVPASSWD, 	            toChar("Device.X_00E0FC.ServiceInfo.UserIDPassword")},
    {fC.SN, 				        toChar("Device.DeviceInfo.SerialNumber")},
    {fC.STBID, 			            toChar("Device.X_00E0FC.STBID")},
    {fC.HARDWARE_VER, 	            toChar("Device.DeviceInfo.HardwareVersion")},
    {fC.SOFTWARE_VER, 	            toChar("Device.DeviceInfo.SoftwareVersion")},
    {fC.MANAGESERVER_URL,           toChar("Device.ManagementServer.URL")},
    {fC.HEARTBEAT, 		            toChar("Device.ManagementServer.PeriodicInformEnable")},
    {fC.HEARTBEAT_INTERVAL,         toChar("Device.ManagementServer.PeriodicInformInterval")},
    {fC.QOSSERVERURL, 	            toChar("Device.X_00E0FC.StatisticConfiguration.LogServerUrl")},
    {fC.QOSUPLOADINTERVAL,          toChar("Device.X_00E0FC.StatisticConfiguration.LogUploadInterval")},
    {fC.RECORDINTERVAL,             toChar("Device.X_00E0FC.StatisticConfiguration.LogRecordInterval")},
    {fC.MONITORINGINTERVAL,         toChar("Device.X_00E0FC.StatisticConfiguration.StatInterval")},
    {fC.LOGMSGORFILE,               toChar("Device.X_CTC_IPTV.LogMsg.MsgOrFile")},
    {fC.LOGSWITCH, 	                toChar("Device.X_CTC_IPTV.LogMsg.Enable")},
    {fC.LOGSERVERURL,               toChar("Device.X_CTC_IPTV.LogMsg.LogFtpServer")},
    {fC.LOGSERVERUSER,              toChar("Device.X_CTC_IPTV.LogMsg.LogFtpUser")},
    {fC.LOGSERVERPASSWD,            toChar("Device.X_CTC_IPTV.LogMsg.LogFtpPassword")},
    {fC.LOGDURATION,                toChar("Device.X_CTC_IPTV.LogMsg.Duration")},
    {fC.IS_AUTO_POWERON, 	        toChar("Device.X_00E0FC.AutoOnOffConfiguration.IsAutoPowerOn")},
    {fC.AUTO_POWERON_TIME, 	        toChar("Device.X_00E0FC.AutoOnOffConfiguration.AutoPowerOnTime")},
    {fC.AUTO_SHUTDOWN_TIME,         toChar("Device.X_00E0FC.AutoOnOffConfiguration.AutoShutdownTime")},
    {-1,                            toChar("")}
})
function isHuNan_LTOffice()
    return true
end

function DPrint(s)
    d = debug.getinfo(2)
    s = string.format("[%s]%s", d.name, s)
    tr069_so.DPrintL(ffi.cast('char*', s))
end

if jit then
    print('close jit')
    jit.off()
    jit.flush()
end
function updateEvent(nType, gSyEvent1, gSyLANStu1, gSyManagementServerStu1, gsyAcsCpeParamStru1)

    local gSyEvent = fn('struct _EventStruct')
    local gSyLANStu = ffi.cast('syLANStruct*', gSyLANStu1)
    local gsyAcsCpeParamStru = ffi.cast('syAcsCpeParamStruct*', gsyAcsCpeParamStru1)
    local gSyManagementServerStu = ffi.cast('syManagementServerStruct*', gSyManagementServerStu1)
    DPrint('nType is '..nType)
    evenNum = 1
    local rr = 1
    if nType == fC.SY_EVENT_BOOT then
        DPrint("type is "..gsyAcsCpeParamStru.HaveReboot)
        if gsyAcsCpeParamStru.HaveReboot == 1 then
            DPrint("HaveReboot")
            evenNum  = 2
            gSyEvent.__ptrEventStruct = ffi.new("struct cwmp__EventStruct[?]", evenNum)
            gSyEvent.__ptrEventStruct[0].EventCode = toChar(gSendEvent[nType].EventStr)
            gSyEvent.__ptrEventStruct[0].CommandKey = toChar(gSendEvent[nType].CommandKey)
            gSyEvent.__ptrEventStruct[1].EventCode = toChar(gSendEvent[fC.SY_EVENT_REBOOT].EventStr)
            gSyEvent.__ptrEventStruct[1].CommandKey = toChar(gSendEvent[fC.SY_EVENT_REBOOT].CommandKey)
        else
           DPrint("else HaveReboot")
            if gsyAcsCpeParamStru.IsAcsContacted == 3 then
                DPrint("SY_ACS_UPDATE_SUCCESS")
                evenNum = 3
                gSyEvent.__ptrEventStruct = ffi.new("struct cwmp__EventStruct[?]", evenNum)
                gSyEvent.__ptrEventStruct[0].EventCode = toChar(gSendEvent[nType].EventStr)
                gSyEvent.__ptrEventStruct[0].CommandKey = toChar(gSendEvent[nType].CommandKey)
                gSyEvent.__ptrEventStruct[1].EventCode = toChar(gSendEvent[fC.SY_EVENT_DOWNLOAD].EventStr)
                gSyEvent.__ptrEventStruct[1].CommandKey = toChar(gSendEvent[fC.SY_EVENT_DOWNLOAD].CommandKey)
                gSyEvent.__ptrEventStruct[2].EventCode = toChar(gSendEvent[fC.SY_EVENT_TRANSFER_COMPLETE].EventStr)
                gSyEvent.__ptrEventStruct[2].CommandKey = toChar(gSendEvent[fC.SY_EVENT_TRANSFER_COMPLETE].CommandKey)
            else
                DPrint("else SY_ACS_UPDATE_SUCCESS")
                rr=1
                if rr == 1 then                    
                    evenNum = 2                    
                    gSyEvent.__ptrEventStruct = ffi.new("struct cwmp__EventStruct[?]", evenNum)                    
                    gSyEvent.__ptrEventStruct[0].EventCode = toChar(gSendEvent[nType].EventStr)
                    gSyEvent.__ptrEventStruct[0].CommandKey = toChar(gSendEvent[nType].CommandKey)                   
                    gSyEvent.__ptrEventStruct[1].EventCode = toChar(gSendEvent[fC.SY_EVENT_VALUE_CHANGE].EventStr)
                    gSyEvent.__ptrEventStruct[1].CommandKey = toChar(gSendEvent[fC.SY_EVENT_VALUE_CHANGE].CommandKey)
                    
                else
                    DPrint("not hunan")
                    evenNum = 1
                    gSyEvent.__ptrEventStruct = ffi.new("struct cwmp__EventStruct[?]", evenNum)
                    gSyEvent.__ptrEventStruct[0].EventCode = toChar(gSendEvent[nType].EventStr)
                    gSyEvent.__ptrEventStruct[0].CommandKey = toChar(gSendEvent[nType].CommandKey)
                end
            end
        end
    elseif 3 == gsyAcsCpeParamStru.IsAcsContacted then
        DPrint("SY_ACS_VALUE_CHANGED")
        evenNum = 3
        gSyEvent.__ptrEventStruct = ffi.new("struct cwmp__EventStruct[?]", evenNum)
        gSyEvent.__ptrEventStruct[0].EventCode = toChar(gSendEvent[nType].EventStr)
        gSyEvent.__ptrEventStruct[0].CommandKey = toChar(gSendEvent[nType].CommandKey)
        gSyEvent.__ptrEventStruct[1].EventCode = toChar(gSendEvent[fC.SY_EVENT_CONNECTION_REQUEST].EventStr)
        gSyEvent.__ptrEventStruct[1].CommandKey = toChar(gSendEvent[fC.SY_EVENT_CONNECTION_REQUEST].CommandKey)
        gSyEvent.__ptrEventStruct[2].EventCode = toChar(gSendEvent[fC.SY_EVENT_BOOT].EventStr)
        gSyEvent.__ptrEventStruct[2].CommandKey = toChar(gSendEvent[fC.SY_EVENT_BOOT].CommandKey)
    elseif gSyUpdateSuccess == 1 then
        gSyUpdateSuccess = 0
        evenNum = 2
        gSyEvent.__ptrEventStruct = ffi.new("struct cwmp__EventStruct[?]", evenNum)
        gSyEvent.__ptrEventStruct[0].EventCode = toChar(gSendEvent[nType].EventStr)
        gSyEvent.__ptrEventStruct[0].CommandKey = toChar(gSendEvent[nType].CommandKey)
        gSyEvent.__ptrEventStruct[1].EventCode = toChar(gSendEvent[fC.SY_EVENT_VALUE_CHANGE].EventStr)
        gSyEvent.__ptrEventStruct[1].CommandKey = toChar(gSendEvent[fC.SY_EVENT_VALUE_CHANGE].CommandKey)
    elseif fC.SY_EVENT_PERIODIC == nType then
        tembuf = SyGetIPAddr()
        if string.len(tembuf) ~= 0 and gSyLANStu.IPAddress ~= tmpBuf then
            if gSyLANStu.IPAddress ~= nil then
                gSyLANStu.IPAddress = tmpBuf
            end
            tmpBuf = string.format("http:")
            if gSyManagementServerStu.ConnectionRequestURL ~= nil then
                gSyManagementServerStu.ConnectionRequestURL = nil
            end
            gSyEvent.__ptrEventStruct = ffi.new("struct cwmp__EventStruct[?]", evenNum)
            gSyEvent.__ptrEventStruct[0].EventCode = toChar(gSendEvent[fC.SY_EVENT_VALUE_CHANGE].EventStr)
            gSyEvent.__ptrEventStruct[0].CommandKey = toChar(gSendEvent[fC.SY_EVENT_VALUE_CHANGE].CommandKey)
        else
            gSyEvent.__ptrEventStruct = ffi.new("struct cwmp__EventStruct[?]", evenNum)
            gSyEvent.__ptrEventStruct[0].EventCode = toChar(gSendEvent[nType].EventStr)
            gSyEvent.__ptrEventStruct[0].CommandKey = toChar(gSendEvent[nType].CommandKey)
        end
    else
        gSyEvent.__ptrEventStruct = ffi.new("struct cwmp__EventStruct[?]", evenNum)
        gSyEvent.__ptrEventStruct[0].EventCode = toChar(gSendEvent[nType].EventStr)
        gSyEvent.__ptrEventStruct[0].CommandKey = toChar(gSendEvent[nType].CommandKey)
    end
    
    gSyEvent.__size = evenNum
    
    fc(gSyEvent1, gSyEvent, ffi.sizeof(gSyEvent))
    DPrint("copy  cwmp__EventStruct")
end

function updateParam(nSize, informSize, gSyParamList1, gSyDeviceInfoStu1, gSyManagementServerStu1, gSyLANStu1, gSyServiceInfoStu1)
    local gSyParamList = fn('struct _ParameterValueStruct')
    ffi.copy(gSyParamList, gSyParamList1, ffi.sizeof(gSyParamList1))
    --local gSyParamList = ffi.cast('struct _ParameterValueStruct*', gSyParamList1)
    local gSyDeviceInfoStu = ffi.cast('syDeviceInfoStruct*', gSyDeviceInfoStu1)
    local gSyManagementServerStu = ffi.cast('syManagementServerStruct*', gSyManagementServerStu1) 
    local gSyLANStu = ffi.cast('syLANStruct*', gSyLANStu1)
    local gSyServiceInfoStu = ffi.cast('syServiceInfoStruct*', gSyServiceInfoStu1)
    
    DPrint(fs(gSyParamList.__ptrParameterValueStruct[0].Type))
    DPrint(string.format('gSyDeviceInfoStu.DeviceSummary:%s', fs(gSyDeviceInfoStu.DeviceSummary)))
    gSyParamList.__ptrParameterValueStruct[fC.SY_INFORM_DEVICE_SUMMARY].Value = mallocStr(gSyDeviceInfoStu.DeviceSummary)
    
    DPrint(string.format('gSyDeviceInfoStu.ModelName:%s', fs(gSyDeviceInfoStu.ModelName)))
    gSyParamList.__ptrParameterValueStruct[fC.SY_INFORM_MODE_NAME].Value = mallocStr(gSyDeviceInfoStu.ModelName)
    
    DPrint(string.format('gSyDeviceInfoStu.Description:%s', fs(gSyDeviceInfoStu.ModelName)))
    gSyParamList.__ptrParameterValueStruct[fC.SY_INFORM_DESCRIPTION].Value = mallocStr(gSyDeviceInfoStu.Description)
    
    DPrint(string.format('gSyDeviceInfoStu.UpTime:%s', fs(gSyDeviceInfoStu.UpTime)))
    gSyParamList.__ptrParameterValueStruct[fC.SY_INFORM_UPTIME].Value = mallocStr(gSyDeviceInfoStu.UpTime)
    
    DPrint(string.format('gSyDeviceInfoStu.FirstUseDate:%s', fs(gSyDeviceInfoStu.FirstUseDate)))
    gSyParamList.__ptrParameterValueStruct[fC.SY_INFORM_FIRST_USEDATE].Value = mallocStr(gSyDeviceInfoStu.FirstUseDate)
    
    DPrint(string.format('gSyDeviceInfoStu.HardwareVersion:%s', fs(gSyDeviceInfoStu.HardwareVersion)))
    gSyParamList.__ptrParameterValueStruct[fC.SY_INFORM_HARDWARE_VERSION].Value = mallocStr(gSyDeviceInfoStu.HardwareVersion)
    
    DPrint(string.format('gSyDeviceInfoStu.SoftwareVersion:%s', fs(gSyDeviceInfoStu.SoftwareVersion)))
    gSyParamList.__ptrParameterValueStruct[fC.SY_INFORM_SOFTWARE_VERSION].Value = mallocStr(gSyDeviceInfoStu.SoftwareVersion)
    
    DPrint(string.format('gSyDeviceInfoStu.AdditionalHardwareVersion:%s', fs(gSyDeviceInfoStu.AdditionalHardwareVersion)))
    gSyParamList.__ptrParameterValueStruct[fC.SY_INFORM_ADDHARDWARE_VERSION].Value = mallocStr(gSyDeviceInfoStu.AdditionalHardwareVersion)
    
    DPrint(string.format('gSyDeviceInfoStu.AdditionalSoftwareVersion:%s', fs(gSyDeviceInfoStu.AdditionalSoftwareVersion)))
    gSyParamList.__ptrParameterValueStruct[fC.SY_INFORM_ADDSOFTWARE_VERSION].Value = mallocStr(gSyDeviceInfoStu.AdditionalSoftwareVersion)
    
    DPrint(string.format('gSyManagementServerStu.ConnectionRequestURL:%s', fs(gSyManagementServerStu.ConnectionRequestURL)))
    gSyParamList.__ptrParameterValueStruct[fC.SY_INFORM_CONNECTION_REQUEST_URL].Value = mallocStr(gSyManagementServerStu.ConnectionRequestURL)
    
    DPrint(string.format('gSyManagementServerStu.ParameterKey:%s', fs(gSyManagementServerStu.ParameterKey)))
    gSyParamList.__ptrParameterValueStruct[fC.SY_INFORM_PARAMETER_KEY].Value = mallocStr(gSyManagementServerStu.ParameterKey)
    
    DPrint(string.format('gSyLANStu.IPAddress:%s', fs(gSyLANStu.IPAddress)))
    gSyParamList.__ptrParameterValueStruct[fC.SY_INFORM_IPADDRESS].Value = mallocStr(gSyLANStu.IPAddress)
    
    DPrint(string.format('gSyLANStu.MACAddress:%s', fs(gSyLANStu.MACAddress)))
    gSyParamList.__ptrParameterValueStruct[fC.SY_INFORM_MACAddress].Value = mallocStr(gSyLANStu.MACAddress)
    
    DPrint(string.format('gSyServiceInfoStu.STBID:%s', fs(gSyServiceInfoStu.STBID)))
    gSyParamList.__ptrParameterValueStruct[fC.SY_INFORM_STBID].Value = mallocStr(gSyServiceInfoStu.STBID)
    
    DPrint(string.format('gSyServiceInfoStu.UserID:%s', fs(gSyServiceInfoStu.UserID)))
    gSyParamList.__ptrParameterValueStruct[fC.SY_INFORM_USER_ID].Value = mallocStr(gSyServiceInfoStu.UserID)
    
    DPrint(string.format('gSyServiceInfoStu.AuthURL:%s', fs(gSyServiceInfoStu.AuthURL)))
    gSyParamList.__ptrParameterValueStruct[fC.SY_INFORM_AUTHURL].Value = mallocStr(gSyServiceInfoStu.AuthURL)
    
    DPrint(string.format('gSyServiceInfoStu.PPPoEID:%s', fs(gSyServiceInfoStu.PPPoEID)))
    gSyParamList.__ptrParameterValueStruct[fC.SY_INFORM_PPPOE_ID].Value = mallocStr(gSyServiceInfoStu.PPPoEID)
    
    gSyParamList.__ptrParameterValueStruct[fC.SY_INFORM_ISENCRYMARK].Value = mallocStr("1")
    ffi.copy(gSyParamList1, gSyParamList, ffi.sizeof(gSyParamList))
end

PathToKey = {
    ["Device.AddRoute"]                                             = "",
    ["Device.X_00E0FC.RemoteControlEnable"]                         = "RemoteControlEnable",
    ["Device.DeviceInfo.ModelName"]                                 = "ModelName",
    ["Device.DeviceInfo.Manufacturer"]                              = "Manufacturer",
    ["Device.DeviceInfo.ProductClass"]                              = "ProductClass",
    ["Device.ManagementServer.URLModifyFlag"]                       = "",
    ["Device.X_00E0FC.PlayDiagnostics.DiagnosticsState"]            = "",
    ["Device.X_00E0FC.PlayDiagnostics.PlayURL"]                     = "PlayUrl",
    ["Device.X_00E0FC.PlayDiagnostics.PlayState"]                   = "PlayUrl",
    ["Device.X_00E0FC.LogParaConfiguration.LogType"]                = "LogType",
    ["Device.X_00E0FC.LogParaConfiguration.LogLevel"]               = "LogLevel",
    ["Device.X_00E0FC.LogParaConfiguration.LogOutPutType"]          = "LogOutPutType",
    ["Device.X_00E0FC.LogParaConfiguration.SyslogServer"]           = "SyslogServer",
    ["Device.X_00E0FC.LogParaConfiguration.SyslogStartTime"]        = "SyslogStartTime",
    ["Device.X_00E0FC.LogParaConfiguration.SyslogContinueTime"]     = "SyslogContinueTime",
    ["Device.X_00E0FC.LogParaConfiguration.LogTimer"]               = "LogTimer",
    ["Device.X_00E0FC.LogParaConfiguration.LogFtpServer"]           = "",
    ["Device.LAN.AddressingType"]                                   = "ConnectMode",
    ["Device.X_00E0FC.ServiceInfo.DHCPID"]                          = "DHCPUserName",
    ["Device.X_00E0FC.ServiceInfo.DHCPPassword"]                    = "DHCPPassword",
    ["Device.X_00E0FC.ServiceInfo.PPPoEID"]                         = "PPPOEUserName",
    ["Device.X_00E0FC.ServiceInfo.PPPoEPassword"]                   = "PPPOEPassword",
    ["Device.LAN.IPAddress"]                                        = "IpAddress",
    ["Device.LAN.SubnetMask"]                                       = "NetMask",
    ["Device.LAN.DefaultGateway"]                                   = "DefaultGate",
    ["Device.LAN.DNSServers"]                                       = "DNS",
    ["Device.LAN.DNSServers2"]                                      = "SecondNDS",
    ["Device.LAN.MACAddress"]                                       = "Mac",
    ["Device.Time.LocalTimeZone"]                                   = "TimeZone",
    ["Device.Time.NTPServer1"]                                      = "ServerNTPUrl",
    ["Device.Time.NTPServer2"]                                      = "ServerNTPBackupUrl",
    ["Device.X_00E0FC.ServiceInfo.AuthURL"]                         = "IPTVauthURL",
    ["Device.X_00E0FC.ServiceInfo.AuthURLBackup"]                   = "SecondIPTVauthURL",
    ["Device.X_00E0FC.ServiceInfo.UserID"]                          = "IPTVaccount",
    ["Device.X_00E0FC.ServiceInfo.UserIDPassword"]                  = "IPTVpassword",
    ["Device.DeviceInfo.SerialNumber"]                              = "",
    ["Device.X_00E0FC.STBID"]                                       = "STBID",
    ["Device.DeviceInfo.HardwareVersion"]                           = "HardwareVersion",
    ["Device.DeviceInfo.SoftwareVersion"]                           = "SoftwareVersion",
    ["Device.ManagementServer.URL"]                                 = "config_WebmasterUrl",
    ["Device.ManagementServer.PeriodicInformEnable"]                = "Congfig_HeartBear",
    ["Device.ManagementServer.PeriodicInformInterval"]              = "config_HeartBeat",
    ["Device.X_00E0FC.StatisticConfiguration.LogServerUrl"]         = "config_QosServerUrl",
    ["Device.X_00E0FC.StatisticConfiguration.LogUploadInterval"]    = "config_LoginTerval",
    ["Device.X_00E0FC.StatisticConfiguration.LogRecordInterval"]    = "config_RecordInterval",
    ["Device.X_00E0FC.StatisticConfiguration.StatInterval"]         = "config_MonitoringInterval",
    ["Device.X_CTC_IPTV.LogMsg.MsgOrFile"]                          = "LogMsgOrFile",
    ["Device.X_CTC_IPTV.LogMsg.Enable"]                             = "LogSwitch",
    ["Device.X_CTC_IPTV.LogMsg.LogFtpServer"]                       = "config_LogServerUrl",
    ["Device.X_CTC_IPTV.LogMsg.LogFtpUser"]                         = "LogserverUser",
    ["Device.X_CTC_IPTV.LogMsg.LogFtpPassword"]                     = "LogserverPassword",
    ["Device.X_CTC_IPTV.LogMsg.Duration"]                           = "Logduration",
    ["Device.X_00E0FC.AutoOnOffConfiguration.IsAutoPowerOn"]        = "IsAutoPowerOn",
    ["Device.X_00E0FC.AutoOnOffConfiguration.AutoPowerOnTime"]      = "AutoPowerOnTime",
    ["Device.X_00E0FC.AutoOnOffConfiguration.AutoShutdownTime"]     = "AutoShutdownTime",
    ["Device.ManagementServer.URLBackup"]                           = "",
    ["Device.ManagementServer.Username"]                            = "config_UserName",
    ["Device.ManagementServer.Password"]                            = "config_Password",
    ["Device.ManagementServer.ConnectionRequestUsername"]           = "config_CPEUser",
    ["Device.ManagementServer.ConnectionRequestPassword"]           = "config_CPEPassword",
    ["Device.DeviceInfo.Manufacturer"]                              = "Manufacturer", 
    ["Device.UserInterface.CurrentLanguage"]                        = "ChooseLanguage",
}

function getvalue(path, value, sizeOfValue)
    DPrint('enter getvalue')
    local p = fs(path)
    local key = PathToKey[p]
    if key == nil then
        DPrint('key is nil')
        return false
    end
    
    if(not g_getData(key, value,sizeOfValue)) then
        fc(value, "")
        return false
    end
    buf = fs(value)
    DPrint("get value is "..buf)
    if p == "ConnectMode" then
        if(buf == "0")then
            fc(value, "PPPoE")
        elseif(buf == "1") then 
            fc(value, "DHCP")
        elseif(buf == "2") then
            fc(value, "Static")
        elseif(buf == "3") then
            fc(value, "IPoE")
        elseif(buf == "4") then
            fc(value, "Wifi")
        elseif(buf == "-1")then 
            fc(value, "None")
        end
    end
    return true
end

function getParamV(cmd, buffer, sizeOfValue)
    length = ffi.sizeof(g_keyList)/ffi.sizeof('xml_key_path_t')
    DPrint('g_keyList len is '..length)
    if cmd == -1 then
        return false
    end
    local index = 0
    
    for i=0,length-1 do
        index = i
        if(g_keyList[i].cmd == cmd and g_keyList[i].key ~= nil) then
            --DPrint(g_keyList[i].key)
            break
        end
     end
    if length == index+1 then
        return false
    end
    DPrint('call getdata')
    if(not g_getData(g_keyList[index].key, buffer,sizeOfValue)) then
        return false
    end
    buf = fs(buffer)
    DPrint("get buffer is"..buf)
    if cmd == 1 then
        if(buf == "0")then
            fc(buffer, "PPPoE")
        elseif(buf == "1") then 
            fc(buffer, "DHCP")
        elseif(buf == "2") then
            fc(buffer, "Static")
        elseif(buf == "3") then
            fc(buffer, "IPoE")
        elseif(buf == "4") then
            fc(buffer, "Wifi")
        elseif(buf == "-1")then 
            fc(buffer, "None")
        end
    end
    return true
end

function getvalue1(path, value, sizeOfValue)
    DPrint('call getvalue')
    p = fs(path)
    gSyCmdStrListLen = ffi.sizeof(gSyCmdStrList)/ffi.sizeof('syCmdStrStu')
    DPrint('gSyCmdStrListLen len is '..gSyCmdStrListLen)
    local nCmd = -1
    local i = 0
    for i=0,gSyCmdStrListLen-1 do
        DPrint(fs(gSyCmdStrList[i].cmdStr))
        if(fs(gSyCmdStrList[i].cmdStr) == fs(path)) then
            nCmd = gSyCmdStrList[i].cmd
            break
        end
    end
    DPrint("call cmd is "..nCmd)
    if nCmd ~= -1 then
        if(getParamV(nCmd, value, sizeOfValue)) then
            DPrint("getParamV() failed.")
        end
    end
    return 1
end



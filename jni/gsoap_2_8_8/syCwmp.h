/***********************************************************************
*
* syCwmp.h
*
* Implementation of user-space cwmp.
*
* Copyright (C) 2012 Lin jieling <mycourage0@126.com>
* Copyright (C) 2003-2012 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/

#ifndef SY_CWMP_H
#define SY_CWMP_H

//gsoap cwmp service name: cwmp
typedef char *xsd__string;
typedef char *cwmp__ParameterKeyType :  32;
typedef char *cwmp__ObjectNameType   ".*\\.":256;
typedef char *cwmp__CommandKeyType   :  32;
typedef char *cwmp__FaultCodeType;

enum xsd__boolean{
    false_, 
    true_
};

enum _cwmp__SetParaAttrStruct_Notification
{
    _cwmp__SetParaAttrStruct_Notification__0 = 0,    
    _cwmp__SetParaAttrStruct_Notification__1 = 1,    
    _cwmp__SetParaAttrStruct_Notification__2 = 2,    
};

enum _cwmp__SetParaValRes_Status
{
    _cwmp__SetParaValRes_Status__0 = 0,
    _cwmp__SetParaValRes_Status__1 = 1,
};

enum _cwmp__SetParaAttrStructRes_Notification
{
    _cwmp__SetParaAttrStructRes_Status__0 = 0,
    _cwmp__SetParaAttrStructRes_Status__1 = 1,
};

enum _cwmp__AddObjRes_Status
{
    _cwmp__AddObjRes_Status__0 = 0,
    _cwmp__AddObjRes_Status__1 = 1,
};

enum _cwmp__DelObjRes_Status
{
    _cwmp__DelObjRes_Status__0 = 0,  
    _cwmp__DelObjRes_Status__1 = 1,  
};

enum _cwmp__DownloadRes_Status
{
    _cwmp__DownloadRes_Status__0 = 0,
    _cwmp__DownloadRes_Status__1 = 1,
};

enum _cwmp__UploadRes_Status
{
    _cwmp__UploadRes_Status__0 = 0,
    _cwmp__UploadRes_Status__1 = 1,
};

struct cwmp__FaultStruct
{

	char    *FaultCode     1;   
	char    *FaultString   1;	
};

struct SOAP_ENV__Header
{
    mustUnderstand  char *cwmp__ID;
};

struct cwmp__MethodList
{
    int __size;
    char **__ptrstring;
};

struct cwmp__GetRPCMethodsResponse
{
    struct cwmp__MethodList __ptrMethodList;
};

typedef struct
{
	char            *Name   1;///< Required element.
	xsd__string     Value   1;///< Required element.
	char            *Type   1;/*Value of type*/
}PVStu, *PVStu;

/// "urn:dslforum-org:cwmp-1-0":ParameterValueList is a complexType with complexContent restriction of SOAP-ENC:Array.
/// SOAP encoded array of "urn:dslforum-org:cwmp-1-0":ParameterValueStruct
typedef struct
{
	PVStu*  *ptrPTS;
	int      size;
} PVList, *PVList;

struct ParameterNames
{
	char*    *__ptrstring;
	int       __size;
};

struct AccessList
{
    char*   *__ptrstring;
	int     __size;
};

/// "urn:dslforum-org:cwmp-1-0":SetParameterAttributesStruct is a complexType.
struct cwmp__SetParameterAttributesStruct
{
	char* 	*Name;
	enum xsd__boolean   NotificationChange          1;
	enum _cwmp__SetParaAttrStruct_Notification Notification     1;	
	char                       *AccessListChange    1;
	struct AccessList          *AccessList          1;	
};

struct SetParameterAttributesList
{
	struct cwmp__SetParameterAttributesStruct* *__ptrSetParameterAttributesStruct;
	int __size;
};

struct cwmp__EventStruct
{
	/// Length of this string is within 0..64 characters
	/// Content pattern is "0 BOOTSTRAP"
	/// Content pattern is "1 BOOT"
	/// Content pattern is "2 PERIODIC"
	/// Content pattern is "3 SCHEDULED"
	/// Content pattern is "4 VALUE CHANGE"
	/// Content pattern is "5 KICKED"
	/// Content pattern is "6 CONNECTION REQUEST"
	/// Content pattern is "7 TRANSFER COMPLETE"
	/// Content pattern is "8 DIAGNOSTICS COMPLETE"
	/// Content pattern is "9 REQUEST DOWNLOAD"
	/// Content pattern is "10 AUTONOMOUS TRANSFER COMPLETE"
	/// Content pattern is "\\d+( \\S+)+"
	/// Content pattern is "M Reboot"
	/// Content pattern is "M ScheduleInform"
	/// Content pattern is "M Download"
	/// Content pattern is "M Upload"
	/// Content pattern is "M \\S+"
	/// Content pattern is "M X_\\S+"
	/// Content pattern is "X [0-9A-F]{6} .*"
	char                    *EventCode      1;
	cwmp__CommandKeyType    CommandKey      1;
};


struct cwmp__GetParameterValuesResponse
{
	struct ParameterValueList   *ParameterList;
};

struct cwmp__SetParameterValuesResponse
{
	enum _cwmp__SetParaValRes_Status Status     1;
};

struct cwmp__GetParameterNamesResponse
{
	struct ParameterInfoList    *ParameterList  1;
};

struct ParameterInfoList
{
	struct cwmp__ParameterInfoStruct*   *__ptrParameterInfoStruct;
	int                                  __size;
};

struct cwmp__ParameterInfoStruct
{
	char*   Name        1;
	int     Writable    1;
};

struct cwmp__SetParameterAttributesResponse
{
	enum _cwmp__SetParaAttrStructRes_Notification Status;	
};

struct cwmp__GetParameterAttributesResponse
{
	struct ParameterAttributeList       *ParameterList;	
};

struct ParameterAttributeList
{
	struct cwmp__ParameterAttributeStruct* *__ptrParameterAttributeStruct ;
    int                                  __size;
};

struct cwmp__ParameterAttributeStruct
{
	char*	Name        1;
	enum _cwmp__SetParaAttrStructRes_Notification Notification  1;
	struct AccessList   *AccessList     1;
};

struct cwmp__AddObjectResponse
{
	unsigned int InstanceNumber;
	enum _cwmp__AddObjRes_Status Status;
};

struct cwmp__DeleteObjectResponse
{
	enum _cwmp__DelObjRes_Status Status;	
};

/// Element "urn:dslforum-org:cwmp-1-0:DownloadResponse of complexType.
struct cwmp__DownloadResponse
{
	enum _cwmp__DownloadRes_Status Status;
	time_t                               StartTime;
	time_t                               CompleteTime;
};

struct cwmp__UploadResponse
{
	enum _cwmp__UploadRes_Status Status;
	time_t                               StartTime;	
	time_t                               CompleteTime;	
};

struct cwmp__FactoryResetResponse {

};

struct cwmp__ScheduleInformResponse {

};

/// "urn:dslforum-org:cwmp-1-0":TransferCompleteResponse is a complexType.
struct cwmp__TransferCompleteResponse {

};

struct cwmp__RebootResponse {

};

struct _cwmp__TransferComplete
{
	cwmp__CommandKeyType                 CommandKey     1;	
	struct cwmp__FaultStruct            *FaultStruct    1;	
	time_t                               StartTime;	
	time_t                               CompleteTime;	
};

struct _cwmp__TransferCompleteResponse {

};

struct _cwmp__Fault_SetParameterValuesFault
{
    char                                *ParameterName  1;
    cwmp__FaultCodeType                  FaultCode      1; 
    char                                *FaultString    0;   
};

struct _cwmp__Fault
{
	cwmp__FaultCodeType                  FaultCode      1;	
	char                                *FaultString    0;	
	int                                  __sizeSetParameterValuesFault;
 	struct _cwmp__Fault_SetParameterValuesFault *SetParameterValuesFault    0;
};

struct _DeviceIdStruct {
    char *Manufacturer;
    char *OUI;
    char *ProductClass;
    char *SerialNumber;
};

struct _EventStruct {
	struct cwmp__EventStruct  *__ptrEventStruct;
	int __size;
};

struct _ParameterValueStruct {
	struct cwmp__ParameterValueStruct *__ptrParameterValueStruct;
	int __size;
};

struct cwmp__InformResponse {
    unsigned int MaxEnvelopes;
};

//gsoap cwmp schema namespace: urn:dslforum-org:cwmp-1-0
int __Inform(
    struct _DeviceIdStruct *DeviceId,
    struct _EventStruct *Event,
    unsigned int MaxEnvelopes,
    char *CurrentTime,
    unsigned int RetryCount,
    struct _ParameterValueStruct *ParameterList,
    struct cwmp__InformResponse *);

int __GetRPCMethods(void *_, 
        struct cwmp__GetRPCMethodsResponse *);

int __GetParaValues(
        struct ParameterNames *ParameterNames, 
        struct cwmp__GetParameterValuesResponse *);

int __SetParaValues(
        struct ParameterValueList *ParameterList, 
        cwmp__ParameterKeyType ParameterKey, 
        struct cwmp__SetParameterValuesResponse *);

int __GetParaNames(
        char **ParameterPath, 
        char *NextLevel, 
        struct cwmp__GetParameterNamesResponse *);

int __SetParaAttr(
        struct SetParameterAttributesList *ParameterList, 
        struct cwmp__SetParameterAttributesResponse *);

int __GetParaAttr(
        struct ParameterNames *ParameterNames,  
        struct cwmp__GetParameterAttributesResponse *);

int __AddObject(
        cwmp__ObjectNameType ObjectName, 
        cwmp__ParameterKeyType ParameterKey,
        struct cwmp__AddObjectResponse *);

int __AddObjectIPTV(
        cwmp__ObjectNameType ObjectName, 
        cwmp__ParameterKeyType ParameterKey, 
        struct cwmp__AddObjectResponse *);

int __DeleteObject(
        cwmp__ObjectNameType ObjectName, 
        cwmp__ParameterKeyType ParameterKey, 
        struct cwmp__DeleteObjectResponse *);

int __Reboot(
        cwmp__CommandKeyType CommandKey, 
        struct cwmp__RebootResponse *);

int __Download(
        cwmp__CommandKeyType CommandKey, 
        char *FileType, 
        char *URL,
        char *Username, 
        char *Password, 
        int FileSize, 
        char *TargetFileName,
        int DelaySeconds, 
        char *SuccessURL, 
        char *FailureURL, 
        struct cwmp__DownloadResponse *);

int __Upload(
        cwmp__CommandKeyType CommandKey,
        char *FileType, 
        char *URL, 
        char *Username,
        char *Password, 
        int DelaySeconds, 
        struct cwmp__UploadResponse *);

int __FactoryReset(
        void *_, 
        struct cwmp__FactoryResetResponse *);

int __ScheduleInform(
        int DelaySeconds, 
        cwmp__CommandKeyType CommandKey, 
        struct cwmp__ScheduleInformResponse *);

int __TransferComplete(
        cwmp__CommandKeyType CommandKey, 
        struct cwmp__FaultStruct FaultStruct, 
        time_t StartTime, 
        time_t CompleteTime,
        struct cwmp__TransferCompleteResponse *);

#endif //SY_CWMP_H


/***********************************************************************
*
* syCwmpCommon.c
*
* Copyright (C) 2012 Lin jieling <mycourage0@126.com>
* Copyright (C) 2003-2012 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/

#include <pthread.h>

#include "syCwmpCommon.h"
#include "syCwmpXml.h"
#include "stdsoap2.h"

extern int gSyPathNumber;
extern mxml_node_t* gnNodeRoot;

/* 0:关闭log开关，1:打开log开关 */
char verboseLogSwitch = 0;
/* 最大函数名长度. */
int maxFuncNameLength = 4;

const char *SyMethodTypeToStr(enum sy_method_type type)
{
    switch(type)
    {
    case SY_GET_RPC_METHODS:
        return "GetRPCMethods";
    case SY_SET_PARAMETER_VALUES:
        return "SetParameterValues";
    case SY_GET_PARAMETER_VALUES:
        return "GetParameterValues";
    case SY_GET_PARAMETER_NAMES:
        return "GetParameterNames";
    case SY_SET_PARAMETER_ATTRIBUTES:
        return "SetParameterAttributes";
    case SY_GET_PARAMETER_ATTRIBUTES:
        return "GetParameterAttributes";
    case SY_ADD_OBJECT:
        return "AddObject";
    case SY_DELETE_OBJECT:
        return "DeleteObject";
    case SY_REBOOT:
        return "Reboot";
    case SY_DOWNLOAD:
        return "Download";
    case SY_UPLOAD:
        return "Upload";
    case SY_FACTORY_RESET:
        return "FactoryReset";
    case SY_GET_QUEUED_TRANSFERS:
        return "GetQueuedTransfers";
    case SY_GET_ALL_QUEUED_TRANSFERS:
        return "GetAllQueuedTransfers";
    case SY_SCHEDULE_INFORM:
        return "ScheduleInform";
    case SY_SET_VOUCHERS:
        return "SetVouchers";
    case SY_GET_OPTIONS:
        return "GetOptions";
    case SY_INFORM:
        return "Inform";
    case SY_TRANSFER_COMPLETE:
        return "TransferComplete";
    case SY_AUTONOMOUS_TRANSFER_COMPLETE:
        return "AutonomousTransferComplete";
    case SY_REQUEST_DOWNLOAD:
        return "RequestDownload";
    case SY_KICKED:
        return "Kicked";
    default:
        return "unknown";
    }
}

bool SyInitConfigXml()
{
    if (!SyLoadConfigXml())
    {
        return false;
    }
    return true;
}

int SyGetNodeAttr(const char* path, char* value)
{
    mxml_node_t* currentNode;

    currentNode = SyFindPathNode(path);
    if(NULL == currentNode)
        return SY_FAILED;
    if (SY_SUCCESS != SyIsNodeCheck(currentNode))
    {
        return SY_FAILED;
    }
    return SyGetAttrAttr(currentNode, value);
}

int SySetNodeAttr(const char* path, int notifyChange, int notify)
{
    char strNotify[4] = {0};
    char strNotifyChange[4] = {0};
    mxml_node_t* currentNode;

    currentNode = SyFindPathNode(path);
    DPrint("path:%s, currentNode:%p\n", path, currentNode);
    if(NULL == currentNode)
        return SY_FAILED;

    if (SY_FAILED == SyIsNodeCheck(currentNode))
    {
        return SY_FAILED;
    }


    DPrint("notify:%d, notifyChange:%d\n",
           notify, notifyChange);
    if (1 == notifyChange)
    {
        sprintf(strNotify, "%d", notify) ;
        sprintf(strNotifyChange, "%d", notifyChange) ;
        SySetAttrAttr(currentNode, strNotify);
        SySetAttrAttrEnable(currentNode, strNotifyChange);
        SySaveConfigXml();
    }
    return SY_SUCCESS;
}

bool SyGetNodeValue(const char* path, char* value)
{
    int nRet = SY_FAILED;
    int v = 0;
    mxml_node_t* currentNode;
    DPrint("get %s.\n", path);
    currentNode = SyFindPathNode(path);
    if(NULL == currentNode) {
		EPrint("currentNode is None");
        return false;
    }
	return (SY_SUCCESS == SyGetAttrValue(currentNode, value))?true:false;

}

int SySetNodeValue(const char* path, char* value)
{
    // DPrint("%s => %s\n", path, value);
    mxml_node_t* currentNode;
    currentNode = SyFindPathNode(path);
    if(NULL == currentNode)
    {
        EPrint("Not found %s in xml.\n", path);
        return SY_FAILED;
    }
    if (SY_SUCCESS == SySetAttrValue(currentNode, value))
    {
        SySaveConfigXml();
    }
    return SY_SUCCESS;

}

int SyGetNodeType(const char* path, char* type, const char* def)
{
    mxml_node_t* currentNode;

    currentNode = SyFindPathNode(path);
    if(NULL == currentNode)
    {
        EPrint("Node not found");
        return SY_FAILED;
    }
    return SyGetAttrType(currentNode, type, def)?SY_SUCCESS:SY_FAILED;
}

int SyGetNodeLevel(const char* path, char* level)
{
    mxml_node_t* currentNode;

    DO;

    currentNode = SyFindPathNode(path);
    if(NULL == currentNode)
        return SY_FAILED;
    return SyGetAttrLevel(currentNode, level);
}

int SyGetNodeRW(const char* path, char* rw)
{
    mxml_node_t* currentNode;

    DO;

    currentNode = SyFindPathNode(path);
    if(NULL == currentNode)
        return SY_FAILED;
    return SyGetAttrRW(currentNode, rw);
}

/*Only type is String, use this function*/
int SyGetNodeMaxLen(const char* path, char* maxLen)
{
    return 0;
}

int SyGetNodeMax(const char* path, char* max)
{
    return 0;
}

int SyGetNodeMin(const char* path, char* min)
{
    return 0;
}

int SyGetNameList(char *path , char *nameList, char* nextLevel)
{
    DO;
    return SyGetNameListByPath(path, nameList, nextLevel);
}

int SyAddInstanceObj(char *path)
{
    int num = 0;
    int num2 = 0;
    char *pNum = NULL;
    char *tmpStr = NULL;
    char *tmpPtr = NULL;
    char cNum[8] = {0};
    char tmpBuf[128] = {0};
    char tmpPath[128] = {0};
    mxml_node_t* tmpNode;
    mxml_node_t* childNode;
    mxml_node_t* parentNode;
    mxml_node_t* currentNode;
    DO;

#if 0
    DPrint("path:%s\n", path);
    currentNode = SyFindPathNode(path);
    if(NULL != currentNode)
        return -1;

    tmpPtr = tmpBuf;
    sprintf(tmpPtr, "%s", path);
    tmpStr = strtok(tmpPtr, ".");
    while(tmpStr)
    {
        if(0 == SyIsNumber(tmpStr))
        {
            memcpy(tmpPath, path, tmpStr - tmpPtr);
            break;
        }
        tmpStr = strtok(NULL, ".");
    }
    if((NULL == tmpPath) || (0 == strlen(tmpPath)))
    {
        memcpy(tmpPath, path, strlen(path));
    }

    DPrint("tmpPath:%s\n", tmpPath);
    parentNode = SyFindPathNode(tmpPath);
    if (NULL == parentNode)
    {
        DPrint("parentNode NULL\n");
        return 0;
    }
#else
    DPrint("path:%s\n", path);
    parentNode = SyFindPathNode(path);
    if (NULL == parentNode)
    {
        DPrint("parentNode NULL\n");
        return 0;
    }
#endif
    childNode = SyGetChildrenNode(parentNode);
    if (NULL == childNode)
    {
        DPrint("childNode NULL\n");
        return 0;
    }
    DPrint("name1:%s\n", childNode->value.element.name);

    pNum = cNum;
    memset(cNum, 0x00, sizeof(cNum));
    num = SyGetNoUsedInstanceNum(parentNode);
    sprintf(pNum, "%d", num);
    DPrint("num:%d\n", num);
    num2 = atoi(childNode->value.element.name);
    childNode->value.element.name = strdup(pNum);

    tmpNode = SySaveNode(childNode);
    if (NULL == tmpNode)
    {
        DPrint("tmpNode NULL\n");
        return 0;
    }
    DPrint("name2:%s\n", tmpNode->value.element.name);
    mxmlAdd(parentNode, MXML_ADD_AFTER, MXML_ADD_TO_PARENT, tmpNode) ;

    sprintf(pNum, "%d", num2);
    childNode->value.element.name = strdup(pNum);

    SySaveConfigXml();
    remove(SY_COMMON_XML);

    DONE;

    return num;
}

int SyDelInstanceObj(char *path)
{
    mxml_node_t* currentNode;

    DO;

    DPrint("path:%s\n", path);
    currentNode = SyFindPathNode(path);
    if(NULL == currentNode)
        return SY_FAILED;

    mxmlDelete(currentNode);
    SySaveConfigXml();

    return SY_SUCCESS;
}

int SyGetSubString(char* str, int number, char* token, char* value)
{
    int i = 1;
    char* ptr = NULL;
    char* pcBuf = NULL;
    char* pcStr = NULL;

    DO;

    ptr = str;
    while(i < number && ptr)
    {
        i++;
        pcBuf = strstr(ptr, token);
        if(NULL == pcBuf)
            return SY_FAILED;
        ptr = pcBuf + strlen(token);
    }

    if(NULL == ptr)
        return SY_FAILED;
    pcBuf = strstr(ptr, token);
    if(!pcBuf)
    {
        if(ptr)
        {
            sprintf(value, "%s", ptr);
            return SY_SUCCESS;
        }
        else
        {
            return SY_FAILED;
        }
    }
    pcStr = ptr;
    i = 0;
    while(*pcStr != *pcBuf)
    {
        pcStr++;
        i++;
    }

    strncat(value, ptr, i);
    value[i] = '\0';
    return SY_SUCCESS;
}

int SyCalcNodeNum(char *path)
{
    int number = 0;
    mxml_node_t* currentNode;

    DPrint("path:%s\n", path);

    currentNode = SyFindPathNode(path);
    if(NULL == currentNode)
    {
        gSyPathNumber = 0;
        return SY_FAILED;
    }

    SyXmlCalcNodeNum(path, currentNode, 0);
    return SY_SUCCESS;
}

/* Look up a system property by name, copying its value and a
 * \0 terminator to the provided pointer.  The total bytes
 * copied will be no greater than PROP_VALUE_MAX.  Returns
 * the string length of the value.  A property that is not
 * defined is identical to a property with a length 0 value.
*/
int getprop(const char* name, char* value, const char* defaultValue)
{
    /*  system_properties.h 中定义
     *  PROP_NAME_MAX =  32
     *  PROP_VALUE_MAX = 92
     */
    int iRet = __system_property_get(name, value);
    //DPrint("name:%s, value:%s\n", name, value);
    if(iRet == 0)
    {
        if (defaultValue != NULL)
        {
            WPrint("Fill default value\n");
            strncpy(value, defaultValue, PROP_VALUE_MAX-1);
            value[PROP_VALUE_MAX-1] = '\0';
        }
    }

    return iRet;
}

void get_debug_switch()
{
    char val[PROP_VALUE_MAX] = {0};
    if(getprop("persist.sy.tr069.verboselog", val, ""))
    {
        if('1' == *val)
        {
            verboseLogSwitch = 1;
        }
    }
	//verboseLogSwitch = 1;

}

int getMaxFuncNameLen(const char* funcName) {
    // TODO:按理这里需要一个lock
    int tmpLen = (int)strlen(funcName);

    if(maxFuncNameLength < tmpLen) {
        maxFuncNameLength = tmpLen;
    }
    return maxFuncNameLength;
}

/**
 * log 拼接打印函数
 */
void PrintLog(int proi,
              const char *tag,
              const char *function,
              const char *format, ...)
{
    if(!format || *format == 0)
        return ;
    if(verboseLogSwitch)
    {
        char buffer[DEBUG_BUFFER_MAX_LENGTH-33] =  {0};
        char fullBuffer[DEBUG_BUFFER_MAX_LENGTH] = {0};
        //char* szPpath = strdup(filename), szFName = szPpath, szSlash;

        va_list args;
        va_start(args, format);
        vsnprintf(buffer, DEBUG_BUFFER_MAX_LENGTH-33, format, args);
        va_end (args);
        /*
        szSlash = strrchr(szPpath, '/');
        if(szSlash) szFName = szSlash + 1;
        */
        snprintf(fullBuffer, DEBUG_BUFFER_MAX_LENGTH, "%5d|%s|%*.*s| %s",
                 gettid(),//pthread_self() 返回的是一个"context"值,在crash的时候用的是pid_t结构的handle.
                 getVer(),
                 maxFuncNameLength,
                 getMaxFuncNameLen(function),
                 function,
                 buffer);

        __android_log_write(proi, tag, fullBuffer);
    }
}


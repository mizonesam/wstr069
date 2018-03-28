
#ifndef __SYMEDIA_EVENT_H__
#define __SYMEDIA_EVENT_H__

#include "syCwmpCommon.h"
#include "cwmpXml.h"

typedef struct xml_event_list_t {
	char* 	eventCode;		//上报事件的字符串
	char** 	paramName;		//上报事件所需携带的参数表
	int 	size;			//本次上报事件所需携带的参数表的参数个数
} xml_event_list_t;

/*
功能:设置eventList.xml的内存中的root节点
输入参数:loadConfigXml返回的eventList.xml的root节点
返回值:无
*/
void setEventRoot(mxml_node_t *pXmlRootPt);

/*
功能:根据输入的事件字符串返回对应的节点表
输入参数:上报事件的字符串
返回值:对应list表的详细数据结构
*/
xml_event_list_t* getEvent(const char *pEventName);

/*
功能:根据输入的事件字符串返回对应的节点表
输入参数:上报事件的字符串
返回值:对应list表的详细数据结构
*/
xml_event_list_t* findNode(const char* pEventName);

/*
功能:根据输入的事件字符串返回对应节点的个数
输入参数:上报事件的字符串
返回值:对应list表的节点个数
*/
int findNum(const char* pEventName);

#endif /* __SYMEDIA_EVENT_H__ */



#ifndef __SYMEDIA_EVENT_H__
#define __SYMEDIA_EVENT_H__

#include "syCwmpCommon.h"
#include "cwmpXml.h"

typedef struct xml_event_list_t {
	char* 	eventCode;		//�ϱ��¼����ַ���
	char** 	paramName;		//�ϱ��¼�����Я���Ĳ�����
	int 	size;			//�����ϱ��¼�����Я���Ĳ�����Ĳ�������
} xml_event_list_t;

/*
����:����eventList.xml���ڴ��е�root�ڵ�
�������:loadConfigXml���ص�eventList.xml��root�ڵ�
����ֵ:��
*/
void setEventRoot(mxml_node_t *pXmlRootPt);

/*
����:����������¼��ַ������ض�Ӧ�Ľڵ��
�������:�ϱ��¼����ַ���
����ֵ:��Ӧlist�����ϸ���ݽṹ
*/
xml_event_list_t* getEvent(const char *pEventName);

/*
����:����������¼��ַ������ض�Ӧ�Ľڵ��
�������:�ϱ��¼����ַ���
����ֵ:��Ӧlist�����ϸ���ݽṹ
*/
xml_event_list_t* findNode(const char* pEventName);

/*
����:����������¼��ַ������ض�Ӧ�ڵ�ĸ���
�������:�ϱ��¼����ַ���
����ֵ:��Ӧlist��Ľڵ����
*/
int findNum(const char* pEventName);

#endif /* __SYMEDIA_EVENT_H__ */


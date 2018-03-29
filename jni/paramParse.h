
#ifndef __SYMEDIA_PARAM_H__
#define __SYMEDIA_PARAM_H__

#include "syCwmpCommon.h"
#include "cwmpXml.h"

#define TAG_MSG_LEN		32
#define NODE_MSG_LEN 	128
#define KEY_MSG_LEN		32
#define TYPE_MSG_LEN	32
#define ATTR_MSG_LEN	8

typedef struct xml_key_path_t {
	char	tag[TAG_MSG_LEN];		//��Ӧxml�в�����ڵ�ı�ǩ
	char 	type[TYPE_MSG_LEN];		//��Ӧxml�в�����ڵ��type
	char	nodename[NODE_MSG_LEN];	//��Ӧxml�в�����ڵ��nodename
	int 	attr;					//��Ӧxml�в�����ڵ��attr,0:��ϵͳ��ȡ,ֻ�� 1:���������Ļ�ȡ,�ɶ�д
	char 	keyname[KEY_MSG_LEN];	//��Ӧxml�в�����ڵ��keyname
} xml_key_path_t;

/*
����:����paramList.xml���ڴ��е�root�ڵ�
�������:loadConfigXml���ص��ڴ���root�ڵ��ָ��
����ֵ:��
*/
void setParamRoot(mxml_node_t *pXmlRootPt);

/*
����:ͨ��tr069�ڵ����ƻ�ȡ��Ӧ��paramList.xml�ж�Ӧ�Ľڵ���Ϣ
�������:tr069�ڵ�����,�ڵ���ϸ��Ϣ�Ľṹ��(��������ѯ���Ľڵ���ϸ��Ϣ)
����ֵ:���ҵĽ����TRUE��ʾ�ɹ���FALSE��ʾʧ��
*/
BOOL getParamForNode(const char *pNamePt, xml_key_path_t *pDataPt);

/*
����:ͨ�����������ڲ��ڵ����ƻ�ȡ��Ӧ��paramList.xml�ж�Ӧ�Ľڵ���Ϣ
�������:���������ڲ��ڵ�����,�ڵ���ϸ��Ϣ�Ľṹ��(��������ѯ���Ľڵ���ϸ��Ϣ)
����ֵ:���ҵĽ����TRUE��ʾ�ɹ���FALSE��ʾʧ��
*/
BOOL getParamForKey(const char *pNamePt, xml_key_path_t *pDataPt);

/*
����:��ӡĳ���ڵ����ϸ��Ϣ
�������:xml_key_path_t�ṹ��ָ��
����ֵ:��
*/
void showNode(xml_key_path_t *pNodePt);

#endif /* __SYMEDIA_PARAM_H__ */


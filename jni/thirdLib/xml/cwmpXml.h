
#ifndef __SYMEDIA_XML_H__
#define __SYMEDIA_XML_H__

#include <sys/types.h>
#include <sys/queue.h>
#include "mxml.h"
#include "syCwmpCommon.h"

/*
����:����·������xml�ļ�������root�ڵ�ָ��
�������:
����1:����������root�ڵ�ĵ�ַ
����2:��������xml·��
����ֵ:�����Ľṹ��TRUE��ʾ�����ɹ���FALSE��ʾ����ʧ��
*/
BOOL loadXml(mxml_node_t **pNodeRootPt, const char *pFile);

/*
����:����eventList.xml���ڴ��е�root�ڵ�
�������:loadConfigXml���ص�eventList.xml��root�ڵ�
����ֵ:��
*/
void saveXml(mxml_node_t *pNodeRootPt, const char* pFilePath);

/*
����:����xml����ں���
�������:
����1:�����ļ���·��
����2:ʵ��ʹ�õ��ļ�·��
����3:�Ƿ�ÿ��ʹ�ö�����һ��,TRUE��ʾÿ��ʹ�ö�������FALSE��ʾֻ�е�һ��ʹ�õ�ʱ��ſ���
����ֵ:����xml���root�ڵ�ָ�룬�������NULL����ʾ����ʧ�ܣ������ʾ�����ɹ�
*/
mxml_node_t* loadConfigXml(const char* pSrcXmlFile, const char* pDstXmlFile, BOOL pFlag);

/*
����:��ȡ����ֵ
�������:
����1:����ȡ���Ե�xml�ڵ�ָ��
����2:��������
����3:�����ȡ��������ֵ��buffer
����4:buffer�ĳ���
����ֵ:�������Ի�ȡ�����TRUE��ʾ��ȡ�ɹ���FALSE��ʾ��ȡʧ��
*/
BOOL getAttrValue(mxml_node_t *pNodePt, char *pAttrName, char *pValue, int pLen);

/*
����:���ýڵ�����
�������:
����1:��Ҫ���������Ե�xml�ڵ�ָ��
����2:����ֵ
����ֵ:�������ý����TRUE��ʾ���óɹ���FALSE��ʾ����ʧ��
*/
BOOL setAttrValue(mxml_node_t* currentNode, char *value);

/*
����:����xml�ļ�ʱ��ĸ�ʽ�ص�����
�������:
����1:���λص�ʱ�����Ŀ��ڵ�ָ��
����2:��ʽ���ƶ�Ӧ������
*/
const char* whitespaceCb(mxml_node_t *node, int where);

/*
����:�����ڴ��е�xml���ݵ��ļ������¼���xml�ļ�
�������:
����1:�����·��
����2:xml��root�ڵ�ָ��
����ֵ:�����Ժ����¼��ص�xml��root�ڵ�
*/
mxml_node_t* saveNode(const char *pXmlPath, mxml_node_t *pNode);

/*
����:�޸��ļ�Ȩ��
�������:
����1:��Ҫ���޸��ļ�Ȩ�޵��ļ�·��
����2:Ŀ��Ȩ�޴��������
����ֵ:�޸�Ȩ�޵Ķ�������TRUE��ʾ�޸ĳɹ���FALSE��ʾ�޸�ʧ��
*/
BOOL chmodFile(const char* pSrcPath, int pMode);

/*
����:�����ļ�
�������:
����1:���������ļ�·��
����2:������Ŀ��·��
����ֵ:�����Ľ����TRUE��ʾ�����ɹ���FALSE��ʾ����ʧ��
*/
BOOL copyFile(const char* pSrcFile, const char* pDstFile);

/*
����:���ݴ���Ľڵ�ָ���ȡ���ڵ��text
�������:��Ҫ��ѯ��text���ݵ�xml�ڵ�ָ��
����ֵ:���ڵ�text���ݵ��ڴ��еĵ�ַ
*/
const char* getXmlText(mxml_node_t* pNode);

#endif /* __SYMEDIA_XML_H__ */



#ifndef __SYMEDIA_XML_H__
#define __SYMEDIA_XML_H__

#include <sys/types.h>
#include <sys/queue.h>
#include "mxml.h"
#include "syCwmpCommon.h"

/*
功能:根据路径解析xml文件并返回root节点指针
输入参数:
参数1:保存解析后的root节点的地址
参数2:被解析的xml路径
返回值:解析的结构，TRUE表示解析成功，FALSE表示解析失败
*/
BOOL loadXml(mxml_node_t **pNodeRootPt, const char *pFile);

/*
功能:设置eventList.xml的内存中的root节点
输入参数:loadConfigXml返回的eventList.xml的root节点
返回值:无
*/
void saveXml(mxml_node_t *pNodeRootPt, const char* pFilePath);

/*
功能:解析xml的入口函数
输入参数:
参数1:备份文件的路径
参数2:实际使用的文件路径
参数3:是否每次使用都拷贝一次,TRUE表示每次使用都拷贝，FALSE表示只有第一次使用的时候才拷贝
返回值:解析xml后的root节点指针，如果返回NULL，表示解析失败，否则表示解析成功
*/
mxml_node_t* loadConfigXml(const char* pSrcXmlFile, const char* pDstXmlFile, BOOL pFlag);

/*
功能:获取属性值
输入参数:
参数1:被获取属性的xml节点指针
参数2:属性名称
参数3:保存获取到的属性值的buffer
参数4:buffer的长度
返回值:返回属性获取结果，TRUE表示获取成功，FALSE表示获取失败
*/
BOOL getAttrValue(mxml_node_t *pNodePt, char *pAttrName, char *pValue, int pLen);

/*
功能:设置节点属性
输入参数:
参数1:需要被设置属性的xml节点指针
参数2:属性值
返回值:返回设置结果，TRUE表示设置成功，FALSE表示设置失败
*/
BOOL setAttrValue(mxml_node_t* currentNode, char *value);

/*
功能:保存xml文件时候的格式回调函数
输入参数:
参数1:本次回调时保存的目标节点指针
参数2:格式控制对应的整型
*/
const char* whitespaceCb(mxml_node_t *node, int where);

/*
功能:保存内存中的xml数据到文件并重新加载xml文件
输入参数:
参数1:保存的路径
参数2:xml的root节点指针
返回值:保存以后重新加载的xml的root节点
*/
mxml_node_t* saveNode(const char *pXmlPath, mxml_node_t *pNode);

/*
功能:修改文件权限
输入参数:
参数1:需要被修改文件权限的文件路径
参数2:目标权限代表的数字
返回值:修改权限的二级果，TRUE表示修改成功，FALSE表示修改失败
*/
BOOL chmodFile(const char* pSrcPath, int pMode);

/*
功能:拷贝文件
输入参数:
参数1:被拷贝的文件路径
参数2:拷贝的目标路径
返回值:拷贝的结果，TRUE表示拷贝成功，FALSE表示拷贝失败
*/
BOOL copyFile(const char* pSrcFile, const char* pDstFile);

/*
功能:根据传入的节点指针获取本节点的text
输入参数:需要查询的text内容的xml节点指针
返回值:本节点text内容的内存中的地址
*/
const char* getXmlText(mxml_node_t* pNode);

#endif /* __SYMEDIA_XML_H__ */


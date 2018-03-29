
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
	char	tag[TAG_MSG_LEN];		//对应xml中参数表节点的标签
	char 	type[TYPE_MSG_LEN];		//对应xml中参数表节点的type
	char	nodename[NODE_MSG_LEN];	//对应xml中参数表节点的nodename
	int 	attr;					//对应xml中参数表节点的attr,0:从系统获取,只读 1:从数据中心获取,可读写
	char 	keyname[KEY_MSG_LEN];	//对应xml中参数表节点的keyname
} xml_key_path_t;

/*
功能:设置paramList.xml的内存中的root节点
输入参数:loadConfigXml返回的内存中root节点的指针
返回值:无
*/
void setParamRoot(mxml_node_t *pXmlRootPt);

/*
功能:通过tr069节点名称获取对应的paramList.xml中对应的节点信息
输入参数:tr069节点名称,节点详细信息的结构体(用来填充查询到的节点详细信息)
返回值:查找的结果，TRUE表示成功，FALSE表示失败
*/
BOOL getParamForNode(const char *pNamePt, xml_key_path_t *pDataPt);

/*
功能:通过数据中心内部节点名称获取对应的paramList.xml中对应的节点信息
输入参数:数据中心内部节点名称,节点详细信息的结构体(用来填充查询到的节点详细信息)
返回值:查找的结果，TRUE表示成功，FALSE表示失败
*/
BOOL getParamForKey(const char *pNamePt, xml_key_path_t *pDataPt);

/*
功能:打印某个节点的详细信息
输入参数:xml_key_path_t结构体指针
返回值:无
*/
void showNode(xml_key_path_t *pNodePt);

#endif /* __SYMEDIA_PARAM_H__ */


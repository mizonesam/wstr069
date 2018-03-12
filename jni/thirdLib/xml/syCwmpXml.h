/***********************************************************************
*
* syCwmpXml.h
*
* Copyright (C) 2012 Lin jieling <mycourage0@126.com> And WangLei
* Copyright (C) 2003-2012 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/

#ifndef SYXML_H
#define SYXML_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/queue.h>
#include "mxml.h"

#define SY_MULTIINSTANCE  "MultiInstance"
#define SY_SINGLEINSTANCE "SingleInstance"

#define SY_BLANKNODE      0
#define SY_NO_BLANKNODE   1

#define SY_MAX_INSTANCE_NUM  255 

#define SY_IS_BLANK(c)	(((c) == '\n')||((c) == '\r')||((c)== '\t')||((c) == ' '))
#if 0
//属性节点
struct sy_attr{
    char name[64];
    char value[64];
};

//整个tag定义
struct sytm_cfg_attr_item{
	char elename[128];
	struct sytm_cfg_attr *attrs;
	//int elesize;
	int attrsize;
	TAILQ_ENTRY(sytm_cfg_attr_item) entry;
};

TAILQ_HEAD(sy_cfg_head, sytm_cfg_attr_item);

//链表定义
struct sytm_cfgattr_list{
	struct sy_cfg_head head;
	unsigned int size;
};

//初始化链表
void sytm_list_init(struct sytm_cfgattr_list *list);

//销毁链表
void sytm_list_clear(struct sytm_cfgattr_list *list);

//得到链表元素个数
inline unsigned int sytm_list_size(struct sytm_cfgattr_list *list);

//向链表中添加数据
int sytm_list_add(struct sytm_cfgattr_list *list,
		struct sytm_cfg_attr_item **item);

//从链表中移除数据
inline void sytm_list_remove(struct sytm_cfgattr_list *list,
		struct sytm_cfg_attr_item *item);

//解析整个xml文件得到素有元素和属性
int parse_xml_attr(mxml_node_t *root,struct sytm_cfgattr_list *attrlist);
#endif

int syADDNode(	const char *srcStr, 
						const char *type,
						const char *level,
						const char *rw,
						const char *ml,
						const char *attr,
						const char *attrEnable,
						const char *value);

//空白回调函数
const char* SyWhitespaceCb(mxml_node_t *node, int where);
bool SyLoadXml(const char *file);
void SySaveXml(const char* filePath);
bool SyLoadConfigXml();
void SySaveConfigXml();
int SySetAttrVar(const char *key, const char *attrname, const char *attrvalue);
int SyGetAttrVar(const char *key, const char *attrname, char *attrvalue);
int SySetAttrAttr(mxml_node_t* currentNode, char *value);
int SyGetAttrAttr(mxml_node_t* currentNode, char *value);
int SySetAttrAttrEnable(mxml_node_t* currentNode, char *value);
int SyGetAttrAttrEnable(mxml_node_t* currentNode, char *value);
int SySetAttrValue(mxml_node_t* currentNode, char *value);
int SyGetAttrValue(mxml_node_t* currentNode, char *value);
bool SyGetAttrType(mxml_node_t* currentNode, char *value, const char*);
int SyGetAttrLevel(mxml_node_t* currentNode, char *value);
int SyGetAttrRW(mxml_node_t* currentNode, char *value);
int SyGetAttrInstance(mxml_node_t* currentNode, char *value);
int SySetAttrInstance(mxml_node_t* currentNode, char *value);
int SyIsNodeCheck(mxml_node_t* currentNode);
int SyGetNameListByPath(char* path, char* list, char* nextLevel);
int SyGetNodeInstance(mxml_node_t* currentNode , char* value);
int SyXmlIsBlankNode(mxml_node_t* node);
mxml_node_t* SyFindPathNode(const char *path);
mxml_node_t* SyFindBrotherName(mxml_node_t *currentNode, char *name);
mxml_node_t* SyGetBrotherNode(mxml_node_t* currentNode);
mxml_node_t* SyFindNextBrother(mxml_node_t* node);
mxml_node_t* SyGetChildrenNode(mxml_node_t* currentNode);
mxml_node_t* SyFindFirstChild(mxml_node_t* node);
mxml_node_t* SySaveNode(mxml_node_t *node);
int SyGetNoUsedInstanceNum(mxml_node_t *node);
char* SyXmlCalcNodeNum(char* path, mxml_node_t *pCurrentNode, int flag);
#endif

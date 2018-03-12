/***********************************************************************
*
* syCwmpXml.c
*
* Copyright (C) 2012 Lin jieling <mycourage0@126.com> And WangLei
* Copyright (C) 2003-2012 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/
#include "syCwmpXml.h"
#include "../../syCwmpCommon.h"

enum{
    LoadSuccess,
    LoadFailed
}LoadConfigXmlStatus;

FILE* gnFpRoot = NULL;
mxml_node_t* gnNodeRoot = NULL;
extern int gSyPathNumber;
extern char gSyPathList[204800];

#if 0
void sytm_list_init(struct sytm_cfgattr_list *list)
{
	TAILQ_INIT(&list->head);
	list->size = 0;
}

void sytm_list_clear(struct sytm_cfgattr_list * list)
{
	struct sytm_cfg_attr_item *item;

	while((item = TAILQ_FIRST(&list->head))) {
		TAILQ_REMOVE(&list->head, item, entry);
		if(item->attrs) free(item->attrs);
		free(item);
	}
}

unsigned int sytm_list_size(struct sytm_cfgattr_list *list)
{
	return list->size;
}

int sytm_list_add(struct sytm_cfgattr_list *list,struct sytm_cfg_attr_item **item)
{
	TAILQ_INSERT_TAIL(&list->head, *item, entry);
	list->size ++;
	return 0;
}

void sytm_list_remove(struct sytm_cfgattr_list *list,
		struct sytm_cfg_attr_item *item)
{
	TAILQ_REMOVE(&list->head, item, entry);
	list->size --;
	free(item);
}

int parse_xml_attr(mxml_node_t *tree,struct sytm_cfgattr_list *attrlist){
	mxml_node_t *node = NULL;
	int i = -1;
	char name[128] = {0};
	
	if(tree == NULL) return SY_FAILED;
	DO;

	sytm_list_init(attrlist);
	for( node = mxmlWalkNext(tree,tree,MXML_DESCEND);
		 node; 
		 node = mxmlWalkNext(node,tree,MXML_DESCEND)
	)
	{
		memset(&name,0,sizeof(name));
		if(node->type != MXML_ELEMENT){
			continue;
		}

		strcpy(name,node->value.element.name);
		int num = node->value.element.num_attrs;
		if(num == 0){
			continue;
		}

		struct sytm_cfg_attr_item *item = malloc(sizeof(struct sytm_cfg_attr_item));
		item->attrs = malloc(sizeof(struct sytm_cfg_attr)*num);
		strcpy(item->elename,name);
		item->attrsize = num;
	
		for(i = 0;i < num ;i++){
			strcpy(item->attrs[i].name,node->value.element.attrs[i].name);
			strcpy(item->attrs[i].value,node->value.element.attrs[i].value);
		}

		sytm_list_add(attrlist,&item);
	}

	return SY_SUCCESS;
}

#endif

int syADDNode(	const char *srcStr, 
						const char *type,
						const char *levels,
						const char *rw,
						const char *ml,
						const char *attr,
						const char *attrEnable,
						const char *value)
{
	DO;
	DPrint("src node:%s, gnNodeRoot:%p\n", srcStr, gnNodeRoot);
	int i, j, z, num = 1;
	
	for (i=0; srcStr[i] != '\0'; i++){
		if (srcStr[i] == '.'){
			num++;
		}
	}

	char buf[10][64] = {0};
	for (i=0, j=0, z=0; srcStr[i] != '\0'; i++){
		if (srcStr[i] != '.'){
			buf[j][z++] = srcStr[i];
		} else {
			j++;
			z = 0;
		}
	}


	mxml_node_t *node = NULL;
	mxml_node_t *lNode = NULL;

	for (i = 0, node = gnNodeRoot; i < num - 1; i++){
		//DPrint("node name:%s\n", buf[i]);
		lNode = mxmlFindElement(node, node, buf[i], NULL, NULL, MXML_DESCEND_FIRST);
		if (NULL != lNode){
			// DPrint("node:%s is exist.\n", buf[i]);
			node = lNode;
			continue;
		}
		lNode = mxmlNewElement(node, buf[i]);
		if (NULL != lNode){
			mxmlElementSetAttr(lNode, "type", "SingleInstance");
			const char *level = mxmlElementGetAttr(node, "level");
			if (NULL != level){
				DPrint("node:%s, set level:%s\n", buf[i], level);
				char lBuf[128] = {0};
				int levelNum = atoi(level);
				levelNum += 1;
				snprintf(lBuf, sizeof(lBuf), "%d", levelNum);
				mxmlElementSetAttr(lNode, "level", lBuf);
			}
		}
		node = lNode;
	}

	lNode = mxmlFindElement(node, node, buf[i], NULL, NULL, MXML_DESCEND_FIRST);
	if (NULL != lNode){
		DPrint("node:%s is exist.\n", buf[i]);
		return SY_SUCCESS;
	}
	lNode = mxmlNewElement(node, buf[i]);
	if (NULL == lNode){
		DPrint("create node:%s error.\n", buf[i]);
		return SY_FAILED;
	}
	mxmlElementSetAttr(lNode, "type", type?type:"string");

	const char *levelPtr = mxmlElementGetAttr(node, "level");
	if (NULL != levelPtr){
		char lBuf[128] = {0};
		int levelNum = atoi(levelPtr);
		levelNum += 1;
		snprintf(lBuf, sizeof(lBuf), "%d", levelNum);
		mxmlElementSetAttr(lNode, "level", lBuf);
	}

	mxmlElementSetAttr(lNode, "rw", rw?rw:"W");
	if (ml){
		mxmlElementSetAttr(lNode, "ml", ml);
	} else {
		if (!type){
			mxmlElementSetAttr(lNode, "ml", "256");
		}
	}
	mxmlElementSetAttr(lNode, "attr", attr?attr:"0");
	mxmlElementSetAttr(lNode, "attrEnable", attrEnable?attrEnable:"0");
	mxmlElementSetAttr(lNode, "value", value?value:"");
	

	DPrint("<---, gnNodeRoot:%p\n", gnNodeRoot);
	return SY_SUCCESS;
}

bool SyLoadXml(const char *file)
{
    DO;

    if (gnNodeRoot){
        mxmlDelete(gnNodeRoot);
    }
    gnFpRoot = fopen(file, "rw");
    if (NULL == gnFpRoot){
        DPrint("Open Config XML Failed\n");
        return false;
    }
    gnNodeRoot = mxmlLoadFile(NULL, gnFpRoot, MXML_TEXT_CALLBACK);
	if (NULL == gnNodeRoot){
		DPrint("gnNodeRoot is NULL, loalXML failed.\n");
		return false;
	}
	else {
		/*远程开关节点*/
		syADDNode("Device.X_00E0FC.RemoteControlEnable", "boolean", NULL, NULL, NULL, NULL, NULL, NULL);
		/*一键信息收集节点*/
		syADDNode("Device.X_00E0FC.DebugInfo.State", "boolean", NULL, NULL, NULL, NULL, NULL, NULL);
		syADDNode("Device.X_00E0FC.DebugInfo.Action", "boolean", NULL, NULL, NULL, NULL, NULL, "1");
		syADDNode("Device.X_00E0FC.DebugInfo.UploadURL", NULL, NULL, NULL, "256", NULL, NULL, NULL);
		syADDNode("Device.X_00E0FC.DebugInfo.CapCommand", NULL, NULL, NULL, "256", NULL, NULL, NULL);
		syADDNode("Device.X_00E0FC.DebugInfo.Username", NULL, NULL, NULL, "256", NULL, NULL, NULL);
		syADDNode("Device.X_00E0FC.DebugInfo.Password", NULL, NULL, NULL, "256", NULL, NULL, NULL);
		/*开机信息收集节点*/
		syADDNode("Device.X_00E0FC.StartupInfo.State", "boolean", NULL, NULL, NULL, NULL, NULL, NULL);
		syADDNode("Device.X_00E0FC.StartupInfo.Action", "boolean", NULL, NULL, NULL, NULL, NULL, "1");
		syADDNode("Device.X_00E0FC.StartupInfo.UploadURL", NULL, NULL, NULL, "256", NULL, NULL, NULL);
		syADDNode("Device.X_00E0FC.StartupInfo.Username", NULL, NULL, NULL, "256", NULL, NULL, NULL);
		syADDNode("Device.X_00E0FC.StartupInfo.Password", NULL, NULL, NULL, "256", NULL, NULL, NULL);
		syADDNode("Device.X_00E0FC.StartupInfo.MaxSize", "int", NULL, NULL, NULL, NULL, NULL, "0");
		/*融合终端节点*/
		syADDNode("Device.X_CTC_IPTV.ServiceInfo.BroadbandAccount", NULL, NULL, NULL, "64", NULL, NULL, NULL);
		syADDNode("Device.X_CTC_IPTV.ServiceInfo.BroadbandPassword", NULL, NULL, NULL, "64", NULL, NULL, NULL);
		syADDNode("Device.X_CTC_IPTV.ServiceInfo.SSID", NULL, NULL, NULL, "64", NULL, NULL, NULL);
		syADDNode("Device.X_00E0FC.ServiceInfo.BroadbandAccount", NULL, NULL, NULL, "64", NULL, NULL, NULL);
		syADDNode("Device.X_00E0FC.ServiceInfo.BroadbandPassword", NULL, NULL, NULL, "64", NULL, NULL, NULL);
		syADDNode("Device.X_00E0FC.ServiceInfo.SSID", NULL, NULL, NULL, "64", NULL, NULL, NULL);

		syADDNode("Device.X_CTC_IPTV.STBID", NULL, NULL, NULL, "256", NULL, NULL, NULL);
		syADDNode("Device.X_CTC_IPTV.ServiceInfo.UserIDPassword", NULL, NULL, NULL, "256", NULL, NULL, NULL);
		syADDNode("Device.X_00E0FC.ServiceInfo.UserIDPassword", NULL, NULL, NULL, "256", NULL, NULL, NULL);
		/*添加后保存一次到本地*/
		SySaveConfigXml();
	}

	fclose(gnFpRoot);
	gnFpRoot = NULL;
    return true;
}

void SySaveXml(const char* filePath)
{
	DO;

    if(!gnNodeRoot){
        DPrint("gnNodeRoot is NULL\n");
        return;
    }

    FILE* fp = NULL;
    fp = fopen(filePath, "w+");
    if (NULL == fp){
        DPrint("Save Config XML Failed\n");
        return;
    }
	mxmlSaveFile(gnNodeRoot, fp, SyWhitespaceCb);
    fclose(fp);
    sync();
}

bool SyLoadConfigXml()
{
    DO;

    char szCopyCmd[64] = {0};
    DPrint("XML_PATH:%s", SY_CONFIG_XML_PATH);   
	if (access(SY_CONFIG_XML_PATH, F_OK) == 0)
	{
		DPrint("%s exist.", SY_CONFIG_XML_PATH);
	}
	else
	{	
		sprintf(szCopyCmd, "cp /system/etc/syConfig.conf %s", SY_CONFIG_XML_PATH);
		system(szCopyCmd);
		DPrint("cp /system/etc/syConfig.conf to %s", SY_CONFIG_XML_PATH);
	}
    /*
	char cpCmd[64] = {0};
	sprintf(cpCmd, "cp /system/etc/syConfig.conf %s",SY_CONFIG_XML_PATH);
	system(cpCmd);
	DPrint("cp /system/etc/syConfig.conf to %s \n", SY_CONFIG_XML_PATH);
	*/
	if (access(SY_CONFIG_XML_PATH, R_OK|W_OK) != 0){
		DPrint("%s have no read and write permission\n", SY_CONFIG_XML_PATH);
		return false;
	}
    return SyLoadXml(SY_CONFIG_XML_PATH);
}

void SySaveConfigXml()
{
    SySaveXml(SY_CONFIG_XML_PATH);
}

mxml_node_t* SyGetRootElement()
{
    return mxmlFindElement(gnNodeRoot, gnNodeRoot, NULL, NULL, NULL, MXML_DESCEND_FIRST);
}

int SyGetAttrVar(const char *key, const char *attrname, char *attrvalue){
    // DO;
    mxml_node_t *node = NULL;

    node = mxmlFindElement(gnNodeRoot, gnNodeRoot, key, attrname, NULL, MXML_DESCEND);
    if(NULL == node){
        DPrint("Not Find Element\n");
        return SY_FAILED;
    }
    const char *value = mxmlElementGetAttr(node, attrname);
    strcpy(attrvalue, value);

    return SY_SUCCESS;
}

int SySetAttrVar(const char *key, const char *attrname, const char *attrvalue){
    // DO;
    mxml_node_t *node = NULL;

    node = mxmlFindElement(gnNodeRoot, gnNodeRoot, key, attrname, NULL, MXML_DESCEND);
    if(NULL == node){
        DPrint("Not Find Element\n");
        return SY_FAILED;
    }

    if(strcmp(attrname, "value") == 0){
        DPrint("Start Write Data\n");
        mxmlElementSetAttr(node, attrname, attrvalue);
    }
    
	return SY_SUCCESS;
}

int SySetAttrAttr(mxml_node_t* currentNode, char *value){
    mxmlElementSetAttr(currentNode, "attr", value);
    return SY_SUCCESS;
}

int SyGetAttrAttr(mxml_node_t* currentNode, char *value){
    const char* tmpStr = mxmlElementGetAttr(currentNode, "attr");
    if(NULL == tmpStr) {
        return SY_FAILED;
    } else {
        strcpy(value, tmpStr);
        return SY_SUCCESS;
    }
}

int SySetAttrAttrEnable(mxml_node_t* currentNode, char *value){
    mxmlElementSetAttr(currentNode, "attrEnable", value);
    return SY_SUCCESS;
}

int SyGetAttrAttrEnable(mxml_node_t* currentNode, char *value){
    const char* tmpStr = mxmlElementGetAttr(currentNode, "attrEnable");
    if(NULL == tmpStr) {
        return SY_FAILED;
    } else {
        strcpy(value, tmpStr);
        return SY_SUCCESS;
    }
}

int SyGetAttrValue(mxml_node_t* currentNode, char *value){
    if(NULL == currentNode)
        return SY_FAILED;
    
    const char* tmpStr = mxmlElementGetAttr(currentNode, "value");
    if(NULL == tmpStr) {
        return SY_FAILED;
    }
    else {
        strcpy(value, tmpStr);
        return SY_SUCCESS;
    }
}

int SySetAttrValue(mxml_node_t* currentNode, char *value){
    mxmlElementSetAttr(currentNode, "value", value);
    return SY_SUCCESS;
}

bool SyGetAttrType(mxml_node_t* currentNode, char *value, const char* def){
    if(NULL == currentNode)
        return false;
    
    const char* tmpStr = mxmlElementGetAttr(currentNode, "type");
    if(NULL == tmpStr) {
        strcpy(value, def);
        return false;
    } else {
        strcpy(value, tmpStr);
        return true;
    }
}

int SyGetAttrLevel(mxml_node_t* currentNode, char *value){ 
    const char* tmpStr = mxmlElementGetAttr(currentNode, "level");
    if(NULL == tmpStr) {
        return SY_FAILED;
    } else {
        strcpy(value, tmpStr);
        return SY_SUCCESS;
    }
}

int SyGetAttrRW(mxml_node_t* currentNode, char *value){
    const char* tmpStr = mxmlElementGetAttr(currentNode, "rw");
    if(NULL == tmpStr) {
        return SY_FAILED;
    } else {
        strcpy(value, tmpStr);
        return SY_SUCCESS;
    }
}

int SyGetAttrInstance(mxml_node_t* currentNode, char *value){
    const char* tmpStr = mxmlElementGetAttr(currentNode, "Instance");
    if(NULL == tmpStr) {
        return SY_FAILED;
    } else {
        strcpy(value, tmpStr);
        return SY_SUCCESS;
    }
}

int SySetAttrInstance(mxml_node_t* currentNode, char *value){
    mxmlElementSetAttr(currentNode, "Instance", value);
    return SY_SUCCESS;
}


int SyIsNodeCheck(mxml_node_t* currentNode){
    char tmpType[256];
    DPrint("name:%s\n", currentNode->value.element.name);
    if(strcmp(currentNode->value.element.name, SY_MULTIINSTANCE) == 0 ||
        strcmp(currentNode->value.element.name, SY_SINGLEINSTANCE) == 0)
        return SY_SUCCESS;
    
    SyGetAttrType(currentNode, tmpType, "string");
    DPrint("tmpType:%s\n", tmpType);
    if(strcmp(tmpType, SY_MULTIINSTANCE) == 0 || 
        strcmp(tmpType, SY_SINGLEINSTANCE) == 0){
        return SY_FAILED;
    } else {
        return SY_SUCCESS;
    }
}

int SyGetNameListByPath(char* path, char* list, char* nextLevel){
    int RetNum = 0;   
    int szNextLevel = 0;
    int szIsCompletePath = 0;
    char num[8];
    char type[64];
    char tmpPath[1024] = {0};
    char* sNum = num;
    char* sType = type;
    mxml_node_t* child;
    mxml_node_t* currentNode;
    // DO;

    if ((0 == strcmp(path, "")) || (0 == strlen(path))){
        strcpy(tmpPath, ".");
    } else {
        strcpy(tmpPath, path);
    }
    DPrint("tmpPath:%s\n", tmpPath);

    if(tmpPath[strlen(tmpPath) - 1] != '.'){
        szIsCompletePath = 1;
    } else {
        szIsCompletePath = 0;
    }
    DPrint("tmpPath:%s\n", tmpPath);

    if (0 == strcmp(nextLevel, "true")){
        szNextLevel = 1;
    } else {
        szNextLevel = 0;
    }
    
    currentNode = SyFindPathNode(tmpPath);
	if (NULL == currentNode){
		return 0;
	}
    DPrint("szIsCompletePath:%d, name:%s\n", szIsCompletePath, 
        currentNode->value.element.name);
    if (1 == szIsCompletePath){
        RetNum = 1;
        strncat(list, "+", 1);
    } else {
        if (0 == strcmp(tmpPath, ".")){
            strncat(list, currentNode->value.element.name, 
                strlen(currentNode->value.element.name));
            strncat(list, ".&", 2);
        }
        
        if(NULL == currentNode){
            return 0;
        }
        currentNode = SyGetChildrenNode(currentNode);
        if(NULL == currentNode){
            return 0;
        }
        
        DPrint("name0:%s\n", currentNode->value.element.name);
        while(currentNode 
            && strcmp(currentNode->value.element.name, "text") != 0){
            RetNum++;
            child = SyGetChildrenNode(currentNode);
            if(child) {
                DPrint("name1:%s\n", currentNode->value.element.name);
                if(!strcmp(currentNode->value.element.name, SY_MULTIINSTANCE)){
                    SyGetNodeInstance(currentNode, sNum);
                    strncat(list, sNum, strlen(sNum));
                } else {
                    strncat(list, currentNode->value.element.name, 
                        strlen(currentNode->value.element.name));
                }
                strncat(list, ".+", 2);
            } else {
                DPrint("name2:%s\n", currentNode->value.element.name);
                strncat(list, currentNode->value.element.name, 
                    strlen(currentNode->value.element.name));
                memset(type, 0, sizeof(type));
                SyGetAttrType(currentNode, sType, "string");
                DPrint("sType:%s\n", sType);
                if(!strcmp(sType, SY_MULTIINSTANCE)){
                    strncat(list, ".+", 2);
                } else {
                    strncat(list, "+", 1);
                }
            }
            currentNode = SyGetBrotherNode(currentNode) ;
        }
    }

    return RetNum;
}

mxml_node_t* SyFindPathNode(const char *path)
{
    char* tmpStr = NULL;
    char  tmpPath[256] = {0};
    mxml_node_t* currentNode = NULL;

    if(!(currentNode = SyGetRootElement()))
    {
    	EPrint("Root node not found");
    	return NULL;
    }
    strncpy(tmpPath, path, strlen(path)>255?255:strlen(path));
    tmpStr = strtok(tmpPath, ".");
    while(currentNode && tmpStr){
        currentNode = SyFindBrotherName(currentNode, tmpStr);
        if(NULL == currentNode)
            return NULL;

        tmpStr = strtok(NULL, ".");
        if(tmpStr){
            currentNode = SyGetChildrenNode(currentNode);
            if(NULL == currentNode)
                return NULL;
        }
    }
    return currentNode;
}

mxml_node_t* SyFindBrotherName(mxml_node_t *currentNode, char *name)
{
    char  tmpData[256] = {0};
    char* pSzdata = tmpData;
    mxml_node_t* ptr = currentNode;

    // DO;

    while(ptr){
        if(!strcmp(SY_MULTIINSTANCE, ptr->value.element.name)){
            SyGetNodeInstance(ptr, pSzdata);
            if(atoi(pSzdata) == atoi(name))
                return ptr;
        }
        else {
            if(!strcmp(ptr->value.element.name, name))
                return ptr;
        }
        ptr = SyGetBrotherNode(ptr);
    }
    return NULL;
}

mxml_node_t* SyGetBrotherNode(mxml_node_t* currentNode){
    mxml_node_t* brother;

    brother = SyFindNextBrother(currentNode);
    if(!brother)
        return NULL;

    if(MXML_ELEMENT != brother->type){
        brother = SyFindNextBrother(brother);
        if(NULL == brother)
            return NULL;
    }
    return brother;
}

mxml_node_t* SyFindNextBrother(mxml_node_t* node){
    mxml_node_t* brother = NULL;

    if(node)
        brother = mxmlFindElement(node, node->parent, NULL, NULL, 
                        NULL, MXML_NO_DESCEND);
    return brother;
}

int SyGetNodeInstance(mxml_node_t* currentNode , char* value){
    strcpy(value, mxmlElementGetAttr(currentNode, "Instance"));
    return SY_SUCCESS;
}

mxml_node_t* SyGetChildrenNode(mxml_node_t* currentNode)
{
    mxml_node_t* child = NULL;

    // DO;

    child = SyFindFirstChild(currentNode);
    if(NULL == child)
        return NULL;

    // D("name:%s, type:%d\n", child->value.element.name, child->type);

    while(child && SyXmlIsBlankNode(child)){
        child = SyFindNextBrother(child);
        if(NULL == child)
            return NULL;
    }

    return child;
}

mxml_node_t* SyFindFirstChild(mxml_node_t* node){
    mxml_node_t* child;
    if(!node)  
        return NULL;
    child = mxmlFindElement(node, node, NULL, NULL, NULL, 
                        MXML_DESCEND_FIRST);
    return child;
}

int SyXmlIsBlankNode(mxml_node_t* node){
    char* current;

    if(NULL == node) 
        return SY_BLANKNODE;
    if(MXML_TEXT != node->type) 
        return SY_BLANKNODE;
    current = node->value.text.string;
#ifdef SY_TEST_DEMO
    DPrint("current:%s\n", current);
#endif
    while(0 != *current){
        if(!SY_IS_BLANK(*current)) 
            return SY_BLANKNODE;
            current++;
    }
    return SY_NO_BLANKNODE;
}

const char* SyWhitespaceCb(mxml_node_t *node, int where){
    mxml_node_t    *parent;
    int        level;
    const char    *name;
    static const char *tabs = "\t\t\t\t";

    name = node->value.element.name;

    if (!strncmp(name, "?xml", 4))
    {
        if (where == MXML_WS_AFTER_OPEN)
            return ("\n");
        else
            return (NULL);
    } else if (where == MXML_WS_BEFORE_OPEN){
        for (level = -1, parent = node->parent; parent; level ++, parent = parent->parent)
            ; // appease -Wempty-body
            if (level > 4)
                level = 4;
            else if (level < 0)
                level = 0;

            return (tabs + 4 - level);
    } else{
        return "\n";
    }

    return (NULL);
}

int SyFindBrotherInstance(mxml_node_t *currentNode, int num){
    mxml_node_t* pNode = NULL;

    pNode = SyGetChildrenNode(currentNode);
    DPrint("pNode:%p\n", pNode);
    while (pNode){
        DPrint("name:%s\n", pNode->value.element.name);
        if (num == atoi(pNode->value.element.name))
            return 1;
        pNode = SyGetBrotherNode(pNode);
    }
    return 0;
}

int SyGetNoUsedInstanceNum(mxml_node_t *node) {
    int number = 1;
    mxml_node_t* pNode = NULL;
    // DO;

#if 0
    while (number < SY_MAX_INSTANCE_NUM) {
        if (0 == SyFindBrotherInstance(node, number)){
            DPrint("number:%d\n", number);
            return number;
        }
        number++;
    }
#else
    pNode = SyGetChildrenNode(node);
    DPrint("pNode:%p\n", pNode);
    while (pNode){
        DPrint("name:%s\n", pNode->value.element.name);
        number = atoi(pNode->value.element.name);
        pNode = SyGetBrotherNode(pNode);
    }

    number++;
    DPrint("number1:%d\n", number);
    if (number < SY_MAX_INSTANCE_NUM) {
        return number;
    }
#endif

    return 0;
}

mxml_node_t* SySaveNode(mxml_node_t *node){
    FILE* fp = NULL;
    mxml_node_t* saveNode;

    DO;
    
    if(!node)  
        return NULL;

    fp = fopen(SY_COMMON_XML, "w+");
    if (NULL == fp){
        DPrint("Save Config XML Failed\n");
        return NULL;
    }
	mxmlSaveFile(node, fp, SyWhitespaceCb);  
    fclose(fp);
    
    fp = fopen(SY_COMMON_XML, "rw");
    if (NULL == fp){
        DPrint("Open Config XML Failed\n");
        return NULL;
    }
    saveNode = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    fclose(fp);
    
    DONE;
    return saveNode;
}

char* SyXmlCalcNodeNum(char* path, mxml_node_t *pCurrentNode, int flag){
    char tmpPath[512] = {0};
    mxml_node_t* tmpNode;
    mxml_node_t* tmpBrotherNode;  

    //DO;
    if (NULL == pCurrentNode) {
        return "";
    }

    if (0 == flag) {
        gSyPathNumber = 0;
    }
    
    tmpNode = SyGetChildrenNode(pCurrentNode);
    if (NULL == tmpNode) {
        gSyPathNumber++;
        //DPrint("gSyPathNumber:%d\n", gSyPathNumber);
        //DPrint("pCurrentNode1:%s\n", path);
        path[strlen(path) - 1] = '&';
        //DPrint("pCurrentNode2:%s\n", path);
        memcpy(gSyPathList + strlen(gSyPathList), path, strlen(path));
        SyXmlCalcNodeNum(NULL, NULL, 1);
    } else {
        tmpNode = SyGetChildrenNode(pCurrentNode);
        //DPrint("tmpNode:%s%s\n", path, tmpNode->value.element.name);
        sprintf(tmpPath, "%s%s.", path, tmpNode->value.element.name);
        SyXmlCalcNodeNum(tmpPath, tmpNode, 1);
        tmpBrotherNode = SyGetBrotherNode(tmpNode);
        while (NULL != tmpBrotherNode) {
            //DPrint("tmpBrotherNode:%s%s\n", path, tmpBrotherNode->value.element.name);
            sprintf(tmpPath, "%s%s.", path, tmpBrotherNode->value.element.name);
            SyXmlCalcNodeNum(tmpPath, tmpBrotherNode, 1);
            tmpBrotherNode = SyGetBrotherNode(tmpBrotherNode);       
        }
        SyXmlCalcNodeNum(NULL, NULL, 1);
    }
    //DONE;
	return (char*)NULL;
}

#if 0
int main(int argc,char **argv){
	struct sy_config_attr_item *item = NULL;
	int i = -1;
	printf("--->\n");
	char *keyname = "ConnectionRequestUsername";
	char *attrname= "v";
	char attrvalue[12] = {0};
	
	SySetAttrVar(keyname, attrname, "123456789");
	SyGetAttrVar(keyname, attrname, attrvalue);
	printf("attrname:%s attrvalue:%s\n",attrname,attrvalue);
#if 0
	parse_xml_attr(CONFIG_PATH);
	TAILQ_FOREACH(item,&(attrlist->head),entry){
		printf("------------------------------\n");
		printf("%s:\n",item->elename);
		for(i = 0; i < item->attrsize; i++){
			printf("%s:%s\n",item->attrs[i].name,item->attrs[i].value);
		}
	}

	sy_list_clear(attrlist);
#endif
	return 0;
}
#endif

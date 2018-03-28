
#include "cwmpXml.h"

BOOL loadXml(mxml_node_t **pNodeRootPt, const char *pFile) {

    DO;
	BOOL result = TRUE;
    if (*pNodeRootPt) {
        mxmlDelete(*pNodeRootPt);
    }
    FILE* tXmlFile = fopen(pFile, "rw");
    if (NULL != tXmlFile) {
        mxmlSetWrapMargin(500);
        *pNodeRootPt = mxmlLoadFile(NULL, tXmlFile, MXML_TEXT_CALLBACK);
		if (NULL == *pNodeRootPt) {
			DPrint("gnNodeRoot is NULL, loalXML failed!");
			result = FALSE;
		}
		fclose(tXmlFile);
		tXmlFile = NULL;
    }
	else {
		DPrint("cannot open file %s!", pFile);
		result = FALSE;
	}
    DONE;
    return result;
}

void saveXml(mxml_node_t *pNodeRootPt, const char* pFilePath) {
	
    if (!pNodeRootPt) {
        DPrint("gnNodeRoot is NULL!");
        return ;
    }

    FILE* tFilePt = NULL;
    tFilePt = fopen(pFilePath, "w+");
    if (NULL == tFilePt) {
        DPrint("Save Config XML Failed!");
        return ;
    }
	mxmlSaveFile(pNodeRootPt, tFilePt, whitespaceCb);
    fclose(tFilePt);
    sync();
}

mxml_node_t* loadConfigXml(const char* pSrcXmlFile, const char* pDstXmlFile, BOOL pFlag) {
	
    DO;

	if (!pDstXmlFile || !pSrcXmlFile) {
		DPrint("xml file is empty, please try again.");
		return FALSE;
	}
	BOOL result = FALSE;
    char szCopyCmd[512] = {0};
	if (!pFlag && access(pDstXmlFile, F_OK) == 0) {
		DPrint("%s exist.", pDstXmlFile);
	}
	else {
		copyFile(pSrcXmlFile, pDstXmlFile);
		chmodFile(pDstXmlFile, 666);
	}

	/* 如果加载失败，尝试加载xml三次,如果三次失败重新拷贝conf文件到/data/data/ */
	int i = 0;
	mxml_node_t *tXmlPt = NULL;
	for (i=0; i<3; i++) {
		result = loadXml(&tXmlPt, pDstXmlFile);
		if (!result) {
			sleep(1);
			continue;
		}
		break;
	}
	if (i == 3) {
		int re;
		re = copyFile(pSrcXmlFile, pDstXmlFile);
		DPrint("copy re is %d", re);
		re = chmodFile(pDstXmlFile, 666);
		DPrint("chmod re is %d", re);
		result = loadXml(&tXmlPt, pDstXmlFile);
	}
    return tXmlPt;
}

BOOL getAttrValue(mxml_node_t *pNodePt, char *pAttrName, char *pValue, int pLen) {
	
    if (NULL == pNodePt) {
        return FALSE;
	}
    const char* tmpStr = mxmlElementGetAttr(pNodePt, pAttrName);
    if (NULL == tmpStr) {
		DPrint("tempStr is NULL!");
        return FALSE;
    } 
    strncpy(pValue, tmpStr, pLen);
	return TRUE;
}

BOOL setAttrValue(mxml_node_t *pNode, char *pValue) {
	
    mxmlElementSetAttr(pNode, "value", pValue);
    return TRUE;
}

const char* whitespaceCb(mxml_node_t *pNode, int pWhere) {
	
    mxml_node_t       *tParent = NULL;
    int                tLevel = 0;
    const char        *tName = NULL;
    static const char *tTabs = "\t\t\t\t";

    tName = pNode->value.element.name;
    if (!strncmp(tName, "?xml", 4)) {
        if (pWhere == MXML_WS_AFTER_OPEN)
            return ("\n");
        else
            return (NULL);
    } 
	else if (pWhere == MXML_WS_BEFORE_OPEN) {
        for (tLevel = -1, tParent = pNode->parent; tParent; tLevel++, tParent = tParent->parent)
            if (tLevel > 4)
                tLevel = 4;
            else if (tLevel < 0)
                tLevel = 0;
            return (tTabs + 4 - tLevel);
    } 
	else {
        return "\n";
    }
    return (NULL);
}

mxml_node_t* saveNode(const char *pXmlPath, mxml_node_t *pNode) {
	
    FILE* tXmlPt = NULL;
    mxml_node_t* tSaveNode = NULL;

    DO;
    
    if (!pNode)  
        return NULL;

    tXmlPt = fopen(pXmlPath, "w+");
    if (NULL == tXmlPt) {
        DPrint("Save Config XML Failed\n");
        return NULL;
    }
	mxmlSaveFile(pNode, tXmlPt, whitespaceCb);  
    fclose(tXmlPt);
    
    tXmlPt = fopen(pXmlPath, "rw");
    if (NULL == tXmlPt) {
        DPrint("Open Config XML Failed");
        return NULL;
    }
    tSaveNode = mxmlLoadFile(NULL, tXmlPt, MXML_TEXT_CALLBACK);
    fclose(tXmlPt);
    
    DONE;
    return tSaveNode;
}

BOOL chmodFile(const char *pSrcPath, int pMode) {

	char tValue[1024] = {0};
	snprintf(tValue, sizeof(tValue), "chmod %d %s", pMode, pSrcPath);
	return system(tValue);
}

BOOL copyFile(const char *pSrcFile, const char *pDstFile) {
	
	char tValue[1024] = {0};
	snprintf(tValue, sizeof(tValue), "cp %s %s", pSrcFile, pDstFile);
	return system(tValue);
}

const char* getXmlText(mxml_node_t *pNode) {
	
	int tWhite = 1;
	return mxmlGetText(pNode, &tWhite);
}



#include "paramParse.h"
#include <pthread.h>

/* paramList.xml的root节点指针 */
static mxml_node_t *gXmlRoot = NULL;

/* 线程锁，确保多线程调用安全 */
static pthread_mutex_t gLock = PTHREAD_MUTEX_INITIALIZER;

void setParamRoot(mxml_node_t *pXmlRootPt) {

	gXmlRoot = pXmlRootPt;
}

BOOL getParamForNode(const char *pNodeNamePt, xml_key_path_t *pDataPt) {

	if (!gXmlRoot) {
		DPrint("param xml is not init.");
		return FALSE;
	}

	pthread_mutex_lock(&gLock);
	//LOGD("param name:%s.", pNodeNamePt);
	BOOL tResult = FALSE;
	mxml_node_t *tParamNodePt = mxmlFindElement(gXmlRoot, gXmlRoot, NULL, "nodename", pNodeNamePt, MXML_DESCEND);
	if (tParamNodePt) {
		memset(pDataPt, 0x00, sizeof(xml_key_path_t));
		strncpy(pDataPt->tag, tParamNodePt->value.element.name, TAG_MSG_LEN);
		getAttrValue(tParamNodePt, "type", pDataPt->type, TYPE_MSG_LEN);
		strncpy(pDataPt->nodename, pNodeNamePt, NODE_MSG_LEN);
		getAttrValue(tParamNodePt, "attr", pDataPt->attr, ATTR_MSG_LEN);
		getAttrValue(tParamNodePt, "keyname", pDataPt->keyname, KEY_MSG_LEN);
		tResult = TRUE;
	}
	pthread_mutex_unlock(&gLock);
	return tResult;
}

BOOL getParamForKey(const char *pKeyPt, xml_key_path_t *pDataPt) {

	if (!gXmlRoot) {
		DPrint("param xml is not init.");
		return FALSE;
	}

	pthread_mutex_lock(&gLock);
	//LOGD("param name:%s.", pKeyPt);
	BOOL tResult = FALSE;
	mxml_node_t *tParamNodePt = mxmlFindElement(gXmlRoot, gXmlRoot, NULL, "keyname", pKeyPt, MXML_DESCEND);
	if (tParamNodePt) {
		memset(pDataPt, 0x00, sizeof(xml_key_path_t));
		strncpy(pDataPt->tag, tParamNodePt->value.element.name, TAG_MSG_LEN);
		getAttrValue(tParamNodePt, "type", pDataPt->type, TYPE_MSG_LEN);
		getAttrValue(tParamNodePt, "nodename", pDataPt->nodename, NODE_MSG_LEN);
		getAttrValue(tParamNodePt, "attr", pDataPt->attr, ATTR_MSG_LEN);
		strncpy(pDataPt->keyname, pKeyPt, KEY_MSG_LEN);
		tResult = TRUE;
	}
	pthread_mutex_unlock(&gLock);
	return tResult;
}

void showNode(xml_key_path_t *pNodePt) {

	DPrint("tag:%s, type:%s, nodename:%s, attr:%s, keyname:%s.", pNodePt->tag, pNodePt->type, pNodePt->nodename, pNodePt->attr, pNodePt->keyname);
}


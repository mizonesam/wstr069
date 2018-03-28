
#include "eventParse.h"
#include "syCwmpManagement.h"

/* eventList.xml文件的内存的root节点 */
static mxml_node_t* gXmlRoot = NULL;

void setEventRoot(mxml_node_t *pXmlRootPt) {

	gXmlRoot = pXmlRootPt;
}

xml_event_list_t* getEvent(const char *pEventName) {

	if (!gXmlRoot) {
		DPrint("event xml is not init.");
		return FALSE;
	}

	return findNode(pEventName);
}

xml_event_list_t* findNode(const char *pEventName) {

	int tNum = findNum(pEventName);

	//LOGD("event:%s.", pEventName);
	xml_event_list_t *tXmlData = NULL;
	mxml_node_t *tNode = NULL;
	mxml_node_t *tListNode = NULL;
	tNode = mxmlFindElement(gXmlRoot, gXmlRoot, "event", "code", pEventName, MXML_DESCEND_FIRST);
	if (tNode) {
		tListNode = mxmlFindElement(tNode, tNode, "list", NULL, NULL, MXML_DESCEND_FIRST);
		if (tListNode) {
			mxml_node_t *tParamNode = NULL;
			tXmlData = (xml_event_list_t*)malloc(sizeof(xml_event_list_t));
			tXmlData->eventCode = strdup(pEventName);
			char** tDataPt = (char**)malloc(tNum * sizeof(char*));
			int i = 0;
			for (tParamNode = mxmlFindElement(tListNode, tListNode, "node", NULL, NULL,MXML_DESCEND);
         		tParamNode != NULL;
         		tParamNode = mxmlFindElement(tParamNode, tListNode, "node", NULL, NULL, MXML_DESCEND), i++)
         	{
         		char tValue[1024] = {0};
				if (getAttrValue(tParamNode, "value", tValue, sizeof(tValue))) {
					//LOGD("node value:%s.", tValue);
				}
				tDataPt[i] = strdup(tValue);
			}
			tXmlData->size = tNum;
			tXmlData->paramName = tDataPt;
		}
	}
	return tXmlData;
}

int findNum(const char* pEventName) {

	//LOGD("event:%s.", pEventName);
	int tNum = 0;
	mxml_node_t* tNode = NULL;
	mxml_node_t* tListNode = NULL;
	tNode = mxmlFindElement(gXmlRoot, gXmlRoot, "event", "code", pEventName, MXML_DESCEND_FIRST);
	if (tNode) {
		tListNode = mxmlFindElement(tNode, tNode, "list", NULL, NULL, MXML_DESCEND_FIRST);
		if (tListNode) {
			mxml_node_t *node = NULL;
			for (node = mxmlFindElement(tListNode, tListNode, "node", NULL, NULL, MXML_DESCEND);
         		node != NULL;
         		node = mxmlFindElement(node, tListNode, "node", NULL, NULL, MXML_DESCEND))
         	{
         		tNum++;
			}
		}
	}
	return tNum;
}

const char* getEventForType(syInformEventType pEventType) {

	switch (pEventType) {
		case SY_EVENT_BOOTSTRAP:
			break;
	    case SY_EVENT_BOOT:
			break;
	    case SY_EVENT_PERIODIC:
			break;
	    case SY_EVENT_VALUE_CHANGE:
			break;
	    case SY_EVENT_CONNECTION_REQUEST:
			break;
	    case SY_EVENT_TRANSFER_COMPLETE:
			break;
	    case SY_EVENT_DIAGNOSTICS_COMPLETE:
			break;
	    case SY_EVENT_REQUEST_DOWNLOAD:
			break;
	    case SY_EVENT_AUTONOMOUS_TRANSFER_COMPLETE:
			break;
	    case SY_EVENT_REBOOT:
			break;
	    case SY_EVENT_DOWNLOAD:
			break;
	    case SY_EVENT_UPLOAD:
			break;
	    case SY_EVENT_X_CTC_SHUT_DOWN:
			break;
	    case SY_EVENT_CTC_LOG_PERIODIC:
			break;
	    case SY_EVENT_X_00E0FC_ErrorCode:
			break;
		default:
			DPrint("error.");
	}
	return "";
}


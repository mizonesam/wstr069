#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include "sy_getparam.h"
#include "mxml.h"
#include "util.h"

struct sy_msg_list *msglist;
int type = -1;
int reboot = -1;
int getrpc = -1;

void GetAllParam(mxml_node_t *root){
	struct sy_msg_list_item *item = NULL;
	char name[128] = {0};
	char value[128] = {0};
	mxml_node_t *node = NULL;
	mxml_node_t *nnode = NULL;
	mxml_node_t *vnode = NULL;
	char p[128] = {0};
	
	msglist = malloc(sizeof(struct sy_msg_list));
	sy_list_init(msglist);
	DPrint("[%s] --->\n",__FUNCTION__);
	
	for (node = mxmlWalkNext(root, root,MXML_DESCEND);
		node != NULL;
	 	node = mxmlWalkNext(node, root,MXML_DESCEND))
	 {
	 	if (!node || node->type != MXML_ELEMENT){
			continue;
		}
			
		strcpy(p,node->value.element.name);
		if(p == NULL) continue;
		
		if(strcmp(p,"ParameterValueStruct") == 0){
			nnode = mxmlFindElement(node,node,"Name",NULL,NULL,MXML_DESCEND);
			if(!nnode){
				DPrint("[%s] nNode Name is NULL\n",__FUNCTION__);
			}else{
				if(nnode->child == NULL){
					strcpy(name,"");
					continue;
				}
			
				if(nnode->child->value.opaque != NULL)
					strcpy(name,nnode->child->value.opaque);
				else
					strcpy(name,"");
			}

			vnode = mxmlFindElement(node,node,"Value",NULL,NULL,MXML_DESCEND);
			if(!vnode){
				DPrint("[%s] vNode Value is NULL\n",__FUNCTION__);
			}else{
				if(vnode->child == NULL){
					strcpy(value,"");
					continue;
				}
				
				if(vnode->child->value.opaque != NULL)
					strcpy(value,vnode->child->value.opaque);
				else
					strcpy(value,"");
			}

			if(strlen(name) != 0 && strlen(value) != 0){
				sy_list_add_msg(msglist,name,value);
			}
		}else{
			continue;
		}
	 }
}

int Parse_Xml(mxml_node_t *root){
	mxml_node_t *node = NULL;
	char name[128] = {0};
	DPrint("[%s] --->\n",__FUNCTION__);
	
    for (node = mxmlWalkNext(root, root,MXML_DESCEND);
         node != NULL;
         node = mxmlWalkNext(node, root,MXML_DESCEND))
    {
		if (!node || node->type != MXML_ELEMENT){
			continue;
		}
			
		strcpy(name,node->value.element.name);
		//if(node->child == NULL && strcmp(name,"cwmp:GetRPCMethods")!= 0){
		//	
		//}else{
		//	continue;
		//}
		if(strcmp(name,"SOAP-ENV:Envelope") == 0){
			DPrint("[%s] exspected parent\n",__FUNCTION__);
			continue;
		}else if(strcmp(name,"SOAP-ENV:Header") == 0){
			continue;
		}else if(strcmp(name,"cwmp:ID") == 0){
			continue;
		}else if(strcmp(name,"SOAP-ENV:Body") == 0){
			DPrint("[%s] BODY--->\n",__FUNCTION__);
			continue;
		}else if(strcmp(name,"cwmp:Reboot") == 0){
			reboot = 1;
			continue;
		}else if(strcmp(name,"cwmp:GetRPCMethods") == 0){
			getrpc = 1;
			continue;
		}else if(strcmp(name,"cwmp:GetParameterNames") == 0){
			 type = 1;
			 continue;
		}else if(strcmp(name,"cwmp:SetParameterValues") == 0){
			DPrint("[%s] SET_PARAM--->\n",__FUNCTION__);
			type = 2;
			GetAllParam(node);
			continue;
		}else{
			//DPrint("[%s] TM Server Message Error\n",__FUNCTION__);
			continue;
		}
	}

	return 0;
}

int sy_msg_to_xml(struct symsg_t *msg, struct evbuffer *buffer)
{
	int rc;
	int major = 1;
	int minor = 1;
	char *session = "abc123defg435nnn32b1";
	
	if ((rc = sy_add_buffer(buffer, "<?xml version=\"1.0\"?>\n"
			"<"EVCPE_SOAP_ENV_XMLNS":Envelope "
			"xmlns:"EVCPE_SOAP_ENV_XMLNS
			"=\"http://schemas.xmlsoap.org/soap/envelope/\" "
			"xmlns:"EVCPE_SOAP_ENC_XMLNS
			"=\"http://schemas.xmlsoap.org/soap/encoding/\" "
			"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
			"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
			"xmlns:"EVCPE_CWMP_XMLNS
			"=\"urn:dslforum-org:cwmp-%d-%d\">\n"
			"<"EVCPE_SOAP_ENV_XMLNS":Header>\n"
			"<"EVCPE_CWMP_XMLNS":ID "
			EVCPE_SOAP_ENV_XMLNS":mustUnderstand=\"1\">%s"
			"</"EVCPE_CWMP_XMLNS":ID>\n"
			"</"EVCPE_SOAP_ENV_XMLNS":Header>\n"
			"<"EVCPE_SOAP_ENV_XMLNS":Body>\n",
			major, minor, session))) {
		DPrint("[%s] failed to append buffer--->\n",__FUNCTION__);
		goto finally;
	}
	#if 0
	switch(msg->type) {
	case SY_MSG_REQUEST:
		if ((rc = sy_add_buffer(buffer, "<"EVCPE_CWMP_XMLNS":%s>\n", method))) {
			DPrint("[%s] failed to append buffer",__FUNCTION__);
			goto finally;
		}
		switch(msg->method_type) {
		case SY_INFORM:
			if ((rc = evcpe_inform_to_xml(msg->data, buffer))) {
				evcpe_error(__func__, "failed to marshal inform");
				goto finally;
			}
			break;
		default:
			DPrint("[%s] unexpected request type: %d\n",__FUNCTION__, msg->method_type);
			rc = EINVAL;
			goto finally;
		}
		if ((rc = sy_add_buffer(buffer, "</"EVCPE_CWMP_XMLNS":%s>\n", method))) {
			DPrint("[%s] failed to append buffer",__FUNCTION__);
			goto finally;
		}
		break;
	case SY_MSG_RESPONSE:
		if ((rc = sy_add_buffer(buffer, "<"EVCPE_CWMP_XMLNS":%sResponse>\n", method))) {
			DPrint("[%s] failed to append buffer",__FUNCTION__);
			goto finally;
		}
		switch(msg->method_type) {
		case SY_GET_RPC_METHODS:
			if ((rc = evcpe_get_rpc_methods_response_to_xml(msg->data, buffer))) {
				DPrint("[%s] failed to get_rpc_methods_response\n",__FUNCTION__);
				goto finally;
			}
			break;
	
		case SY_GET_PARAMETER_VALUES:
			if ((rc = evcpe_get_param_values_response_to_xml(msg->data, buffer))) {
				DPrint("[%s] failed to marshal get_param_values_response\n",__FUNCTION__);
				goto finally;
			}
			break;
		case SY_SET_PARAMETER_VALUES:
			if ((rc = evcpe_set_param_values_response_to_xml(msg->data, buffer))) {
				DPrint("[%s] failed to marshal SET_param_values_response\n",__FUNCTION__);
				goto finally;
			}
			break;
		default:
			DPrint("[%s] unexpected response type: %d\n", msg->method_type);
			rc = EINVAL;
			goto finally;
		}
		if ((rc = sy_add_buffer(buffer, "</"EVCPE_CWMP_XMLNS":%sResponse>\n", method))) {
			DPrint("[%s] failed to append buffer",__FUNCTION__);
			goto finally;
		}
		break;
	case SY_MSG_FAULT:
		if ((rc = sy_add_buffer(buffer, "<"EVCPE_SOAP_ENV_XMLNS":Fault>\n"
				"<faultcode>Client</faultcode>\n"
				"<faultstring>CWMP fault</faultstring>\n"
				"<detail>\n"
				"<"EVCPE_CWMP_XMLNS":Fault>\n"))) {
			DPrint("[%s] failed to append buffer",__FUNCTION__);
			goto finally;
		}
		if ((rc = evcpe_fault_to_xml(msg->data, buffer))) {
			DPrint("[%s] failed to append buffer",__FUNCTION__);
			goto finally;
		}
		if ((rc = sy_add_buffer(buffer, "</"EVCPE_CWMP_XMLNS":Fault>\n"
				"</detail>\n"
				"</"EVCPE_SOAP_ENV_XMLNS":Fault>\n"))) {
			DPrint("[%s] failed to append buffer",__FUNCTION__);
			goto finally;
		}
		break;
	default:
		DPrint("[%s] unexpected message type: %d\n",__FUNCTION__, msg->type);
		rc = EINVAL;
		goto finally;
	}
	if ((rc = sy_add_buffer(buffer,
			"</"EVCPE_SOAP_ENV_XMLNS":Body>\n</"EVCPE_SOAP_ENV_XMLNS":Envelope>\n"))) {
		DPrint("[%s] failed to append buffer",__FUNCTION__);
		goto finally;
	}
	#endif
	rc = 0;

finally:
	return rc;
}

int CrateXml(){
	
}

int main(int argc,char **argv){
	FILE *fp = NULL;
	mxml_node_t *tree = NULL;
	mxml_node_t *root1 = NULL;
	struct sy_msg_list_item *item;
	
	fp = fopen(XMLPATH,"r");

	tree = mxmlLoadFile(NULL,fp,MXML_OPAQUE_CALLBACK/*MXML_NO_CALLBACK MXML_IGNORE_CALLBACK*/);
	if(tree == NULL){
		printf("Tree Is NULL\n");
	}
	fclose(fp);
	
	DPrint("[%s] --->\n",__FUNCTION__);
	Parse_Xml(tree);

	if(reboot == 1){
		DPrint("[%s] REBOOT...\n",__FUNCTION__);
	}

	if(getrpc == 1){
		DPrint("[%s] GetRPCMethod...\n",__FUNCTION__);
	}

	if(&(msglist->head) == NULL) return 0;
	TAILQ_FOREACH(item,&(msglist->head),entry){
		if(type == 2){
			DPrint("[%s] SET_PARAM:\n",__FUNCTION__);
			DPrint("[%s] name:%s value:%s\n",__FUNCTION__,item->name,item->value);
		}
	}

	sy_list_clear(msglist);
	mxmlRelease(tree);
	
	return 0;
}

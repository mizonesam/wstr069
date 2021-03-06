/* soapClient.c
   Generated by gSOAP 2.8.8 from syCwmp.h

Copyright(C) 2000-2012, Robert van Engelen, Genivia Inc. All Rights Reserved.
The generated code is released under one of the following licenses:
1) GPL or 2) Genivia's license for commercial use.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
*/

#if defined(__BORLANDC__)
#pragma option push -w-8060
#pragma option push -w-8004
#endif
#include "soapH.h"
#ifdef __cplusplus
extern "C" {
#endif

SOAP_SOURCE_STAMP("@(#) soapClient.c ver 2.8.8 2012-12-05 06:27:30 GMT")
void useless1(){}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_cwmp__Inform(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct _DeviceIdStruct *DeviceId, struct _EventStruct *Event, unsigned int MaxEnvelopes, char *CurrentTime, unsigned int RetryCount, struct _ParameterValueStruct *ParameterList, struct cwmp__InformResponse *_param_1)
{	struct cwmp__Inform soap_tmp_cwmp__Inform;
	soap->encodingStyle = NULL;
	soap_tmp_cwmp__Inform.DeviceId = DeviceId;
	soap_tmp_cwmp__Inform.Event = Event;
	soap_tmp_cwmp__Inform.MaxEnvelopes = MaxEnvelopes;
	soap_tmp_cwmp__Inform.CurrentTime = CurrentTime;
	soap_tmp_cwmp__Inform.RetryCount = RetryCount;
	soap_tmp_cwmp__Inform.ParameterList = ParameterList;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_cwmp__Inform(soap, &soap_tmp_cwmp__Inform);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_cwmp__Inform(soap, &soap_tmp_cwmp__Inform, "cwmp:Inform", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_cwmp__Inform(soap, &soap_tmp_cwmp__Inform, "cwmp:Inform", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_param_1)
		return soap_closesock(soap);
	soap_default_cwmp__InformResponse(soap, _param_1);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_cwmp__InformResponse(soap, _param_1, "cwmp:InformResponse", "");
	if (soap->error)
	{
		if(soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		return soap_recv_fault(soap, 0);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_cwmp__GetRPCMethods(struct soap *soap, const char *soap_endpoint, const char *soap_action, void *_, struct cwmp__GetRPCMethodsResponse *_param_2)
{	struct cwmp__GetRPCMethods soap_tmp_cwmp__GetRPCMethods;
	soap->encodingStyle = NULL;
	soap_tmp_cwmp__GetRPCMethods._ = _;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_cwmp__GetRPCMethods(soap, &soap_tmp_cwmp__GetRPCMethods);
    SOAP_DPrint("[%s] 1\n", __FUNCTION__);//linjieling
	if (soap_begin_count(soap))
		return soap->error;
    SOAP_DPrint("[%s] 2\n", __FUNCTION__);//linjieling
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_cwmp__GetRPCMethods(soap, &soap_tmp_cwmp__GetRPCMethods, "cwmp:GetRPCMethods", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
    SOAP_DPrint("[%s] 3\n", __FUNCTION__);//linjieling
	if (soap_end_count(soap))
		return soap->error;
    SOAP_DPrint("[%s] 4\n", __FUNCTION__);//linjieling
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_cwmp__GetRPCMethods(soap, &soap_tmp_cwmp__GetRPCMethods, "cwmp:GetRPCMethods", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
    SOAP_DPrint("[%s] 5\n", __FUNCTION__);//linjieling
	if (!_param_2)
		return soap_closesock(soap);
    SOAP_DPrint("[%s] 6\n", __FUNCTION__);//linjieling
	soap_default_cwmp__GetRPCMethodsResponse(soap, _param_2);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
    SOAP_DPrint("[%s] 7\n", __FUNCTION__);//linjieling
	if (soap_recv_fault(soap, 1))
		return soap->error;
    SOAP_DPrint("[%s] 8\n", __FUNCTION__);//linjieling
	soap_get_cwmp__GetRPCMethodsResponse(soap, _param_2, "cwmp:GetRPCMethodsResponse", "");
	if (soap->error)
	{
		if(soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		    return soap_recv_fault(soap, 0);
		return soap_closesock(soap);
	}
	SOAP_DPrint("[%s] 7\n", __FUNCTION__);//linjieling
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_cwmp__GetParameterValues(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct ParameterNames *ParameterNames, struct cwmp__GetParameterValuesResponse *_param_3)
{	struct cwmp__GetParameterValues soap_tmp_cwmp__GetParameterValues;
	soap->encodingStyle = NULL;
	soap_tmp_cwmp__GetParameterValues.ParameterNames = ParameterNames;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_cwmp__GetParameterValues(soap, &soap_tmp_cwmp__GetParameterValues);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_cwmp__GetParameterValues(soap, &soap_tmp_cwmp__GetParameterValues, "cwmp:GetParameterValues", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_cwmp__GetParameterValues(soap, &soap_tmp_cwmp__GetParameterValues, "cwmp:GetParameterValues", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_param_3)
		return soap_closesock(soap);
	soap_default_cwmp__GetParameterValuesResponse(soap, _param_3);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_cwmp__GetParameterValuesResponse(soap, _param_3, "cwmp:GetParameterValuesResponse", "");
	if (soap->error)
	{
		if(soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		    return soap_recv_fault(soap, 0);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_cwmp__SetParameterValues(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct ParameterValueList *ParameterList, char *ParameterKey, struct cwmp__SetParameterValuesResponse *_param_4)
{	struct cwmp__SetParameterValues soap_tmp_cwmp__SetParameterValues;
	soap->encodingStyle = NULL;
	soap_tmp_cwmp__SetParameterValues.ParameterList = ParameterList;
	soap_tmp_cwmp__SetParameterValues.ParameterKey = ParameterKey;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_cwmp__SetParameterValues(soap, &soap_tmp_cwmp__SetParameterValues);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_cwmp__SetParameterValues(soap, &soap_tmp_cwmp__SetParameterValues, "cwmp:SetParameterValues", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_cwmp__SetParameterValues(soap, &soap_tmp_cwmp__SetParameterValues, "cwmp:SetParameterValues", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_param_4)
		return soap_closesock(soap);
	soap_default_cwmp__SetParameterValuesResponse(soap, _param_4);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_cwmp__SetParameterValuesResponse(soap, _param_4, "cwmp:SetParameterValuesResponse", "");
	if(soap->error)
	{
		if(soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		    return soap_recv_fault(soap, 0);
		return soap_closesock(soap);
	}
	if(soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_cwmp__GetParameterNames(struct soap *soap, const char *soap_endpoint, const char *soap_action, char **ParameterPath, char *NextLevel, struct cwmp__GetParameterNamesResponse *_param_5)
{	struct cwmp__GetParameterNames soap_tmp_cwmp__GetParameterNames;
	soap->encodingStyle = NULL;
	soap_tmp_cwmp__GetParameterNames.ParameterPath = ParameterPath;
	soap_tmp_cwmp__GetParameterNames.NextLevel = NextLevel;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_cwmp__GetParameterNames(soap, &soap_tmp_cwmp__GetParameterNames);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_cwmp__GetParameterNames(soap, &soap_tmp_cwmp__GetParameterNames, "cwmp:GetParameterNames", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_cwmp__GetParameterNames(soap, &soap_tmp_cwmp__GetParameterNames, "cwmp:GetParameterNames", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_param_5)
		return soap_closesock(soap);
	soap_default_cwmp__GetParameterNamesResponse(soap, _param_5);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_cwmp__GetParameterNamesResponse(soap, _param_5, "cwmp:GetParameterNamesResponse", "");
	if(soap->error)
	{
		if(soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		    return soap_recv_fault(soap, 0);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_cwmp__SetParameterAttributes(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct SetParameterAttributesList *ParameterList, struct cwmp__SetParameterAttributesResponse *_param_6)
{	struct cwmp__SetParameterAttributes soap_tmp_cwmp__SetParameterAttributes;
	soap->encodingStyle = NULL;
	soap_tmp_cwmp__SetParameterAttributes.ParameterList = ParameterList;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_cwmp__SetParameterAttributes(soap, &soap_tmp_cwmp__SetParameterAttributes);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_cwmp__SetParameterAttributes(soap, &soap_tmp_cwmp__SetParameterAttributes, "cwmp:SetParameterAttributes", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_cwmp__SetParameterAttributes(soap, &soap_tmp_cwmp__SetParameterAttributes, "cwmp:SetParameterAttributes", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_param_6)
		return soap_closesock(soap);
	soap_default_cwmp__SetParameterAttributesResponse(soap, _param_6);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_cwmp__SetParameterAttributesResponse(soap, _param_6, "cwmp:SetParameterAttributesResponse", "");
	if(soap->error)
	{
		if(soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		    return soap_recv_fault(soap, 0);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_cwmp__GetParameterAttributes(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct ParameterNames *ParameterNames, struct cwmp__GetParameterAttributesResponse *_param_7)
{	struct cwmp__GetParameterAttributes soap_tmp_cwmp__GetParameterAttributes;
	soap->encodingStyle = NULL;
	soap_tmp_cwmp__GetParameterAttributes.ParameterNames = ParameterNames;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_cwmp__GetParameterAttributes(soap, &soap_tmp_cwmp__GetParameterAttributes);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_cwmp__GetParameterAttributes(soap, &soap_tmp_cwmp__GetParameterAttributes, "cwmp:GetParameterAttributes", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_cwmp__GetParameterAttributes(soap, &soap_tmp_cwmp__GetParameterAttributes, "cwmp:GetParameterAttributes", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_param_7)
		return soap_closesock(soap);
	soap_default_cwmp__GetParameterAttributesResponse(soap, _param_7);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_cwmp__GetParameterAttributesResponse(soap, _param_7, "cwmp:GetParameterAttributesResponse", "");
	if (soap->error)
	{
		if(soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		    return soap_recv_fault(soap, 0);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_cwmp__AddObject(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *ObjectName, char *ParameterKey, struct cwmp__AddObjectResponse *_param_8)
{	struct cwmp__AddObject soap_tmp_cwmp__AddObject;
	soap->encodingStyle = NULL;
	soap_tmp_cwmp__AddObject.ObjectName = ObjectName;
	soap_tmp_cwmp__AddObject.ParameterKey = ParameterKey;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_cwmp__AddObject(soap, &soap_tmp_cwmp__AddObject);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_cwmp__AddObject(soap, &soap_tmp_cwmp__AddObject, "cwmp:AddObject", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_cwmp__AddObject(soap, &soap_tmp_cwmp__AddObject, "cwmp:AddObject", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_param_8)
		return soap_closesock(soap);
	soap_default_cwmp__AddObjectResponse(soap, _param_8);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_cwmp__AddObjectResponse(soap, _param_8, "cwmp:AddObjectResponse", "");
	if (soap->error)
	{
		if(soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		    return soap_recv_fault(soap, 0);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_cwmp__AddObjectIPTV(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *ObjectName, char *ParameterKey, struct cwmp__AddObjectResponse *_param_9)
{	struct cwmp__AddObjectIPTV soap_tmp_cwmp__AddObjectIPTV;
	soap->encodingStyle = NULL;
	soap_tmp_cwmp__AddObjectIPTV.ObjectName = ObjectName;
	soap_tmp_cwmp__AddObjectIPTV.ParameterKey = ParameterKey;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_cwmp__AddObjectIPTV(soap, &soap_tmp_cwmp__AddObjectIPTV);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_cwmp__AddObjectIPTV(soap, &soap_tmp_cwmp__AddObjectIPTV, "cwmp:AddObjectIPTV", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_cwmp__AddObjectIPTV(soap, &soap_tmp_cwmp__AddObjectIPTV, "cwmp:AddObjectIPTV", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_param_9)
		return soap_closesock(soap);
	soap_default_cwmp__AddObjectResponse(soap, _param_9);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_cwmp__AddObjectResponse(soap, _param_9, "cwmp:AddObjectResponse", "");
	if (soap->error)
	{
		if(soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		    return soap_recv_fault(soap, 0);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_cwmp__DeleteObject(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *ObjectName, char *ParameterKey, struct cwmp__DeleteObjectResponse *_param_10)
{	struct cwmp__DeleteObject soap_tmp_cwmp__DeleteObject;
	soap->encodingStyle = NULL;
	soap_tmp_cwmp__DeleteObject.ObjectName = ObjectName;
	soap_tmp_cwmp__DeleteObject.ParameterKey = ParameterKey;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_cwmp__DeleteObject(soap, &soap_tmp_cwmp__DeleteObject);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_cwmp__DeleteObject(soap, &soap_tmp_cwmp__DeleteObject, "cwmp:DeleteObject", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_cwmp__DeleteObject(soap, &soap_tmp_cwmp__DeleteObject, "cwmp:DeleteObject", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_param_10)
		return soap_closesock(soap);
	soap_default_cwmp__DeleteObjectResponse(soap, _param_10);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_cwmp__DeleteObjectResponse(soap, _param_10, "cwmp:DeleteObjectResponse", "");
	if (soap->error)
	{
		if(soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		    return soap_recv_fault(soap, 0);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_cwmp__Reboot(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *CommandKey, struct cwmp__RebootResponse *_param_11)
{	struct cwmp__Reboot soap_tmp_cwmp__Reboot;
	soap->encodingStyle = NULL;
	soap_tmp_cwmp__Reboot.CommandKey = CommandKey;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_cwmp__Reboot(soap, &soap_tmp_cwmp__Reboot);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_cwmp__Reboot(soap, &soap_tmp_cwmp__Reboot, "cwmp:Reboot", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_cwmp__Reboot(soap, &soap_tmp_cwmp__Reboot, "cwmp:Reboot", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_param_11)
		return soap_closesock(soap);
	soap_default_cwmp__RebootResponse(soap, _param_11);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_cwmp__RebootResponse(soap, _param_11, "cwmp:RebootResponse", "");
	if (soap->error)
	{
		if(soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		    return soap_recv_fault(soap, 0);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_cwmp__Download(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *CommandKey, char *FileType, char *URL, char *Username, char *Password, int FileSize, char *TargetFileName, int DelaySeconds, char *SuccessURL, char *FailureURL, struct cwmp__DownloadResponse *_param_12)
{	struct cwmp__Download soap_tmp_cwmp__Download;
	soap->encodingStyle = NULL;
	soap_tmp_cwmp__Download.CommandKey = CommandKey;
	soap_tmp_cwmp__Download.FileType = FileType;
	soap_tmp_cwmp__Download.URL = URL;
	soap_tmp_cwmp__Download.Username = Username;
	soap_tmp_cwmp__Download.Password = Password;
	soap_tmp_cwmp__Download.FileSize = FileSize;
	soap_tmp_cwmp__Download.TargetFileName = TargetFileName;
	soap_tmp_cwmp__Download.DelaySeconds = DelaySeconds;
	soap_tmp_cwmp__Download.SuccessURL = SuccessURL;
	soap_tmp_cwmp__Download.FailureURL = FailureURL;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_cwmp__Download(soap, &soap_tmp_cwmp__Download);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_cwmp__Download(soap, &soap_tmp_cwmp__Download, "cwmp:Download", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_cwmp__Download(soap, &soap_tmp_cwmp__Download, "cwmp:Download", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_param_12)
		return soap_closesock(soap);
	soap_default_cwmp__DownloadResponse(soap, _param_12);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_cwmp__DownloadResponse(soap, _param_12, "cwmp:DownloadResponse", "");
	if (soap->error)
	{
		if(soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		    return soap_recv_fault(soap, 0);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_cwmp__Upload(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *CommandKey, char *FileType, char *URL, char *Username, char *Password, int DelaySeconds, struct cwmp__UploadResponse *_param_13)
{	struct cwmp__Upload soap_tmp_cwmp__Upload;
	soap->encodingStyle = NULL;
	soap_tmp_cwmp__Upload.CommandKey = CommandKey;
	soap_tmp_cwmp__Upload.FileType = FileType;
	soap_tmp_cwmp__Upload.URL = URL;
	soap_tmp_cwmp__Upload.Username = Username;
	soap_tmp_cwmp__Upload.Password = Password;
	soap_tmp_cwmp__Upload.DelaySeconds = DelaySeconds;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_cwmp__Upload(soap, &soap_tmp_cwmp__Upload);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_cwmp__Upload(soap, &soap_tmp_cwmp__Upload, "cwmp:Upload", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_cwmp__Upload(soap, &soap_tmp_cwmp__Upload, "cwmp:Upload", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_param_13)
		return soap_closesock(soap);
	soap_default_cwmp__UploadResponse(soap, _param_13);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_cwmp__UploadResponse(soap, _param_13, "cwmp:UploadResponse", "");
	if (soap->error)
	{
		if(soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		    return soap_recv_fault(soap, 0);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_cwmp__FactoryReset(struct soap *soap, const char *soap_endpoint, const char *soap_action, void *_, struct cwmp__FactoryResetResponse *_param_14)
{	struct cwmp__FactoryReset soap_tmp_cwmp__FactoryReset;
	soap->encodingStyle = NULL;
	soap_tmp_cwmp__FactoryReset._ = _;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_cwmp__FactoryReset(soap, &soap_tmp_cwmp__FactoryReset);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_cwmp__FactoryReset(soap, &soap_tmp_cwmp__FactoryReset, "cwmp:FactoryReset", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_cwmp__FactoryReset(soap, &soap_tmp_cwmp__FactoryReset, "cwmp:FactoryReset", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_param_14)
		return soap_closesock(soap);
	soap_default_cwmp__FactoryResetResponse(soap, _param_14);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_cwmp__FactoryResetResponse(soap, _param_14, "cwmp:FactoryResetResponse", "");
	if (soap->error)
	{
		if(soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		    return soap_recv_fault(soap, 0);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_cwmp__ScheduleInform(struct soap *soap, const char *soap_endpoint, const char *soap_action, int DelaySeconds, char *CommandKey, struct cwmp__ScheduleInformResponse *_param_15)
{	struct cwmp__ScheduleInform soap_tmp_cwmp__ScheduleInform;
	soap->encodingStyle = NULL;
	soap_tmp_cwmp__ScheduleInform.DelaySeconds = DelaySeconds;
	soap_tmp_cwmp__ScheduleInform.CommandKey = CommandKey;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_cwmp__ScheduleInform(soap, &soap_tmp_cwmp__ScheduleInform);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_cwmp__ScheduleInform(soap, &soap_tmp_cwmp__ScheduleInform, "cwmp:ScheduleInform", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_cwmp__ScheduleInform(soap, &soap_tmp_cwmp__ScheduleInform, "cwmp:ScheduleInform", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_param_15)
		return soap_closesock(soap);
	soap_default_cwmp__ScheduleInformResponse(soap, _param_15);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_cwmp__ScheduleInformResponse(soap, _param_15, "cwmp:ScheduleInformResponse", "");
	if (soap->error)
	{
		if(soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		    return soap_recv_fault(soap, 0);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_cwmp__TransferComplete(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *CommandKey, struct cwmp__FaultStruct FaultStruct, time_t StartTime, time_t CompleteTime, struct cwmp__TransferCompleteResponse *_param_16)
{	struct cwmp__TransferComplete soap_tmp_cwmp__TransferComplete;
	soap->encodingStyle = NULL;
	soap_tmp_cwmp__TransferComplete.CommandKey = CommandKey;
	soap_tmp_cwmp__TransferComplete.FaultStruct = FaultStruct;
	soap_tmp_cwmp__TransferComplete.StartTime = StartTime;
	soap_tmp_cwmp__TransferComplete.CompleteTime = CompleteTime;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_cwmp__TransferComplete(soap, &soap_tmp_cwmp__TransferComplete);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_cwmp__TransferComplete(soap, &soap_tmp_cwmp__TransferComplete, "cwmp:TransferComplete", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_cwmp__TransferComplete(soap, &soap_tmp_cwmp__TransferComplete, "cwmp:TransferComplete", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_param_16)
		return soap_closesock(soap);
	soap_default_cwmp__TransferCompleteResponse(soap, _param_16);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_cwmp__TransferCompleteResponse(soap, _param_16, "cwmp:TransferCompleteResponse", "");
	if (soap->error)
	{
		if(soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		    return soap_recv_fault(soap, 0);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

#ifdef __cplusplus
}
#endif

#if defined(__BORLANDC__)
#pragma option pop
#pragma option pop
#endif

/* End of soapClient.c */

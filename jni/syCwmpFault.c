/***********************************************************************
*
* syCwmpFault.c
*
* Copyright (C) 2012 Lin jieling <mycourage0@126.com>
* Copyright (C) 2003-2012 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/
#include "syCwmpCommon.h"
#include "syCwmpFault.h"

const char* SyGetFaultString(int faultCode)
{
    switch(faultCode)
    {
    case SY_CPE_METHOD_NOT_SUPPORTED:
        return "Method not supported";
    case SY_CPE_REQUEST_DENIED:
        return "Request denied";
    case SY_CPE_INTERNAL_ERROR:
        return "Internal Error";
    case SY_CPE_INVALID_ARGUMENTS:
        return "Invalid arguments";
    case SY_CPE_RESOUCES_EXCEEDS:
        return "Resources Exceeded";
    case SY_CPE_INVALID_PARAM_NAME:
        return "Invalid Parameter Name";
    case SY_CPE_INVALID_PARAM_TYPE:
        return "Invalid parameter type";
    case SY_CPE_INVALID_PARAM_VALUE:
        return "Invalid parameter value";
    case SY_CPE_NON_WRITABLE_PARAM:
        return "Attempt to set a non-writeable parameter";
    case SY_CPE_NOTIFICATION_REQUEST_REJECTED:
        return "Notification request rejected";
    case SY_CPE_DOWNLOAD_FAILURE:
        return "Download failure";
    case SY_CPE_UPLOAD_FAILURE:
        return "Upload failure";
    case SY_CPE_FILE_SERVER_AUTH_FAILURE:
        return "File transfer server authentication failure";
    case SY_CPE_UNSUPPORTED_FILE_TRANSFER_PROTOCOL:
        return "Unsupported protocol for file transfer";
    case SY_CPE_DOWNLOAD_FAILURE_UNABLE_TO_JOIN_MULTICAST:
        return "MaxEnvelopes exceeded";
    default:
        return "Vendor defined fault";
    }
}


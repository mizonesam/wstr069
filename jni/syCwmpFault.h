/***********************************************************************
*
* syCwmpFault.h
*
* Copyright (C) 2012 Lin jieling <mycourage0@126.com>
* Copyright (C) 2003-2012 by ShenZhen SyMedia Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 1.0 or (at your option) any later version.
*
***********************************************************************/
#ifndef SYCWMPFAULT_H
#define SYCWMPFAULT_H

enum sy_fault_code
{
    SY_ACS_FAULT_MIN = 8000,
    SY_ACS_METHOD_NOT_SUPPORTED = 8000,
    SY_ACS_REQUEST_DENIED = 8001,
    SY_ACS_INTERNAL_ERROR = 8002,
    SY_ACS_INVALID_ARGUMENTS = 8003,
    SY_ACS_RESOUCES_EXCEEDS = 8004,
    SY_ACS_RETRY_REQUEST = 8005,
    SY_ACS_FAULT_MAX = 8899,
    SY_CPE_FAULT_MIN = 9000,
    SY_CPE_METHOD_NOT_SUPPORTED = 9000,
    SY_CPE_REQUEST_DENIED = 9001,
    SY_CPE_INTERNAL_ERROR = 9002,
    SY_CPE_INVALID_ARGUMENTS = 9003,
    SY_CPE_RESOUCES_EXCEEDS = 9004,
    SY_CPE_INVALID_PARAM_NAME = 9005,
    SY_CPE_INVALID_PARAM_TYPE = 9006,
    SY_CPE_INVALID_PARAM_VALUE = 9007,
    SY_CPE_NON_WRITABLE_PARAM = 9008,
    SY_CPE_NOTIFICATION_REQUEST_REJECTED = 9009,
    SY_CPE_DOWNLOAD_FAILURE = 9010,
    SY_CPE_UPLOAD_FAILURE = 9011,
    SY_CPE_FILE_SERVER_AUTH_FAILURE = 9012,
    SY_CPE_UNSUPPORTED_FILE_TRANSFER_PROTOCOL = 9013,
    SY_CPE_DOWNLOAD_FAILURE_UNABLE_TO_JOIN_MULTICAST = 9014,
    SY_CPE_DOWNLOAD_FAILURE_UNABLE_TO_CONTACT_SERVER = 9015,
    SY_CPE_DOWNLOAD_FAILURE_UNABLE_TO_ACCESS_FILE = 9016,
    SY_CPE_DOWNLOAD_FAILURE_UNABLE_TO_COMPLETE = 9017,
    SY_CPE_DOWNLOAD_FAILURE_FILE_CORRUPTED = 9018,
    SY_CPE_DOWNLOAD_FAILURE_FILE_AUTH_FAILURE = 9019,
    SY_CPE_FAULT_MAX = 9899,
};


const char *SyGetFaultString(int faultCode);
#endif


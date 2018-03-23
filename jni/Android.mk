LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)
LOCAL_MODULE := libsyxml
LOCAL_SRC_FILES := thirdLib/xml/libsyxml.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcurl
LOCAL_SRC_FILES := thirdLib/curl/libcurl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libssh2
LOCAL_SRC_FILES := thirdLib/ssh/libssh2.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcrypto_static
LOCAL_SRC_FILES := libcrypto_static.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libpcap
LOCAL_SRC_FILES := thirdLib/pcap/libpcap.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libluajit
LOCAL_SRC_FILES := libluajit.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)

include $(CLEAR_VARS)

WSNAME = syCwmp
LOCAL_MODULE:= cwmp

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
    $(LOCAL_PATH)/thirdLib/xml \
    $(LOCAL_PATH)/thirdLib/xml/jni \
    $(LOCAL_PATH)/gsoap_2_8_8 \

	
LOCAL_SRC_FILES:= main.c \
    soapC.c  \
    soapServer.c \
    soapClient.c \
    gsoap_2_8_8/stdsoap2.c \
    md5.c \
    sha256.c \
    base64.c \
    cJSON.c \
    cJSON_Utils.c \
    sylualib.c \
    $(WSNAME)Log.c \
    $(WSNAME)LogAndPcap.c \
    $(WSNAME)Server.c \
    $(WSNAME)Common.c \
    $(WSNAME)Socket.c \
    $(WSNAME)Management.c \
    $(WSNAME)Util.c \
    $(WSNAME)Fault.c \
    $(WSNAME)Http.c \
    $(WSNAME)Crypto.c \
	$(WSNAME)TaskQueue.c \
    thirdLib/xml/$(WSNAME)Xml.c \
    NAT/syNATClient.c \
    NAT/syNATStun.c \
    NAT/syNATUdp.c \

LOCAL_CFLAGS += -static -std=c99

LOCAL_LDFLAGS := -L. -llog -lz -lm

LOCAL_STATIC_LIBRARIES := libcurl libssh2 libsyxml libpcap libcrypto_static libluajit

#LOCAL_CFLAGS  += -pic -fPIC
#LOCAL_LDFLAGS += -pic -fPIC

include $(BUILD_SHARED_LIBRARY)

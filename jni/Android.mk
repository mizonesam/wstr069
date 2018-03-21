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

#######################################################################################################################
ifeq ($(mode),ff)
else
    ifneq ($(mode),test)
        mode := release
        is_clear := "$(shell svn st)"
        ifneq ($(is_clear),"")
            $(warning $(is_clear))
            $(warning Please submit your copy of the changes)
            $(error (you can type "ndk-bild mode=test" or "ndk-bild mode=ff"))
        else
            ifeq ($(mode),not-up)
                $(warning Not update COPY)
            else ifeq ($(repo_user),CI)
                $(warning Building on CI,Not update COPY)
            else
                $(shell svn up)
                $(warning Updated)
            endif
        endif
    endif


    ifeq ($(HOST_OS),windows)

        LOCAL_CFLAGS := -DHOST_ON_WINDOWS
        $(warning build on WINDOWS)
        
        retCode := $(shell sh $(LOCAL_PATH)/build.sh "$(LOCAL_PATH)+$(mode)" && echo yes)
        $(warning Is the script finished? $(retCode))
        ifneq ($(retCode),yes)
            curr_date := "$(shell date /T)
            curr_time :=  $(shell time /T)"
            repo_ver  := "$(shell for /f "usebackq tokens=2 delims= " %%i in (`"svn info |findstr Revision"`) do echo %%i)"
            _uuid_    :=  $(shell for /f "usebackq tokens=3 delims= " %%i in (`"svn info |findstr UUID"`) do echo %%i)
            _uuid_line_ :=  $(shell svn auth > d:\tmp.txt && type d:\tmp.txt | findstr /n $(_uuid_))
            _user_line_ :=  $(shell @setlocal enabledelayedexpansion&&@for /f "tokens=1 delims=:" %%i in ("$(_uuid_line_)") do (set /a n=2+%%i&&echo !n!))
            repo_user   := "$(shell for /f "skip=$(_user_line_) tokens=2 delims= " %%i in ('type d:\tmp.txt') do (echo %%i) && exit)"
            repo_path   := "$(shell for /f "usebackq tokens=2 delims=^" %%i in (`"svn info |findstr Relative"`) do (echo %%i))"
            $(warning uuid = $(_uuid_))
            $(shell del "d:\tmp.txt")
            
            #$(warning _uuid_line_ = "$(_uuid_line_)")
            #$(warning _user_line_ = $(_user_line_))
            #$(warning repo_user = $(repo_user))
            
            $(shell echo #ifndef _VERSION_H_  > $(LOCAL_PATH)/version.h)
            $(shell echo #define _VERSION_H_ >> $(LOCAL_PATH)/version.h)
            
            $(shell echo #define VERSION    $(repo_ver)>>  $(LOCAL_PATH)/version.h)
            $(shell echo #define REPO_USER  $(repo_user)>> $(LOCAL_PATH)/version.h)
            $(shell echo #define TIMESTAMP  $(curr_date)@ $(curr_time) >> $(LOCAL_PATH)/version.h)
            $(shell echo #define BUILD_TYPE "$(mode)">>    $(LOCAL_PATH)/version.h)
            $(shell echo #define REPO_PATH  $(repo_path)>> $(LOCAL_PATH)/version.h)

            $(shell echo.>> $(LOCAL_PATH)/version.h)
            $(shell echo #define VER_STR "Version: "       VERSION",\n"   \>> $(LOCAL_PATH)/version.h)
            $(shell echo                 " Who compiled: " REPO_USER",\n" \>> $(LOCAL_PATH)/version.h)
            $(shell echo                 " Timestamp: "    TIMESTAMP",\n" \>> $(LOCAL_PATH)/version.h)
            $(shell echo                 " Build type: "   BUILD_TYPE",\n"\>> $(LOCAL_PATH)/version.h)
            $(shell echo                 " Relative path: "REPO_PATH".\n" \>> $(LOCAL_PATH)/version.h)
            $(shell echo.>> $(LOCAL_PATH)/version.h)
            $(shell echo #endif /* _VERSION_H_ */ >> $(LOCAL_PATH)/version.h)
            
            $(warning repo_ver=$(repo_ver) repo_user=$(repo_user))
        endif
    else

        LOCAL_CFLAGS := -DHOST_ON_LINUX
        $(warning build on LINUX or OTHER)
        curr_time := $(shell date +%Y/%m/%d,%H:%M:%S)
        repo_ver  := $(shell svn info $(LOCAL_PATH) | grep "Revision" |awk '{print $$2}')
        repo_path := $(shell svn info $(LOCAL_PATH) | grep "Relative" |awk '{print $$3}')
        ifeq ($(repo_user),)
            repo_ip   := $(shell svn info|grep "Repository Root:" |grep -o '[0-9]*\.[0-9]*\.[0-9]*\.[0-9]*')
            repo_URL  := <svn://$(repo_ip):3690>
            repo_UUID := $(shell svn info $(LOCAL_PATH) | grep "Repository UUID:" | awk '{print $$3}')
            URLandUUID := "$(repo_URL) $(repo_UUID)"

            #$(shell echo -n $(URLandUUID) > ~/1.log)
            filename := $(shell echo -n $(URLandUUID) | md5sum |cut -d' ' -f1)
            
            #$(warning currentURL=$(repo_URL))
            #$(warning repo_UUID=$(repo_UUID))
            #$(warning URLandUUID=$(URLandUUID))
            #$(warning filename=$(filename))
            
            repo_user := $(shell cat ~/.subversion/auth/svn.simple/$(filename) | tail -n2 | head -n1 )
            #$(warning repo_user=$(repo_user))
        endif
        $(shell echo "#ifndef _VERSION_H_"  > $(LOCAL_PATH)/version.h)
        $(shell echo "#define _VERSION_H_" >> $(LOCAL_PATH)/version.h)
        $(shell echo -e "\x0D"             >> $(LOCAL_PATH)/version.h)
        
        $(shell echo "#define VERSION    \"$(repo_ver)\""              >> $(LOCAL_PATH)/version.h)
        $(shell echo "#define REPO_USER  \"$(repo_user)\""             >> $(LOCAL_PATH)/version.h)
        $(shell echo "#define TIMESTAMP  \"$(curr_date)$(curr_time)\"" >> $(LOCAL_PATH)/version.h)
        $(shell echo "#define BUILD_TYPE \"$(mode)\""                 >> $(LOCAL_PATH)/version.h)
        $(shell echo "#define REPO_PATH  \"$(repo_path)\""            >> $(LOCAL_PATH)/version.h)
        
        $(shell echo -e "\x0D"                                        >> $(LOCAL_PATH)/version.h)
        $(shell echo "#define VER_STR \"Version: \"      VERSION\",\\n\"   \\" >> $(LOCAL_PATH)/version.h)
        $(shell echo "                \"Who compiled: \" REPO_USER\",\\n\" \\" >> $(LOCAL_PATH)/version.h)
        $(shell echo "                \"Timestamp: \"    TIMESTAMP\",\\n\" \\" >> $(LOCAL_PATH)/version.h)
        $(shell echo "                \"Build type: \"   BUILD_TYPE\",\\n\"\\" >> $(LOCAL_PATH)/version.h)
        $(shell echo "                \"Relative path: \"REPO_PATH\".\\n\" \\" >> $(LOCAL_PATH)/version.h)
        $(shell echo -e "\x0D"                                                >> $(LOCAL_PATH)/version.h)
        $(shell echo  "#endif /* _VERSION_H_ */" >> $(LOCAL_PATH)/version.h)
        
        $(warning repo_ver=$(repo_ver) repo_user=$(repo_user))
    endif
endif
#######################################################################################################################
    
LOCAL_CFLAGS += -static -std=c99

LOCAL_LDFLAGS := -L. -llog -lz -lm

LOCAL_STATIC_LIBRARIES := libcurl libssh2 libsyxml libpcap libcrypto_static libluajit

#LOCAL_CFLAGS  += -pic -fPIC
#LOCAL_LDFLAGS += -pic -fPIC

include $(BUILD_SHARED_LIBRARY)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE    := libsyxml
LOCAL_CFLAGS := -DANDROID_NDK \
                -DDISABLE_IMPORTGL
LOCAL_CFLAGS += -I../amcodec/include
LOCAL_CXXFLAGS := -DHAVE_SYS_UIO_H
LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	mxml-string.c\
	mxml-node.c\
	mxml-file.c\
	mxml-get.c\
	mxml-attr.c\
	mxml-index.c\
	mxml-private.c\
	mxml-search.c\
	mxml-set.c\
	mxml-entity.c\

#LOCAL_SHARED_LIBRARIES := liblog
LOCAL_LDLIBS    := -lc -llog
#LOCAL_LDLIBS   := -lpthread

include $(BUILD_STATIC_LIBRARY)
#include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_EXECUTABLE)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := tinyalsa 
LOCAL_SRC_FILES := libtinyalsa.a
include $(PREBUILT_STATIC_LIBRARY) 

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc .cpp
LOCAL_C_INCLUDES :=$(LOCAL_PATH)
LOCAL_MODULE    := tinycap
LOCAL_STATIC_LIBRARIES += tinyalsa
LOCAL_SRC_FILES :=tinycap.cpp
LOCAL_CPP_FEATURES := rtti exceptions
LOCAL_MODULE_TAGS := optional
#liblog.so libGLESv2.so
LOCAL_LDLIBS += -llog
include $(BUILD_EXECUTABLE)

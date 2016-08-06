# NOTE: Included Android.mk files seem to mess things up when they set this:
# And we have to assume that the "current" directory is actually the parent.
#LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := libjpeg
LOCAL_SRC_FILES := \
    libjpeg/jcapimin.c \
    libjpeg/jcapistd.c \
    libjpeg/jccoefct.c \
    libjpeg/jccolor.c \
    libjpeg/jcdctmgr.c \
    libjpeg/jchuff.c \
    libjpeg/jcinit.c \
    libjpeg/jcmainct.c \
    libjpeg/jcmarker.c \
    libjpeg/jcmaster.c \
    libjpeg/jcomapi.c \
    libjpeg/jcparam.c \
    libjpeg/jcphuff.c \
    libjpeg/jcprepct.c \
    libjpeg/jcsample.c \
    libjpeg/jctrans.c \
    libjpeg/jdapimin.c \
    libjpeg/jdapistd.c \
    libjpeg/jdatadst.c \
    libjpeg/jdatasrc.c \
    libjpeg/jdcoefct.c \
    libjpeg/jdcolor.c \
    libjpeg/jddctmgr.c \
    libjpeg/jdhuff.c \
    libjpeg/jdinput.c \
    libjpeg/jdmainct.c \
    libjpeg/jdmarker.c \
    libjpeg/jdmaster.c \
    libjpeg/jdmerge.c \
    libjpeg/jdphuff.c \
    libjpeg/jdpostct.c \
    libjpeg/jdsample.c \
    libjpeg/jdtrans.c \
    libjpeg/jerror.c \
    libjpeg/jfdctflt.c \
    libjpeg/jfdctfst.c \
    libjpeg/jfdctint.c \
    libjpeg/jidctflt.c \
    libjpeg/jidctfst.c \
    libjpeg/jidctint.c \
    libjpeg/jidctred.c \
    libjpeg/jquant1.c \
    libjpeg/jquant2.c \
    libjpeg/jutils.c \
    libjpeg/jmemmgr.c \
    libjpeg/jmemansi.c

LOCAL_LDLIBS := -lz
include $(BUILD_STATIC_LIBRARY)


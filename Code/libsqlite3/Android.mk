include $(CLEAR_VARS)

LOCAL_MODULE    := libsqlite3
LOCAL_SRC_FILES :=\
    libsqlite3/sqlite3.c \

LOCAL_LDLIBS := -lz

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)

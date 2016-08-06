
LOCAL_SRC_FILES += \
	GLContext/GLPath.cpp \
	GLContext/GLTimer.cpp \
	GLContext/GLLogger.cpp \
	GLContext/GLMath.cpp \
	GLContext/GLNoise.cpp \
	GLContext/GLContext.cpp \
	GLContext/GLPixelBuffer.cpp \
	GLContext/GLTextureObject.cpp \
	GLContext/GLTextureArray.cpp \
	GLContext/GLBufferObject.cpp \
	GLContext/GLRenderBufferObject.cpp \
	GLContext/GLFrameBufferObject.cpp \
	GLContext/GLShaderProgram.cpp \
	GLContext/GLShaderTechnique.cpp \
	GLContext/GLShaderFile.cpp \
	GLContext/GLShape.cpp \
	GLContext/GLManager.cpp \
	GLContext/GLFont.cpp \
	GLContext/GLDatabase.cpp \
	GLContext/GLLua.cpp

LOCAL_C_INCLUDES += $(LOCAL_PATH)/libpng/ $(LOCAL_PATH)/libjpeg/ $(LOCAL_PATH)/libzip/ $(LOCAL_PATH)/libsqlite3/ $(LOCAL_PATH)/liblua/


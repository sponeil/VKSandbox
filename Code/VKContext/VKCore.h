// VKCore.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

//#ifndef _DEBUG
//#define _DEBUG
//#endif
namespace VK {
	typedef void (*ThrowExceptionFunc)(const char *);
	extern ThrowExceptionFunc Throw;
};

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef _WIN32
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdarg.h>
#else
//#include <limits.h>
//#include <sys/time.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <unistd.h>
#ifdef PATH_MAX
#define _MAX_PATH PATH_MAX
#else
#define _MAX_PATH 16384
#endif
#endif

#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <map>


// The main Vulkan header(s)
#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#define WIN32_LEAN_AND_MEAN 1
#define _USE_MATH_DEFINES
#include "vulkan/vulkan.h"
#include "vulkan/VKFunctions.h"
#include "vulkan/VKStruct.h"

#include "VKSingleton.h"
#include "VKPath.h"
#include "VKTimer.h"
#include "VKThread.h"
#include "VKLogger.h"
#include "VKProfiler.h"

#ifdef _WIN32
#include <algorithm>
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#endif

/*

// Utility and math headers
#include "VKTransform.h"
#include "VKGeometry.h"
#include "VKString.h"

// Core headers
#include "VKContext.h"
//#include "VKWindow.h"

// OpenGL object headers
#include "VKPixelBuffer.h"
#include "VKTextureObject.h"
#include "VKTextureArray.h"
#include "VKIndexBufferObject.h"
#include "VKVertexBufferObject.h"
#include "VKFeedbackBuffer.h"
#include "VKFrameBufferObject.h"
#include "VKUniformBufferObject.h"
#include "VKShaderProgram.h"
#include "VKShaderTechnique.h"
#include "VKShaderFile.h"
#include "VKShape.h"
#include "VKFont.h"
#include "VKManager.h"

*/

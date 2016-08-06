// Copyright 2016 Intel Corporation All Rights Reserved
// 
// Intel makes no representations about the suitability of this software for any purpose.
// THIS SOFTWARE IS PROVIDED ""AS IS."" INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES,
// EXPRESS OR IMPLIED, AND ALL LIABILITY, INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES,
// FOR THE USE OF THIS SOFTWARE, INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY
// RIGHTS, AND INCLUDING THE WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
// Intel does not assume any responsibility for any errors which may appear in this software
// nor any responsibility to update it.

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include "vulkan.h"

// Declare the non-extern function pointers
#define VK_EXPORTED_FUNCTION( fun ) PFN_##fun fun = nullptr;
#define VK_GLOBAL_LEVEL_FUNCTION( fun ) PFN_##fun fun = nullptr;
#define VK_INSTANCE_LEVEL_FUNCTION( fun ) PFN_##fun fun = nullptr;
#define VK_DEVICE_LEVEL_FUNCTION( fun ) PFN_##fun fun = nullptr;

#include "VKFunctions.inl"

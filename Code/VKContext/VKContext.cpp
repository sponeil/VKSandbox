// GLContext.cpp
// This code is part of the GLContext library, an object-oriented class
// library designed to make OpenGL 3.x easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#include "VKCore.h"
#include "VKContext.h"
#include "VKImage.h"
#include "../glslang/glslang/Public/ShaderLang.h"

namespace VK {

// VK globals from VKStruct.h
VkPhysicalDeviceProperties deviceProperties;
VkPhysicalDeviceFeatures deviceFeatures;
VkPhysicalDeviceMemoryProperties memoryProperties;
std::vector<VkFormatProperties> formatProperties;

// Context static members
uint32_t Context::nextID = 0;
Context *Context::pCurrent = NULL;
LibraryHandle Context::hLib = NULL;

VKAPI_ATTR VkBool32 VKAPI_CALL Context::dbgFunc(
	VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
	uint64_t srcObject, size_t location, int32_t msgCode,
	const char *pLayerPrefix, const char *pMsg, void *pUserData) {
	if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		VKLogError("[%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
	else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		VKLogWarning("[%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
	else
		VKLogNotice("[%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
	return false;
}

// Helpers used for error checking
void ThrowException(const char *psz) { throw psz; }
ThrowExceptionFunc Throw = &ThrowException;
const char *ResultString(VkResult result) {
	if(result == VK_SUCCESS)
		return "VK_SUCCESS";

	switch (result) {
		case VK_NOT_READY: return "VK_NOT_READY";
		case VK_TIMEOUT: return "VK_TIMEOUT";
		case VK_EVENT_SET: return "VK_EVENT_SET";
		case VK_EVENT_RESET: return "VK_EVENT_RESET";
		case VK_INCOMPLETE: return "VK_INCOMPLETE";
		case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
		case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
		case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
		case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
		case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
		case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
		case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
		case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
		case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
		case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
		//case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
		//case VK_RESULT_BEGIN_RANGE: return "VK_RESULT_BEGIN_RANGE";
		//case VK_RESULT_END_RANGE: return "VK_RESULT_END_RANGE";
		case VK_RESULT_RANGE_SIZE: return "VK_RESULT_RANGE_SIZE";
		case VK_RESULT_MAX_ENUM: return "VK_RESULT_MAX_ENUM";
	}
	static char err[32];
	sprintf(err, "Unknown (%d)", result);
	return err;
}

// Context static methods
bool Context::Init() {
	// Load the Vulkan library and find vkGetInstanceProcAddr
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	hLib = LoadLibrary("vulkan-1.dll");
	if(hLib)
		vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(hLib, "vkGetInstanceProcAddr");
#elif defined(VK_USE_PLATFORM_XCB_KHR) || defined(VK_USE_PLATFORM_XLIB_KHR)
	hLib = dlopen("libvulkan.so", RTLD_NOW);
	if (hLib)
		vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(hLib, "vkGetInstanceProcAddr");
#endif
	// If it fails, log it as an exception and return false
	if (hLib == nullptr || vkGetInstanceProcAddr == nullptr) {
		VKLogException("Failed to load Vulkan library!");
		return false;
	}

	// This macro and include will vkGetInstanceProcAddr tp find all of the global Vulkan functions.
	// (Some may only be found after creating the instance, and some only after creating the device).
#define VK_GLOBAL_LEVEL_FUNCTION( fun ) \
	if( !(fun = (PFN_##fun)vkGetInstanceProcAddr( nullptr, #fun )) ) { \
		VKLogException("Failed to load global function: %s", #fun); \
		return false; \
	}
#include "vulkan/VKFunctions.inl"

	// Initialize the GLSL compiler so we can compile to SPIRV in-process
	glslang::InitializeProcess();
	return true;
}

void Context::Cleanup() {
	if (hLib) {
		// Clean up the GLSL->SPIRV compiler for Vulkan shaders
		glslang::FinalizeProcess();

		// Release the Vulkan library
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		FreeLibrary(hLib);
#elif defined(VK_USE_PLATFORM_XCB_KHR) || defined(VK_USE_PLATFORM_XLIB_KHR)
		dlclose(hLib);
#endif
		hLib = NULL;
	}
}

// Context instance methods
Context::Context()
	: hInstance(NULL)
	, hWnd(NULL)
	, validate(false)
	, nLastError(VK_SUCCESS)
	, pszLastError(NULL)
	, instance(NULL)
	, physicalDevice(NULL)
	, surface(NULL)
	, device(NULL)
	, debugCallback(NULL)
	, presentIndex(-1)
	, graphicsIndex(-1)
	, sigImageAvailable(NULL)
	, sigRenderingFinished(NULL)
	, queue(NULL)
	, pool(NULL)
	, cmd(NULL)
	, swapchain(NULL)
{
	pCurrent = this; // Set this before create() is called in case VK::Object instances are declared at the same scope
}

bool Context::create(InstanceHandle inst, WindowHandle wnd, bool val, const char *appName, uint32_t nVersion) {
	if (!hLib)
		VKLogException("ERROR: You must call Context::Init() before Context::create()!");
	makeCurrent();

	uint32_t n = 0;
	hInstance = inst;
	hWnd = wnd;
	validate = val;

	VK_CHECK(vkEnumerateInstanceLayerProperties(&n, NULL));
	if (n > 0) {
		instanceLayers.resize(n);
		VK_CHECK(vkEnumerateInstanceLayerProperties(&n, &instanceLayers[0]));
		for (uint32_t i = 0; i < n; i++) {
			if (validate && strcmp(instanceLayers[i].layerName, "VK_LAYER_LUNARG_standard_validation") == 0)
				enabledInstanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
			else if (validate && strcmp(instanceLayers[i].layerName, "VK_LAYER_LUNARG_core_validation") == 0)
				enabledInstanceLayers.push_back("VK_LAYER_LUNARG_core_validation");
			else if (validate && strcmp(instanceLayers[i].layerName, "VK_LAYER_LUNARG_parameter_validation") == 0)
				enabledInstanceLayers.push_back("VK_LAYER_LUNARG_parameter_validation");
		}
	}

	bool surface_extension = false, os_surface_extension = false;
	VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &n, NULL));
	if (n > 0) {
		instanceExtensions.resize(n);
		VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &n, &instanceExtensions[0]));
		for (uint32_t i = 0; i < n; i++) {
			if (strcmp(instanceExtensions[i].extensionName, VK_KHR_SURFACE_EXTENSION_NAME) == 0) {
				enabledInstanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
				surface_extension = true;
			} else if (strcmp(instanceExtensions[i].extensionName, VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0) {
				enabledInstanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
				os_surface_extension = true;
			} else if (validate && strcmp(instanceExtensions[i].extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0) {
				enabledInstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			}
		}
	}
	if (!surface_extension)
		VKLogException("Unable to find OS-independent surface extension on physical device!");
	if (!os_surface_extension)
		VKLogException("Unable to find OS-specific surface extension on physical device!");

	InstanceCreateInfo instInfo(appName, &enabledInstanceLayers, &enabledInstanceExtensions);
	instInfo.appInfo.apiVersion = nVersion;
	DebugReportCallbackCreateInfoEXT dbgCreateInfo(dbgFunc);

	VK_CHECK(vkCreateInstance(&instInfo, NULL, &instance));

// As soon as we create the instance, we need to call vkGetInstanceProcAddr for all instance functions
#define VK_INSTANCE_LEVEL_FUNCTION( fun ) if( !(fun = (PFN_##fun)vkGetInstanceProcAddr( instance, #fun )) ) VKLogDebug("Instance function failed to load: %s", #fun);
#include "vulkan/VKFunctions.inl"

	if (validate)
		VK_CHECK(vkCreateDebugReportCallbackEXT(instance, &dbgCreateInfo, NULL, &debugCallback));

	std::vector<VkPhysicalDevice> physicalDevices;
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &n, NULL));
	if (n <= 0)
		VKLogException("No Vulkan physical devices found!");
	physicalDevices.resize(n);
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &n, &physicalDevices[0]));
	physicalDevice = physicalDevices[0];

	VK_CHECK(vkEnumerateDeviceLayerProperties(physicalDevice, &n, NULL));
	if (n > 0) {
		deviceLayers.resize(n);
		VK_CHECK(vkEnumerateDeviceLayerProperties(physicalDevice, &n, &deviceLayers[0]));
		for (uint32_t i = 0; i < n; i++) {
			if (validate && strcmp(deviceLayers[i].layerName, "VK_LAYER_LUNARG_standard_validation") == 0)
				enabledDeviceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
		}
	}

	bool swapchain_extension = false;
	VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &n, NULL));
	if (n > 0) {
		deviceExtensions.resize(n);
		VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &n, &deviceExtensions[0]));
		for (uint32_t i = 0; i < n; i++) {
			if (strcmp(deviceExtensions[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
				enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
				swapchain_extension = true;
			} else if (strcmp(deviceExtensions[i].extensionName, VK_NV_GLSL_SHADER_EXTENSION_NAME) == 0) {
				enabledDeviceExtensions.push_back(VK_NV_GLSL_SHADER_EXTENSION_NAME);
			}
		}
	}
	if (!swapchain_extension)
		VKLogException("Unable to find swapchain extension!");

	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &n, NULL);
	queueFamilies.resize(n);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &n, &queueFamilies[0]);

	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	formatProperties.resize(VK_FORMAT_END_RANGE + 1);
	for (n = VK_FORMAT_BEGIN_RANGE; n < VK_FORMAT_END_RANGE; n++)
		vkGetPhysicalDeviceFormatProperties(physicalDevice, (VkFormat)n, &formatProperties[n]);

	Win32SurfaceCreateInfoKHR surfaceInfo(inst, wnd);
	VK_CHECK(vkCreateWin32SurfaceKHR(instance, &surfaceInfo, NULL, &surface));

	presentIndex = UINT32_MAX;
	graphicsIndex = UINT32_MAX;
	for (n = 0; n < queueFamilies.size(); n++) {
		VkBool32 supportsPresent;
		VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, n, surface, &supportsPresent));
		if (supportsPresent == VK_TRUE) {
			if ((queueFamilies[n].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
				graphicsIndex = presentIndex = n; // We found a queue that supports both, use it and break out of loop
				break;
			} else if (presentIndex == UINT32_MAX)
				presentIndex = n; // Otherwise use the first present queue we find...
		} else if (graphicsIndex == UINT32_MAX && (queueFamilies[n].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			graphicsIndex = n; // ...together with the first graphics queue we find
	}

	std::vector<DeviceQueueCreateInfo> queueInfo = { DeviceQueueCreateInfo(graphicsIndex) };
	if (graphicsIndex != presentIndex)
		queueInfo.push_back(DeviceQueueCreateInfo(presentIndex));
	DeviceCreateInfo deviceInfo(queueInfo, enabledDeviceLayers, enabledDeviceExtensions);
	deviceInfo.pEnabledFeatures = &deviceFeatures;
	VK_CHECK(vkCreateDevice(physicalDevice, &deviceInfo, NULL, &device));

// As soon as we create the device, we need to call vkGetDeviceProcAddr for all instance functions
#define VK_DEVICE_LEVEL_FUNCTION( fun ) if( !(fun = (PFN_##fun)vkGetDeviceProcAddr( device, #fun )) ) VKLogDebug("Device function failed to load: %s", #fun);
#include "vulkan/VKFunctions.inl"

	// Create semaphores to synchronize swapping images between the back buffer and the screen
	VkSemaphoreCreateInfo semaphore_create_info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,  NULL, 0 };
	VK_CHECK(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &sigImageAvailable));
	VK_CHECK(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &sigRenderingFinished));

	// Get the graphics device queue and create a command pool and buffer for initialization tasks
	vkGetDeviceQueue(device, graphicsIndex, 0, &queue);
	CommandPoolCreateInfo poolInfo(graphicsIndex);
	VK_CHECK(vkCreateCommandPool(device, &poolInfo, NULL, &pool));
	CommandBufferAllocateInfo bufferInfo(pool, 1);
	CommandBufferBeginInfo cmdBeginInfo(0);
	VK_CHECK(vkAllocateCommandBuffers(device, &bufferInfo, &cmd));
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	return true;
}

void Context::destroy() {
	if (debugCallback)
		vkDestroyDebugReportCallbackEXT(instance, debugCallback, NULL);
	debugCallback = NULL;

	if (device) {
		vkDeviceWaitIdle(device);

		if (swapchain) {
			vkDestroySwapchainKHR(device, swapchain, NULL);
			swapchain = NULL;
		}

		if (cmd) {
			vkEndCommandBuffer(cmd);
			vkDeviceWaitIdle(device);
			vkFreeCommandBuffers(device, pool, 1, &cmd);
			cmd = NULL;
		}
		if (pool) {
			vkDestroyCommandPool(device, pool, NULL);
			pool = VK_NULL_HANDLE;
		}
		queue = VK_NULL_HANDLE;

		if (sigImageAvailable)
			vkDestroySemaphore(device, sigImageAvailable, NULL);
		sigImageAvailable = NULL;
		if (sigRenderingFinished)
			vkDestroySemaphore(device, sigRenderingFinished, NULL);
		sigRenderingFinished = NULL;
		vkDestroyDevice(device, NULL);
		device = NULL;
	}

	if (instance) {
		if (surface)
			vkDestroySurfaceKHR(instance, surface, NULL);
		surface = NULL;
		vkDestroyInstance(instance, NULL);
		instance = NULL;
	}
}

bool Context::buildSwapchain(uint32_t w, uint32_t h) {
	VkResult err = VK_SUCCESS;
	uint32_t n = 0;

	// If we're rebuilding the swapchain, destroy the image views (which will be rebuilt at the bottom)
	for (size_t n = 0; n < views.size(); n++)
		vkDestroyImageView(device, views[n], NULL);
	views.clear();
	images.clear();

	// Use the first surface format unless it's undefined.
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &n, NULL));
	std::vector<VkSurfaceFormatKHR> surfaceFormats(n);
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &n, &surfaceFormats[0]));
	surfaceFormat = surfaceFormats[0];
	if (surfaceFormat.format == VK_FORMAT_UNDEFINED) {
		surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
		surfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	}

	// I don't have a device to test mailbox mode on, but the client code using this Context
	// class should be able to choose between tearing (immediate) and non-tearing (mailbox or FIFO).
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &n, NULL));
	std::vector<VkPresentModeKHR> presentModes(n);
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &n, &presentModes[0]));
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	if (std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != presentModes.end())
		swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;

	// width and height are either both -1, or both not -1.
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities));
	if (surfaceCapabilities.currentExtent.width == (uint32_t)-1) {
		// If the surface size is undefined, the size is set to the size of the images requested.
		extent.width = w;
		extent.height = h;
	} else {
		// If the surface size is defined, the swap chain size must match
		extent.width = surfaceCapabilities.currentExtent.width;
		extent.height = surfaceCapabilities.currentExtent.height;
	}

	// Determine the number of VkImage's to use in the swap chain (we desire to
	// own only 1 image at a time, besides the images being displayed and
	// queued for display):
	uint32_t desiredNumberOfSwapchainImages = surfaceCapabilities.minImageCount + 1;
	if ((surfaceCapabilities.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfaceCapabilities.maxImageCount)) {
		// Application must settle for fewer images than desired:
		desiredNumberOfSwapchainImages = surfaceCapabilities.maxImageCount;
	}

	SwapchainCreateInfoKHR swapchainInfo(surface, desiredNumberOfSwapchainImages, surfaceFormats[0].format, surfaceFormats[0].colorSpace, extent.width, extent.height, surfaceCapabilities, swapchainPresentMode);
	VkSwapchainKHR oldSwapchain = swapchainInfo.oldSwapchain = swapchain;
	VK_CHECK(vkCreateSwapchainKHR(device, &swapchainInfo, NULL, &swapchain));

	// If we just re-created an existing swapchain, we should destroy the old swapchain at this point.
	// Note: destroying the swapchain also cleans up all its associated presentable images once the platform is done with them.
	if (oldSwapchain != VK_NULL_HANDLE)
		vkDestroySwapchainKHR(device, oldSwapchain, NULL);

	VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &n, NULL));
	images.resize(n);
	VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &n, &images[0]));

	views.resize(n);
	for (size_t n = 0; n < views.size(); n++) {
		// Initialize the swapchain image to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR. The top of the "present" function needs
		// to know what format it's in, and since the bottom of the "present" function needs to change it to this,
		// it makes sense to start it off in that format.
		Image(images[n]).setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		ImageViewCreateInfo viewInfo(images[n], surfaceFormats[0].format, VK_IMAGE_ASPECT_COLOR_BIT);
		VK_CHECK(vkCreateImageView(device, &viewInfo, NULL, &views[n]));
	}

	return true;
}

/// Flushes and rebuilds the primary command buffer tied to the graphics queue
void Context::flush() {
	vkEndCommandBuffer(cmd);
	SubmitInfo submitInfo(&cmd);
	VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK(vkQueueWaitIdle(queue));

	vkFreeCommandBuffers(device, pool, 1, &cmd);
	CommandBufferAllocateInfo bufferInfo(pool, 1);
	CommandBufferBeginInfo cmdBeginInfo(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
	VK_CHECK(vkAllocateCommandBuffers(device, &bufferInfo, &cmd));
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));
}

bool Context::present(VkImage image) {
	flush();

	uint32_t i;
	nLastError = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, sigImageAvailable, VK_NULL_HANDLE, &i);
	if (nLastError != VK_SUCCESS) {
		if (nLastError != VK_SUBOPTIMAL_KHR) {
			// Caller needs to check last error for VK_ERROR_OUT_OF_DATE_KHR
			return false;
		}
		VKLogDebug("Suboptimal KHR!");
	}

	Image nextImage(images[i]);
	nextImage.setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	ImageCopy copy_region(extent.width, extent.height);
	vkCmdCopyImage(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, nextImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
	nextImage.setLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	flush();

	std::vector<VkSemaphore> sem = { sigImageAvailable };
	PresentInfoKHR present_info(&swapchain, &i, &sem);
	nLastError = vkQueuePresentKHR(queue, &present_info);
	return true;
}

} // namespace VK

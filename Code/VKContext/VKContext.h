// VKContext.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#ifndef __VKContext_h__
#define __VKContext_h__

#include "VKCore.h"

namespace VK {

extern const char *ResultString(VkResult result);

#if defined(VK_USE_PLATFORM_WIN32_KHR)
typedef HMODULE LibraryHandle;
typedef HINSTANCE InstanceHandle;
typedef HWND WindowHandle;
#elif defined(VK_USE_PLATFORM_XCB_KHR)
typedef void* LibraryHandle;
typedef xcb_connection_t InstanceHandle;
typedef xcb_window_t WindowHandle;
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
typedef void* LibraryHandle;
typedef Display InstanceHandle;
typedef Window WindowHandle;
#endif

#ifdef _DEBUG
#define PRE_VK_CHECK() if(pCurrent != this) VKLogException(pszLastError = "Messing with the wrong context")
#define POST_VK_CHECK() if(nLastError != VK_SUCCESS) VKLogException(pszLastError = (const char *)ResultString(nLastError))
#define PRE_OBJ_CHECK() if(&vk != Context::GetCurrent()) vk.fail(__FILE__, __LINE__, "Messing with the wrong context")
#define POST_OBJ_CHECK() if(vk.nLastError != VK_SUCCESS) vk.fail(__FILE__, __LINE__, (const char *)ResultString(vk.nLastError))
#else
#define PRE_VK_CHECK() NULL
#define POST_VK_CHECK() NULL
#define PRE_OBJ_CHECK() NULL
#define POST_OBJ_CHECK() NULL
#endif

#define VK_CHECK(func) { PRE_VK_CHECK(); nLastError = func; POST_VK_CHECK(); }
#define OBJ_CHECK(func) { PRE_OBJ_CHECK(); vk.nLastError = func; POST_OBJ_CHECK(); }

class Object;
class Context : public NoCopy {
public:
	typedef std::map<uint32_t, Object *> ObjectMap;
	typedef std::list<std::string> WarningList;

private:
	static uint32_t nextID; ///< The next ID for the Vulkan object map below
	static Context *pCurrent; ///< Points to the current Vulkan context (set using makeCurrent())
	static LibraryHandle hLib; ///< Handle to the Vulkan library
	InstanceHandle hInstance; ///< Handle to the process instance
	WindowHandle hWnd; ///< Handle to the display window

	bool validate; ///< Set to true to enable Vulkan validation
	ObjectMap objects; ///< Map of Vulkan objects created under this context
	WarningList warnings; ///< List of warnings/errors associated with this context

	std::vector<const char *> enabledInstanceLayers, enabledInstanceExtensions;
	std::vector<const char *> enabledDeviceLayers, enabledDeviceExtensions;
	std::vector<VkLayerProperties> instanceLayers, deviceLayers;
	std::vector<VkExtensionProperties> instanceExtensions, deviceExtensions;
	std::vector<VkQueueFamilyProperties> queueFamilies;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VkSurfaceFormatKHR surfaceFormat;
	Extent2D extent;

	// These are tied to instance and device creation, and only need to be initialized once
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkSurfaceKHR surface;
	VkDevice device;
	VkDebugReportCallbackEXT debugCallback;
	uint32_t presentIndex, graphicsIndex;
	VkSemaphore sigImageAvailable, sigRenderingFinished;

	// These are tied to the graphics queue, which only needs to be initialized once
	// (The command buffer will get rebuilt every time commands are flushed.)
	VkQueue queue;
	VkCommandPool pool;
	VkCommandBuffer cmd;

	// These are tied to the swapchain, which needs to be rebuild every time the window changes size/position
	VkSwapchainKHR swapchain;
	std::vector<VkImage> images;
	std::vector<VkImageView> views;
	//Image depthBuffer;

	static VKAPI_ATTR VkBool32 VKAPI_CALL Context::dbgFunc(
		VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
		uint64_t srcObject, size_t location, int32_t msgCode,
		const char *pLayerPrefix, const char *pMsg, void *pUserData
	);

public:
	VkResult nLastError; ///< The last error code set by a VK call
	const char *pszLastError; ///< The last error message set by a VK call

	static Context *GetCurrent() { return pCurrent; } ///< Returns the currently active context
	static bool Init();
	static void Cleanup();

	Context();

	bool valid() const { return instance != NULL; }
	bool create(InstanceHandle inst, WindowHandle wnd, bool val, const char *appName = "VKTest", uint32_t nVersion = VK_API_VERSION_1_0);
	void destroy(); ///< Destroys this Vulkan context

	/// Call to build the swapchain (call again to rebuild when the window size changes).
	bool buildSwapchain(uint32_t w, uint32_t h);

	void makeCurrent() { pCurrent = this; } ///< Makes this the currently active Vulkan context
	void flush(); ///< Flushes and rebuilds the primary command buffer tied to the graphics queue
	bool present(VkImage image); ///< Presents the specified image to the next swapchain image

	/// Call to add a warning
	void addWarning(const char *psz) { warnings.push_back(psz); }

	/// Call to add a warning
	void addWarning(const std::string &str) { warnings.push_back(str); }

	/// Call to log an error and (optionally) throw an exception 
	void fail(const char *file, int line, const char *psz) { VK::Logger::GetRef().logException(file, line, "%s", psz); }

	/// Call to retrieve the list of warnings
	WarningList &getWarnings() { return warnings; }

	/// Call to add a VK::Object to this context's list of objects.
	/// (This should only be called by VK::Object's constructor)
	uint32_t addObject(Object *pObj) { objects[++nextID] = pObj; return nextID; }

	/// Call to remove a VK::Object from this context's list of objects.
	/// (This should only be called by VK::Object's destructor)
	void removeObject(uint32_t id, Object *pObj) {
		ObjectMap::iterator it = objects.find(id);
		if (it == objects.end())
			VKLogException("Failed to find object %u in object map!", id);
		else if (it->second != pObj)
			VKLogException("Incorrect object %u found in object map!", id);
		else
			objects.erase(it);
	}

	/// Casting operators to provide easy access to any handle when you need to call a Vulkan function manually
	operator VkInstance() const { return instance; }
	operator VkPhysicalDevice() const { return physicalDevice; }
	operator VkSurfaceKHR() const { return surface; }
	operator VkDevice() const { return device; }
	operator VkQueue() const { return queue; }
	operator VkCommandPool() const { return pool; }
	operator VkCommandBuffer() const { return cmd; }
	operator VkSwapchainKHR() const { return swapchain; }

	/// Getters for things we can't use casting operators for
	const VkSurfaceFormatKHR &getSurfaceFormat() const { return surfaceFormat; }
	const Extent2D &getExtent() const { return extent; }
	const std::vector<VkImage> &getSwapchainImages() const { return images; }
	const std::vector<VkImageView> &getSwapchainViews() const { return views; }
}; // class Context


/// A base class for an object dependent upon a Vulkan context.
/// Each instance has a reference back to the context it was created under.
class Object : public NoCopy {
protected:
	uint32_t id;
	Context &vk; ///< Every VK::Object has a reference to its context.

public:
	/// Default constructor, initializes the gl context and adds itself to the object list.
	Object() : vk(*Context::GetCurrent()) { id = vk.addObject(this); }

	/// Default destructor, destroys object and removes itself from the object list.
	virtual ~Object() { destroy(); vk.removeObject(id, this); }

	/// Virtual destroy method for cleaning up Vulkan objects
	virtual void destroy() {}

	/// Virtual method for making sure an object was created properly.
	virtual bool isValid() const { return false; }

	/// Virtual method for getting the size of a Vulkan object (for tracking video memory usage)
	virtual uint32_t getSize() const { return 0; }
};

} // namespace VK

#if GL
#include "VKCore.h"
#include "VKStates.h"
#include "VKIndexBuffer.h"

#ifdef _WIN32
#define WGL_ARB_create_context 1
#define WGL_CONTEXT_DEBUG_BIT_ARB 0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB 0x2093
#define WGL_CONTEXT_FLAGS_ARB 0x2094
extern "C" typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int* attribList);
#else
// X implementation?
#endif

namespace VK {

class Object; // Forward reference

#ifndef GLU_VERSION_1_1
inline const GLubyte* gluErrorString(GLenum errCode) {
	static char szBuffer[256];
	switch(errCode) {
		case GL_NO_ERROR:
			strcpy(szBuffer, "GL_NO_ERROR");
			break;
		case GL_INVALID_ENUM:
			strcpy(szBuffer, "GL_INVALID_ENUM");
			break;
		case GL_INVALID_VALUE:
			strcpy(szBuffer, "GL_INVALID_VALUE");
			break;
		case GL_INVALID_OPERATION:
			strcpy(szBuffer, "GL_INVALID_OPERATION");
			break;
		case GL_OUT_OF_MEMORY:
			strcpy(szBuffer, "GL_OUT_OF_MEMORY");
			break;
		default:
			sprintf(szBuffer, "GL error %u", errCode);
			break;
	}
	return (GLubyte *)szBuffer;
}
#endif

#ifdef _DEBUG
#define PRE_GL_CHECK() if(pCurrent != this) VKLogException(pszLastError = "Messing with the wrong context")
#define POST_GL_CHECK() if((nLastError = glGetError()) != GL_NO_ERROR) VKLogException(pszLastError = (const char *)gluErrorString(nLastError))
#else
#define PRE_GL_CHECK() NULL
#define POST_GL_CHECK() NULL
#endif

/// Encapsulates an OpenGL 3.x context and all standard extensions.
/// The OpenGL Context is the heart of any OpenGL application. All OpenGL
/// functions are wrapped as members for various reasons. One is to allow
/// multiple contexts, each of which may support different extensions. Another
/// is to provide automatic error checking before and after each GL call.
/// It also tracks certain state changes and allows sets of states to be
/// pushed/popped (very useful for ShaderTechnique).
class Context {
private:
	static Context *pCurrent; ///< Points to the current OpenGL context (set using makeCurrent())

	std::string m_strVendor; ///< The vendor string for the current context's driver
	std::string m_strRenderer; ///< The renderer string for the current context's driver
	std::string m_strVersion; ///< The version string for the current context's driver
	std::string m_strExtensions; ///< The extensions string for the current context's driver

	GLenum nLastError; ///< The last error code set by a GL call
	const char *pszLastError; ///< The last error message set by a GL call
	std::list<Object *> m_lObjects; ///< A list of objects tied to this context

#ifdef _WIN32
	HWND m_hWnd;
	HDC m_hDC;

#ifdef EGL_VERSION_1_0
	EGLDisplay	m_eglDisplay;
	EGLContext	m_eglContext;
	EGLSurface	m_eglSurface;
	void *getProcAddress(const char *psz) { return eglGetProcAddress(psz); }
#else
	HGLRC m_hGLRC;
	void *getProcAddress(const char *psz) { return wglGetProcAddress(psz); }
#endif

#else
	// X implementation?
#endif

	unsigned int m_nEnabledStates; ///< A bit mask to track which states are enabled/disabled
	std::vector<unsigned int> m_lEnabledStates; ///< A stack for pushing/popping enabled states (used by ShaderTechnique)

	unsigned int m_nEnabledBlendStates; ///< A bit mask to track which blend states are enabled/disabled
	std::vector<unsigned int> m_lEnabledBlendStates; ///< A stack for pushing/popping enabled blend states (used by ShaderTechnique)

	unsigned int m_nEnumStates; ///< A bit mask to track common enumerated states
	std::vector<unsigned int> m_lEnumStates; ///< A stack for pushing/popping common enumerated states (used by ShaderTechnique)

	struct Viewport {
		GLint x, y, width, height;

		Viewport() : x(0), y(0), width(0), height(0) {}
		Viewport(GLint a, GLint b, GLint w, GLint h) : x(a), y(b), width(w), height(h) {}
	};
	Viewport m_viewport; ///< The current viewport
	std::vector<Viewport> m_lViewports; ///< A stack for pushing/popping viewports

	std::list<std::string> m_lWarnings; ///< List of warnings/errors associated with this context

public:
	static Context *GetCurrent() { return pCurrent; } ///< Returns the currently active OpenGL context
	void getGPUInfo();

	Context(); ///< Default constructor
	~Context() { destroy(); } ///< Default destructor (destroys OpenGL context)

	/// Call to get the GL_VENDOR string for this context.
	const char *getVendor() const { return m_strVendor.c_str(); }
	/// Call to get the GL_RENDERER string for this context.
	const char *getRenderer() const { return m_strRenderer.c_str(); }
	/// Call to get the GL_VERSION string for this context.
	const char *getVersion() const { return m_strVersion.c_str(); }
	/// Call to get the GL_EXTENSIONS string for this context.
	const char *getExtensions() const { return m_strExtensions.c_str(); }

#ifdef _WIN32
	/// Creates a new OpenGL 3.x context.
	/// @param[in] hWnd The window handle to attach the context to
	/// @param[in] nMajor The major OpenGL version to use
	/// @param[in] nMinor The minor OpenGL version to use
	/// @param[in] bForwardCompatible Set to true to disable compatibility with GL 2.x
	bool create(HWND hWnd, int nMajor = 3, int nMinor = 0, bool bForwardCompatible = true);
#else
	// X implementation?
#endif

	void destroy(); ///< Destroys this OpenGL context
	void makeCurrent(); ///< Makes this context the current one
	void swapBuffers(); ///< Swaps the back buffer to the front

	/// Converts C types to GL enums like GL_UNSIGNED_SHORT, GL_FLOAT, etc.
	template<class T> static GLenum GLDataType();

	/// Call to add a VK::Object to this context's list of objects.
	/// (This should only be called by VK::Object's constructor)
	void addObject(Object *pObj) { m_lObjects.push_back(pObj); }

	/// Call to remove a VK::Object to this context's list of objects.
	/// (This should only be called by VK::Object's destructor)
	void removeObject(Object *pObj) {
		for (std::list<Object *>::iterator it = m_lObjects.begin(); it != m_lObjects.end(); it++) {
			if (*it == pObj) {
				m_lObjects.erase(it);
				break;
			}
		}
	}

	/// Call to add a warning
	void addWarning(const char *psz) { m_lWarnings.push_back(psz); }

	/// Call to add a warning
	void addWarning(const std::string &str) { m_lWarnings.push_back(str); }

	/// Call to retrieve the list of warnings
	std::list<std::string> &getWarningList() { return m_lWarnings; }

};

/// The base class for an object dependent upon an OpenGL context.
/// Each instance has a reference back to the context it was created under.
class Object {
protected:
	Context &gl; ///< Every VK::Object has a reference to its context.

public:
	/// Default constructor, initializes the gl context and adds itself to the object list.
	Object() : gl(*Context::GetCurrent()) { gl.addObject(this); }

	/// Default destructor, destroys object and removes itself from the object list.
	virtual ~Object() { destroy(); gl.removeObject(this); }

	/// Virtual destroy method for cleaning up OpenGL objects
	virtual void destroy() {}

	/// Virtual method for making sure an object was created properly.
	virtual bool isValid() const { return false; }

	/// Virtual method for getting the size of an OpenGL object (for tracking video memory usage)
	virtual GLuint getSize() const { return 0; }
};

} // namespace VK
#endif

#endif // __VKContext_h__

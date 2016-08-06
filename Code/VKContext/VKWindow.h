// VKWindow.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan API easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#ifndef __VKWindow_h__
#define __VKWindow_h__

#include "VKContext.h"

namespace VK {

/// Encapsulates an OpenVK Window.
/// An OpenVK app can have multiple windows, and each window can have more than
/// one context associated with it, but each context must be tied to a window.
/// This class creates a window and an OpenVK context.
class Window : public NoCopy {
private:
	static std::list<HWND> m_lWindows; ///< A list of OpenVK windows (onIdle is called for each in the main game loop)

protected:
	std::string m_strName; ///< The name (or title) of this window
	unsigned short m_nWidth; ///< The current width of this window
	unsigned short m_nHeight; ///< The current height of this window
	bool m_bFullScreen; ///< Set to true for fullscreen mode (currently only sets window style, does not change screen resolution)
	bool m_bActive; ///< Set to true when the window is active, false otherwise
	bool m_bSizing; ///< Set to true when the window is being resized

// Private platform-specific members
#ifdef _WIN32
	static HINSTANCE m_hInstance; ///< The global application instance handle
	HWND m_hWnd; ///< This window's handle

	/// The default window proc for VK::Window
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

public:
	Context vk; ///< The primary OpenVK context associated with this window

	// Is this the best way to handle global Init/Cleanup when there can be multiple instances?
	class Init {
	public:
		Init(void *pInstace = NULL);
		~Init();
	};

	Window() : m_hWnd(NULL) {}

	/// Creates a window and its primary OpenVK context.
	/// @param[in] nVersion The Vulkan SDK version
	/// @param[in] pszName The name you want to show in the window's title bar
	/// @param[in] nWidth The starting width of the window
	/// @param[in] nHeight The starting height of the window
	/// @param[in] bFullScreen Set to true for fullscreen mode (currently only sets window style, does not change screen resolution)
	/// @param[in] hParent Optional handle to a parent window (defaults to NULL)
	void create(uint32_t nVersion, const char *pszName, unsigned short nWidth, unsigned short nHeight, bool bFullScreen, HWND hParent = NULL);

	/// Destroys the window, its primary OpenVK context, and all objects tied to that context.
	~Window();

	/// Call to destroy the window and cleanup
	void destroy();

	/// Call to see if a specific key is currently pressed
	bool isKeyDown(uint16_t nKey) const;

	bool isActive() const { return m_bActive; }
	bool isFullscreen() const { return m_bFullScreen; }
	uint16_t getWidth() const { return m_nWidth; }
	uint16_t getHeight() const { return m_nHeight; }
	Context &getContext() { return vk; }

	/// Override to add code to render your frame
	virtual void onIdle() = 0;

	/// Override to add special handling for the WM_CREATE message
	virtual void onCreate() {}
	/// Override to add special handling for the WM_DESTROY message
	virtual void onDestroy() {}
	/// Override to add special handling for the WM_KEYDOWN message
	virtual void onKeyDown(uint16_t nKey) {}
	/// Override to add special handling for the WM_KEYUP message
	virtual void onKeyUp(uint16_t nKey) {}
	/// Override to add special handling for the WM_SIZE message
	virtual void onSize(unsigned short nWidth, unsigned short nHeight) {
		m_nHeight = nHeight;
		m_nWidth = nWidth;
	}
	/// Override to add special handling for the WM_ACTIVATE message
	virtual void onActivate(bool bActive) { m_bActive = bActive; }

	/// Runs the Windows message pump.
	/// You define your own game loop iteration in your onIdle() method,
	/// but the loop itself is controlled in here. It will exit only when
	/// all windows have been destroyed or when it receives WM_QUIT.
	static void Run();

	/// Closes all open windows.
	static void Shutdown();

// Public platform-specific members
#ifdef _WIN32
	operator HWND()	{ return m_hWnd; }
#endif
};

} // namespace VK

#endif // __VKWindow_h__

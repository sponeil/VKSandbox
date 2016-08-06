// VKWindow.cpp
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan API easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#include "VKCore.h"
#include "VKContext.h"
#include "VKWindow.h"


namespace VK {

std::list<HWND> Window::m_lWindows;


#ifdef _WIN32
const char *WINDOW_CLASS_NAME = "VK::Window";
HINSTANCE Window::m_hInstance = NULL;

Window::Init::Init(void *pInstance) {
	VK::Window::m_hInstance = (HINSTANCE)pInstance;
	WNDCLASS wc = {CS_OWNDC | CS_VREDRAW | CS_HREDRAW, (WNDPROC)VK::Window::WindowProc, 0, 0, VK::Window::m_hInstance, NULL, LoadCursor((HINSTANCE)NULL, IDC_ARROW), (HBRUSH)GetStockObject(BLACK_BRUSH), NULL, WINDOW_CLASS_NAME};
	if(!::RegisterClass(&wc))
		VKLogException("Failed to register window class, aborting.");
}

Window::Init::~Init() {
	if(!::UnregisterClass(WINDOW_CLASS_NAME, VK::Window::m_hInstance))
		VKLogException("Failed to unregister window class, aborting.");
	VK::Window::m_hInstance = NULL;
}

void Window::create(uint32_t nVersion, const char *pszName, unsigned short nWidth, unsigned short nHeight, bool bFullScreen, HWND hParent) {
	m_strName = pszName;
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_bFullScreen = bFullScreen;
	m_bSizing = false;

	DWORD dwStyle = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | (bFullScreen ? WS_POPUP : WS_OVERLAPPEDWINDOW);
	DWORD dwExStyle = (bFullScreen ? WS_EX_TOPMOST : 0);
#ifdef _DEBUG
	// There's nothing worse than hitting a breakpoint with a top-most full-screen window
	dwExStyle = 0;
#endif

	RECT wr = { 0, 0, nWidth, nHeight };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	LONG w = wr.right - wr.left, h = wr.bottom - wr.top;

	m_hWnd = ::CreateWindowEx(dwExStyle, WINDOW_CLASS_NAME, pszName, dwStyle, 50, 50, w, h, hParent, NULL, m_hInstance, this);
	if(m_hWnd == NULL)
		VKLogException("Failed to create window");
	m_lWindows.push_back(m_hWnd);
	::ShowWindow(m_hWnd, SW_SHOWNORMAL);
	vk.create(m_hInstance, m_hWnd, true, pszName, nVersion);
	onCreate();
	RECT rect;
	::GetClientRect(m_hWnd, &rect);
	onSize(rect.right - rect.left, rect.bottom - rect.top);
}

void Window::destroy() {
	vk.destroy();
	if(m_hWnd)
		::DestroyWindow(m_hWnd);
}

Window::~Window() {
	destroy();
}

LRESULT CALLBACK Window::WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam) {
	if(nMsg == WM_CREATE) {
		CREATESTRUCT *p = (CREATESTRUCT *)lParam;
		Window *pWin = (Window *)p->lpCreateParams;
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pWin);
		return 0;
	}

	Window *pWin = (Window *)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
	switch(nMsg) {
		case WM_KEYDOWN:
			if((lParam & 0x40000000) == 0)
				pWin->onKeyDown((uint16_t)wParam);
			break;
		case WM_KEYUP:
			pWin->onKeyUp((uint16_t)wParam);
			break;
		case WM_DESTROY:
			pWin->onDestroy();
			pWin->m_hWnd = NULL;
			for(std::list<HWND>::iterator it = m_lWindows.begin(); it != m_lWindows.end(); ) {
				if(*it == hWnd)
					it = m_lWindows.erase(it);
				else
					it++;
			}
			break;

		case WM_ENTERSIZEMOVE:           // Sent one time to a window after it enters the moving or sizing modal loop.
			if (pWin)                    // Set a flag to avoid resizing the swapchain repeatedly.
				pWin->m_bSizing = true;  // The AMD Vulkan DLL I'm loading seems to leak memory badly if you do that.
			break;
		case WM_SIZE:                    // Sent to a window after its size has changed.
			if (pWin && pWin->m_bSizing) // If the window is in a resize modal loop, wait until the loop exits.
				break;
			// If not in a move/resize loop, fall through to resize the swapchain
		case WM_EXITSIZEMOVE:
			if (pWin && pWin->vk.valid()) {
				uint16_t w = LOWORD(lParam), h = HIWORD(lParam);
				if (w == 0 || h == 0) {
					RECT r;
					::GetClientRect(hWnd, &r);
					w = (uint16_t)(r.right - r.left);
					h = (uint16_t)(r.bottom - r.top);
				}
				pWin->onSize(w, h);
				pWin->m_bSizing = false;
			}
			break;

		case WM_ACTIVATE:
			if(pWin)
				pWin->onActivate(wParam != 0);
			break;
		case WM_CLOSE:
			::DestroyWindow(hWnd);
			return 0;
	}
	return ::DefWindowProc(hWnd, nMsg, wParam, lParam);
}

bool Window::isKeyDown(uint16_t nKey) const {
	return m_bActive && (::GetAsyncKeyState(nKey) & 0x8000) != 0;
}

void Window::Run() {
	MSG msg;
	msg.message = 0;
	while(msg.message != WM_QUIT && !m_lWindows.empty()) {
		for(std::list<HWND>::iterator it = m_lWindows.begin(); it != m_lWindows.end(); it++) {
			if(Window *pWin = (Window *)::GetWindowLongPtr(*it, GWLP_USERDATA))
				pWin->onIdle();
		}
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) && msg.message != WM_QUIT) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

void Window::Shutdown() {
	std::vector<Window *> vWindows;
	for(std::list<HWND>::iterator it = m_lWindows.begin(); it != m_lWindows.end(); it++) {
		if(Window *pWin = (Window *)::GetWindowLongPtr(*it, GWLP_USERDATA))
			vWindows.push_back(pWin);
	}
	for(size_t i=0; i<vWindows.size(); i++)
		vWindows[i]->destroy();
}

#else // _WIN32

#endif // _WIN32

} // namespace VK

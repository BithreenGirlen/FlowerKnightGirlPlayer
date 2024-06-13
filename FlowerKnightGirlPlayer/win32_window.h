#ifndef WIN32_WINDOW_H_
#define WIN32_WINDOW_H_

#include <Windows.h>

namespace win32_window
{
	void ResizeWindow(HWND hWnd, int iBaseWidth, int iBaseHeight, float fScale);
	bool IsWidowBarHidden(HWND hWnd);
}
#endif // !WIN32_WINDOW_H_



#include "win32_window.h"

/*窓寸法調整*/
void win32_window::ResizeWindow(HWND hWnd, int iBaseWidth, int iBaseHeight, float fScale)
{
	if (hWnd != nullptr)
	{
		bool bBarHidden = IsWidowBarHidden(hWnd);
		RECT rect;
		if (!bBarHidden)
		{
			::GetWindowRect(hWnd, &rect);
		}
		else
		{
			::GetClientRect(hWnd, &rect);
		}

		float fDpiScale = ::GetDpiForWindow(hWnd) / 96.f;
		int iX = static_cast<int>(iBaseWidth * fScale);
		int iY = static_cast<int>(iBaseHeight * fScale);
		rect.right = iX + rect.left;
		rect.bottom = iY + rect.top;
		if (!bBarHidden)
		{
			LONG lStyle = ::GetWindowLong(hWnd, GWL_STYLE);
			::AdjustWindowRect(&rect, lStyle, TRUE);
			::SetWindowPos(hWnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
		}
		else
		{
			RECT rc;
			::GetWindowRect(hWnd, &rc);
			::MoveWindow(hWnd, rc.left, rc.top, rect.right, rect.bottom, TRUE);
		}
	}
}
/*枠縁有無*/
bool win32_window::IsWidowBarHidden(HWND hWnd)
{
	if (hWnd != nullptr)
	{
		LONG lStyle = ::GetWindowLong(hWnd, GWL_STYLE);
		return !((lStyle & WS_CAPTION) && (lStyle & WS_SYSMENU));
	}
	return false;
}

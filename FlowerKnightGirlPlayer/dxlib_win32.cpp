

#include "dxlib_win32.h"

#ifndef DX_NON_USING_NAMESPACE_DXLIB
#define DX_NON_USING_NAMESPACE_DXLIB
#endif // DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

/*緩衝容量再設定*/
void dxlib_win32::ResizeBuffer(HWND hRenderWnd)
{
	if (hRenderWnd != nullptr)
	{
		RECT rc;
		::GetClientRect(hRenderWnd, &rc);

		int iClientWidth = rc.right - rc.left;
		int iClientHeight = rc.bottom - rc.top;

		int iDesktopWidth = ::GetSystemMetrics(SM_CXSCREEN);
		int iDesktopHeight = ::GetSystemMetrics(SM_CYSCREEN);

		DxLib::SetGraphMode
		(
			iClientWidth < iDesktopWidth ? iClientWidth : iDesktopWidth,
			iClientHeight < iDesktopHeight ? iClientHeight : iDesktopHeight,
			32
		);
	}
}

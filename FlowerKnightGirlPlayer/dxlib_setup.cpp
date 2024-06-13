
#include <Windows.h>

#include "dxlib_setup.h"

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

namespace dxlib_setup
{
	static int g_iDxLibInitialised = -1;
}

bool dxlib_setup::SetupDxLib(void* pWindowHandle)
{
	if (g_iDxLibInitialised == 0)return true;

	HWND hRenderWnd = static_cast<HWND>(pWindowHandle);

	int iRet = -1;
	iRet = DxLib::SetOutApplicationLogValidFlag(FALSE);
	if (iRet == -1)return false;

	if (hRenderWnd != nullptr)
	{
		iRet = DxLib::SetUserWindow(hRenderWnd);
		if (iRet == -1)return false;
	}
	iRet = DxLib::SetUserWindowMessageProcessDXLibFlag(hRenderWnd != nullptr ? FALSE : TRUE);
	if (iRet == -1)return false;

	iRet = DxLib::SetChangeScreenModeGraphicsSystemResetFlag(hRenderWnd != nullptr ? FALSE : TRUE);
	if (iRet == -1)return false;

	iRet = DxLib::ChangeWindowMode(TRUE);
	if (iRet == -1)return false;

	iRet = DxLib::SetDrawMode(DX_DRAWMODE_BILINEAR);
	if (iRet == -1)return false;

	iRet = DxLib::SetMultiThreadFlag(TRUE);
	if (iRet == -1)return false;

	iRet = DxLib::SetUseTransColor(FALSE);
	if (iRet == -1)return false;

	g_iDxLibInitialised = DxLib::DxLib_Init();
	if (g_iDxLibInitialised == -1)return false;

	iRet = DxLib::SetDrawScreen(DX_SCREEN_BACK);
	if (iRet == -1)
	{
		DxLib::DxLib_End();
		g_iDxLibInitialised = -1;
	}

	return g_iDxLibInitialised != -1;
}

void dxlib_setup::ShutdownDxLib()
{
	if (g_iDxLibInitialised != -1)
	{
		DxLib::DxLib_End();
	}
}


#include "framework.h"
#include "main_window.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    int iRet = 0;
    CMainWindow sWindow;
    bool bRet = sWindow.Create(hInstance, L"FKG player");
    if (bRet)
    {
        ::ShowWindow(sWindow.GetHwnd(), nCmdShow);
        iRet = sWindow.MessageLoop();
    }

    return iRet;
}
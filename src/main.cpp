
#include <locale.h>

#ifdef _DEBUG
#pragma comment(lib, "spine-cpp-3.8/lib/spine-cpp-d.lib")
#else
#pragma comment(lib, "spine-cpp-3.8/lib/spine-cpp.lib")
#endif // _DEBUG

#include "framework.h"
#include "main_window.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    ::setlocale(LC_ALL, ".utf8");

    int iRet = 0;
    CMainWindow mainWindow;
    bool bRet = mainWindow.Create(hInstance);
    if (bRet)
    {
        ::ShowWindow(mainWindow.GetHwnd(), nCmdShow);
        iRet = mainWindow.MessageLoop();
    }

    return iRet;
}

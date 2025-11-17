
#include <Windows.h>
#include <CommCtrl.h>

#include "main_window.h"

#include "win_dialogue.h"
#include "win_filesystem.h"
#include "path_utility.h"
#include "native-ui/media_setting_dialogue.h"
#include "native-ui/window_menu.h"

CMainWindow::CMainWindow()
{

}

CMainWindow::~CMainWindow()
{

}

bool CMainWindow::Create(HINSTANCE hInstance, const wchar_t* pwzWindowName, HICON hIcon)
{
	WNDCLASSEXW wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = ::GetSysColorBrush(COLOR_BTNFACE);
	wcex.lpszClassName = m_pwzClassName;
	if (hIcon != nullptr)
	{
		wcex.hIcon = hIcon;
		wcex.hIconSm = hIcon;
	}

	if (::RegisterClassExW(&wcex))
	{
		m_hInstance = hInstance;

		UINT uiDpi = ::GetDpiForSystem();
		int iWindowWidth = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);
		int iWindowHeight = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);

		if (pwzWindowName != nullptr)m_pwzDefaultWindowName = pwzWindowName;
		m_hWnd = ::CreateWindowW(m_pwzClassName, m_pwzDefaultWindowName, WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
			CW_USEDEFAULT, CW_USEDEFAULT, iWindowWidth, iWindowHeight, nullptr, nullptr, hInstance, this);
		if (m_hWnd != nullptr)
		{
			return true;
		}
		else
		{
			std::wstring wstrMessage = L"CreateWindowW failed; code: " + std::to_wstring(::GetLastError());
			::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
		}
	}
	else
	{
		std::wstring wstrMessage = L"RegisterClassExW failed; code: " + std::to_wstring(::GetLastError());
		::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
	}

	return false;
}

int CMainWindow::MessageLoop()
{
	MSG msg;

	for (;;)
	{
		BOOL bRet = ::GetMessageW(&msg, 0, 0, 0);
		if (bRet > 0)
		{
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
		}
		else if (bRet == 0)
		{
			/*ループ終了*/
			return static_cast<int>(msg.wParam);
		}
		else
		{
			/*ループ異常*/
			std::wstring wstrMessage = L"GetMessageW failed; code: " + std::to_wstring(::GetLastError());
			::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
			return -1;
		}
	}
	return 0;
}
/*C CALLBACK*/
LRESULT CMainWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CMainWindow* pThis = nullptr;
	if (uMsg == WM_NCCREATE)
	{
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = reinterpret_cast<CMainWindow*>(pCreateStruct->lpCreateParams);
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
	}

	pThis = reinterpret_cast<CMainWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (pThis != nullptr)
	{
		return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*メッセージ処理*/
LRESULT CMainWindow::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		return OnCreate(hWnd);
	case WM_DESTROY:
		return OnDestroy();
	case WM_CLOSE:
		return OnClose();
	case WM_SIZE:
		return OnSize(wParam, lParam);
	case WM_PAINT:
		return OnPaint();
	case WM_ERASEBKGND:
		return 1;
	case WM_KEYDOWN:
		return OnKeyDown(wParam, lParam);
	case WM_KEYUP:
		return OnKeyUp(wParam, lParam);
	case WM_COMMAND:
		return OnCommand(wParam, lParam);
	case WM_MOUSEMOVE:
		return OnMouseMove(wParam, lParam);
	case WM_MOUSEWHEEL:
		return OnMouseWheel(wParam, lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam, lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(wParam, lParam);
	case WM_RBUTTONUP:
		return OnRButtonUp(wParam, lParam);
	case WM_MBUTTONUP:
		return OnMButtonUp(wParam, lParam);
	default:
		break;
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*WM_CREATE*/
LRESULT CMainWindow::OnCreate(HWND hWnd)
{
	m_hWnd = hWnd;

	InitialiseMenuBar();

	m_pDxLibInit = std::make_unique<SDxLibInit>(m_hWnd);

	m_pScenePlayer = std::make_unique<CFkgScenePlayer>();
	SetMenuCheckState(MenuBar::kImage, Menu::kSyncImage, m_pScenePlayer->IsImageSynced());

	return 0;
}
/*WM_DESTROY*/
LRESULT CMainWindow::OnDestroy()
{
	::PostQuitMessage(0);

	return 0;
}
/*WM_CLOSE*/
LRESULT CMainWindow::OnClose()
{
	::DestroyWindow(m_hWnd);
	::UnregisterClassW(m_pwzClassName, m_hInstance);

	m_pScenePlayer.reset();
	m_pDxLibInit.reset();

	return 0;
}
/*WM_PAINT*/
LRESULT CMainWindow::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint(m_hWnd, &ps);

	if (m_pScenePlayer.get() != nullptr && m_pScenePlayer->HasScenarioData())
	{
		m_pScenePlayer->Update();

		DxLib::ClearDrawScreen();

		m_pScenePlayer->Redraw();

		DxLib::ScreenFlip();

		::InvalidateRect(m_hWnd, nullptr, FALSE);
	}

	::EndPaint(m_hWnd, &ps);

	return 0;
}
/*WM_SIZE*/
LRESULT CMainWindow::OnSize(WPARAM wParam, LPARAM lParam)
{
	int iClientWidth = LOWORD(lParam);
	int iClientHeight = HIWORD(lParam);

	int iDesktopWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int iDesktopHeight = ::GetSystemMetrics(SM_CYSCREEN);

	int iGraphWidth = iClientWidth < iDesktopWidth ? iClientWidth : iDesktopWidth;
	int iGraphHeight = iClientHeight < iDesktopHeight ? iClientHeight : iDesktopHeight;

	DxLib::SetGraphMode(iGraphWidth, iGraphHeight, 32);

	return 0;
}
/*WM_KEYDOWN*/
LRESULT CMainWindow::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_RIGHT:
		if (m_pScenePlayer.get() != nullptr)
		{
			if (!m_pScenePlayer->HasReachedLastScene())
			{
				m_pScenePlayer->ShiftScene(true);
			}
		}
		break;
	case VK_LEFT:
		if (m_pScenePlayer.get() != nullptr)
		{
			m_pScenePlayer->ShiftScene(false);
		}
		break;
	default:
		break;
	}

	return 0;
}
/*WM_KEYUP*/
LRESULT CMainWindow::OnKeyUp(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_ESCAPE:
		::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
		break;
	case VK_UP:
		KeyOnForeFile();
		break;
	case VK_DOWN:
		KeyOnNextFile();
		break;
	case 'C':
		if (m_pScenePlayer.get() != nullptr)
		{
			m_pScenePlayer->ToggleTextColour();
		}
		break;
	case 'T':
		if (m_pScenePlayer.get() != nullptr)
		{
			m_pScenePlayer->SetTextVisibility(!m_pScenePlayer->IsTextVisible());
		}
		break;
	}
	return 0;
}
/*WM_COMMAND*/
LRESULT CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	int wmKind = LOWORD(lParam);
	if (wmKind == 0)
	{
		/*Menus*/
		switch (wmId)
		{
		case Menu::kOpenFile:
			MenuOnOpenFile();
			break;
		case Menu::kAudioSetting:
			MenuOnAudioSetting();
			break;
		case Menu::kSyncImage:
			MenuOnSyncImage();
			break;
		default:
			if (wmId >= Menu::kLabelStartIndex)
			{
				JumpToLabel(static_cast<size_t>(wmId - Menu::kLabelStartIndex));
			}
			break;
		}
	}
	else
	{
		/*Controls*/

	}

	return 0;
}
/*WM_MOUSEMOVE */
LRESULT CMainWindow::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	WORD usKey = LOWORD(wParam);
	if (usKey == MK_LBUTTON)
	{
		if (m_wasLeftCombinated)return 0;

		POINT pt{};
		::GetCursorPos(&pt);
		int iX = m_lastCursorPos.x - pt.x;
		int iY = m_lastCursorPos.y - pt.y;

		if (m_pScenePlayer.get() != nullptr && m_hasLeftBeenDragged)
		{
			m_pScenePlayer->MoveViewPoint(iX, iY);
		}

		m_lastCursorPos = pt;
		m_hasLeftBeenDragged = true;
	}

	return 0;
}
/*WM_MOUSEWHEEL*/
LRESULT CMainWindow::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
	int iScroll = -static_cast<short>(HIWORD(wParam)) / WHEEL_DELTA;
	WORD usKey = LOWORD(wParam);

	if (usKey == MK_LBUTTON)
	{
		if (m_pScenePlayer.get() != nullptr)
		{
			m_pScenePlayer->RescaleAnimationTime(iScroll > 0);
		}
	}
	else if (usKey == MK_RBUTTON)
	{
		if (m_pScenePlayer.get() != nullptr)
		{
			m_pScenePlayer->ShiftScene(iScroll > 0);
		}

		m_wasRightCombinated = true;
	}
	else
	{
		if (m_pScenePlayer.get() != nullptr)
		{
			m_pScenePlayer->RescaleImage(iScroll > 0);
			if (!(usKey & MK_CONTROL))
			{
				ResizeWindow();
			}
		}
	}

	return 0;
}
/*WM_LBUTTONDOWN*/
LRESULT CMainWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	::GetCursorPos(&m_lastCursorPos);

	m_wasLeftPressed = true;

	return 0;
}
/*WM_LBUTTONUP*/
LRESULT CMainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (m_wasLeftCombinated || m_hasLeftBeenDragged)
	{
		m_hasLeftBeenDragged = false;
		m_wasLeftCombinated = false;
		m_wasLeftPressed = false;

		return 0;
	}
	WORD usKey = LOWORD(wParam);
	if (usKey == MK_RBUTTON)
	{
		if (m_isFramelessWindow)
		{
			::PostMessage(m_hWnd, WM_SYSCOMMAND, SC_MOVE, 0);
			INPUT input{};
			input.type = INPUT_KEYBOARD;
			input.ki.wVk = VK_DOWN;
			::SendInput(1, &input, sizeof(input));

			m_wasRightCombinated = true;
		}
	}
	else if (usKey == 0 && m_wasLeftPressed)
	{
		POINT pt{};
		::GetCursorPos(&pt);
		int iX = m_lastCursorPos.x - pt.x;
		int iY = m_lastCursorPos.y - pt.y;

		if (iX == 0 && iY == 0)
		{
			if (m_pScenePlayer.get() != nullptr)
			{
				if (!m_pScenePlayer->IsImageSynced())
				{
					m_pScenePlayer->ShiftImage();
				}
			}
		}
	}
	m_wasLeftPressed = false;

	return 0;
}
/*WM_RBUTTONUP*/
LRESULT CMainWindow::OnRButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (m_wasRightCombinated)
	{
		m_wasRightCombinated = false;
		return 0;
	}

	if (m_pScenePlayer.get() == nullptr || !m_pScenePlayer->HasScenarioData())return 0;

	WORD usKey = LOWORD(wParam);
	if (usKey == 0)
	{
		const auto& labelData = m_pScenePlayer->GetLabelData();
		if (labelData.empty())return 0;

		window_menu::CContextMenu contextMenu;
		for (size_t i = 0; i < labelData.size(); ++i)
		{
			contextMenu.AddItems({ {Menu::kLabelStartIndex + i, labelData[i].wstrCaption.c_str()} });
		}
		contextMenu.Display(m_hWnd);
	}

	return 0;
}
/*WM_MBUTTONUP*/
LRESULT CMainWindow::OnMButtonUp(WPARAM wParam, LPARAM lParam)
{
	WORD usKey = LOWORD(wParam);

	if (usKey == 0)
	{
		if (m_pScenePlayer.get() != nullptr && m_pScenePlayer->HasScenarioData())
		{
			m_pScenePlayer->ResetScale();
			ResizeWindow();
		}
	}
	else if (usKey == MK_RBUTTON)
	{
		ToggleWindowBorderStyle();

		m_wasRightCombinated = true;
	}

	return 0;
}
/*操作欄作成*/
void CMainWindow::InitialiseMenuBar()
{
	if (::IsMenu(m_hMenuBar))return;

	HMENU hMenu = window_menu::MenuBuilder(
		{
			{0, L"File", window_menu::MenuBuilder(
				{
					{Menu::kOpenFile, L"Open"},
				}).Get(),
			},
			{0, L"Setting", window_menu::MenuBuilder(
				{
					{Menu::kAudioSetting, L"Audio"},
				}).Get()
			},
			{0, L"Image", window_menu::MenuBuilder(
				{
					{Menu::kSyncImage, L"Sync"}
				}).Get()
			}
		}).Get();

	if (!::IsMenu(hMenu) || ::SetMenu(m_hWnd, hMenu) == 0)
	{
		std::wstring wstrMessage = L"Failed to create menu; code: " + std::to_wstring(::GetLastError());
		::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);

		if (::IsMenu(hMenu))::DestroyMenu(hMenu);
	}
	else
	{
		m_hMenuBar = hMenu;
	}
}

void CMainWindow::MenuOnOpenFile()
{
	std::wstring wstrPickedFile = win_dialogue::SelectOpenFile(L"script file", L"*.txt", nullptr, m_hWnd);
	if (!wstrPickedFile.empty())
	{
		bool bRet = SetupScenario(wstrPickedFile.c_str());
		if (bRet)
		{
			m_scriptFilePaths.clear();
			m_nScriptFilePathIndex = 0;
			win_filesystem::GetFilePathListAndIndex(wstrPickedFile.c_str(), L"*.txt", m_scriptFilePaths, &m_nScriptFilePathIndex);
		}
	}
}

void CMainWindow::KeyOnNextFile()
{
	if (m_scriptFilePaths.empty())return;

	++m_nScriptFilePathIndex;
	if (m_nScriptFilePathIndex >= m_scriptFilePaths.size())m_nScriptFilePathIndex = 0;
	SetupScenario(m_scriptFilePaths[m_nScriptFilePathIndex].c_str());
}

void CMainWindow::KeyOnForeFile()
{
	if (m_scriptFilePaths.empty())return;

	--m_nScriptFilePathIndex;
	if (m_nScriptFilePathIndex >= m_scriptFilePaths.size())m_nScriptFilePathIndex = m_scriptFilePaths.size() - 1;
	SetupScenario(m_scriptFilePaths[m_nScriptFilePathIndex].c_str());
}

void CMainWindow::MenuOnAudioSetting()
{
	if (m_pScenePlayer.get() != nullptr)
	{
		CMediaSettingDialogue mediaSettingDialogue;
		mediaSettingDialogue.Open(m_hInstance, m_hWnd, const_cast<void*>(static_cast<const void*>(m_pScenePlayer->GetAudioPlayer())), L"Audio");
	}
}

void CMainWindow::MenuOnSyncImage()
{
	if (m_pScenePlayer.get() != nullptr)
	{
		bool bRet = SetMenuCheckState(MenuBar::kImage, Menu::kSyncImage, !m_pScenePlayer->IsImageSynced());
		if (bRet)
		{
			m_pScenePlayer->ToggleImageSync();
		}
	}
}

bool CMainWindow::SetupScenario(const std::wstring& wstrFolderPath)
{
	bool bRet = false;
	if (m_pScenePlayer.get() != nullptr)
	{
		bRet = m_pScenePlayer->ReadScenario(wstrFolderPath);
		if (bRet)
		{
			ResizeWindow();
		}
	}

	if (bRet)
	{
		std::wstring wstrWindowTitle = path_utility::TruncateFilePath(wstrFolderPath);
		::SetWindowTextW(m_hWnd, wstrWindowTitle.c_str());
	}
	else
	{
		::SetWindowTextW(m_hWnd, m_pwzDefaultWindowName);
	}

	return bRet;
}

void CMainWindow::JumpToLabel(size_t nIndex)
{
	if (m_pScenePlayer.get() == nullptr || !m_pScenePlayer->HasScenarioData())return;

	m_pScenePlayer->JumpToLabel(nIndex);
}
/*表示形式変更*/
void CMainWindow::ToggleWindowBorderStyle()
{
	if (m_pScenePlayer.get() == nullptr || !m_pScenePlayer->HasScenarioData())return;

	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

	m_isFramelessWindow ^= true;

	if (m_isFramelessWindow)
	{
		::SetWindowLong(m_hWnd, GWL_STYLE, lStyle & ~WS_CAPTION & ~WS_SYSMENU);
		::SetWindowPos(m_hWnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
		::SetMenu(m_hWnd, nullptr);
	}
	else
	{
		::SetWindowLong(m_hWnd, GWL_STYLE, lStyle | WS_CAPTION | WS_SYSMENU);
		::SetMenu(m_hWnd, m_hMenuBar);
	}

	ResizeWindow();
}

bool CMainWindow::SetMenuCheckState(unsigned int uiMenuIndex, unsigned int uiItemIndex, bool checked) const
{
	HMENU hMenuBar = ::GetMenu(m_hWnd);
	if (hMenuBar != nullptr)
	{
		HMENU hMenu = ::GetSubMenu(hMenuBar, uiMenuIndex);
		if (hMenu != nullptr)
		{
			DWORD ulRet = ::CheckMenuItem(hMenu, uiItemIndex, checked ? MF_CHECKED : MF_UNCHECKED);
			return ulRet != (DWORD)-1;
		}
	}

	return false;
}
/*窓寸法変更*/
void CMainWindow::ResizeWindow()
{
	unsigned int uiWidth = 0;
	unsigned int uiHeight = 0;
	m_pScenePlayer->GetStillImageSize(&uiWidth, &uiHeight);

	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	int iX = static_cast<int>(uiWidth);
	int iY = static_cast<int>(uiHeight);

	rect.right = iX + rect.left;
	rect.bottom = iY + rect.top;

	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
	const auto IsWidowBarHidden = [&lStyle]()
		-> bool
		{
			return !((lStyle & WS_CAPTION) && (lStyle & WS_SYSMENU));
		};

	::AdjustWindowRect(&rect, lStyle, IsWidowBarHidden() ? FALSE : TRUE);
	::SetWindowPos(m_hWnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
}

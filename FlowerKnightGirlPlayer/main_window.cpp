
#define WIN32_NO_STATUS
#include <Windows.h>
#undef WIN32_NO_STATUS
#include <ntstatus.h>
#include <CommCtrl.h>

#include "main_window.h"
#include "win_filesystem.h"
#include "win_dialogue.h"
#include "win_text.h"
#include "fkg.h"
#include "media_setting_dialogue.h"

CMainWindow::CMainWindow()
{

}

CMainWindow::~CMainWindow()
{

}

bool CMainWindow::Create(HINSTANCE hInstance, const wchar_t* pwzWindowName)
{
	WNDCLASSEXW wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	//wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_APP));
	wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = ::GetSysColorBrush(COLOR_BTNFACE);
	//wcex.lpszMenuName = MAKEINTRESOURCE(IDI_ICON_APP);
	wcex.lpszClassName = m_swzClassName;
	//wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON_APP));

	if (::RegisterClassExW(&wcex))
	{
		m_hInstance = hInstance;

		UINT uiDpi = ::GetDpiForSystem();
		int iWindowWidth = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);
		int iWindowHeight = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);

		m_hWnd = ::CreateWindowW(m_swzClassName, pwzWindowName, WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
			CW_USEDEFAULT, CW_USEDEFAULT, iWindowWidth, iWindowHeight, nullptr, nullptr, hInstance, this);
		if (m_hWnd != nullptr)
		{
			return true;
		}
		else
		{
			std::wstring wstrMessage = L"CreateWindowExW failed; code: " + std::to_wstring(::GetLastError());
			::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
		}
	}
	else
	{
		std::wstring wstrMessage = L"RegisterClassW failed; code: " + std::to_wstring(::GetLastError());
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
	case WM_PAINT:
		return OnPaint();
	case WM_ERASEBKGND:
		return 1;
	case WM_KEYUP:
		return OnKeyUp(wParam, lParam);
	case WM_COMMAND:
		return OnCommand(wParam, lParam);
	case WM_TIMER:
		return OnTimer(wParam);
	case WM_MOUSEWHEEL:
		return OnMouseWheel(wParam, lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam, lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(wParam, lParam);
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

	m_pDxLibInit = new SDxLibInit(m_hWnd);
	if (m_pDxLibInit->iDxLibInitialised == -1)
	{
		::MessageBoxW(m_hWnd, L"Failed to setup DxLib.", L"Error", MB_ICONERROR);
	}

	UpdateDrawingInterval();
	CheckWebpSupport();

	m_pDxLibMidway = new CDxLibMidway(m_hWnd, m_bWebpSupported);
	if (m_pDxLibMidway != nullptr)
	{
		m_pDxLibMidway->SetFont(L"游明朝 Demibold", 24, true, true);
	}
	m_pMfMediaPlayer = new CMfMediaPlayer(nullptr, 0);

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
	if (m_pMfMediaPlayer != nullptr)
	{
		delete m_pMfMediaPlayer;
		m_pMfMediaPlayer = nullptr;
	}
	if (m_pDxLibMidway != nullptr)
	{
		delete m_pDxLibMidway;
		m_pDxLibMidway = nullptr;
	}
	if (m_pDxLibInit != nullptr)
	{
		delete m_pDxLibInit;
		m_pDxLibInit = nullptr;
	}

	::DestroyWindow(m_hWnd);
	::UnregisterClassW(m_swzClassName, m_hInstance);

	return 0;
}
/*WM_PAINT*/
LRESULT CMainWindow::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint(m_hWnd, &ps);

	if (m_pDxLibMidway != nullptr)
	{
		m_pDxLibMidway->Redraw(m_fDelta);
		if (m_pDxLibMidway->IsPlayReady())
		{
			CheckTimer();
		}
	}

	::EndPaint(m_hWnd, &ps);

	return 0;
}
/*WM_SIZE*/
LRESULT CMainWindow::OnSize()
{
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
		KeyUpOnForeFile();
		break;
	case VK_DOWN:
		KeyUpOnNextFile();
		break;
	case 'C':
		if (m_pDxLibMidway != nullptr)
		{
			m_pDxLibMidway->SwitchTextColour();
		}
		break;
	case 'T':
		if (m_pDxLibMidway != nullptr)
		{
			m_pDxLibMidway->SwitchMessageVisibility();
		}
		break;
	default:
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
		case Menu::kSelectFile:
			MenuOnSelectFile();
			break;
		case Menu::kAudioSetting:
			MenuOnAudioSetting();
			break;
		}
	}
	else
	{
		/*Controls*/
	}

	return 0;
}
/*WM_TIMER*/
LRESULT CMainWindow::OnTimer(WPARAM wParam)
{
	return 0;
}
/*WM_MOUSEWHEEL*/
LRESULT CMainWindow::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
	int iScroll = -static_cast<short>(HIWORD(wParam)) / WHEEL_DELTA;
	WORD usKey = LOWORD(wParam);

	if (m_pDxLibMidway != nullptr)
	{
		if (usKey == 0)
		{
			m_pDxLibMidway->RescaleSize(iScroll > 0);
		}

		if (usKey == MK_LBUTTON)
		{
			m_pDxLibMidway->RescaleTime(iScroll > 0);
			m_bSpeedHavingChanged = true;
		}

		if (usKey == MK_RBUTTON)
		{
			ShiftText(iScroll > 0);
		}
	}

	return 0;
}
/*WM_LBUTTONDOWN*/
LRESULT CMainWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	::GetCursorPos(&m_CursorPos);

	m_bLeftDowned = true;

	return 0;
}
/*WM_LBUTTONUP*/
LRESULT CMainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (m_bSpeedHavingChanged)
	{
		m_bSpeedHavingChanged = false;
		return 0;
	}

	WORD usKey = LOWORD(wParam);

	if (usKey == MK_RBUTTON && m_bBarHidden)
	{
		::PostMessage(m_hWnd, WM_SYSCOMMAND, SC_MOVE, 0);
		INPUT input{};
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = VK_DOWN;
		::SendInput(1, &input, sizeof(input));
	}

	if (usKey == 0 && m_bLeftDowned)
	{
		POINT pt{};
		::GetCursorPos(&pt);
		int iX = m_CursorPos.x - pt.x;
		int iY = m_CursorPos.y - pt.y;

		if (m_pDxLibMidway != nullptr)
		{
			if (iX == 0 && iY == 0)
			{
				m_pDxLibMidway->ShiftImage();
			}
			else
			{
				m_pDxLibMidway->MoveViewPoint(iX, iY);
			}
		}
	}

	m_bLeftDowned = false;

	return 0;
}
/*WM_MBUTTONUP*/
LRESULT CMainWindow::OnMButtonUp(WPARAM wParam, LPARAM lParam)
{
	WORD usKey = LOWORD(wParam);

	if (usKey == 0)
	{
		if (m_pDxLibMidway != nullptr)
		{
			m_pDxLibMidway->ResetScale();
		}
	}

	if (usKey == MK_RBUTTON)
	{
		SwitchWindowMode();
	}

	return 0;
}
/*操作欄作成*/
void CMainWindow::InitialiseMenuBar()
{
	HMENU hMenuFile = nullptr;
	HMENU hMenuAudio = nullptr;
	HMENU hMenuBar = nullptr;
	BOOL iRet = FALSE;

	if (m_hMenuBar != nullptr)return;

	hMenuFile = ::CreateMenu();
	if (hMenuFile == nullptr)goto failed;

	iRet = ::AppendMenuA(hMenuFile, MF_STRING, Menu::kSelectFile, "Select file");
	if (iRet == 0)goto failed;

	hMenuAudio = ::CreateMenu();
	if (hMenuAudio == nullptr)goto failed;

	iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kAudioSetting, "Setting");
	if (iRet == 0)goto failed;

	hMenuBar = ::CreateMenu();
	if (hMenuBar == nullptr) goto failed;

	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuFile), "File");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuAudio), "Audio");
	if (iRet == 0)goto failed;

	iRet = ::SetMenu(m_hWnd, hMenuBar);
	if (iRet == 0)goto failed;

	m_hMenuBar = hMenuBar;

	return;

failed:
	std::wstring wstrMessage = L"Failed to create menu; code: " + std::to_wstring(::GetLastError());
	::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
	if (hMenuFile != nullptr)
	{
		::DestroyMenu(hMenuFile);
	}
	if (hMenuAudio != nullptr)
	{
		::DestroyMenu(hMenuAudio);
	}
	if (hMenuBar != nullptr)
	{
		::DestroyMenu(hMenuBar);
	}
}
/*台本選択*/
void CMainWindow::MenuOnSelectFile()
{
	std::wstring wstrPickedFilePath = win_dialogue::SelectOpenFile(L"Text file", L"*.txt;", nullptr, m_hWnd);
	if (wstrPickedFilePath.empty())return;

	ClearTextFilePathsInfo();
	win_filesystem::GetFilePathListAndIndex(wstrPickedFilePath, L".txt", m_textFiles, &m_nTextFilesIndex);
	if (!m_textFiles.empty())
	{
		SetupScenario(m_textFiles.at(m_nTextFilesIndex).c_str());
	}
}
/*音量・再生速度変更*/
void CMainWindow::MenuOnAudioSetting()
{
	CMediaSettingDialogue sMediaSettingDialogue;
	sMediaSettingDialogue.Open(m_hInstance, m_hWnd, m_pMfMediaPlayer, L"Audio");
}
/*次の台本に移動*/
void CMainWindow::KeyUpOnNextFile()
{
	if (m_textFiles.empty())return;

	++m_nTextFilesIndex;
	if (m_nTextFilesIndex >= m_textFiles.size())m_nTextFilesIndex = 0;
	SetupScenario(m_textFiles.at(m_nTextFilesIndex).c_str());
}
/*前の台本に移動*/
void CMainWindow::KeyUpOnForeFile()
{
	if (m_textFiles.empty())return;

	--m_nTextFilesIndex;
	if (m_nTextFilesIndex >= m_textFiles.size())m_nTextFilesIndex = m_textFiles.size() - 1;
	SetupScenario(m_textFiles.at(m_nTextFilesIndex).c_str());
}
/*標題変更*/
void CMainWindow::ChangeWindowTitle(const wchar_t* pwzTitle)
{
	std::wstring wstr;
	if (pwzTitle != nullptr)
	{
		std::wstring wstrTitle = pwzTitle;
		size_t nPos = wstrTitle.find_last_of(L"\\/");
		wstr = nPos == std::wstring::npos ? wstrTitle : wstrTitle.substr(nPos + 1);
	}

	::SetWindowTextW(m_hWnd, wstr.c_str());
}
/*表示形式変更*/
void CMainWindow::SwitchWindowMode()
{
	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

	m_bBarHidden ^= true;

	if (m_bBarHidden)
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

	if (m_pDxLibMidway != nullptr)
	{
		m_pDxLibMidway->OnStyleChange();
	}
}
/*寸劇構築*/
bool CMainWindow::SetupScenario(const wchar_t* pwzFilePath)
{
	if (pwzFilePath == nullptr)return false;

	std::vector<adv::TextDatum> textData;
	std::vector<adv::ImageDatum> imageData;
	bool bRet = fkg::LoadScenarioFile(pwzFilePath, textData, imageData);
	if (!bRet)
	{
		::MessageBoxW(m_hWnd, std::wstring(L"Failed to load file: ").append(pwzFilePath).c_str(), L"Load error", MB_ICONERROR);
	}

	if (m_pDxLibMidway != nullptr)
	{
		bRet = m_pDxLibMidway->SetResources(imageData);
		if (bRet)
		{
			ClearTextData();
			m_textData = textData;
			UpdateText();
		}
	}

	return bRet;
}
/*台本経路情報消去*/
void CMainWindow::ClearTextFilePathsInfo()
{
	m_textFiles.clear();
	m_nTextFilesIndex = 0;
}
/*文章情報消去*/
void CMainWindow::ClearTextData()
{
	m_textData.clear();
	m_nTextIndex = 0;
}
/*描画間隔更新*/
void CMainWindow::UpdateDrawingInterval()
{
	DEVMODE sDevMode{};
	::EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &sDevMode);
	m_fDelta = 1 / static_cast<float>(sDevMode.dmDisplayFrequency);
}
/*webp対応確認*/
void CMainWindow::CheckWebpSupport()
{
	HMODULE hModule = ::GetModuleHandle(L"ntdll.dll");
	if (hModule == nullptr)return;

	NTSTATUS(*pRtlGetVersion)(PRTL_OSVERSIONINFOW) = nullptr;
	pRtlGetVersion = reinterpret_cast<NTSTATUS(*)(PRTL_OSVERSIONINFOW)>(::GetProcAddress(hModule, "RtlGetVersion"));
	if (pRtlGetVersion == nullptr)return;

	RTL_OSVERSIONINFOW sRtlOsVersionInfoEx{};
	sRtlOsVersionInfoEx.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);

	BOOL iRet = pRtlGetVersion(&sRtlOsVersionInfoEx);
	if (iRet == STATUS_SUCCESS)
	{
		m_bWebpSupported = sRtlOsVersionInfoEx.dwMajorVersion >= 10 && sRtlOsVersionInfoEx.dwBuildNumber >= 17763;
	}
}
/*文章送り・戻し*/
void CMainWindow::ShiftText(bool bForward)
{
	if (bForward)
	{
		++m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = 0;
	}
	else
	{
		--m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = m_textData.size() - 1;
	}
	UpdateText();
}
/*文章更新*/
void CMainWindow::UpdateText()
{
	if (m_nTextIndex < m_textData.size())
	{
		const adv::TextDatum& t = m_textData.at(m_nTextIndex);
		if (!t.wstrVoicePath.empty())
		{
			if (m_pMfMediaPlayer != nullptr)
			{
				m_pMfMediaPlayer->Play(t.wstrVoicePath.c_str());
			}
		}

		m_textClock.Restart();

		if (m_pDxLibMidway != nullptr)
		{
			std::wstring wstr = t.wstrText;
			if (!t.wstrText.empty() && t.wstrText.back() != L'\n') wstr += L'\n';
			wstr += std::to_wstring(m_nTextIndex + 1) + L"/" + std::to_wstring(m_textData.size());
			m_pDxLibMidway->SetMessageToDraw(wstr);
		}
	}
}

void CMainWindow::CheckTimer()
{
	constexpr float fAutoPlayInterval = 2000.f;
	float fMilliSecond = m_textClock.GetElapsedTime();
	if (m_pMfMediaPlayer != nullptr)
	{
		if (m_pMfMediaPlayer->IsEnded() && fMilliSecond > fAutoPlayInterval)
		{
			if (m_nTextIndex < m_textData.size() - 1)ShiftText(true);
		}
	}
}

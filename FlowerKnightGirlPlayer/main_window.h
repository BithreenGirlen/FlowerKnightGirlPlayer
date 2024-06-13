#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

#include <string>
#include <vector>

#include "dxlib_midway.h"
#include "mf_media_player.h"

class CMainWindow
{
public:
	CMainWindow();
	~CMainWindow();
	bool Create(HINSTANCE hInstance, const wchar_t* pwzWindowName);
	int MessageLoop();
	HWND GetHwnd()const { return m_hWnd;}
private:
	const wchar_t* m_swzClassName = L"FKG player window";
	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd);
	LRESULT OnDestroy();
	LRESULT OnClose();
	LRESULT OnPaint();
	LRESULT OnSize();
	LRESULT OnKeyUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(WPARAM wParam);
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMButtonUp(WPARAM wParam, LPARAM lParam);

	enum Menu
	{
		kSelectFile = 1, kCheckCommentary,
		kAudioSetting
	};
	enum MenuBar
	{
		kFolder, kAduio,
	};
	enum EventMessage
	{
		kAudioPlayer = WM_USER + 1
	};
	enum Timer
	{
		kText = 1,
	};
	POINT m_CursorPos{};
	bool m_bSpeedHavingChanged = false;
	bool m_bLeftDowned = false;

	HMENU m_hMenuBar = nullptr;
	bool m_bBarHidden = false;
	bool m_bTransparent = false;

	bool m_bWebpSupported = false;

	std::vector<std::wstring> m_textFiles;
	size_t m_nTextFilesIndex = 0;

	float m_fDelta = 1 / 60.f;

	void InitialiseMenuBar();

	void MenuOnSelectFile();
	void MenuOnAudioSetting();

	void KeyUpOnNextFile();
	void KeyUpOnForeFile();

	void ChangeWindowTitle(const wchar_t* pwzTitle);
	void SwitchWindowMode();

	bool SetupScenario(const wchar_t* pwzFilePath);
	void ClearTextFilePathsInfo();
	void ClearTextData();

	void UpdateDrawingInterval();
	void CheckWebpSupport();

	CDxLibMidway* m_pDxLibMidway = nullptr;
	CMfMediaPlayer* m_pMfMediaPlayer = nullptr;

	void UpdateScreen();

	std::wstring m_wstrTextFontName = L"游明朝 Demibold";

	std::vector<adv::TextDatum> m_textData;
	size_t m_nTextIndex = 0;

	void ShiftText(bool bForward);
	void UpdateText();
	void OnAudioPlayerEvent(unsigned long ulEvent);
	void AutoTexting();

};

#endif //MAIN_WINDOW_H_
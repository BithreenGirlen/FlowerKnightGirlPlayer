#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

#include <string>
#include <vector>
#include <memory>

#include "dxlib_init.h"
#include "fkg_scene_player.h"

class CMainWindow
{
public:
	CMainWindow();
	~CMainWindow();

	bool Create(HINSTANCE hInstance, const wchar_t* pwzWindowName = nullptr, HICON hIcon = nullptr);
	int MessageLoop();

	HWND GetHwnd()const { return m_hWnd; }
private:
	const wchar_t* m_pwzClassName = L"FKG player window";
	const wchar_t* m_pwzDefaultWindowName = L"FKG player";

	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd);
	LRESULT OnDestroy();
	LRESULT OnClose();
	LRESULT OnPaint();
	LRESULT OnSize(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnRButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMButtonUp(WPARAM wParam, LPARAM lParam);

	struct Menu abstract final
	{
		enum
		{
			kOpenFile = 1,
			kAudioSetting, kFilterNone, kFilterAvir, kFilterLanczos, kFilterCubic,
			kSyncImage,
			kLabelStartIndex
		};
	};
	struct MenuBar abstract final
	{
		enum
		{
			kFile, kAudio, kImage
		};
	};

	HMENU m_hMenuBar = nullptr;
	bool m_isFramelessWindow = false;

	POINT m_lastCursorPos{};
	bool m_wasLeftCombinated = false;
	bool m_wasLeftPressed = false;
	bool m_hasLeftBeenDragged = false;
	bool m_wasRightCombinated = false;

	void InitialiseMenuBar();

	void MenuOnOpenFile();

	void KeyOnNextFile();
	void KeyOnForeFile();

	void MenuOnAudioSetting();

	void MenuOnSyncImage();

	std::unique_ptr<SDxLibInit> m_pDxLibInit;
	std::unique_ptr<CFkgScenePlayer> m_pScenePlayer;

	std::vector<std::wstring> m_scriptFilePaths;
	size_t m_nScriptFilePathIndex = 0;

	bool SetupScenario(const std::wstring& wstrFolderPath);
	void JumpToLabel(size_t nIndex);

	void ToggleWindowBorderStyle();
	bool SetMenuCheckState(unsigned int uiMenuIndex, unsigned int uiItemIndex, bool checked) const;
	bool UpdateFilterState(unsigned int uiMenuIndexToCheck) const;

	void ResizeWindow();
};

#endif //MAIN_WINDOW_H_
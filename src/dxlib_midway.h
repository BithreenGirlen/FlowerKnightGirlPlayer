#ifndef DXLIB_MIDWAY_H_
#define DXLIB_MIDWAY_H_

#include <Windows.h>

#include <string>
#include <vector>

#include "adv.h"
#include "dxlib_text_writer.h"
#include "dxlib_still_image_drawer.h"
#include "dxlib_spine_player.h"

class CDxLibMidway
{
public:
	CDxLibMidway(HWND hRenderWnd, bool bWebpSupported);
	~CDxLibMidway();

	bool SetFont(const wchar_t* pwzFontFileName, int iFontSize, bool bBold, bool bItalic);
	bool SetResources(const std::vector<adv::ImageDatum>& imageData);
	void SetMessageToDraw(const std::wstring& wstrText);

	void Redraw(float fDelta);

	void SwitchMessageVisibility();
	void SwitchTextColour();

	void RescaleSize(bool bUpscale);
	void RescaleTime(bool bHasten);
	void ResetScale();
	void ShiftImage();
	void MoveViewPoint(int iX, int iY);

	void OnStyleChange();

	bool IsPlayReady() const { return m_bReady; }
	bool IsStillMode() const { return m_bStillImageMode; }
private:
	HWND m_hRenderWnd = nullptr;

	bool m_bWebpSupported = false;
	bool m_bStillImageMode = true;
	bool m_bReady = false;
	bool m_bTextHidden = false;

	std::wstring m_wstrMessage;

	CDxLibTextWriter m_DxLibTextWriter;
	CDxLibStillImageDrawer m_DxLibStillImageDrawer;
	CDxLibSpinePlayer m_DxLibSpinePlayer;

	void RequestRedraw();
};
#endif // !DXLIB_MIDWAY_H_

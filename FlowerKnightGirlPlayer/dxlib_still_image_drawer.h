#ifndef DXLIB_STILL_IMAGE_DRAWER_H_
#define DXLIB_STILL_IMAGE_DRAWER_H_

#include <Windows.h>

#include <string>
#include <vector>

#include "image_info.h"

class CDxLibStillImageDrawer
{
public:
	CDxLibStillImageDrawer();
	~CDxLibStillImageDrawer();
	bool SetImageFromFilePath(const std::vector<std::wstring> &filePaths, HWND hRenderWnd);
	bool SetImageFromMemory(const std::vector<ImageInfo> &imageInfoArray, HWND hRenderWnd);

	bool Draw();

	void Rescale(bool bfUpscale);
	void ResetScale();
	void SetOffset(int iX, int iY);
	void ShiftImage();

	void OnStyleChanged();
private:
	HWND m_hRenderWnd = nullptr;

	enum Constants{kBaseWidth = 960, kBaseHeight = 640};
	int m_iBaseWidth = Constants::kBaseWidth;
	int m_iBaseHeight = Constants::kBaseWidth;

	std::vector<int> m_imageHandles;
	size_t m_nImageIndex = 0;

	double m_dbScale = 1.0;
	int m_iOffsetX = 0;
	int m_iOffsetY = 0;

	bool m_bBarHidden = false;

	void Clear();
	void Update();
	void WorkOutDefaultSize();
	void AdjustOffset();
	void ResizeWindow();
	void ResizeBuffer();
};

#endif // !DXLIB_STILL_IMAGE_DRAWER_H_

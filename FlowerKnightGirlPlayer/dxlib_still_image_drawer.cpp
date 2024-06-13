

#include "dxlib_still_image_drawer.h"
#include "win32_window.h"
#include "dxlib_win32.h"

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>


CDxLibStillImageDrawer::CDxLibStillImageDrawer()
{

}

CDxLibStillImageDrawer::~CDxLibStillImageDrawer()
{
	Clear();
}
/*ファイル取り込み*/
bool CDxLibStillImageDrawer::SetImageFromFilePath(const std::vector<std::wstring> &filePaths, HWND hRenderWnd)
{
	Clear();
	ResetScale();

	for (const auto& filePath : filePaths)
	{
		int iHandle = DxLib::LoadGraph(filePath.c_str());
		if (iHandle != -1)
		{
			m_imageHandles.push_back(iHandle);
		}
	}

	if (!m_imageHandles.empty())
	{
		m_hRenderWnd = hRenderWnd;
	}
	WorkOutDefaultSize();

	return !m_imageHandles.empty();
}
/*メモリ取り込み*/
bool CDxLibStillImageDrawer::SetImageFromMemory(const std::vector<ImageInfo>& imageInfoArray, HWND hRenderWnd)
{
	Clear();
	ResetScale();

	for (const auto& s : imageInfoArray)
	{
		std::vector<unsigned char> rgbArray;

		rgbArray.reserve(s.pixels.size() * 3 / 4);
		/*RGBA => RGB; A情報は破棄*/
		for (size_t i = 0; i < s.pixels.size(); i += 4)
		{
			rgbArray.push_back(s.pixels.at(i));
			rgbArray.push_back(s.pixels.at(i + 1));
			rgbArray.push_back(s.pixels.at(i + 2));
		}
		int iPitch = s.iStride * 3 / 4;
		int iHandle = DxLib::CreateGraph(s.uiWidth, s.uiHeight, iPitch, rgbArray.data());
		if (iHandle != -1)
		{
			m_imageHandles.push_back(iHandle);
		}
	}

	if (!m_imageHandles.empty())
	{
		m_hRenderWnd = hRenderWnd;
	}
	WorkOutDefaultSize();

	return !m_imageHandles.empty();
}

/*描画*/
bool CDxLibStillImageDrawer::Draw()
{
	if (m_imageHandles.empty() || m_nImageIndex > m_imageHandles.size() - 1)return false;

	const int iHandle = m_imageHandles.at(m_nImageIndex);

	int iWidth = 0;
	int iHeight = 0;
	int iRet = DxLib::GetGraphSize(iHandle, &iWidth, &iHeight);
	if (iRet == -1)return false;

	iRet = ::DxLib::DrawRectExtendGraph
	(
		0, 0,
		static_cast<int>(iWidth * m_dbScale), static_cast<int>(iHeight * m_dbScale),
		m_iOffsetX, m_iOffsetY,
		iWidth, iHeight,
		iHandle, FALSE
	);
	return false;
}
/*拡縮*/
void CDxLibStillImageDrawer::Rescale(bool bfUpscale)
{
	constexpr double kdbScalePortion = 0.05;
	constexpr double kdbMinScale = 0.5;
	if (bfUpscale)
	{
		m_dbScale += kdbScalePortion;
	}
	else
	{
		m_dbScale -= kdbScalePortion;
	}

	if (m_dbScale < kdbMinScale)m_dbScale = kdbMinScale;
	ResizeWindow();
}
/*尺度初期化*/
void CDxLibStillImageDrawer::ResetScale()
{
	m_dbScale = 1.0;
	m_iOffsetX = 0;
	m_iOffsetY = 0;
	ResizeWindow();
}
/*原点位置移動*/
void CDxLibStillImageDrawer::SetOffset(int iX, int iY)
{
	m_iOffsetX += iX;
	m_iOffsetY += iY;
	AdjustOffset();
	Update();
}
/*次画像*/
void CDxLibStillImageDrawer::ShiftImage()
{
	++m_nImageIndex;
	if (m_nImageIndex > m_imageHandles.size() - 1)m_nImageIndex = 0;
	Update();
}
/*表示形式変更通知*/
void CDxLibStillImageDrawer::OnStyleChanged()
{
	ResizeWindow();
}
/*消去*/
void CDxLibStillImageDrawer::Clear()
{
	for (const auto imageHandle : m_imageHandles)
	{
		if (imageHandle > 0)
		{
			DxLib::DeleteGraph(imageHandle);
		}
	}
	m_nImageIndex = 0;
	m_imageHandles.clear();
}
/*画面更新*/
void CDxLibStillImageDrawer::Update()
{
	if (m_hRenderWnd != nullptr)
	{
		::InvalidateRect(m_hRenderWnd, NULL, TRUE);
	}
}
/*標準寸法算出*/
void CDxLibStillImageDrawer::WorkOutDefaultSize()
{
	if (m_imageHandles.empty())return;

	int iWidth = 0;
	int iHeight = 0;
	int iRet = DxLib::GetGraphSize(m_imageHandles.at(0), &iWidth, &iHeight);
	if (iRet != -1)
	{
		m_iBaseWidth = iWidth;
		m_iBaseHeight = iHeight;
		ResizeWindow();
	}
}
/*原点位置調整*/
void CDxLibStillImageDrawer::AdjustOffset()
{
	if (m_hRenderWnd != nullptr)
	{
		int iImageWidth = static_cast<int>(m_iBaseWidth * m_dbScale);
		int iImageHeight = static_cast<int>(m_iBaseHeight * m_dbScale);

		RECT rc;
		::GetClientRect(m_hRenderWnd, &rc);

		int iClientWidth = static_cast<int>((rc.right - rc.left));
		int iClientHeight = static_cast<int>((rc.bottom - rc.top));

		int iXOffsetMax = iImageWidth > iClientWidth ? static_cast<int>(::floor((iImageWidth - iClientWidth) / m_dbScale)) : 0;
		int iYOffsetMax = iImageHeight > iClientHeight ? static_cast<int>(::floor((iImageHeight - iClientHeight) / m_dbScale)) : 0;

		if (m_iOffsetX < 0)m_iOffsetX = 0;
		if (m_iOffsetY < 0)m_iOffsetY = 0;

		if (m_iOffsetX > iXOffsetMax)m_iOffsetX = iXOffsetMax;
		if (m_iOffsetY > iYOffsetMax)m_iOffsetY = iYOffsetMax;
	}

}

void CDxLibStillImageDrawer::ResizeWindow()
{
	if (m_imageHandles.empty())return;

	win32_window::ResizeWindow(m_hRenderWnd, m_iBaseWidth, m_iBaseHeight, static_cast<float>(m_dbScale));
	ResizeBuffer();
	AdjustOffset();
	Update();
}

void CDxLibStillImageDrawer::ResizeBuffer()
{
	dxlib_win32::ResizeBuffer(m_hRenderWnd);
}

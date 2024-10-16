﻿

#include "dxlib_still_image_drawer.h"

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
bool CDxLibStillImageDrawer::SetImageFromMemory(const std::vector<SImageFrame>& imageFrames, HWND hRenderWnd)
{
	Clear();
	ResetScale();

	for (const auto& s : imageFrames)
	{
		std::vector<unsigned char> rgbArray;
		rgbArray.resize(s.pixels.size() * 3 / 4);
		/*RGBA => RGB; A情報は破棄*/
		const unsigned char* pSrc = s.pixels.data();
		unsigned char* pDst = rgbArray.data();
		for (size_t i = 0; i < s.pixels.size() - 3; i += 4)
		{
			*pDst++ = *pSrc++;
			*pDst++ = *pSrc++;
			*pDst++ = *pSrc++;
			pSrc++;
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
	if (m_nImageIndex >= m_imageHandles.size())return false;

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
}
/*次画像*/
void CDxLibStillImageDrawer::ShiftImage()
{
	++m_nImageIndex;
	if (m_nImageIndex > m_imageHandles.size() - 1)m_nImageIndex = 0;
}
/*表示形式変更通知*/
void CDxLibStillImageDrawer::OnStyleChanged()
{
	ResizeWindow();
}
/*消去*/
void CDxLibStillImageDrawer::Clear()
{
	for (const auto &imageHandle : m_imageHandles)
	{
		if (imageHandle > 0)
		{
			DxLib::DeleteGraph(imageHandle);
		}
	}
	m_nImageIndex = 0;
	m_imageHandles.clear();
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

	if (m_hRenderWnd != nullptr)
	{
		RECT rect;
		::GetWindowRect(m_hRenderWnd, &rect);
		int iX = static_cast<int>(m_iBaseWidth * m_dbScale);
		int iY = static_cast<int>(m_iBaseHeight * m_dbScale);

		rect.right = iX + rect.left;
		rect.bottom = iY + rect.top;
		LONG lStyle = ::GetWindowLong(m_hRenderWnd, GWL_STYLE);
		const auto HasWindowMenu = [&lStyle]()
			-> bool
			{
				return !((lStyle & WS_CAPTION) && (lStyle & WS_SYSMENU));
			};
		::AdjustWindowRect(&rect, lStyle, HasWindowMenu() ? FALSE : TRUE);
		::SetWindowPos(m_hRenderWnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);

		ResizeBuffer();
	}

	AdjustOffset();
}

void CDxLibStillImageDrawer::ResizeBuffer()
{
	if (m_hRenderWnd != nullptr)
	{
		RECT rc;
		::GetClientRect(m_hRenderWnd, &rc);

		int iClientWidth = rc.right - rc.left;
		int iClientHeight = rc.bottom - rc.top;

		int iDesktopWidth = ::GetSystemMetrics(SM_CXSCREEN);
		int iDesktopHeight = ::GetSystemMetrics(SM_CYSCREEN);

		DxLib::SetGraphMode
		(
			iClientWidth < iDesktopWidth ? iClientWidth : iDesktopWidth,
			iClientHeight < iDesktopHeight ? iClientHeight : iDesktopHeight,
			32
		);
	}
}

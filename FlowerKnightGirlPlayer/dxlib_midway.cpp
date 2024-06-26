﻿/*DxLib中継部*/

#include "dxlib_midway.h"
#include "win_text.h"
#include "win_image.h"

CDxLibMidway::CDxLibMidway(HWND hRenderWnd, bool bWebpSupported)
	:m_hRenderWnd(hRenderWnd), m_bWebpSupported(bWebpSupported)
{
	m_DxLibSpinePlayer.SetRenderWindow(m_hRenderWnd);
}

CDxLibMidway::~CDxLibMidway()
{

}
/*字体設定*/
bool CDxLibMidway::SetFont(const wchar_t* pwzFontFileName, int iFontSize, bool bBold, bool bItalic)
{
	return m_DxLibTextWriter.SetFont(pwzFontFileName, iFontSize, bBold, bItalic);
}
/*描画素材設定*/
bool CDxLibMidway::SetResources(const std::vector<adv::ImageDatum>& imageData)
{
	const auto HasSpine = [&imageData]()
		-> bool
		{
			for (const auto& imageDatum : imageData)
			{
				if (imageDatum.bSpine)return true;
			}
			return false;
		};

	m_bStillImageMode = !(m_bWebpSupported && HasSpine());

	if (m_bStillImageMode)
	{
		/*
		* 元々の画像の解像度が現代の水準からすると低いが、DxLibは基本的に三次元向けのライブラリであり、
		* 二次元に対する補間法は充実していないので、予めWICで双三次補間してから画素情報を渡す。
		*/
		//std::vector<std::wstring> imageFilePaths;

		//for (const auto& imageDatum : imageData)
		//{
		//	if (!imageDatum.bSpine)
		//	{
		//		imageFilePaths.emplace_back(win_text::WidenANSI(imageDatum.strFilePath));
		//	}
		//}
		//m_bReady = m_DxLibStillImageDrawer.SetImageFromFilePath(imageFilePaths, m_hRenderWnd);

		std::vector<ImageInfo> imageInfoArray;
		for (const auto& imageDatum : imageData)
		{
			ImageInfo s{};
			if (!imageDatum.bSpine)
			{
				const std::wstring& wstrFilePath = win_text::WidenANSI(imageDatum.strFilePath);
				bool bRet = win_image::LoadImageToMemory(wstrFilePath.c_str(), &s, 2.0f);
				if (bRet)
				{
					imageInfoArray.push_back(s);
				}
			}
		}
		m_bReady = m_DxLibStillImageDrawer.SetImageFromMemory(imageInfoArray, m_hRenderWnd);

	}
	else
	{
		std::vector<std::string> atlasPaths;
		std::vector<std::string> skelPaths;

		for (const auto& imageDatum : imageData)
		{
			if (imageDatum.bSpine)
			{
				std::string strAtlasPath = imageDatum.strFilePath + ".atlas";
				std::string strSkelPath = imageDatum.strFilePath + ".json";

				atlasPaths.push_back(strAtlasPath);
				skelPaths.push_back(strSkelPath);
			}
		}
		m_bReady = m_DxLibSpinePlayer.SetSpineFromFile(atlasPaths, skelPaths, false);
		//if (m_bReady)
		//{
		//	const std::vector<std::string> leaveOutList{ "WhiteSpot" };
		//	m_DxLibSpinePlayer.SetSlotsToExclude(leaveOutList);
		//}
	}

	return m_bReady;
}
/*表示文字列格納*/
void CDxLibMidway::SetMessageToDraw(const std::wstring& wstrText)
{
	m_wstrMessage = wstrText;
}
/*再描画*/
void CDxLibMidway::Redraw(float fDelta)
{
	if (m_bReady)
	{
		DxLib::ClearDrawScreen();

		if (m_bStillImageMode)
		{
			m_DxLibStillImageDrawer.Draw();
		}
		else
		{
			m_DxLibSpinePlayer.Redraw(fDelta);
		}

		if (!m_wstrMessage.empty() && !m_bTextHidden)
		{
			DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
			m_DxLibTextWriter.Draw(m_wstrMessage.c_str(), static_cast<unsigned long>(m_wstrMessage.size()));
		}

		DxLib::ScreenFlip();
	}
}
/*文章表示・非表示切り替え*/
void CDxLibMidway::SwitchMessageVisibility()
{
	m_bTextHidden ^= true;
}
/*文字色切り替え*/
void CDxLibMidway::SwitchTextColour()
{
	m_DxLibTextWriter.SwitchTextColour();
}
/*拡縮変更*/
void CDxLibMidway::RescaleSize(bool bUpscale)
{
	if (m_bStillImageMode)
	{
		m_DxLibStillImageDrawer.Rescale(bUpscale);
	}
	else
	{
		m_DxLibSpinePlayer.RescaleSkeleton(bUpscale);
	}
}
/*時間尺度変更*/
void CDxLibMidway::RescaleTime(bool bHasten)
{
	if (!m_bStillImageMode)
	{
		m_DxLibSpinePlayer.RescaleTime(bHasten);
	}
}
/*速度・尺度・視点初期化*/
void CDxLibMidway::ResetScale()
{
	if (m_bStillImageMode)
	{
		m_DxLibStillImageDrawer.ResetScale();
	}
	else
	{
		m_DxLibSpinePlayer.ResetScale();
	}
}
/*画像・動作移行*/
void CDxLibMidway::ShiftImage()
{
	if (m_bStillImageMode)
	{
		m_DxLibStillImageDrawer.ShiftImage();
	}
	else
	{
		m_DxLibSpinePlayer.ShiftAnimation();
	}
}
/*視点移動*/
void CDxLibMidway::MoveViewPoint(int iX, int iY)
{
	if (m_bStillImageMode)
	{
		m_DxLibStillImageDrawer.SetOffset(iX, iY);
	}
	else
	{
		m_DxLibSpinePlayer.MoveViewPoint(iX, iY);
	}
}
/*表示形式変更通知*/
void CDxLibMidway::OnStyleChange()
{
	if (m_bStillImageMode)
	{
		m_DxLibStillImageDrawer.OnStyleChanged();
	}
	else
	{
		m_DxLibSpinePlayer.OnStyleChanged();
	}
}

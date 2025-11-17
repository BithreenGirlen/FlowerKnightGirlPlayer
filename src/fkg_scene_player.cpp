

/* Rely on WIC for Bicubic interpolation */
#include <atlbase.h>
#include <wincodec.h>

#include "fkg_scene_player.h"

#include "fkg.h"
#include "win_text.h"
#include "win_support.h"

/* AVIR interpolation */
#include "deps/avir-3.1/avir.h"
/* Lanczos interpolation */
#include "deps/avir-3.1/lancir.h"

#pragma comment (lib, "Windowscodecs.lib")

static int LoadRemovingAlphaFromRGBA(const int srcWidth, const int srcHeight, const int srcPitch, const unsigned char* const srcPixels)
{
	size_t nSrcSize = static_cast<size_t>(srcPitch * srcHeight);
	std::vector<unsigned char> rgbArray(nSrcSize * 3 / 4);

	/* RGBA => RGB; A相は破棄 */
	const unsigned char* pSrc = srcPixels;
	unsigned char* pDst = rgbArray.data();
	for (size_t i = 0; i < nSrcSize - 3; i += 4)
	{
		*pDst++ = *pSrc++;
		*pDst++ = *pSrc++;
		*pDst++ = *pSrc++;
		pSrc++;
	}
	int iPitch = srcPitch * 3 / 4;
	int iHandle = DxLib::CreateGraph(srcWidth, srcHeight, iPitch, rgbArray.data());

	return iHandle;
}

namespace wic
{
	static bool ResizeImage(
		const unsigned char* const srcPixels, const int srcWidth, const int srcHeight, const int srcPitch,
		unsigned char* const dstPixels, const int dstWidth, const int dstHeight, const int dstPitch,
		const int channel)
	{
		if (channel != 3 && channel != 4)return false;

		CComPtr<IWICImagingFactory> pWicImageFactory;
		HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWicImageFactory));
		if (FAILED(hr))return false;

		REFWICPixelFormatGUID pixelFormat = channel == 4 ? GUID_WICPixelFormat32bpp3ChannelsAlpha : GUID_WICPixelFormat24bpp3Channels;
		CComPtr<IWICBitmap> pSrcWicBitmap;
		hr = pWicImageFactory->CreateBitmapFromMemory(srcWidth, srcHeight, pixelFormat, srcPitch, srcPitch * srcHeight, const_cast<BYTE*>(srcPixels), &pSrcWicBitmap);
		if (FAILED(hr))return false;

		CComPtr<IWICBitmapScaler> pWicBmpScaler;
		hr = pWicImageFactory->CreateBitmapScaler(&pWicBmpScaler);
		if (FAILED(hr))return false;

		hr = pWicBmpScaler->Initialize(pSrcWicBitmap, dstWidth, dstHeight, WICBitmapInterpolationMode::WICBitmapInterpolationModeCubic);
		if (FAILED(hr))return false;

		CComPtr<IWICBitmap> pDstWicBitmap;
		hr = pWicImageFactory->CreateBitmapFromSource(pWicBmpScaler, WICBitmapCacheOnDemand, &pDstWicBitmap);
		if (FAILED(hr))return false;

		UINT uiDstWidth = 0, uiDstHeight = 0;
		hr = pWicBmpScaler->GetSize(&uiDstWidth, &uiDstHeight);
		if (FAILED(hr))return false;

		CComPtr<IWICBitmapLock> pWicBitmapLock;
		WICRect wicRect{ 0, 0, dstWidth, dstHeight };
		hr = pDstWicBitmap->Lock(&wicRect, WICBitmapLockRead, &pWicBitmapLock);
		if (FAILED(hr))return false;

		UINT dstStride = 0;
		hr = pWicBitmapLock->GetStride(&dstStride);
		if (FAILED(hr))return false;

		/* 元々のα相の有無・形式に関わらず、BGRAとなる。 */
		if (dstPitch == dstStride)
		{
			CComPtr<IWICFormatConverter> pWicFormatConverter;
			hr = pWicImageFactory->CreateFormatConverter(&pWicFormatConverter);
			if (FAILED(hr))return false;

			/* BGRA => RGBA */
			hr = pWicFormatConverter->Initialize(pDstWicBitmap, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
			if (FAILED(hr))return false;

			hr = pWicFormatConverter->CopyPixels(nullptr, dstStride, dstStride * uiDstHeight, dstPixels);
		}
		else
		{
			CComPtr<IWICFormatConverter> pWicFormatConverter;
			hr = pWicImageFactory->CreateFormatConverter(&pWicFormatConverter);
			if (FAILED(hr))return false;

			/* BGRA => RGB */
			hr = pWicFormatConverter->Initialize(pDstWicBitmap, GUID_WICPixelFormat24bppRGB, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
			if (FAILED(hr))return false;

			hr = pWicFormatConverter->CopyPixels(nullptr, dstPitch, dstPitch * uiDstHeight, dstPixels);
		}

		return true;
	}
}

CFkgScenePlayer::CFkgScenePlayer()
{
	m_dxLibTextWriter.SetFont(L"游明朝", 28, true, true);

	m_pAudioPlayer = std::make_unique<CMfMediaPlayer>();

	m_isWebpSupported = win_support::IsWebpSupported();
}

CFkgScenePlayer::~CFkgScenePlayer()
{

}
/*台本読み込み*/
bool CFkgScenePlayer::ReadScenario(const std::wstring& wstrFolderPath)
{
	ClearScenarioData();

	std::vector<adv::ImageFileDatum> imageFileData;
	fkg::LoadScenarioFile(wstrFolderPath, m_textData, imageFileData, m_sceneData, m_labelData);

	for (const auto& imageFileDatum : imageFileData)
	{
		if (!imageFileDatum.bAnimation)
		{
			if (m_filter == FilterOnLoading::None)
			{
				/* 補間無しでGPUに転送 */
				int iWidth = 0;
				int iHeight = 0;
				float fScale = 1.f;
				if (DxLib::GetImageSize_File(imageFileDatum.wstrFilePath.c_str(), &iWidth, &iHeight) != -1)
				{
					fScale = static_cast<float>(kDefaultHeight) / iHeight;
				}

				DxLibImageHandle dxLibImageHandle(DxLib::LoadGraph(imageFileDatum.wstrFilePath.c_str()));
				if (dxLibImageHandle.Get() != -1)
				{
					m_imageHandles.push_back(std::move(dxLibImageHandle));

					SImageDatum imageDatum;
					imageDatum.bAnimation = false;
					imageDatum.stillParams.usIndex = static_cast<unsigned short>(m_imageHandles.size() - 1);
					imageDatum.stillParams.fScale = fScale;

					m_imageData.push_back(std::move(imageDatum));
				}
			}
			else
			{
				/* 一旦CPU上で拡大処理を施した上でGPUに転送 */
				using DxLibSoftwareImageHandle = DxLibHandle<&DxLib::DeleteSoftImage>;
				DxLibSoftwareImageHandle softImageHandle(DxLib::LoadSoftImage(imageFileDatum.wstrFilePath.c_str()));
				if (!softImageHandle.Empty())
				{
					int iWidth = 0, iHeight = 0;
					int iRet = DxLib::GetSoftImageSize(softImageHandle.Get(), &iWidth, &iHeight);
					if (iRet == -1)continue;

					const int iPitch = DxLib::GetPitchSoftImage(softImageHandle.Get());
					const int iChannel = iPitch / iWidth;
					const unsigned char* pPixels = static_cast<const unsigned char*>(DxLib::GetImageAddressSoftImage(softImageHandle.Get()));

					const float fScale = ::ceil(static_cast<float>(kDefaultHeight) / iHeight);
					const int iDstWidth = static_cast<int>(iWidth * fScale);
					const int iDstHeight = static_cast<int>(iHeight * fScale);
					const int iDstPitch = static_cast<int>(iPitch * fScale);
					const size_t dstBufSize = static_cast<size_t>(iDstWidth * iDstHeight * iChannel);
					std::vector<unsigned char> dstBuffer(dstBufSize);

					if (m_filter == FilterOnLoading::Avir)
					{
						const avir::CImageResizerParamsHigh params;
						avir::CImageResizer<> avirImageResizer(8, 0, params);
						avir::CImageResizerVars vars;
						vars.UseSRGBGamma = true;
						avirImageResizer.resizeImage(pPixels, iWidth, iHeight, iPitch, dstBuffer.data(), iDstWidth, iDstHeight, iChannel, 0, &vars);
					}
					else if (m_filter == FilterOnLoading::Lanczos)
					{
						avir::CLancIR lancIr;
						lancIr.resizeImage(pPixels, iWidth, iHeight, iPitch, dstBuffer.data(), iDstWidth, iDstHeight, iDstPitch, iChannel);
					}
					else if (m_filter == FilterOnLoading::Cubic)
					{
						wic::ResizeImage(pPixels, iWidth, iHeight, iPitch, dstBuffer.data(), iDstWidth, iDstHeight, iDstPitch, iChannel);
					}
					softImageHandle.Reset();

					DxLibImageHandle dxLibImageHandle(
						iChannel == 3 ? 
						DxLib::CreateGraph(iDstWidth, iDstHeight, iDstPitch, dstBuffer.data()) : 
						LoadRemovingAlphaFromRGBA(iDstWidth, iDstHeight, iDstPitch, dstBuffer.data()));
					if (!dxLibImageHandle.Empty())
					{
						m_imageHandles.push_back(std::move(dxLibImageHandle));

						SImageDatum imageDatum;
						imageDatum.bAnimation = false;
						imageDatum.stillParams.usIndex = static_cast<unsigned short>(m_imageHandles.size() - 1);
						imageDatum.stillParams.fScale = 1.f;

						m_imageData.push_back(std::move(imageDatum));
					}
				}
			}
		}
		else
		{
			/* webp非対応の場合、静止画のみの構成とする。 */
			if (!m_isWebpSupported)continue;

			if (!imageFileDatum.wstrFilePath.empty())
			{
				/* ファイル情報 */
				if (m_dxLibSpinePlayer.HasSpineBeenLoaded())continue;

				std::vector<std::string> atlasPaths;
				std::vector<std::string> skelPaths;

				std::string strAtlasPath = win_text::NarrowUtf8(imageFileDatum.wstrFilePath) + ".atlas";
				std::string strSkelPath = win_text::NarrowUtf8(imageFileDatum.wstrFilePath) + ".json";

				atlasPaths.push_back(std::move(strAtlasPath));
				skelPaths.push_back(std::move(strSkelPath));

				bool bRet = m_dxLibSpinePlayer.LoadSpineFromFile(atlasPaths, skelPaths, false);
				if (bRet)
				{
					m_dxLibSpinePlayer.PremultiplyAlpha(false);
					m_dxLibSpinePlayer.ForceBlendModeNormal(true);
				}
			}
			else
			{
				/* 動作情報 */
				if (!m_dxLibSpinePlayer.HasSpineBeenLoaded())continue;

				const auto& animationNames = m_dxLibSpinePlayer.GetAnimationNames();
				const auto& iter = std::find(animationNames.begin(), animationNames.end(), imageFileDatum.strData);
				if (iter != animationNames.cend())
				{
					SImageDatum imageDatum;
					imageDatum.bAnimation = true;
					imageDatum.animationParams.usIndex = static_cast<unsigned short>(std::distance(animationNames.begin(), iter));
					m_imageData.push_back(std::move(imageDatum));
				}
			}
		}
	}

	WorkOutDefaultScale();
	ResetScale();

	PrepareScene();

	m_spineClock.Restart();
	m_textClock.Restart();

	return !m_imageData.empty();
}

bool CFkgScenePlayer::HasScenarioData() const
{
	return !m_sceneData.empty();
}

void CFkgScenePlayer::Update()
{
	float fDelta = m_spineClock.GetElapsedTime();
	const auto* p = GetCurrentImageDatum();
	if (p != nullptr)
	{
		if (p->bAnimation)
		{
			m_dxLibSpinePlayer.Update(fDelta);
		}
	}

	CheckTextClock();

	m_spineClock.Restart();
}

void CFkgScenePlayer::Redraw()
{
	DrawCurrentImage();
	DrawFormattedText();
}

void CFkgScenePlayer::GetStillImageSize(unsigned int* uiWidth, unsigned int* uiHeight) const
{
	if (uiWidth != nullptr)*uiWidth = static_cast<unsigned int>(kDefaultWidth * m_fScale);
	if (uiHeight != nullptr)*uiHeight = static_cast<unsigned int>(kDefaultHeight * m_fScale);
}
/*場面移行*/
void CFkgScenePlayer::ShiftScene(bool bForward)
{
	if (m_sceneData.empty())return;

	if (bForward)
	{
		if (++m_nSceneIndex >= m_sceneData.size())
		{
			m_nSceneIndex = 0;
		}
	}
	else
	{
		if (--m_nSceneIndex >= m_sceneData.size())
		{
			m_nSceneIndex = m_sceneData.size() - 1;
		}
	}

	PrepareScene();
}
/*最終場面是否*/
bool CFkgScenePlayer::HasReachedLastScene() const
{
	return m_nSceneIndex == m_sceneData.size() - 1;
}
/*文字色切り替え*/
void CFkgScenePlayer::ToggleTextColour()
{
	m_dxLibTextWriter.ToggleTextColour();
}

void CFkgScenePlayer::ToggleImageSync()
{
	m_isImageSynced ^= true;
}

void CFkgScenePlayer::ShiftImage()
{
	if (!m_isImageSynced)
	{
		++m_nImageIndex;
		if (m_nImageIndex >= m_imageData.size())m_nImageIndex = 0;

		CheckAnimationTrack();
	}
}
/*尺度変更*/
void CFkgScenePlayer::RescaleImage(bool bUpscale)
{
	if (bUpscale)
	{
		m_fScale += kfScalePortion;
	}
	else
	{
		m_fScale -= kfScalePortion;
		if (m_fScale < kfMinScale)m_fScale = kfMinScale;
	}

	const auto& skeletonSize = m_dxLibSpinePlayer.GetBaseSize();
	float fSkeletonScaleToBe = kDefaultWidth * m_fScale / skeletonSize.x;
	m_dxLibSpinePlayer.SetSkeletonScale(fSkeletonScaleToBe);

}
/*時間尺度変更*/
void CFkgScenePlayer::RescaleAnimationTime(bool bFaster)
{
	m_dxLibSpinePlayer.RescaleTime(bFaster);
}
/*視点移動*/
void CFkgScenePlayer::MoveViewPoint(int iX, int iY)
{
	const auto* p = GetCurrentImageDatum();
	if (p == nullptr)return;

	if (p->bAnimation)
	{
		m_dxLibSpinePlayer.MoveViewPoint(iX, iY);
	}
	else
	{
		m_fOffset.u += iX * m_fScale;
		m_fOffset.v += iY * m_fScale;

		const auto AdjustViewForStill = [this]()
			-> void
			{
				int iClientWidth = 0;
				int iClientHeight = 0;
				DxLib::GetScreenState(&iClientWidth, &iClientHeight, nullptr);

				float fScaledWidth = kDefaultWidth * m_fScale;
				float fScaledHeight = kDefaultHeight * m_fScale;

				float fMaxOffsetX = (fScaledWidth - iClientWidth) / 1.f;
				float fMaxOffsetY = (fScaledHeight - iClientHeight) / 1.f;

				m_fOffset.u = (std::max)(-fMaxOffsetX, m_fOffset.u);
				m_fOffset.v = (std::max)(-fMaxOffsetY, m_fOffset.v);

				m_fOffset.u = (std::min)(fMaxOffsetX, m_fOffset.u);
				m_fOffset.v = (std::min)(fMaxOffsetY, m_fOffset.v);
			};

		AdjustViewForStill();
	}
}
/*尺度・位置初期化*/
void CFkgScenePlayer::ResetScale()
{
	m_fScale = m_fDefaultScale;
	m_fOffset = {};

	ResetSpinePlayerScale();
}

std::vector<adv::LabelDatum>& CFkgScenePlayer::GetLabelData()
{
	return m_labelData;
}

bool CFkgScenePlayer::JumpToLabel(size_t nLabelIndex)
{
	if (nLabelIndex < m_labelData.size())
	{
		const auto& labelDatum = m_labelData[nLabelIndex];

		if (labelDatum.nSceneIndex < m_sceneData.size())
		{
			m_nSceneIndex = labelDatum.nSceneIndex;
			PrepareScene();

			return true;
		}
	}
	return false;
}

const CMfMediaPlayer* CFkgScenePlayer::GetAudioPlayer() const
{
	return m_pAudioPlayer.get();
}
void CFkgScenePlayer::SetFilterOnLoading(FilterOnLoading filter)
{
	m_filter = filter;
}
CFkgScenePlayer::FilterOnLoading CFkgScenePlayer::GetFilterMethod() const
{
	return m_filter;
}
/*台本データ消去*/
void CFkgScenePlayer::ClearScenarioData()
{
	m_textData.clear();

	m_sceneData.clear();
	m_nSceneIndex = 0;

	m_imageData.clear();
	m_imageHandles.clear();
	m_nImageIndex = 0;

	m_wstrFormattedText.clear();
	m_usLastAnimationIndex = 0;

	m_labelData.clear();
}
/*標準尺度算出*/
void CFkgScenePlayer::WorkOutDefaultScale()
{
	m_fDefaultScale = 1.f;

	int iSceneWidth = kDefaultWidth;
	int iSceneHeight = kDefaultHeight;

	int iDisplayWidth = 0;
	int iDisplayHeight = 0;
#if defined _WIN32
	DxLib::GetDisplayMaxResolution(&iDisplayWidth, &iDisplayHeight);
#elif defined __ANDROID__
	DxLib::GetAndroidDisplayResolution(&iDisplayWidth, &iDisplayHeight);
#elif defined __APPLE__
	DxLib::GetDisplayResolution_iOS(&iDisplayWidth, &iDisplayHeight);
#endif
	if (iDisplayWidth == 0 || iDisplayHeight == 0)return;

	if (iSceneWidth > iDisplayWidth || iSceneHeight > iDisplayHeight)
	{
		float fScaleX = static_cast<float>(iDisplayWidth) / iSceneWidth;
		float fScaleY = static_cast<float>(iDisplayHeight) / iSceneHeight;

		if (fScaleX > fScaleY)
		{
			m_fDefaultScale = fScaleY;
		}
		else
		{
			m_fDefaultScale = fScaleX;
		}
	}
}
/*現在の画像データ取り出し*/
CFkgScenePlayer::SImageDatum* CFkgScenePlayer::GetCurrentImageDatum()
{
	if (m_nSceneIndex < m_sceneData.size())
	{
		if (m_isImageSynced)
		{
			m_nImageIndex = m_sceneData[m_nSceneIndex].nImageIndex;
		}

		if (m_nImageIndex < m_imageData.size())
		{
			return &m_imageData[m_nImageIndex];
		}
	}
	return nullptr;
}
/*場面描画事前準備*/
void CFkgScenePlayer::PrepareScene()
{
	PrepareText();
	CheckAnimationTrack();
}
/*動作切り替わり場面か確認*/
void CFkgScenePlayer::CheckAnimationTrack()
{
	const auto* p = GetCurrentImageDatum();
	if (p != nullptr)
	{
		if (p->bAnimation)
		{
			if (m_usLastAnimationIndex != p->animationParams.usIndex)
			{
				m_usLastAnimationIndex = p->animationParams.usIndex;
				m_dxLibSpinePlayer.SetAnimationByIndex(m_usLastAnimationIndex);
			}
		}
		else
		{
			m_usLastAnimationIndex = 0;
		}
	}
}
/*文章作成・附随音声再生*/
void CFkgScenePlayer::PrepareText()
{
	if (m_nSceneIndex < m_sceneData.size())
	{
		size_t nTextIndex = m_sceneData[m_nSceneIndex].nTextIndex;
		if (nTextIndex < m_textData.size())
		{
			std::wstring& wstr = m_wstrFormattedText;
			const adv::TextDatum& t = m_textData[nTextIndex];
			wstr = t.wstrText;
			if (!wstr.empty() && wstr.back() != L'\n')wstr.push_back(L'\n');
			wstr += std::to_wstring(nTextIndex + 1) + L"/" + std::to_wstring(m_textData.size());

			if (!t.wstrVoicePath.empty())
			{
				if (m_pAudioPlayer.get() != nullptr)
				{
					m_pAudioPlayer->Play(t.wstrVoicePath.c_str());
				}
			}

			m_textClock.Restart();
		}
	}
}
/*文章表示経過時間確認*/
void CFkgScenePlayer::CheckTextClock()
{
	float fElapsed = m_textClock.GetElapsedTime();
	if (::isgreaterequal(fElapsed, 3.f))
	{
		m_textClock.Restart();

		if (m_pAudioPlayer.get() != nullptr && m_pAudioPlayer->IsEnded())
		{
			if (!HasReachedLastScene())
			{
				ShiftScene(true);
			}
		}
	}
}

void CFkgScenePlayer::DrawCurrentImage()
{
	const auto* p = GetCurrentImageDatum();
	if (p != nullptr)
	{
		if (p->bAnimation)
		{
			m_dxLibSpinePlayer.Redraw();
		}
		else
		{
			size_t nImageIndex = p->stillParams.usIndex;
			if (nImageIndex < m_imageHandles.size())
			{
				const auto& imageHandle = m_imageHandles[nImageIndex];

				SetTransformMatrixForStill(imageHandle, p->stillParams.fScale * m_fScale);

				DxLib::DrawGraph(0, 0, imageHandle.Get(), FALSE);

				DxLib::ResetTransformTo2D();
			}
		}
	}
}

void CFkgScenePlayer::DrawFormattedText()
{
	if (m_isTextShown)
	{
		m_dxLibTextWriter.Draw(m_wstrFormattedText.c_str(), static_cast<unsigned long>(m_wstrFormattedText.size()));
	}
}

void CFkgScenePlayer::SetTransformMatrixForStill(const DxLibImageHandle& imageHandle, const float fScale) const
{
	int iGraphWidth = 0;
	int iGraphsHeight = 0;
	int iRet = DxLib::GetGraphSize(imageHandle.Get(), &iGraphWidth, &iGraphsHeight);
	if (iRet == -1)return;

	int iClientWidth = 0;
	int iClientHeight = 0;
	DxLib::GetScreenState(&iClientWidth, &iClientHeight, nullptr);
	float fX = (iGraphWidth * fScale - iClientWidth) / 2 + m_fOffset.u / 2;
	float fY = (iGraphsHeight * fScale - iClientHeight) / 2 + m_fOffset.v / 2;

	DxLib::MATRIX matrix = DxLib::MGetScale(DxLib::VGet(fScale, fScale, 1.f));
	DxLib::MATRIX tranlateMatrix = DxLib::MGetTranslate(DxLib::VGet(-fX, -fY, 0.f));
	matrix = DxLib::MMult(matrix, tranlateMatrix);

	DxLib::SetTransformTo2D(&matrix);
}

void CFkgScenePlayer::ResetSpinePlayerScale()
{
	m_dxLibSpinePlayer.ResetScale();
#if defined(TO_DO)
	m_dxLibSpinePlayer.SetZoom(1.05f);
#endif
}

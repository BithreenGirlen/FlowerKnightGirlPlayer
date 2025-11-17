#ifndef FKG_SCENE_PLAYER_H_
#define FKG_SCENE_PLAYER_H_

#include <string>
#include <vector>
#include <memory>

#include "adv.h"
#include "dxlib_spine_cpp/dxlib_spine_player.h"
#include "dxlib_clock.h"
#include "dxlib_text_writer.h"
#include "dxlib_handle.h"
#include "mf_media_player.h"

class CFkgScenePlayer
{
public:
	CFkgScenePlayer();
	~CFkgScenePlayer();

	bool ReadScenario(const std::wstring& wstrFolderPath);
	bool HasScenarioData() const;

	void Update();
	void Redraw();

	void GetStillImageSize(unsigned int* uiWidth, unsigned int* uiHeight) const;

	void ShiftScene(bool bForward);
	bool HasReachedLastScene() const;

	bool IsTextVisible()const { return m_isTextShown; }
	void SetTextVisibility(bool bShown) { m_isTextShown = bShown; }
	void ToggleTextColour();

	bool IsImageSynced() const { return m_isImageSynced; };
	void ToggleImageSync();
	void ShiftImage();

	void RescaleImage(bool bUpscale);
	void RescaleAnimationTime(bool bFaster);

	void MoveViewPoint(int iX, int iY);

	void ResetScale();

	std::vector<adv::LabelDatum>& GetLabelData();
	bool JumpToLabel(size_t nLabelIndex);

	const CMfMediaPlayer* GetAudioPlayer() const;

	enum class FilterOnLoading
	{
		None,
		Avir,
		Lanczos,
		Cubic
	};

	void SetFilterOnLoading(FilterOnLoading filter);
	FilterOnLoading GetFilterMethod() const;
private:
	using DxLibImageHandle = DxLibHandle<&DxLib::DeleteGraph>;
	static constexpr int kDefaultWidth = 1920;
	static constexpr int kDefaultHeight = 1280;

	static constexpr float kfScalePortion = 0.025f;
	static constexpr float kfMinScale = 0.15f;

	struct SImageDatum
	{
		bool bAnimation = false;

		struct AnimationParams
		{
			bool bLoop = true;
			unsigned short usIndex = 0;
		};
		AnimationParams animationParams;

		struct StillParams
		{
			unsigned short usIndex = 0;
			float fScale = 1.f;
		};
		StillParams stillParams;
	};

	bool m_isWebpSupported = false;
	FilterOnLoading m_filter = FilterOnLoading::Cubic;

	std::vector<adv::TextDatum> m_textData;
	std::vector<SImageDatum> m_imageData;
	size_t m_nImageIndex = 0;
	std::vector<DxLibImageHandle> m_imageHandles;

	std::vector<adv::SceneDatum> m_sceneData;
	size_t m_nSceneIndex = 0;

	std::vector<adv::LabelDatum> m_labelData;

	CDxLibSpinePlayer m_dxLibSpinePlayer;
	CDxLibClock m_spineClock;

	CDxLibTextWriter m_dxLibTextWriter;
	CDxLibClock m_textClock;
	std::unique_ptr<CMfMediaPlayer> m_pAudioPlayer;
	bool m_isTextShown = true;

	bool m_isImageSynced = true;

	std::wstring m_wstrFormattedText;
	unsigned short m_usLastAnimationIndex = 0;

	float m_fDefaultScale = 1.f;

	float m_fScale = 1.f;
	DxLib::FLOAT2 m_fOffset{};

	void ClearScenarioData();
	void WorkOutDefaultScale();

	SImageDatum* GetCurrentImageDatum();

	void PrepareScene();

	void CheckAnimationTrack();
	void PrepareText();

	void CheckTextClock();

	void DrawCurrentImage();
	void DrawFormattedText();
	void SetTransformMatrixForStill(const DxLibImageHandle &imageHandle, const float fScale) const;

	void ResetSpinePlayerScale();
};

#endif // !FKG_SCENE_PLAYER_H_

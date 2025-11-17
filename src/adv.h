#ifndef ADV_H_
#define ADV_H_

#include <string>

namespace adv
{
	struct TextDatum
	{
		std::wstring wstrText;
		std::wstring wstrVoicePath;
	};

	struct ImageFileDatum
	{
		bool bAnimation = false;
		std::wstring wstrFilePath;
		std::string strData;
	};

	struct SceneDatum
	{
		size_t nTextIndex = 0;
		size_t nImageIndex = 0;
	};

	struct LabelDatum
	{
		std::wstring wstrCaption;
		size_t nSceneIndex = 0;
	};
}
#endif // !ADV_H_

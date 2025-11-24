#ifndef FKG_H_
#define FKG_H_

#include <string>
#include <vector>

#include "adv.h"

namespace fkg
{
	bool LoadScenarioFile(
		const std::wstring wstrFilePath,
		std::vector<adv::TextDatum>& textData,
		std::vector<adv::ImageFileDatum> &imageFileData,
		std::wstring& spineFileName,
		std::vector<adv::SceneDatum>& sceneData,
		std::vector<adv::LabelDatum>& labelData);
}
#endif // !FKG_H_

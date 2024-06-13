#ifndef FKG_H_
#define FKG_H_

#include <string>
#include <vector>

#include "adv.h"

namespace fkg
{
	bool LoadScenarioFile(const std::wstring wstrFilePath, std::vector<adv::TextDatum>& textData, std::vector<adv::ImageDatum> &imageData);
}
#endif // !FKG_H_

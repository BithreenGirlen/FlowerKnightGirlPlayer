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

	struct ImageDatum
	{
		std::wstring wstrFilePath;
		bool bSpine = false;
	};
}
#endif // !ADV_H_

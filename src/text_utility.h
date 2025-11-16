#ifndef TEXT_UTILITY_H_
#define TEXT_UTILITY_H_

#include <string>
#include <vector>

namespace text_utility
{
	void TextToLines(const std::string& strText, std::vector<std::string>& lines);
	void SplitTextBySeparator(const std::string& strText, const char cSeparator, std::vector<std::string>& splits);
	void ReplaceAll(std::string& strText, const std::string& strOld, const std::string& strNew);
}

#endif // !TEXT_UTILITY_H_

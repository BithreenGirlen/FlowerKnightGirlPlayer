

#include "text_utility.h"

void text_utility::TextToLines(const std::string& strText, std::vector<std::string>& lines)
{
	std::string strTemp;
	for (size_t nRead = 0; nRead < strText.size(); ++nRead)
	{
		if (strText.at(nRead) == '\r' || strText.at(nRead) == '\n')
		{
			if (!strTemp.empty())
			{
				lines.push_back(strTemp);
				strTemp.clear();
			}
			continue;
		}

		strTemp.push_back(strText.at(nRead));
	}
}

void text_utility::SplitTextBySeparator(const std::string& strText, const char cSeparator, std::vector<std::string>& splits)
{
	for (size_t nRead = 0; nRead < strText.size();)
	{
		const char* p = strchr(&strText[nRead], cSeparator);
		if (p == nullptr)
		{
			size_t nLen = strText.size() - nRead;
			splits.emplace_back(strText.substr(nRead, nLen));
			break;
		}

		size_t nLen = p - &strText[nRead];
		splits.emplace_back(strText.substr(nRead, nLen));
		nRead += nLen + 1;
	}
}

void text_utility::ReplaceAll(std::string& strText, const std::string& strOld, const std::string& strNew)
{
	if (strOld == strNew)return;

	for (size_t nRead = 0;;)
	{
		size_t nPos = strText.find(strOld, nRead);
		if (nPos == std::string::npos)break;
		strText.replace(nPos, strOld.size(), strNew);
		nRead = nPos + strNew.size();
	}
}
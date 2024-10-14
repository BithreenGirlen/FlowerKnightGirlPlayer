

#include "fkg.h"

#include "text_utility.h"

#include "win_filesystem.h"
#include "win_text.h"

namespace fkg
{
	namespace token_type
	{
		enum TokenType
		{
			kVoice = 1,
			kImage,
			kSpine
		};
	}

	struct ResourceToken
	{
		int iType = 0;
		std::string strFileName;
		std::string strText;
	};

	void PickupResourceTokensFromBook(const std::string& strBook, std::vector<fkg::ResourceToken>& resourceTokens)
	{
		std::vector<std::string> lines;
		text_utility::TextToLines(strBook, lines);

		for (const auto& line : lines)
		{
			fkg::ResourceToken resourceToken;

			std::vector<std::string> columns;
			text_utility::SplitTextBySeparator(line, ',', columns);

			if (columns.empty())continue;

			const auto& strType = columns.at(0);
			for (auto& column : columns)text_utility::ReplaceAll(column, "\t", "");

			if (strType == "mess")
			{
				if (columns.size() > 2)
				{
					resourceToken.iType = fkg::token_type::kVoice;
					resourceToken.strText.reserve(256);
					if (!columns.at(1).empty())
					{
						resourceToken.strText = columns.at(1);
						resourceToken.strText += ":\n";
					}
					resourceToken.strText += columns.at(2);
					if (columns.size() > 3)resourceToken.strFileName = columns.at(3);
					resourceTokens.push_back(resourceToken);
				}
			}
			else if (strType == "image")
			{
				if (columns.size() > 1)
				{
					resourceToken.iType = fkg::token_type::kImage;
					resourceToken.strFileName = columns.at(1);
					resourceTokens.push_back(resourceToken);
				}
			}
			else if (strType == "spine")
			{
				if (columns.size() > 1)
				{
					resourceToken.iType = fkg::token_type::kSpine;
					resourceToken.strFileName = columns.at(1);
					resourceTokens.push_back(resourceToken);
				}
			}
		}
	}

	std::string g_strResourceFolderPath;

	bool SetResouceFolderPath(const std::wstring& wstrBookFilePath)
	{
		size_t nPos = wstrBookFilePath.rfind(L"Episode");
		if (nPos == std::wstring::npos)return false;

		g_strResourceFolderPath = win_text::NarrowANSI(wstrBookFilePath.substr(0, nPos)) + "Resource\\";
		return true;
	}

	std::string RelativePathToAbsoluteFilePath(std::string& strPath)
	{
		text_utility::ReplaceAll(strPath, "/", "\\");
		return g_strResourceFolderPath + strPath;
	}

	void ReplaceNewLineEscape(std::string& strText)
	{
		text_utility::ReplaceAll(strText, "\\n", "\n");
	}
	
} // namespace fkg

bool fkg::LoadScenarioFile(const std::wstring wstrFilePath, std::vector<adv::TextDatum>& textData, std::vector<adv::ImageDatum> &imageData)
{
	std::string strFile = win_filesystem::LoadFileAsString(wstrFilePath.c_str());
	if (strFile.empty())return false;

	if (!SetResouceFolderPath(wstrFilePath))return false;

	std::vector<fkg::ResourceToken> resourceTokens;
	PickupResourceTokensFromBook(strFile, resourceTokens);

	for (auto& resourceToken : resourceTokens)
	{
		adv::TextDatum textDatum;
		adv::ImageDatum imageDatum;
		switch (resourceToken.iType)
		{
		case fkg::token_type::kVoice:
			ReplaceNewLineEscape(resourceToken.strText);
			textDatum.wstrText = win_text::WidenUtf8(resourceToken.strText);
			if (!resourceToken.strFileName.empty())
			{
				textDatum.wstrVoicePath = win_text::WidenANSI(RelativePathToAbsoluteFilePath(resourceToken.strFileName)) + L".mp3";
			}
			textData.push_back(textDatum);
			break;
		case fkg::token_type::kImage:
			imageDatum.bSpine = false;
			imageDatum.strFilePath = RelativePathToAbsoluteFilePath(resourceToken.strFileName) + ".png";
			imageData.push_back(imageDatum);
			break;
		case fkg::token_type::kSpine:
			imageDatum.bSpine = true;
			imageDatum.strFilePath = RelativePathToAbsoluteFilePath(resourceToken.strFileName);
			imageData.push_back(imageDatum);
			break;
		}
	}
    return !textData.empty() && !imageData.empty();
}

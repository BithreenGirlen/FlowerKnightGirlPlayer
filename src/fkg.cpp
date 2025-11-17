

#include "fkg.h"

#include "text_utility.h"
#include "path_utility.h"

#include "win_filesystem.h"
#include "win_text.h"

namespace fkg
{
	enum class TokenType
	{
		kUnknown = -1,
		kVoice = 1,
		kImage,
		kSpine,
		kSpinePlay
	};

	struct ResourceToken
	{
		TokenType tokenType = TokenType::kUnknown;
		std::string strFileName;
		std::string strData;
	};

	static void PickupResourceTokensFromBook(const std::string& strBook, std::vector<ResourceToken>& resourceTokens)
	{
		std::vector<std::string> lines;
		text_utility::TextToLines(strBook, lines);

		for (const auto& line : lines)
		{
			fkg::ResourceToken resourceToken;

			std::vector<std::string> columns;
			text_utility::SplitTextBySeparator(line, ',', columns);

			if (columns.empty())continue;

			const auto& strType = columns[0];
			for (auto& column : columns)text_utility::ReplaceAll(column, "\t", "");

			if (strType == "mess")
			{
				if (columns.size() > 2)
				{
					resourceToken.tokenType = TokenType::kVoice;
					resourceToken.strData.reserve(256);
					if (!columns[1].empty())
					{
						resourceToken.strData = columns[1];
						resourceToken.strData += ":\n";
					}
					resourceToken.strData += columns[2];
					if (columns.size() > 3)resourceToken.strFileName = columns[3];
					resourceTokens.push_back(resourceToken);
				}
			}
			else if (strType == "image")
			{
				if (columns.size() > 1)
				{
					resourceToken.tokenType = TokenType::kImage;
					resourceToken.strFileName = columns[1];
					resourceTokens.push_back(resourceToken);
				}
			}
			else if (strType == "spine")
			{
				if (columns.size() > 1)
				{
					resourceToken.tokenType = TokenType::kSpine;
					resourceToken.strFileName = columns[1];
					resourceTokens.push_back(resourceToken);
				}
			}
			else if(strType == "spine_play")
			{
				if (columns.size() > 1)
				{
					resourceToken.tokenType = TokenType::kSpinePlay;
					resourceToken.strData = columns[1];
					resourceTokens.push_back(resourceToken);
				}
			}
		}
	}

	static std::wstring g_wstrResourceFolderPath;

	static bool SetResouceFolderPath(const std::wstring& wstrBookFilePath)
	{
		size_t nPos = wstrBookFilePath.rfind(L"Episode");
		if (nPos == std::wstring::npos)return false;

		g_wstrResourceFolderPath = wstrBookFilePath.substr(0, nPos) + L"Resource\\";
		return true;
	}

	static std::wstring RelativePathToAbsoluteFilePath(std::string& strPath)
	{
		text_utility::ReplaceAll(strPath, "/", "\\");
		return g_wstrResourceFolderPath + win_text::WidenUtf8(strPath);
	}

	static void ReplaceNewLineEscape(std::string& strData)
	{
		text_utility::ReplaceAll(strData, "\\n", "\n");
	}
	
} // namespace fkg

bool fkg::LoadScenarioFile(const std::wstring wstrFilePath, std::vector<adv::TextDatum>& textData, std::vector<adv::ImageFileDatum> &imageFileData, std::vector<adv::SceneDatum>& sceneData, std::vector<adv::LabelDatum>& labelData)
{
	std::string strFile = win_filesystem::LoadFileAsString(wstrFilePath.c_str());
	if (strFile.empty())return false;

	if (!SetResouceFolderPath(wstrFilePath))return false;

	std::vector<fkg::ResourceToken> resourceTokens;
	PickupResourceTokensFromBook(strFile, resourceTokens);

	/*
	* Spineファイルと動作情報は別行になっているので、ファイル情報を確認したら減算する。
	* 設計上、adv::ImageFileDatumとは別の構造体に分けた方がよいかもしれないが…
	*/
	long long nImageDatumOffset = 0;
	std::wstring labelBuffer;
	for (auto& resourceToken : resourceTokens)
	{
		adv::TextDatum textDatum;
		adv::ImageFileDatum imageDatum;
		switch (resourceToken.tokenType)
		{
		case TokenType::kVoice:
			ReplaceNewLineEscape(resourceToken.strData);
			textDatum.wstrText = win_text::WidenUtf8(resourceToken.strData);
			if (!resourceToken.strFileName.empty())
			{
				textDatum.wstrVoicePath = RelativePathToAbsoluteFilePath(resourceToken.strFileName) + L".mp3";
			}
			textData.push_back(std::move(textDatum));

			{
				adv::SceneDatum sceneDatum;
				sceneDatum.nImageIndex = imageFileData.empty() ? 0 : imageFileData.size() - 1 + nImageDatumOffset;
				sceneDatum.nTextIndex = textData.size() - 1;
				sceneData.push_back(std::move(sceneDatum));
			}

			if (!labelBuffer.empty())
			{
				labelData.emplace_back(adv::LabelDatum{ labelBuffer, sceneData.size() - 1 });
				labelBuffer.clear();
			}
			break;
		case TokenType::kImage:
			imageDatum.bAnimation = false;
			imageDatum.wstrFilePath = RelativePathToAbsoluteFilePath(resourceToken.strFileName) + L".png";
			imageFileData.push_back(std::move(imageDatum));
			labelBuffer = win_text::WidenUtf8(path_utility::TruncateFilePath(resourceToken.strFileName));
			break;
		case TokenType::kSpine:
			imageDatum.bAnimation = true;
			imageDatum.wstrFilePath = RelativePathToAbsoluteFilePath(resourceToken.strFileName);
			imageFileData.push_back(std::move(imageDatum));
			nImageDatumOffset = -1;
			break;
		case TokenType::kSpinePlay:
			imageDatum.bAnimation = true;
			imageDatum.strData = resourceToken.strData;
			imageFileData.push_back(std::move(imageDatum));
			/* "Finish"と"After"の間には文章が存在しないので、前の紐づけを上書きする。*/
			if (labelBuffer == L"Finish")
			{
				if (sceneData.size() > 3)
				{
					/* 通常位置の1つ手前 */
					sceneData.back().nImageIndex = imageFileData.size() - 1 + nImageDatumOffset - 1;
				}
				labelData.emplace_back(adv::LabelDatum{ labelBuffer, sceneData.size() - 1 });
				labelBuffer.clear();
			}
			labelBuffer = win_text::WidenUtf8(resourceToken.strData);
			break;
		}
	}
    return !textData.empty() && !imageFileData.empty();
}

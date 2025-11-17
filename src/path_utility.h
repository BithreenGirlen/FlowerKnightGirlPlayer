#ifndef PATH_UTILITY_H_
#define PATH_UTILITY_H_

#include <string>

namespace path_utility
{
	template <typename CharType>
	std::basic_string<CharType> ExtractDirectory(const std::basic_string<CharType>& filePath)
	{
		const CharType separators[] = { '\\', '/', '\0' };
		size_t nPos = filePath.find_last_of(separators);
		if (nPos != std::basic_string<CharType>::npos)
		{
			return filePath.substr(0, nPos);
		}
		return filePath;
	}

	template <typename CharType>
	std::basic_string<CharType> ExtractFileName(const std::basic_string<CharType>& filePath)
	{
		const CharType separators[] = { '\\', '/', '\0' };
		size_t nPos = filePath.find_last_of(separators);
		nPos = nPos == std::basic_string<CharType>::npos ? 0 : nPos + 1;

		size_t nPos2 = filePath.find(CharType('.'), nPos);
		if (nPos2 == std::basic_string<CharType>::npos)nPos2 = filePath.size();

		return filePath.substr(nPos, nPos2 - nPos);
	}
	template <typename CharType>
	std::basic_string<CharType> ExtractFileName(const CharType* filePath)
	{
		return ExtractFileName(std::basic_string<CharType>(filePath));
	}

	template <typename CharType>
	std::basic_string<CharType> TruncateFilePath(const std::basic_string<CharType>& filePath)
	{
		const CharType separators[] = { '\\', '/', '\0' };
		size_t nPos = filePath.find_last_of(separators);
		if (nPos != std::basic_string<CharType>::npos)
		{
			return filePath.substr(nPos + 1);
		}
		return filePath;
	}

	template <typename CharType>
	std::basic_string<CharType> GetExtensionFromFileName(const std::basic_string<CharType>& filePath)
	{
		size_t nPos = filePath.rfind(CharType('/'));
		nPos = nPos != std::basic_string<CharType>::npos ? nPos + 1 : 0;

		nPos = filePath.find(CharType('.'), nPos);
		if (nPos != std::basic_string<CharType>::npos)
		{
			return filePath.substr(nPos);
		}

		return std::basic_string<CharType>();
	}
}
#endif // !PATH_UTILITY_H_

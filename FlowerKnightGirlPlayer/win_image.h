#ifndef WIN_IMAGE_H_
#define WIN_IMAGE_H_

#include "image_info.h"

namespace win_image
{
	bool LoadImageToMemory(const wchar_t* wpzFilePath, ImageInfo* pImageInfo, float fScale);
}
#endif // !WIN_IMAGE_H_

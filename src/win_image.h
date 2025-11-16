#ifndef WIN_IMAGE_H_
#define WIN_IMAGE_H_

#include "image_frame.h"

namespace win_image
{
	bool LoadImageToMemory(const wchar_t* wpzFilePath, SImageFrame* pImageInfo, float fScale);
}
#endif // !WIN_IMAGE_H_

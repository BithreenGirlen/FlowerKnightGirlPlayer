#ifndef IMAGE_FRAME_H_
#define IMAGE_FRAME_H_

#include <vector>

struct SImageFrame
{
	unsigned int uiWidth = 0;
	unsigned int uiHeight = 0;
	int iStride = 0;
	std::vector<unsigned char> pixels;
};

#endif // !IMAGE_FRAME_H_

#ifndef IMAGE_INFO_H_
#define IMAGE_INFO_H_

#include <vector>

struct ImageInfo
{
	unsigned int uiWidth = 0;
	unsigned int uiHeight = 0;
	int iStride = 0;
	std::vector<unsigned char> pixels;
};

#endif // !IMAGE_INFO_H_

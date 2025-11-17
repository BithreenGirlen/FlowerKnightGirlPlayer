#ifndef DXLIB_TEXT_WRITER_H_
#define DXLIB_TEXT_WRITER_H_

class CDxLibTextWriter
{
public:
	CDxLibTextWriter();
	~CDxLibTextWriter();

	bool SetFont(const wchar_t* pwzFontFileName, int iFontSize, bool bBold, bool bItalic);
	void Draw(const wchar_t* pwzText, unsigned long ulTextLength, int iPosX = 0, int iPosY = 0);

	void ToggleTextColour();
private:
	int m_iFontHandle = -1;

	enum Colours : unsigned int
	{
		kBlack = 0xff000000,
		kWhite = 0xffffffff,
	};

	unsigned int m_uiFillColour = Colours::kBlack;
	unsigned int m_uiOutLineColour = Colours::kWhite;

	bool DeleteFont();
};
#endif // !DXLIB_TEXT_WRITER_H_

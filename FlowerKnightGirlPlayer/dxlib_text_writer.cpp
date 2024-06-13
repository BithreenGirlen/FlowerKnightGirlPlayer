

#include "dxlib_text_writer.h"

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

CDxLibTextWriter::CDxLibTextWriter()
{

}

CDxLibTextWriter::~CDxLibTextWriter()
{
    DeleteFont();
}

bool CDxLibTextWriter::SetFont(const wchar_t* pwzFontFileName, int iFontSize, bool bBold, bool bItalic)
{
    DeleteFont();

    m_iFontHandle = DxLib::CreateFontToHandle
    (
        pwzFontFileName,
        iFontSize > 0 ? iFontSize : DEFAULT_FONT_SIZE,
        bBold ? DEFAULT_FONT_THINCK : DEFAULT_FONT_THINCK / 2,
        DX_FONTTYPE_ANTIALIASING_EDGE_4X4,
        DX_CHARSET_UTF8,
        DEFAULT_FONT_EDGESIZE,
        bItalic ? TRUE : FALSE
    );

    return m_iFontHandle != -1;
}

void CDxLibTextWriter::Draw(const wchar_t* pwzText, unsigned long ulTextLength, int iPosX, int iPosY)
{
    if (m_iFontHandle != -1)
    {
        DxLib::DrawNStringToHandle(iPosX + 4, iPosY + 4, pwzText, ulTextLength, m_uiFillColour, m_iFontHandle, m_uiOutLineColour);
    }
}

void CDxLibTextWriter::SwitchTextColour()
{
    m_uiFillColour = m_uiFillColour == Colours::kBlack ? Colours::kWhite : Colours::kBlack;
    m_uiOutLineColour = m_uiOutLineColour == Colours::kBlack ? Colours::kWhite : Colours::kBlack;
}

bool CDxLibTextWriter::DeleteFont()
{
    if (m_iFontHandle != -1)
    {
        int iRet = DxLib::DeleteFontToHandle(m_iFontHandle);
        if (iRet == -1)return false;

        m_iFontHandle = -1;
    }
    return true;
}

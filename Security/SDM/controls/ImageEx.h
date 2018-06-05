#pragma once

class CImageEx
{
    public:
        CImageEx(void);
        virtual ~CImageEx(void);

        bool IsNull();
        INT GetWidth();
        INT GetHeight();
        
        bool DestroyImage();
        bool LoadImage(LPCTSTR pszFileName);
        bool LoadImage(HINSTANCE hInstance, LPCTSTR pszResourceName);
        
        bool DrawImage(CDC * pDC, INT nXPos, INT nYPos);
        bool DrawImage(CDC * pDC, INT nXPos, INT nYPos, INT nDestWidth, INT nDestHeight);
        bool DrawImage(CDC * pDC, INT nXDest, INT nYDest, INT nDestWidth, INT nDestHeight, INT nXScr, INT nYSrc);
        bool DrawImage(CDC * pDC, INT nXDest, INT nYDest, INT nDestWidth, INT nDestHeight, INT nXScr, INT nYSrc, INT nSrcWidth, INT nSrcHeight);
        
        bool AlphaDrawImage(CDC * pDestDC, INT xDest, INT yDest, BYTE cbAlphaDepth);
        bool AlphaDrawImage(CDC * pDestDC, INT xDest, INT yDest, INT cxDest, INT cyDest, INT xSrc, INT ySrc, BYTE cbAlphaDepth);
        bool AlphaDrawImage(CDC * pDestDC, INT xDest, INT yDest, INT cxDest, INT cyDest, INT xSrc, INT ySrc, INT cxSrc, INT cySrc, BYTE cbAlphaDepth);
        
    protected:
        Image * m_pImage;
        TCHAR m_strImageName[MAX_PATH];
        
};

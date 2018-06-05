#include "stdafx.h"
#include "../SDM.h"
#include "ImageEx.h"

CImageEx::CImageEx(void)
{
    m_pImage = NULL;
    ZeroMemory(m_strImageName, sizeof(m_strImageName));
}

CImageEx::~CImageEx(void)
{
    DestroyImage();
    return;
}

bool CImageEx::IsNull()
{
    if(m_pImage == NULL)
    {
        return true;
    }
    
    if(m_pImage->GetLastStatus() != Ok)
    {
        return true;
    }
    
    return false;
}

INT CImageEx::GetWidth()
{
    ASSERT(IsNull() == false);
    
    if(IsNull() == true)
    {
        return 0;
    }
    
    return m_pImage->GetWidth();
}

INT CImageEx::GetHeight()
{
    ASSERT(IsNull() == false);
    
    if(IsNull() == true)
    {
        return 0;
    }
    
    return m_pImage->GetHeight();
}

bool CImageEx::DestroyImage()
{
    if(m_pImage != NULL)
    {
        SAFE_DELETE_CLASS(m_pImage);
    }
    
    return true;
}

bool CImageEx::LoadImage(LPCTSTR pszFileName)
{
    ASSERT(m_pImage == NULL);
    
    if(m_pImage != NULL)
    {
        return false;
    }
    
    CT2CW strFileName(pszFileName);
    m_pImage = Image::FromFile((LPCWSTR)strFileName);
    
    if((m_pImage == NULL) || (m_pImage->GetLastStatus() != Ok))
    {
        DestroyImage();
        return false;
    }
    
    return true;
}

bool CImageEx::LoadImage(HINSTANCE hInstance, LPCTSTR pszResourceName)
{
    ASSERT(m_pImage == NULL);
    
    if(m_pImage != NULL)
    {
        return false;
    }
    
    HRSRC hResource = FindResource(hInstance, pszResourceName, TEXT("IMAGE"));
    
    if(hResource == NULL)
    {
        return false;
    }
    
    DWORD dwImageSize = SizeofResource(hInstance, hResource);
    LPVOID pImageBuffer = LoadResource(hInstance, hResource);
    IStream * pIStream = NULL;
    
    if(CreateStreamOnHGlobal(NULL, TRUE, &pIStream) != S_OK)
    {
        ASSERT(FALSE);
        return false;
    }
    
    pIStream->Write(pImageBuffer, dwImageSize, NULL);
    m_pImage = Image::FromStream(pIStream);
    SafeRelease(pIStream);
    
    if((m_pImage == NULL) || (m_pImage->GetLastStatus() != Ok))
    {
        ASSERT(FALSE);
        return false;
    }
    
    return true;
}

bool CImageEx::DrawImage(CDC * pDC, INT nXPos, INT nYPos)
{
    ASSERT(m_pImage != NULL);
    
    if(m_pImage == NULL)
    {
        return false;
    }
    
    ASSERT(pDC != NULL);
    Graphics graphics(pDC->GetSafeHdc());
    INT nImageWidth = m_pImage->GetWidth();
    INT nImageHeight = m_pImage->GetHeight();
    RectF rcDrawRect;
    rcDrawRect.X = (REAL)nXPos;
    rcDrawRect.Y = (REAL)nYPos;
    rcDrawRect.Width = (REAL)nImageWidth;
    rcDrawRect.Height = (REAL)nImageHeight;
    graphics.DrawImage(m_pImage, rcDrawRect, 0, 0, (REAL)nImageWidth, (REAL)nImageHeight, UnitPixel);
    return true;
}

bool CImageEx::DrawImage(CDC * pDC, INT nXPos, INT nYPos, INT nDestWidth, INT nDestHeight)
{
    ASSERT(m_pImage != NULL);
    
    if(m_pImage == NULL)
    {
        return false;
    }
    
    ASSERT(pDC != NULL);
    Graphics graphics(pDC->GetSafeHdc());
    RectF rcDrawRect;
    rcDrawRect.X = (REAL)nXPos;
    rcDrawRect.Y = (REAL)nYPos;
    rcDrawRect.Width = (REAL)nDestWidth;
    rcDrawRect.Height = (REAL)nDestHeight;
    graphics.DrawImage(m_pImage, rcDrawRect, 0, 0, (REAL)GetWidth(), (REAL)GetHeight(), UnitPixel);
    return true;
}

bool CImageEx::DrawImage(CDC * pDC, INT nXDest, INT nYDest, INT nDestWidth, INT nDestHeight, INT nXScr, INT nYSrc)
{
    ASSERT(m_pImage != NULL);
    
    if(m_pImage == NULL)
    {
        return false;
    }
    
    ASSERT(pDC != NULL);
    Graphics graphics(pDC->GetSafeHdc());
    RectF rcDrawRect;
    rcDrawRect.X = (REAL)nXDest;
    rcDrawRect.Y = (REAL)nYDest;
    rcDrawRect.Width = (REAL)nDestWidth;
    rcDrawRect.Height = (REAL)nDestHeight;
    graphics.DrawImage(m_pImage, rcDrawRect, (REAL)nXScr, (REAL)nYSrc, (REAL)nDestWidth, (REAL)nDestHeight, UnitPixel);
    return true;
}

bool CImageEx::DrawImage(CDC * pDC, INT nXDest, INT nYDest, INT nDestWidth, INT nDestHeight, INT nXScr, INT nYSrc, INT nSrcWidth, INT nSrcHeight)
{
    ASSERT(m_pImage != NULL);
    
    if(m_pImage == NULL)
    {
        return false;
    }
    
    ASSERT(pDC != NULL);
    Graphics graphics(pDC->GetSafeHdc());
    RectF rcDrawRect;
    rcDrawRect.X = (REAL)nXDest;
    rcDrawRect.Y = (REAL)nYDest;
    rcDrawRect.Width = (REAL)nDestWidth;
    rcDrawRect.Height = (REAL)nDestHeight;
    graphics.DrawImage(m_pImage, rcDrawRect, (REAL)nXScr, (REAL)nYSrc, (REAL)nSrcWidth, (REAL)nSrcHeight, UnitPixel);
    return true;
}

bool CImageEx::AlphaDrawImage(CDC * pDestDC, INT xDest, INT yDest, BYTE cbAlphaDepth)
{
    AlphaDrawImage(pDestDC, xDest, yDest, GetWidth(), GetHeight(), 0, 0, cbAlphaDepth);
    return true;
}

bool CImageEx::AlphaDrawImage(CDC * pDestDC, INT xDest, INT yDest, INT cxDest, INT cyDest, INT xSrc, INT ySrc, BYTE cbAlphaDepth)
{
    ASSERT(m_pImage != NULL);
    
    if(m_pImage == NULL)
    {
        return false;
    }
    
    ASSERT(pDestDC != NULL);
    Graphics graphics(pDestDC->GetSafeHdc());
    RectF rcDrawRect;
    rcDrawRect.X = (REAL)xDest;
    rcDrawRect.Y = (REAL)yDest;
    rcDrawRect.Width = (REAL)cxDest;
    rcDrawRect.Height = (REAL)cyDest;
    ColorMatrix Matrix =
    {
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, cbAlphaDepth / 255.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f
    };
    ImageAttributes Attributes;
    Attributes.SetColorMatrix(&Matrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
    graphics.DrawImage(m_pImage, rcDrawRect, (REAL)xSrc, (REAL)ySrc, (REAL)cxDest, (REAL)cyDest, UnitPixel, &Attributes);
    return true;
}

bool CImageEx::AlphaDrawImage(CDC * pDestDC, INT xDest, INT yDest, INT cxDest, INT cyDest, INT xSrc, INT ySrc, INT cxSrc, INT cySrc, BYTE cbAlphaDepth)
{
    if((cxDest != cxSrc) || (cyDest != cySrc))
    {
        ASSERT(m_pImage != NULL);
        
        if(m_pImage == NULL)
        {
            return false;
        }
        
        ASSERT(pDestDC != NULL);
        Graphics graphics(pDestDC->GetSafeHdc());
        RectF rcDrawRect;
        rcDrawRect.X = (REAL)xDest;
        rcDrawRect.Y = (REAL)yDest;
        rcDrawRect.Width = (REAL)cxDest;
        rcDrawRect.Height = (REAL)cyDest;
        ColorMatrix Matrix =
        {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, cbAlphaDepth / 255.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f
        };
        ImageAttributes Attributes;
        Attributes.SetColorMatrix(&Matrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
        graphics.DrawImage(m_pImage, rcDrawRect, (REAL)xSrc, (REAL)ySrc, (REAL)cxSrc, (REAL)cySrc, UnitPixel, &Attributes);
    }
    else
    {
        AlphaDrawImage(pDestDC, xDest, yDest, cxDest, cyDest, xSrc, ySrc, cbAlphaDepth);
    }
    
    return true;
}
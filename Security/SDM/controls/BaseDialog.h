#pragma once

#include "ImageEx.h"
#include "ButtonEx.h"

class CBaseDialog : public CDialog
{
        DECLARE_DYNAMIC(CBaseDialog)
        
    public:
        CBaseDialog(UINT nIDTemplate, CWnd* pParent = NULL);
        ~CBaseDialog(void);

        bool LoadBackSkin(LPCTSTR pszResourcePath);
        bool LoadBackSkin(HINSTANCE hInstance, LPCTSTR pszResourceName);
        void RemoveBorder();
        
        virtual void OnClientDraw(CDC*pDC, INT nWidth, INT nHeight) {}
        
        DECLARE_MESSAGE_MAP()
        
    protected:    
        virtual void DoDataExchange(CDataExchange* pDX);
        virtual BOOL OnInitDialog();
        
        afx_msg void OnPaint();
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        
        CImageEx m_ImageBack;      
        CButtonEx m_btClose;
        
        bool m_bIsInit;
        bool m_bIsZoomed;

};


#include "StdAfx.h"
#include "ButtonEx.h"

IMPLEMENT_DYNAMIC(CButtonEx, CButton)

CButtonEx::CButtonEx(void)
{
    m_bExpand = true;
    m_bHovering = false;
    m_crTextColor = RGB(0, 0, 0);
    m_nState = CTRL_NOFOCUS;
    m_nBtnType = BTN_TYPE_NORMAL;
    m_bMenuOn = FALSE;
}

CButtonEx::~CButtonEx(void)
{
}

BEGIN_MESSAGE_MAP(CButtonEx, CButton)
    ON_WM_CREATE()
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

void CButtonEx::PreSubclassWindow()
{
    __super::PreSubclassWindow();
    SetButtonStyle(GetButtonStyle() | BS_OWNERDRAW);
}

int CButtonEx::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if(__super::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }
    
    SetButtonStyle(GetButtonStyle() | BS_OWNERDRAW);
    return 0;
}

bool CButtonEx::SetButtonImage(LPCTSTR pszFileName, bool bExpandImage)
{
    ASSERT(pszFileName);
    
    if(pszFileName == NULL)
    {
        return false;
    }
    
    m_bExpand = bExpandImage;
    m_ImageBack.LoadImage(pszFileName);
    
    if(m_bExpand == false)
    {
        FixButtonSize();
    }
    
    if(GetSafeHwnd())
    {
        Invalidate(FALSE);
    }
    
    return true;
}

bool CButtonEx::SetButtonImage(HINSTANCE hInstance, LPCTSTR pszResourceName, bool bExpandImage)
{
    ASSERT(pszResourceName != 0);
    
    if(pszResourceName == NULL)
    {
        return false;
    }
    
    m_bExpand = bExpandImage;
    m_ImageBack.LoadImage(hInstance, pszResourceName);
    
    if(m_bExpand == false)
    {
        FixButtonSize();
    }
    
    if(GetSafeHwnd())
    {
        Invalidate(FALSE);
    }
    
    m_nBtnType = BTN_TYPE_NORMAL;
    return true;
}

bool CButtonEx::FixButtonSize()
{
    if(!m_ImageBack.IsNull() && GetSafeHwnd())
    {
        SetWindowPos(NULL, 0, 0, m_ImageBack.GetWidth() / 4, m_ImageBack.GetHeight(), SWP_NOMOVE);
        return true;
    }
    
    return false;
}

void CButtonEx::OnMouseMove(UINT nFlags, CPoint point)
{
    if(m_bHovering == false)
    {
        //m_bHovering=true;
        //Invalidate(FALSE);
        TRACKMOUSEEVENT TrackMouseEvent;
        ZeroMemory(&TrackMouseEvent, sizeof(TRACKMOUSEEVENT));
        TrackMouseEvent.cbSize = sizeof(TrackMouseEvent);
        TrackMouseEvent.dwFlags = TME_HOVER | TME_LEAVE;
        TrackMouseEvent.hwndTrack = this->GetSafeHwnd();
        //TrackMouseEvent.dwHoverTime=HOVER_DEFAULT;
        //_TrackMouseEvent(&TrackMouseEvent);
        TrackMouseEvent.dwHoverTime = 1;
        
        if(::_TrackMouseEvent(&TrackMouseEvent))
        {
            m_bHovering = true;
        }
    }
    
    __super::OnMouseMove(nFlags, point);
}

LRESULT CButtonEx::OnMouseLeave(WPARAM wparam, LPARAM lparam)
{
    m_bHovering = false;
    //Invalidate(FALSE);
    
    if(m_nBtnType == BTN_TYPE_NORMAL)
    {
        m_nState = CTRL_NOFOCUS;
    }
    else if(m_nBtnType == BTN_TYPE_MENU)
    {
        if(m_bMenuOn)
        {
            m_nState = CTRL_SELECTED;
        }
        else
        {
            m_nState = CTRL_NOFOCUS;
        }
    }
    
    PaintParent();
    return 0;
}

bool CButtonEx::SetTextColor(COLORREF crTextColor)
{
    m_crTextColor = crTextColor;
    
    if(GetSafeHwnd())
    {
        Invalidate(FALSE);
    }
    
    return true;
}

void CButtonEx::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CRect rcClient;
    GetClientRect(&rcClient);
    bool bDisable = ((lpDrawItemStruct->itemState & ODS_DISABLED) != 0);
    bool bButtonDown = ((lpDrawItemStruct->itemState & ODS_SELECTED) != 0);
    CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
    CString strText;
    GetWindowText(strText);
    CRect rcWindow;
    CClientDC clDC(GetParent());
    GetWindowRect(rcWindow);
    GetParent()->ScreenToClient(rcWindow);
    CDC dcParent;
    CBitmap bmp, *pOldBitmap;
    dcParent.CreateCompatibleDC(&clDC);
    bmp.CreateCompatibleBitmap(&clDC, rcClient.Width(), rcClient.Height());
    pOldBitmap = dcParent.SelectObject(&bmp);
    dcParent.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &clDC, rcWindow.left, rcWindow.top, SRCCOPY);
    bmp.DeleteObject();
    
    if(!m_ImageBack.IsNull())
    {
        int nWidth = m_ImageBack.GetWidth() / 4, nDrawPos = 0;
        //if (bDisable) nDrawPos=nWidth*3;
        //else if (bButtonDown) nDrawPos=nWidth*2;
        //else if (m_bHovering) nDrawPos=nWidth*1;
        
        if(m_nState == CTRL_NOFOCUS)
        {
            nDrawPos = 0;
        }
        else if(m_nState == CTRL_FOCUS)
        {
            nDrawPos = nWidth;
        }
        else if(m_nState == CTRL_SELECTED)
        {
            nDrawPos = nWidth * 2;
        }
        else if(m_nState == CTRL_DISABLE)
        {
            nDrawPos = nWidth * 3;
        }
        else
        {
            nDrawPos = 0;
        }
        
        if(m_bExpand == false)
        {
            m_ImageBack.DrawImage(pDC, 0, 0, rcClient.Width(), rcClient.Height(), nDrawPos, 0);
        }
        else
        {
            m_ImageBack.DrawImage(pDC, 0, 0, rcClient.Width(), rcClient.Height(), nDrawPos, 0, nWidth, m_ImageBack.GetHeight());
        }
    }
    else
    {
        pDC->FillSolidRect(&rcClient, GetSysColor(COLOR_BTNFACE));
        
        if(bButtonDown)
        {
            pDC->Draw3dRect(&rcClient, GetSysColor(COLOR_WINDOWFRAME), GetSysColor(COLOR_3DHILIGHT));
        }
        else
        {
            pDC->Draw3dRect(&rcClient, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_WINDOWFRAME));
        }
    }
    
    rcClient.top += 1;
    pDC->SetBkMode(TRANSPARENT);
    
    if(bDisable)
    {
        pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
    }
    else
    {
        pDC->SetTextColor(m_crTextColor);
    }
    
    pDC->DrawText(strText, strText.GetLength(), rcClient, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    return;
}

BOOL CButtonEx::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
    //	return CButton::OnEraseBkgnd(pDC);
}

LRESULT CButtonEx::OnMouseHover(WPARAM wParam, LPARAM lParam)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    if(m_nState != CTRL_FOCUS)
    {
        m_nState = CTRL_FOCUS;
        PaintParent();
    }
    
    return 0;
}

void CButtonEx::PaintParent()
{
    CRect rect;
    GetWindowRect(&rect);
    GetParent()->ScreenToClient(&rect);
    GetParent()->InvalidateRect(&rect);
}

void CButtonEx::OnLButtonDown(UINT nFlags, CPoint point)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    if(m_nState != CTRL_SELECTED)
    {
        m_nState = CTRL_SELECTED;
        PaintParent();
    }
    
    CButton::OnLButtonDown(nFlags, point);
}

void CButtonEx::OnLButtonUp(UINT nFlags, CPoint point)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    if(m_nState != CTRL_FOCUS)
    {
        m_nState = CTRL_FOCUS;
        PaintParent();
    }
    
    CButton::OnLButtonUp(nFlags, point);
}

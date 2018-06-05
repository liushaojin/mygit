#include "StdAfx.h"
#include "BaseDialog.h"

#define BORDERWIDTH 3

IMPLEMENT_DYNAMIC(CBaseDialog, CDialog)

CBaseDialog::CBaseDialog(UINT nIDTemplate, CWnd * pParentWnd)
    : CDialog(nIDTemplate, pParentWnd)
{
    m_bIsInit = false;
    m_bIsZoomed = false;
}

CBaseDialog::~CBaseDialog(void)
{
}

void CBaseDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBaseDialog, CDialog)
    ON_WM_PAINT()
    ON_WM_SIZE()
    ON_WM_LBUTTONDOWN()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL CBaseDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    RemoveBorder();
    CRect rcControl(0, 0, 0, 0);
    m_btClose.Create(NULL, WS_CHILD | WS_VISIBLE, rcControl, this, IDCANCEL);
    m_btClose.SetButtonImage(AfxGetInstanceHandle(), TEXT("BTN_CLOSE"));
    m_bIsInit = true;
    return TRUE;  // return TRUE unless you set the focus to a control
}

void CBaseDialog::RemoveBorder()
{
    DWORD dwStyle = GetStyle();
    DWORD dwNewStyle = WS_OVERLAPPED | WS_VISIBLE | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    dwNewStyle &= dwStyle;
    SetWindowLong(m_hWnd, GWL_STYLE, dwNewStyle);
    DWORD dwExStyle = GetExStyle();
    DWORD dwNewExStyle = WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR;
    dwNewExStyle &= dwExStyle;
    SetWindowLong(m_hWnd, GWL_EXSTYLE, dwNewExStyle);
}

bool CBaseDialog::LoadBackSkin(LPCTSTR pszResourcePath)
{
    return m_ImageBack.LoadImage(pszResourcePath);
}

bool CBaseDialog::LoadBackSkin(HINSTANCE hInstance, LPCTSTR pszResourceName)
{
    return m_ImageBack.LoadImage(hInstance, pszResourceName);
}

void CBaseDialog::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    CRect rcClient;
    GetClientRect(&rcClient);
    CDC BufferDC;
    CBitmap BufferBmp;
    BufferDC.CreateCompatibleDC(&dc);
    BufferBmp.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());
    BufferDC.SelectObject(&BufferBmp);
    BufferDC.FillSolidRect(&rcClient, RGB(255, 255, 255));
    m_ImageBack.DrawImage(&BufferDC, 0, 0, rcClient.Width(), rcClient.Height());
    CRect rcNewClient;
    rcNewClient.left = 0;
    rcNewClient.top = 0;
    rcNewClient.right = rcClient.right - 3;
    rcNewClient.bottom = rcClient.bottom - 3;
    CImage ImageBuffer;
    ImageBuffer.Create(rcNewClient.Width(), rcNewClient.Height(), 32);
    CImageDC ImageDC(ImageBuffer);
    CDC * pBufferDC = CDC::FromHandle(ImageDC);
    pBufferDC->SetBkMode(TRANSPARENT);
    // 	PrintWindow(pBufferDC,PW_CLIENTONLY);
    m_ImageBack.DrawImage(pBufferDC, 0, 0, rcNewClient.Width(), rcNewClient.Height(), rcNewClient.left, rcNewClient.top, rcNewClient.Width(), rcNewClient.Height());
    BufferDC.SelectObject(BufferBmp);
    OnClientDraw(pBufferDC, rcNewClient.Width(), rcNewClient.Height());
    BufferDC.BitBlt(0, 0, rcNewClient.Width(), rcNewClient.Height(), pBufferDC, 0, 0, SRCCOPY);
    dc.BitBlt(rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), &BufferDC, 0, 0, SRCCOPY);
    BufferDC.DeleteDC();
    BufferBmp.DeleteObject();
}

void CBaseDialog::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);
    
    if(!m_bIsInit) { return; }
    
    const UINT uFlags = SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOSIZE;
    LockWindowUpdate();
    HDWP hDwp = BeginDeferWindowPos(32);
    CRect rcButton;
    m_btClose.GetWindowRect(&rcButton);
    DeferWindowPos(hDwp, m_btClose, NULL, cx - rcButton.Width() - 3, 4, 0, 0, uFlags);
    EndDeferWindowPos(hDwp);
    Invalidate(FALSE);
    UpdateWindow();
    UnlockWindowUpdate();
}

void CBaseDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
    if(!m_bIsZoomed)
    {
        PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
        return;
    }
    
    CDialog::OnLButtonDown(nFlags, point);
}

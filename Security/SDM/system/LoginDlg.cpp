// LoginDlg.cpp : 实现文件
//

#include "StdAfx.h"
#include "../SDM.h"
#include "LoginDlg.h"
#include "../IpSetDlg.h"
#include "../database/DataBaseT.h"

#define IDC_BUTTON_LOGIN  WM_USER+0x020
#define IDC_BUTTON_CANCEL WM_USER+0x021
#define IDC_BUTTON_IPPORT WM_USER+0x022

int g_nManagerType = -1; //用户类型,0:管理员 1:普通用户
CString g_strCurrentUser = "";
extern CString g_strCurrentLanguage;
extern CString g_strIniFilePath;
extern CDataBaseT*	g_pDBT;
extern int g_dayRemain;
extern bool g_autoStartApp;
extern bool g_setIpPort;
extern int g_port;
extern CString g_ip;

// CLoginDlg 对话框
extern CString GetIniStringValue(CString strSection, CString strSectionKey, CString strFileName);

IMPLEMENT_DYNAMIC(CLoginDlg, CBaseDialog)

CLoginDlg::CLoginDlg(CWnd* pParent /*=NULL*/)
    : CBaseDialog(CLoginDlg::IDD, pParent)
    , m_strpwd(_T(""))
{
    m_strTitle = "Login";
}

CLoginDlg::~CLoginDlg()
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
    CBaseDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_USERNAME, m_cmbUserName);
    DDX_Text(pDX, IDC_EDIT_PASSWORD, m_strpwd);
    DDX_Control(pDX, IDC_STATIC_USER, m_loadUserImage);
}


BEGIN_MESSAGE_MAP(CLoginDlg, CBaseDialog)
    ON_WM_CTLCOLOR()
    //ON_BN_CLICKED(IDOK, &CLoginDlg::OnBnClickedOk)
    //ON_BN_CLICKED(IDCANCEL, &CLoginDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_IPPORT, OnBnClickedIpPort)
    ON_BN_CLICKED(IDC_BUTTON_LOGIN, OnBnClickedOk)
    ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


void CLoginDlg::Init(int dayRemain)
{
    m_dayRemain = dayRemain;
}

// CLoginDlg 消息处理程序
BOOL CLoginDlg::OnInitDialog()
{
    CBaseDialog::OnInitDialog();
    InitStyle();
    FillUser();
    return TRUE;  // return TRUE unless you set the focus to a control
}

void CLoginDlg::OnBnClickedIpPort()
{
	IpSetDlg ipDlg;
	CString tip = _T("");
	ipDlg.SetTipContent(tip);
	if (ipDlg.DoModal() == IDOK)
	{
		g_ip = ipDlg.m_ip;
		g_port = atoi(ipDlg.m_port);
		g_setIpPort = true;
	}
}

void CLoginDlg::OnBnClickedOk()
{
    static int count = 1;
    UpdateData();
    CString username(_T(""));
    m_cmbUserName.GetWindowText(username);
    
    if(username.IsEmpty())
    {
        GetDlgItem(IDC_COMBO_USERNAME)->SetFocus();
        AfxMessageBox(GetIniStringValue("Login", "InputUserNamePrompt", g_strCurrentLanguage)); //请输入用户名称
        return;
    }
    
    if(m_strpwd.IsEmpty())
    {
        GetDlgItem(IDC_EDIT_PASSWORD)->SetFocus();
        AfxMessageBox(GetIniStringValue("Login", "InputPasswordPrompt", g_strCurrentLanguage)); //请输入用户密码
        return;
    }
    
    if(3 > count)
    {
        if(CheckUserAndPwd(username, m_strpwd))
        {
            CBaseDialog::OnOK();
        }
        else
        {
            m_strpwd = "";
            AfxMessageBox(GetIniStringValue("Login", "LoginFailPrompt", g_strCurrentLanguage)); //登陆失败，请联系管理员
            GetDlgItem(IDC_EDIT_PASSWORD)->SetFocus();
            count++;
        }
    }
    else
    {
        AfxMessageBox(GetIniStringValue("Login", "LoginFailExitPrompt", g_strCurrentLanguage)); //登陆失败，程序即将退出，请联系管理员
        CDialog::OnCancel();
    }
    
    UpdateData(FALSE);
}

//void CLoginDlg::LoadImageLogo()
//{
//	CString logopath = "images\\other\\user.png";
//	CRect rect;
//	m_loadUserImage.GetClientRect(&rect);
//	CImage imglogo;
//	HRESULT ret = imglogo.Load(logopath);
//	if (FAILED(ret))
//	{
//		LOG(INFO) <<"Load the login image failure!";
//		return;
//	}
//	CDC* pDC = m_loadUserImage.GetWindowDC();
//	imglogo.Draw(pDC->GetSafeHdc(),rect);
//	imglogo.Destroy();
//	ReleaseDC(pDC);
//}

void CLoginDlg::InitStyle(void)
{
    HINSTANCE hInstance = AfxGetInstanceHandle();
    LoadBackSkin(hInstance, TEXT("LOGIN_DLG_BG"));
    SetWindowPos(NULL, 0, 0, 362, 228, SWP_NOMOVE);
    m_titleImage.LoadImage(hInstance, TEXT("LOGIN_DLG_TITLE"));
    m_UserImage.LoadImage(hInstance, TEXT("USER"));

	m_btnIpPort.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(12, 180, 0, 0), this, IDC_BUTTON_IPPORT);
	m_btnIpPort.SetButtonImage(hInstance, TEXT("BTN_REGISTER"));
	m_btnIpPort.SetWindowText("IP端口设置");
    m_btnLogin.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(127, 180, 0, 0), this, IDC_BUTTON_LOGIN);
    m_btnLogin.SetButtonImage(hInstance, TEXT("BTN_REGISTER"));
    m_btnLogin.SetWindowText("登入");
    m_btExit.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(242, 180, 0, 0), this, IDC_BUTTON_CANCEL);
    m_btExit.SetButtonImage(hInstance, TEXT("BTN_EXIT"));
    m_btExit.SetWindowText("退出");
}

void CLoginDlg::OnClientDraw(CDC*pDC, INT nWidth, INT nHeight)
{
    m_titleImage.DrawImage(pDC, 5, 4, 24, 24);
    m_UserImage.DrawImage(pDC, 32, 96, 51, 55);
    pDC->SetTextColor(RGB(255, 255, 255));
    CRect rcText(35, 10, 250, 26);
    DrawText(pDC, rcText, m_strTitle, DT_LEFT | DT_END_ELLIPSIS);
}

void CLoginDlg::DrawText(CDC* pDC, CRect rcText, LPCTSTR lpszText, UINT uFormat, INT nHeight/*=18*/, bool bBold/*=true*/, LPCTSTR lpszName/*=TEXT("微软雅黑")*/)
{
    CFont font;
    font.CreateFont(nHeight, 0, 0, 0, bBold ? FW_BOLD : FW_NORMAL, 0, FALSE, 0, 0, 0, 0, 0, 0, lpszName);
    CFont*  pOldFont = pDC->SelectObject(&font);
    pDC->DrawText(lpszText, &rcText, uFormat);
    pDC->SelectObject(pOldFont);
}


void CLoginDlg::OnBnClickedCancel()
{
    OnCancel();
}

void CLoginDlg::FillUser()
{
    m_cmbUserName.ResetContent();
    BOOL retVal = g_pDBT->OpenSql("CUser", "*", "", "", 10);
    
    if(retVal && g_pDBT->GetDBRecordSet()->GetRecordCount() > 0)
    {
        g_pDBT->GetDBRecordSet()->MoveFirst();
        CString strValue;
        
        while(!g_pDBT->GetDBRecordSet()->IsEOF())
        {
            g_pDBT->GetDBRecordSet()->GetValueString(strValue, "UserName");
            m_cmbUserName.AddString(strValue);
            g_pDBT->GetDBRecordSet()->MoveNext();
        }
        
        g_pDBT->GetDBRecordSet()->Close();
        m_cmbUserName.SetCurSel(0);
    }
}

bool CLoginDlg::CheckUserAndPwd(CString username, CString pwd)
{
    ASSERT(!username.IsEmpty() && !pwd.IsEmpty());
    char pWhere[200] = {0};
    sprintf(pWhere, " where UserName='%s' and Pwd='%s' ", username, pwd);
    BOOL retVal = g_pDBT->OpenSql("CUser", "*", pWhere, "");
    
    if(retVal && g_pDBT->GetDBRecordSet()->GetRecordCount() > 0)
    {
        g_strCurrentUser = username;
        g_pDBT->GetDBRecordSet()->MoveFirst();
        
        while(!g_pDBT->GetDBRecordSet()->IsEOF())
        {
            CString strValue = "";
            g_pDBT->GetDBRecordSet()->GetValueString(strValue, "RoleID");
            g_nManagerType = atoi(strValue);
            g_pDBT->GetDBRecordSet()->MoveNext();
        }
        
        g_pDBT->GetDBRecordSet()->Close();
        return true;
    }
    else
    {
        return false;
    }
}

HBRUSH CLoginDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    switch(nCtlColor)
    {
        case CTLCOLOR_BTN://按钮？
            break;
            
        case CTLCOLOR_STATIC://我们所需要改画的静态
            {
                pDC->SetBkMode(TRANSPARENT);//透明
                return  HBRUSH(GetStockObject(HOLLOW_BRUSH));
            }
            break;
            
        default:
            break;
    }
    
    return(CDialog::OnCtlColor(pDC, pWnd, nCtlColor));
}
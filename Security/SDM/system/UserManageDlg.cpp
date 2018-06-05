// UserManageDlg.cpp : 实现文件
//

#include "StdAfx.h"
#include "../SDM.h"
#include "./UserManageDlg.h"
#include "../common/DataDefine.h"
#include "../database/DataBaseT.h"

#define IDC_USERQUERY_QUERY WM_USER+0x60
#define IDC_USERQUERY_QUERYALL WM_USER+0x61
#define IDC_USERQUERY_DEL WM_USER+0x62
#define IDC_USERQUERY_CLEAR WM_USER+0x63
// CUserManageDlg 对话框
extern CDataBaseT*	g_pDBT;
extern CString g_strCurrentUser;
extern int g_nManagerType;

IMPLEMENT_DYNAMIC(CUserManageDlg, CBaseDialog)

CUserManageDlg::CUserManageDlg(CWnd* pParent /*=NULL*/)
    : CBaseDialog(CUserManageDlg::IDD, pParent)
    , m_strEditUserName(_T(""))
    , m_strEditPassword(_T(""))
    , m_strEditNewPwd(_T(""))
    , m_strEditConfirmNewPwd(_T(""))
{
    m_strTitle = "UserManage";
    m_nRoleID = 0;
}

CUserManageDlg::~CUserManageDlg()
{
}

void CUserManageDlg::DoDataExchange(CDataExchange* pDX)
{
    CBaseDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BTN_ADD, m_btnAdd);
    DDX_Control(pDX, IDC_BTN_MODIFY, m_btnModify);
    DDX_Control(pDX, IDC_BTN_DEL, m_btnDelete);
    DDX_Text(pDX, IDC_EDIT_USERNAME, m_strEditUserName);
    DDX_Text(pDX, IDC_EDIT_PASSWORD, m_strEditPassword);
    DDX_Text(pDX, IDC_EDIT_NEWPASSWORD, m_strEditNewPwd);
    DDX_Text(pDX, IDC_EDIT_CONFIRMPWD, m_strEditConfirmNewPwd);
    DDX_Control(pDX, IDC_COMBO_ROLE, m_cmbUserRole);
    DDX_Control(pDX, IDC_LIST_USERINFO, m_lstUserInfo);
}


BEGIN_MESSAGE_MAP(CUserManageDlg, CBaseDialog)
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_BTN_ADD, &CUserManageDlg::OnBnClickedAdd)
    ON_BN_CLICKED(IDC_BTN_MODIFY, &CUserManageDlg::OnBnClickedModify)
    ON_BN_CLICKED(IDC_BTN_DEL, &CUserManageDlg::OnBnClickedDel)
    ON_BN_CLICKED(IDC_BTN_QUIT, &CUserManageDlg::OnBnClickedQuit)
    ON_NOTIFY(NM_CLICK, IDC_LIST_USERINFO, &CUserManageDlg::OnNMClickListUserinfo)
    ON_BN_CLICKED(IDC_USERQUERY_QUERY, OnBnClickedAdd)
    ON_BN_CLICKED(IDC_USERQUERY_QUERYALL, OnBnClickedModify)
    ON_BN_CLICKED(IDC_USERQUERY_DEL, OnBnClickedDel)
    ON_BN_CLICKED(IDC_USERQUERY_CLEAR, OnBnClickedQuit)
END_MESSAGE_MAP()


// CUserManageDlg 消息处理程序


BOOL CUserManageDlg::OnInitDialog()
{
    CBaseDialog::OnInitDialog();
    m_nRoleID = g_nManagerType;
    InitStyle();
    InitUserListCtrl();
    FillDataAndSetFun();
    return TRUE;  // return TRUE unless you set the focus to a control
}


void CUserManageDlg::InitStyle(void)
{
    HINSTANCE hInstance = AfxGetInstanceHandle();
    LoadBackSkin(hInstance, TEXT("USERMANAGE_DLG_BG"));
    SetWindowPos(NULL, 0, 0, 700, 500, SWP_NOMOVE);
    m_titleImage.LoadImage(hInstance, TEXT("USERMANAGE_DLG_TITLE"));
    //USERMANAGE_DLG_TITLE
    CRect rectDlg;
    GetClientRect(rectDlg);//获得窗体的大小
    int bottom = rectDlg.bottom - 60;
    int left = 60;
    int interval = 150;
    m_btnQuery.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(left, bottom, 0, 0), this, IDC_USERQUERY_QUERY);
    m_btnQuery.SetButtonImage(hInstance, TEXT("BTN_REGISTER"));
    m_btnQuery.SetWindowText("添加");
    m_btnQueryAll.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(left + interval, bottom, 0, 0), this, IDC_USERQUERY_QUERYALL);
    m_btnQueryAll.SetButtonImage(hInstance, TEXT("BTN_REGISTER"));
    m_btnQueryAll.SetWindowText("修改");
    m_btnDel.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(left + 2 * interval, bottom, 0, 0), this, IDC_USERQUERY_DEL);
    m_btnDel.SetButtonImage(hInstance, TEXT("BTN_REGISTER"));
    m_btnDel.SetWindowText("删除");
    m_btnClear.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(left + 3 * interval, bottom, 0, 0), this, IDC_USERQUERY_CLEAR);
    m_btnClear.SetButtonImage(hInstance, TEXT("BTN_EXIT"));
    m_btnClear.SetWindowText("退出");
}

void CUserManageDlg::OnClientDraw(CDC*pDC, INT nWidth, INT nHeight)
{
    m_titleImage.DrawImage(pDC, 5, 4, 24, 24);
    pDC->SetTextColor(RGB(255, 255, 255));
    CRect rcText(35, 10, 250, 26);
    DrawText(pDC, rcText, m_strTitle, DT_LEFT | DT_END_ELLIPSIS);
}

void CUserManageDlg::DrawText(CDC* pDC, CRect rcText, LPCTSTR lpszText, UINT uFormat, INT nHeight/*=18*/, bool bBold/*=true*/, LPCTSTR lpszName/*=TEXT("微软雅黑")*/)
{
    CFont font;
    font.CreateFont(nHeight, 0, 0, 0, bBold ? FW_BOLD : FW_NORMAL, 0, FALSE, 0, 0, 0, 0, 0, 0, lpszName);
    CFont*  pOldFont = pDC->SelectObject(&font);
    pDC->DrawText(lpszText, &rcText, uFormat);
    pDC->SelectObject(pOldFont);
}

void CUserManageDlg::InitUserListCtrl(void)
{
    DWORD dwStyle = m_lstUserInfo.GetExtendedStyle();
    dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES |
               LVS_EX_HEADERDRAGDROP;
    m_lstUserInfo.SetExtendedStyle(dwStyle);
    m_lstUserInfo.InsertColumn(0, "ID", LVCFMT_LEFT, 0);
    m_lstUserInfo.InsertColumn(1, "用户名", LVCFMT_CENTER, 80); //用户名
    m_lstUserInfo.InsertColumn(2, "密码", LVCFMT_CENTER, 72);//密码
    m_lstUserInfo.InsertColumn(3, "级别", LVCFMT_CENTER, 100); //级别
}

void CUserManageDlg::OnBnClickedAdd()
{
    UpdateData(TRUE);
    
    if(0 == m_strEditUserName.Compare("Admin"))
    {
        AfxMessageBox(GetIniStringValue("Password", "AdminUserPrompt", g_strCurrentLanguage), eInformation); //该用户只允许进行修改密码操作
        m_strEditNewPwd = "";
        m_strEditConfirmNewPwd = "";
        UpdateData(FALSE);
        return;
    }
    
    if(m_strEditUserName.IsEmpty() || m_strEditNewPwd.IsEmpty() || m_strEditConfirmNewPwd.IsEmpty())
    {
        AfxMessageBox(GetIniStringValue("Password", "InputUserInfoPrompt", g_strCurrentLanguage), eInformation); //请输入用户信息
        return;
    }
    
    if(0 != m_strEditNewPwd.Compare(m_strEditConfirmNewPwd))
    {
        GetDlgItem(IDC_EDIT_NEWPASSWORD)->SetFocus();
        AfxMessageBox(GetIniStringValue("Password", "InputPasswordPrompt3", g_strCurrentLanguage), eInformation); //新密码与确认密码不一致，请重新输入
        m_strEditNewPwd = "";
        m_strEditConfirmNewPwd = "";
        UpdateData(FALSE);
        return;
    }
    
    BOOL retVal = FALSE;
    char pWhere[200] = {0};
    sprintf(pWhere, " where UserName='%s'", m_strEditUserName);
    retVal = g_pDBT->OpenSql("CUser", "*", pWhere);
    
    if(retVal)
    {
        CString destemp = "";
        int RoleId = m_cmbUserRole.GetCurSel();
        GetCtrlUserRole(RoleId, destemp);
        
        if(g_pDBT->GetDBRecordSet()->GetRecordCount() > 0) //用户已经存在
        {
            AfxMessageBox(GetIniStringValue("Password", "UserNameExistPrompt", g_strCurrentLanguage), eInformation); //用户已经存在
            g_pDBT->GetDBRecordSet()->Close();
            m_strEditUserName = "";
            m_strEditPassword = "";
            m_strEditNewPwd = "";
            m_strEditConfirmNewPwd = "";
            m_cmbUserRole.SetCurSel(1);
            UpdateData(FALSE);
            return;
        }
        else//添加新用户
        {
            sprintf(pWhere, "insert into CUser(UserName,Pwd,RoleID,Des) values('%s','%s',%d,'%s')", m_strEditUserName, m_strEditNewPwd, RoleId, destemp);
            retVal = g_pDBT->ExecSql(pWhere);
        }
        
        if(retVal)
        {
            AfxMessageBox(GetIniStringValue("Password", "AddSuccPrompt", g_strCurrentLanguage), eInformation); //添加成功
            int n = m_lstUserInfo.GetItemCount();
            m_lstUserInfo.InsertItem(n, _T(""), NULL);
            m_lstUserInfo.SetItemText(n, 1, m_strEditUserName);
            m_lstUserInfo.SetItemText(n, 2, "*****");
            m_lstUserInfo.SetItemText(n, 3, destemp);
        }
        else
        {
            AfxMessageBox(GetIniStringValue("Password", "AddFailPrompt", g_strCurrentLanguage)); //添加失败
        }
        
        m_strEditUserName = "";
        m_strEditPassword = "";
        m_strEditNewPwd = "";
        m_strEditConfirmNewPwd = "";
        m_cmbUserRole.SetCurSel(1);
        UpdateData(FALSE);
    }
    else
    {
        AfxMessageBox(GetIniStringValue("Password", "OpenSqlFailPrompt", g_strCurrentLanguage), eInformation); //打开数据库失败
        return;
    }
}


void CUserManageDlg::OnBnClickedModify()
{
    if(0 == m_nRoleID)
    {
        SuperModify();
    }
    else
    {
        if(OrdinaryModify())
        {
            CDialog::OnCancel();
        }
    }
}

void CUserManageDlg::SuperModify()
{
    UpdateData(TRUE);
    POSITION pos = m_lstUserInfo.GetFirstSelectedItemPosition();
    
    if((pos == NULL) || (m_strEditUserName.IsEmpty())) //选中行
    {
        AfxMessageBox(GetIniStringValue("Password", "ListSelectPrompt", g_strCurrentLanguage), eInformation); //请选中某一行列表后，再进行该操作
        m_strEditNewPwd = "";
        m_strEditConfirmNewPwd = "";
        UpdateData(FALSE);
        return;
    }
    
    if(m_strEditNewPwd.IsEmpty())
    {
        GetDlgItem(IDC_EDIT_NEWPASSWORD)->SetFocus();
        AfxMessageBox(GetIniStringValue("Password", "InputPasswordPrompt", g_strCurrentLanguage), eInformation); //请输入新密码
        return;
    }
    
    if(m_strEditConfirmNewPwd.IsEmpty())
    {
        GetDlgItem(IDC_EDIT_CONFIRMPWD)->SetFocus();
        AfxMessageBox(GetIniStringValue("Password", "InputPasswordPrompt2", g_strCurrentLanguage), eInformation); //请再次输入新密码
        return;
    }
    
    if(0 != m_strEditNewPwd.Compare(m_strEditConfirmNewPwd))
    {
        GetDlgItem(IDC_EDIT_NEWPASSWORD)->SetFocus();
        AfxMessageBox(GetIniStringValue("Password", "InputPasswordPrompt3", g_strCurrentLanguage), eInformation); //两次密码不一致
        m_strEditNewPwd = "";
        m_strEditConfirmNewPwd = "";
        UpdateData(FALSE);
        return;
    }
    
    int nItem = m_lstUserInfo.GetNextSelectedItem(pos);
    CString strUserNametemp = m_lstUserInfo.GetItemText(nItem, 1);
    
    if(0 != m_strEditUserName.Compare(strUserNametemp))
    {
        AfxMessageBox(GetIniStringValue("Password", "ListSelectPrompt", g_strCurrentLanguage), eInformation); //请选中某一行列表后，再进行该操作
        m_strEditUserName = "";
        m_strEditNewPwd = "";
        m_strEditConfirmNewPwd = "";
        UpdateData(FALSE);
        return;
    }
    
    BOOL ret = FALSE;
    char pWhere[200] = "";
    CString destemp = "";
    int RoleId = m_cmbUserRole.GetCurSel();
    GetCtrlUserRole(RoleId, destemp);
    sprintf(pWhere, "update CUser set Pwd = '%s', RoleID = %d ,Des = '%s' where UserName = '%s'", m_strEditNewPwd, RoleId, destemp, m_strEditUserName);
    ret = g_pDBT->ExecSql(pWhere);
    
    if(ret)
    {
        AfxMessageBox(GetIniStringValue("Password", "ModifySuccPrompt", g_strCurrentLanguage), eInformation); //修改成功
        m_lstUserInfo.SetItemText(nItem, 1, m_strEditUserName);
        m_lstUserInfo.SetItemText(nItem, 2, "*****");
        m_lstUserInfo.SetItemText(nItem, 3, destemp);
        m_strEditUserName = "";
        m_strEditPassword = "";
        m_strEditNewPwd = "";
        m_strEditConfirmNewPwd = "";
        m_cmbUserRole.SetCurSel(1);
        UpdateData(FALSE);
    }
    else
    {
        AfxMessageBox(GetIniStringValue("Password", "ModifyFailPrompt", g_strCurrentLanguage)); //修改失败
    }
}

bool CUserManageDlg::OrdinaryModify()
{
    UpdateData();
    
    if(m_strEditNewPwd.IsEmpty())
    {
        GetDlgItem(IDC_EDIT_NEWPASSWORD)->SetFocus();
        AfxMessageBox(GetIniStringValue("Password", "InputPasswordPrompt", g_strCurrentLanguage), eInformation); //请输入新密码
        return false;
    }
    
    if(m_strEditConfirmNewPwd.IsEmpty())
    {
        GetDlgItem(IDC_EDIT_CONFIRMPWD)->SetFocus();
        AfxMessageBox(GetIniStringValue("Password", "InputPasswordPrompt2", g_strCurrentLanguage), eInformation); //请再次输入新密码
        return false;
    }
    
    if(0 != m_strEditNewPwd.Compare(m_strEditConfirmNewPwd))
    {
        GetDlgItem(IDC_EDIT_NEWPASSWORD)->SetFocus();
        AfxMessageBox(GetIniStringValue("Password", "InputPasswordPrompt3", g_strCurrentLanguage), eInformation); //两次密码不一致
        m_strEditNewPwd = "";
        m_strEditConfirmNewPwd = "";
        UpdateData(FALSE);
        return false;
    }
    
    BOOL ret = FALSE;
    char pWhere[200] = "";
    sprintf(pWhere, "update CUser set Pwd = '%s' where UserName = '%s'", m_strEditNewPwd, m_strEditUserName);
    ret = g_pDBT->ExecSql(pWhere);
    
    if(ret)
    {
        AfxMessageBox(GetIniStringValue("Password", "ModifySuccPrompt", g_strCurrentLanguage), eInformation); //修改成功
        return true;
    }
    else
    {
        AfxMessageBox(GetIniStringValue("Password", "ModifyFailPrompt", g_strCurrentLanguage)); //修改失败
        return false;
    }
}

void CUserManageDlg::OnBnClickedDel()
{
    UpdateData(TRUE);
    
    if(0 == m_strEditUserName.Compare(g_strCurrentUser))
    {
        GetDlgItem(IDC_EDIT_USERNAME)->SetFocus();
        AfxMessageBox(GetIniStringValue("Password", "UsingUserPrompt", g_strCurrentLanguage)); //用户名正在使用，不允许进行该操作
        return;
    }
    
    POSITION pos = m_lstUserInfo.GetFirstSelectedItemPosition();
    
    if(pos == NULL)
    {
        AfxMessageBox(GetIniStringValue("Password", "ListSelectPrompt", g_strCurrentLanguage)); //请选中某个用户后，再进行该操作
        return;
    }
    
    if(0 == m_strEditUserName.Compare("Admin"))
    {
        AfxMessageBox(GetIniStringValue("Password", "AdminUserPrompt", g_strCurrentLanguage), eInformation); //该用户只允许进行修改密码
        m_strEditNewPwd = "";
        m_strEditConfirmNewPwd = "";
        UpdateData(FALSE);
        return;
    }
    
    if(m_strEditUserName.IsEmpty())
    {
        AfxMessageBox(GetIniStringValue("Password", "ListSelectPrompt", g_strCurrentLanguage), eInformation); //请选中某个用户后，再进行该操作
        return;
    }
    
    BOOL retVal = FALSE;
    char pWhere[150] = "";
    sprintf(pWhere, "Delete from CUser where UserName = '%s'", m_strEditUserName);
    retVal = g_pDBT->ExecSql(pWhere);
    int nItem = m_lstUserInfo.GetNextSelectedItem(pos);
    m_lstUserInfo.DeleteItem(nItem);
    m_strEditUserName = "";
    m_strEditPassword = "";
    m_strEditNewPwd = "";
    m_strEditConfirmNewPwd = "";
    m_cmbUserRole.SetCurSel(1);
    UpdateData(FALSE);
}


void CUserManageDlg::OnBnClickedQuit()
{
    OnCancel();
}


BOOL CUserManageDlg::GetCtrlUserRole(int id, CString &userRole)
{
    return m_CtrlUserRoleMap.Lookup(id, userRole);
}

BOOL CUserManageDlg::GetListCtrlUserRole(CString listRole, CString &cmbRole)
{
    return m_ListUserRoleMap.Lookup(listRole, cmbRole);
}
void CUserManageDlg::FillDataAndSetFun()
{
    CString strDestemp1 = "管理员";//管理员
    CString strDestemp2 = "普通用户";//普通用户
    m_cmbUserRole.AddString(strDestemp1);
    m_cmbUserRole.AddString(strDestemp2);
    m_CtrlUserRoleMap[0] = strDestemp1;
    m_CtrlUserRoleMap[1] = strDestemp2;
    m_ListUserRoleMap[strDestemp1] = "0";     //列表控件角色
    m_ListUserRoleMap[strDestemp2] = "1";     //列表控件角色
    char pWhere[100] = "";
    
    if(1 == m_nRoleID) //普通用户
    {
        m_btnAdd.EnableWindow(FALSE);
        m_btnDelete.EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_USERNAME)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_PASSWORD)->EnableWindow(FALSE);
        m_cmbUserRole.EnableWindow(FALSE);
        sprintf(pWhere, "where UserName = '%s'", m_strEditUserName);
    }
    else//管理员
    {
        if(0 == m_strEditUserName.Compare("Admin"))
        {
            GetDlgItem(IDC_EDIT_USERNAME)->EnableWindow(FALSE);
            m_cmbUserRole.EnableWindow(FALSE);
        }
        
        sprintf(pWhere, "order by ID ASC");
    }
    
    UpdateUserList(pWhere);
    m_cmbUserRole.SetCurSel(m_nRoleID);
    UpdateData(FALSE);
}

void CUserManageDlg::UpdateUserList(LPCTSTR sqlstr)
{
    m_lstUserInfo.DeleteAllItems();
    BOOL retVal = FALSE;
    retVal = g_pDBT->OpenSql("CUser", "*", sqlstr);
    int n = m_lstUserInfo.GetItemCount();
    
    if(retVal && g_pDBT->GetDBRecordSet()->GetRecordCount() > 0)
    {
        g_pDBT->GetDBRecordSet()->MoveFirst();
        
        while(!g_pDBT->GetDBRecordSet()->IsEOF())
        {
            CString strValue = "";
            m_lstUserInfo.InsertItem(n, "");
            g_pDBT->GetDBRecordSet()->GetValueString(strValue, "ID");
            m_lstUserInfo.SetItemText(n, 0, strValue);
            g_pDBT->GetDBRecordSet()->GetValueString(strValue, "UserName");
            m_lstUserInfo.SetItemText(n, 1, strValue);
            g_pDBT->GetDBRecordSet()->GetValueString(strValue, "Pwd");
            m_lstUserInfo.SetItemText(n, 2, "*****");
            g_pDBT->GetDBRecordSet()->GetValueString(strValue, "RoleID");
            CString temp = "";
            GetCtrlUserRole(atoi(strValue), temp);
            m_lstUserInfo.SetItemText(n, 3, temp);
            g_pDBT->GetDBRecordSet()->MoveNext();
            n++;
        }
        
        g_pDBT->GetDBRecordSet()->Close();
    }
}

void CUserManageDlg::OnNMClickListUserinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
    //LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<NMITEMACTIVATE>(pNMHDR);
    
    //*pResult = 0;
    if(1 == m_nRoleID)
    {
        return;
    }
    
    POSITION pos = m_lstUserInfo.GetFirstSelectedItemPosition();
    
    if(pos == NULL)
    {
        return;
    }
    
    int nItem = m_lstUserInfo.GetNextSelectedItem(pos);
    m_strEditUserName = m_lstUserInfo.GetItemText(nItem, 1);
    
    if(0 == m_strEditUserName.Compare("Admin"))
    {
        GetDlgItem(IDC_EDIT_USERNAME)->EnableWindow(FALSE);
        m_cmbUserRole.EnableWindow(FALSE);
    }
    else
    {
        GetDlgItem(IDC_EDIT_USERNAME)->EnableWindow(TRUE);
        m_cmbUserRole.EnableWindow(TRUE);
    }
    
    m_strEditPassword = m_lstUserInfo.GetItemText(nItem, 2);
    CString strvalue = "";
    GetListCtrlUserRole(m_lstUserInfo.GetItemText(nItem, 3), strvalue);
    m_cmbUserRole.SetCurSel(atoi(strvalue));
    UpdateData(FALSE);
}

HBRUSH CUserManageDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
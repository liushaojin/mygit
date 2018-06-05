#include "StdAfx.h"
#include "../SDM.h"
#include "./DevManageDlg.h"
#include "../common/DataDefine.h"
#include "./database/DataBaseT.h"

#define IDC_DEVQUERY_QUERY WM_USER+0x70
#define IDC_DEVQUERY_QUERYALL WM_USER+0x71
#define IDC_DEVQUERY_DEL WM_USER+0x72
#define IDC_DEVQUERY_CLEAR WM_USER+0x73

extern CDataBaseT* g_pDBT;
extern CString g_strCurrentUser;
extern int g_nManagerType;

IMPLEMENT_DYNAMIC(CDevManageDlg, CBaseDialog)


CDevManageDlg::CDevManageDlg(CWnd* pParent /*=NULL*/)
    : CBaseDialog(CDevManageDlg::IDD, pParent)
{
    m_strTitle = "DevManage";
}

CDevManageDlg::~CDevManageDlg()
{
}

void CDevManageDlg::DoDataExchange(CDataExchange* pDX)
{
    CBaseDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BTN_ADD, m_btnAdd);
    DDX_Control(pDX, IDC_BTN_MODIFY, m_btnModify);
    DDX_Control(pDX, IDC_BTN_DEL, m_btnDelete);
    DDX_Control(pDX, IDC_ID_LBL, m_idLbl);
    DDX_Control(pDX, IDC_NAME_LBL, m_nameLbl);
    DDX_Control(pDX, IDC_ID_COMBO, m_idCombo);
    DDX_Control(pDX, IDC_NAME_EDIT, m_nameEdit);
    DDX_Control(pDX, IDC_REMARK_GROUP, m_remarkGrp);
    DDX_Control(pDX, IDC_REMARK_EDIT, m_remarkEdit);
    DDX_Control(pDX, IDC_BTN_QUIT, m_btnQuit);
}


BEGIN_MESSAGE_MAP(CDevManageDlg, CBaseDialog)
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_BTN_ADD, &CDevManageDlg::OnBnClickedAdd)
    ON_BN_CLICKED(IDC_BTN_MODIFY, &CDevManageDlg::OnBnClickedModify)
    ON_BN_CLICKED(IDC_BTN_DEL, &CDevManageDlg::OnBnClickedDel)
    ON_BN_CLICKED(IDC_BTN_QUIT, &CDevManageDlg::OnBnClickedQuit)
    ON_CBN_SELCHANGE(IDC_ID_COMBO, &CDevManageDlg::OnCbnSelchangeIdCombo)
    ON_BN_CLICKED(IDC_DEVQUERY_QUERY, OnBnClickedAdd)
    ON_BN_CLICKED(IDC_DEVQUERY_QUERYALL, OnBnClickedModify)
    ON_BN_CLICKED(IDC_DEVQUERY_DEL, OnBnClickedDel)
    ON_BN_CLICKED(IDC_DEVQUERY_CLEAR, OnBnClickedQuit)
END_MESSAGE_MAP()


BOOL CDevManageDlg::OnInitDialog()
{
    CBaseDialog::OnInitDialog();
    InitStyle();
    InitData();
    return TRUE;
}


void CDevManageDlg::InitStyle(void)
{
    HINSTANCE hInstance = AfxGetInstanceHandle();
    LoadBackSkin(hInstance, TEXT("USERMANAGE_DLG_BG"));
    SetWindowPos(NULL, 0, 0, 700, 500, SWP_NOMOVE);
    m_titleImage.LoadImage(hInstance, TEXT("USERMANAGE_DLG_TITLE"));
    CRect rect;
    GetClientRect(rect);
    int left = rect.left;
    int top = rect.top;
    int right = rect.right;
    int bottom = rect.bottom;
    int width = rect.Width();
    int height = rect.Height();
    int lblHeight = 20;
    int lblTop = top + 50;
    int btnHeight = 40;
    int btnTop = bottom - 50;
    m_idLbl.MoveWindow(left + 20, lblTop, width / 6, lblHeight);
    m_idCombo.MoveWindow(width / 6 + 20, lblTop, width / 3 - 40, lblHeight);
    m_nameLbl.MoveWindow(width / 2 + 20, lblTop, width / 6, lblHeight);
    m_nameEdit.MoveWindow(width * 2 / 3 + 20, lblTop, width / 3 - 40, lblHeight);
    m_remarkGrp.MoveWindow(left + 20, top + 80, width - 40, height - 140);
    m_remarkEdit.MoveWindow(left + 30, top + 100, width - 60, height - 170);
    m_btnAdd.MoveWindow(width / 9, btnTop, width / 9, btnHeight);
    m_btnModify.MoveWindow(width * 3 / 9, btnTop, width / 9, btnHeight);
    m_btnDelete.MoveWindow(width * 5 / 9, btnTop, width / 9, btnHeight);
    m_btnQuit.MoveWindow(width * 7 / 9, btnTop, width / 9, btnHeight);
    bottom = rect.bottom - 45;
    left = 65;
    int interval = 150;
    m_btnQuery.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(left, bottom, 0, 0), this, IDC_DEVQUERY_QUERY);
    m_btnQuery.SetButtonImage(hInstance, TEXT("BTN_REGISTER"));
    m_btnQuery.SetWindowText("添加");
    m_btnQueryAll.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(left + interval, bottom, 0, 0), this, IDC_DEVQUERY_QUERYALL);
    m_btnQueryAll.SetButtonImage(hInstance, TEXT("BTN_REGISTER"));
    m_btnQueryAll.SetWindowText("修改");
    m_btnDel.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(left + 2 * interval, bottom, 0, 0), this, IDC_DEVQUERY_DEL);
    m_btnDel.SetButtonImage(hInstance, TEXT("BTN_REGISTER"));
    m_btnDel.SetWindowText("删除");
    m_btnClear.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(left + 3 * interval, bottom, 0, 0), this, IDC_DEVQUERY_CLEAR);
    m_btnClear.SetButtonImage(hInstance, TEXT("BTN_EXIT"));
    m_btnClear.SetWindowText("退出");
}

void CDevManageDlg::OnClientDraw(CDC*pDC, INT nWidth, INT nHeight)
{
    m_titleImage.DrawImage(pDC, 5, 4, 24, 24);
    pDC->SetTextColor(RGB(255, 255, 255));
    CRect rcText(35, 10, 250, 26);
    DrawText(pDC, rcText, m_strTitle, DT_LEFT | DT_END_ELLIPSIS);
}

void CDevManageDlg::DrawText(CDC* pDC, CRect rcText, LPCTSTR lpszText, UINT uFormat, INT nHeight/*=18*/, bool bBold/*=true*/, LPCTSTR lpszName/*=TEXT("微软雅黑")*/)
{
    CFont font;
    font.CreateFont(nHeight, 0, 0, 0, bBold ? FW_BOLD : FW_NORMAL, 0, FALSE, 0, 0, 0, 0, 0, 0, lpszName);
    CFont*  pOldFont = pDC->SelectObject(&font);
    pDC->DrawText(lpszText, &rcText, uFormat);
    pDC->SelectObject(pOldFont);
}

void CDevManageDlg::OnBnClickedAdd()
{
    CString id;
    CString name;
    CString remark;
    int nIndex = m_idCombo.GetCurSel();
    m_idCombo.GetLBText(nIndex, id);
    
    if(id == _T(""))
    {
        return;
    }
    
    GetDlgItem(IDC_NAME_EDIT)->GetWindowText(name);
    GetDlgItem(IDC_REMARK_EDIT)->GetWindowText(remark);
    g_pDBT->AddNewBeiZhu(id, name, remark);
    InitData();
}


void CDevManageDlg::OnBnClickedModify()
{
    CString id;
    CString name;
    CString remark;
    int nIndex = m_idCombo.GetCurSel();
    m_idCombo.GetLBText(nIndex, id);
    GetDlgItem(IDC_NAME_EDIT)->GetWindowText(name);
    GetDlgItem(IDC_REMARK_EDIT)->GetWindowText(remark);
    g_pDBT->AddNewBeiZhu(id, name, remark);
}

void CDevManageDlg::OnBnClickedDel()
{
    if(MessageBox("Confirm delete?", "Delete confirm", MB_YESNO | MB_DEFBUTTON1) == IDNO)
    {
        return;
    }
    
    CString idname;
    int nIndex = m_idCombo.GetCurSel();
    m_idCombo.GetLBText(nIndex, idname);
    g_pDBT->DelDevFromDb(idname);
    InitData();
}


void CDevManageDlg::OnBnClickedQuit()
{
    OnCancel();
}


void CDevManageDlg::InitData()
{
    CString str;
    char pWhere[150] = "";
    char pOrder[30] = "";
    LPCTSTR TableName = "DEVICES";
    m_idCombo.ResetContent();
    
    //char pWhereTemp[100] = "";
    //sprintf(pWhereTemp, "where COMID='%s'", name);
    
    if(g_pDBT->OpenSql(TableName, "*", pWhere, pOrder))
    {
        int nItem = g_pDBT->GetDBRecordSet()->GetRecordCount();
        
        //allreocd = nItem;
        if(nItem > 0)
        {
            g_pDBT->GetDBRecordSet()->MoveFirst();
            //int ID;
            CString ID;
            
            //更新内容
            while(!g_pDBT->GetDBRecordSet()->IsEOF())
            {
                g_pDBT->GetDBRecordSet()->GetValueString(ID, cnt_Endevices[0]);
                m_idCombo.AddString(ID);
                g_pDBT->GetDBRecordSet()->MoveNext();
            }
        }
        
        g_pDBT->GetDBRecordSet()->Close();
    }
    
    m_idCombo.SetCurSel(0);
    OnCbnSelchangeIdCombo();
    UpdateData(FALSE);
}

void CDevManageDlg::OnCbnSelchangeIdCombo()
{
    CString name;
    int nSel = m_idCombo.GetCurSel();
    m_idCombo.GetLBText(nSel, name);
    char pWhere[150] = "";
    char pOrder[30] = "";
    LPCTSTR TableName = "DEVICES";
    sprintf(pWhere, "where COMID='%s'", name);
    
    if(g_pDBT->OpenSql(TableName, "*", pWhere, pOrder))
    {
        int nItem = g_pDBT->GetDBRecordSet()->GetRecordCount();
        
        if(nItem > 0)
        {
            g_pDBT->GetDBRecordSet()->MoveFirst();
            CString ID, name, remark;
            
            //更新内容
            while(!g_pDBT->GetDBRecordSet()->IsEOF())
            {
                g_pDBT->GetDBRecordSet()->GetValueString(ID, cnt_Endevices[0]);
                g_pDBT->GetDBRecordSet()->GetValueString(name, cnt_Endevices[1]);
                g_pDBT->GetDBRecordSet()->GetValueString(remark, cnt_Endevices[2]);
                m_nameEdit.SetWindowText(name);
                m_remarkEdit.SetWindowText(remark);
                g_pDBT->GetDBRecordSet()->MoveNext();
            }
        }
        
        g_pDBT->GetDBRecordSet()->Close();
    }
    
    //UpdateData(FALSE);
}

HBRUSH CDevManageDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
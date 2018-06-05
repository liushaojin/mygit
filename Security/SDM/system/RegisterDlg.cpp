#include "StdAfx.h"
#include <string>
#include <sstream>
#include "../SDM.h"
#include "./RegisterDlg.h"	//本类头文件的引用位置，会影响程序的编译或处理
#include "./Des/MemoryStream.h"
#include "./Des/DES.h"
#include "./Des/CRC32.h"
#include "./common/DataDefine.h"

#define WM_CURRENTTIME      WM_USER+0x010
#define IDC_BUTTON_REGISTER WM_USER+0x011
#define IDC_BUTTON_EXIT     WM_USER+0x012

extern int g_dayRemain;
extern bool g_bIsTimeExpired;
extern bool g_bIsRegistered;
extern CString GetExePath();
extern CString g_strCurrentLanguage;
extern CString g_strIniFilePath;

extern CString GetIniStringValue(CString strSection, CString strSectionKey, CString strFileName);

IMPLEMENT_DYNAMIC(CRegisterDlg, CBaseDialog)

CRegisterDlg::CRegisterDlg(CWnd* pParent /*=NULL*/)
    : CBaseDialog(CRegisterDlg::IDD, pParent)
    , m_strEditSN(_T(""))
{
    m_strTitle = "Register";
}

CRegisterDlg::~CRegisterDlg()
{
}

void CRegisterDlg::DoDataExchange(CDataExchange* pDX)
{
    CBaseDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_MACHINECODE, m_strMCode);
    DDX_Text(pDX, IDC_EDIT_SN, m_strEditSN);
}

//事件映射
BEGIN_MESSAGE_MAP(CRegisterDlg, CBaseDialog)
    ON_WM_CTLCOLOR()
    //ON_BN_CLICKED(IDOK, &CRegisterDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDC_BUTTON_REGISTER, OnBnClickedOk)
    //ON_BN_CLICKED(IDCANCEL, &CRegisterDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_BUTTON_EXIT, OnBnClickedCancel)
    ON_WM_TIMER()
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CRegisterDlg 消息处理程序

BOOL CRegisterDlg::OnInitDialog()
{
    CBaseDialog::OnInitDialog();
    InitStyle();
    SetTimer(WM_CURRENTTIME, 1000, NULL);
    GetSNByNIC();
    initRegStatus();
    return TRUE;  // return TRUE unless you set the focus to a control
}

void CRegisterDlg::OnBnClickedOk()
{
    UpdateData(TRUE);
    static int count = 1;
    
    if(m_strEditSN.IsEmpty() || m_strEditSN.GetLength() <= 16)
    {
        MessageBoxA(GetIniStringValue("Register", "RegCodePrompt", g_strCurrentLanguage)); //请输入有效的注册码
        m_strEditSN = "";
        UpdateData(FALSE);
        return;
    }
    
    if(g_bIsRegistered)
    {
        //说明用户是第二次注册
        //分析注册码的有效性,进行注册
        WriteLicenseInformation(m_strEditSN.Trim());
        
        if(g_bIsTimeExpired)
        {
            MessageBoxA(GetIniStringValue("Register", "RegCodeErrorPrompt", g_strCurrentLanguage)); //注册码非法，请获取正确的注册码
            count ++;
            m_strEditSN = "";
            UpdateData(FALSE);
        }
        else
        {
            CDialog::OnOK();
        }
    }
    else
    {
        //用户首次注册
        //分析注册码的有效性,进行注册
        WriteLicenseInformation(m_strEditSN.Trim());
        
        if(g_bIsTimeExpired)
        {
            //注册码无效
            MessageBoxA(GetIniStringValue("Register", "RegCodeErrorPrompt", g_strCurrentLanguage)); //注册码非法，请获取正确的注册码
            count ++;
            m_strEditSN = "";
            UpdateData(FALSE);
        }
        else
        {
            //写注册表
            DWORD dwtemp = 1;
            int RetRegValue = CPublicFunction::SetRegValue("RegisterData", dwtemp);
            
            if(0 == RetRegValue)
            {
                CDialog::OnOK();
            }
            
            if(1 == RetRegValue)
            {
                MessageBoxA(GetIniStringValue("Register", "RegCodePromptNoAuthority", g_strCurrentLanguage)); //注册失败，你不是超级管理员
                CDialog::OnCancel();
            }
            
            if(2 == RetRegValue)
            {
                MessageBoxA(GetIniStringValue("Register", "RegCodePromptFail", g_strCurrentLanguage)); //注册失败，你不是超级管理员
                CDialog::OnCancel();
            }
        }
    }
    
    if(count > 3)
    {
        MessageBoxA(GetIniStringValue("Register", "InputRegCodeErrorExitPrompt", g_strCurrentLanguage)); //您已连续输入非法的注册码超过三次，系统将退出，请联系管理员
        CDialog::OnCancel();
    }
}

void CRegisterDlg::OnTimer(UINT_PTR nIDEvent)
{
    if(WM_CURRENTTIME == nIDEvent)
    {
        CString strTime = getCurrentDateTime();
        GetDlgItem(IDC_STATIC_TIME_DISPLAY)->SetWindowText(strTime);
    }
    
    CBaseDialog::OnTimer(nIDEvent);
}

CString CRegisterDlg::getCurrentDateTime()
{
    CTime cTime = CTime::GetCurrentTime();
    CString strCurrentTime(_T(""));
    strCurrentTime.Format("%04d-%.2d-%.2d %.2d:%.2d:%.2d",
                          cTime.GetYear(), cTime.GetMonth(), cTime.GetDay(),
                          cTime.GetHour(), cTime.GetMinute(), cTime.GetSecond());
    return strCurrentTime;
}

//初始化注册状态
void CRegisterDlg::initRegStatus(void)
{
    //CSDMRegister sdmRegister;
    //LONG64 nDay = sdmRegister.GetDayRemain();
    LONG64 nDay = g_dayRemain;
    CString strMsg = "";
    
    if(g_bIsRegistered)
    {
        if(nDay <= 30)
        {
            strMsg.Format("Registration period only:%ld (Days)", nDay);
        }
        else
        {
            strMsg = "Registered";
        }
    }
    else
    {
        strMsg = (0 > nDay) ? "Expired" : "Not registered";
    }
    
    GetDlgItem(IDC_STATIC_STATUS_DISPLAY)->SetWindowText(strMsg);
}

HBRUSH CRegisterDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    //HBRUSH hbr = CBaseDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    //
    //if(pWnd->GetDlgCtrlID() == IDC_STATIC_STATUS_DISPLAY)
    //{
    //    CFont font;
    //    //if (0==g_strCurrentLanguage.CompareNoCase("Chinese"))
    //    font.CreatePointFont(160, "隶书");
    //    //else
    //    //font.CreatePointFont(165,"Times New Roman");
    //    pDC->SelectObject(&font);
    //    pDC->SetBkMode(TRANSPARENT);
    //    pDC->SetTextColor(RGB(255, 0, 0));
    //    return (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    //}
    //
    //return hbr;
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

void CRegisterDlg::InitStyle(void)
{
    HINSTANCE hInstance = AfxGetInstanceHandle();
    LoadBackSkin(hInstance, TEXT("REGISTER_DLG_BG"));
    SetWindowPos(NULL, 0, 0, 555, 280, SWP_NOMOVE);
    m_titleImage.LoadImage(hInstance, TEXT("REGISTER_DLG_TITLE"));
    m_btnReg.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(125, 215, 0, 0), this, IDC_BUTTON_REGISTER);
    m_btnReg.SetButtonImage(hInstance, TEXT("BTN_REGISTER"));
    m_btnReg.SetWindowText("Register");
    m_btExit.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(320, 215, 0, 0), this, IDC_BUTTON_EXIT);
    m_btExit.SetButtonImage(hInstance, TEXT("BTN_EXIT"));
    m_btExit.SetWindowText("Cancel");
}

void CRegisterDlg::OnClientDraw(CDC*pDC, INT nWidth, INT nHeight)
{
    m_titleImage.DrawImage(pDC, 5, 4, 24, 24);
    pDC->SetTextColor(RGB(255, 255, 255));
    CRect rcText(35, 10, 250, 26);
    DrawText(pDC, rcText, m_strTitle, DT_LEFT | DT_END_ELLIPSIS);
}

void CRegisterDlg::DrawText(CDC* pDC, CRect rcText, LPCTSTR lpszText, UINT uFormat, INT nHeight/*=18*/, bool bBold/*=true*/, LPCTSTR lpszName/*=TEXT("微软雅黑")*/)
{
    CFont font;
    font.CreateFont(nHeight, 0, 0, 0, bBold ? FW_BOLD : FW_NORMAL, 0, FALSE, 0, 0, 0, 0, 0, 0, lpszName);
    CFont*  pOldFont = pDC->SelectObject(&font);
    pDC->DrawText(lpszText, &rcText, uFormat);
    pDC->SelectObject(pOldFont);
}


void CRegisterDlg::OnBnClickedCancel()
{
    OnCancel();
}

void CRegisterDlg::GetSNByNIC()
{
    unsigned long code = GetHardwareNumber();
    CString str;
    str.Format("%u", code);
    CString msg;
    msg.Format("%s\r\n", str);
    int selStart, selEnd;
    
    if(m_strMCode.GetLineCount() > 800)
    {
        selStart = m_strMCode.LineIndex(0);
        selEnd = m_strMCode.LineIndex(400);
        m_strMCode.SetSel(selStart, selEnd);
        m_strMCode.ReplaceSel("");
    }
    
    selStart = m_strMCode.LineIndex(m_strMCode.GetLineCount() - 1);
    m_strMCode.SetSel(selStart, selStart);
    m_strMCode.ReplaceSel(msg);
}

void CRegisterDlg::WriteLicenseInformation(CString License)
{
    LOGMESSAGE("CDialogReg::WriteLicenseInformation", "Enter the check the RegCode module!");
    unsigned long number = GetHardwareNumber();
    std::ostringstream stm;
    std::string strResult;
    stm << number;
    strResult = stm.str();
    TDES des;
    des.MakeKey(strResult);
    License.Trim();
    int len = License.GetLength();
    TMemoryStream stream;
    stream.SetSize(len);
    stream.SetPosition(0);
    stream.Write(License.GetString(), len);
    TMemoryStream dest;
    
    if(des.DecryptNumber(stream, dest) == true)
    {
        std::string decry(dest.Memory());
        int last = decry.find_first_of(':');
        
        if(last < 4)
        {
            g_bIsTimeExpired = true;
            LOGMESSAGE("CDialogReg::WriteLicenseInformation", " The RegCode is invalid!");
            return;
        }
        
        int posHead = last + 1;
        
        if(last == std::string::npos)
        {
            last = 0;
        }
        else
        {
            last++;
        }
        
        decry.resize(last - 1);
        last = decry.find_last_not_of('\0');
        
        if(last == std::string::npos)
        {
            last = 0;
        }
        else
        {
            last++;
        }
        
        decry.resize(last);
        
        if(decry != strResult || posHead < 2)
        {
            g_bIsTimeExpired = true;
            LOGMESSAGE("CDialogReg::WriteLicenseInformation", " The RegCode is invalid!");
            return;
        }
        
        //分析授权码的时间段
        char* pData = (char*)dest.Memory();
        pData = pData + posHead;
        TModuleData* pModuleData = (TModuleData*)pData;
        TDateTime minDate = TDateTime::OriginDateTime();
        TDateTime nowDateTime = TDateTime::GetCurrentTime();
        TDateTime startTime = minDate + (int)pModuleData->StartTime;
        pModuleData->Day += 1;
        TDateTime endTime = startTime + (int)pModuleData->Day;
        int CheckDay = (int)pModuleData->Day;
        int year = endTime.GetYear() - minDate.GetYear();
        
        if(year > 30)
        {
            g_bIsTimeExpired = true;
            LOGMESSAGE("CDialogReg::WriteLicenseInformation", " The RegCode is invalid!");
            return;
        }
        
        if(nowDateTime > endTime)
        {
            g_bIsTimeExpired = true;
            LOGMESSAGE("CDialogReg::WriteLicenseInformation", " The RegCode is invalid!");
            return;
        }
        
        if(nowDateTime < startTime)
        {
            g_bIsTimeExpired = true;
            LOGMESSAGE("CDialogReg::WriteLicenseInformation", " The RegCode is invalid!");
            return;
        }
        
        int tempday = static_cast<int>(endTime) - static_cast<int>(nowDateTime);
        
        if(CheckDay < 1 || tempday <= 0)
        {
            g_bIsTimeExpired = true;
            LOGMESSAGE("CDialogReg::WriteLicenseInformation", " The RegCode is invalid!");
            return;
        }
        
        LOGMESSAGE("CDialogReg::WriteLicenseInformation", "The RegCode is valid!");
        g_dayRemain = tempday;
        CString strlicPath = GetExePath() + "\\License.lic";
        LOGMESSAGE("CDialogReg::WriteLicenseInformation", "Enter the write license file module!");
        BOOL retVal = CPublicFunction::WriteCodeToFile(License, strlicPath);
        
        if(!retVal)
        {
            LOGMESSAGE("CDialogReg::WriteLicenseInformation", "Write the license file failure!");
            g_bIsTimeExpired = true;
            return;
        }
        
        LOGMESSAGE("CDialogReg::WriteLicenseInformation", "Write the license file success!");
        g_bIsTimeExpired = false;
        CPublicFunction::WriteExitTimeReg();
    }
    else
    {
        LOGMESSAGE("CDialogReg::WriteLicenseInformation", " The RegCode is invalid!");
        g_bIsTimeExpired = true;
    }
}


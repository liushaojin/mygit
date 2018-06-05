
// SDMDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SDM.h"
#include "SDMDlg.h"
#include "IpSetDlg.h"
//#include "afxdialogex.h"
#include "./database/DataExport.h"
#include "./system/DataQueryDlg.h"
#include "./system/UserManageDlg.h"
#include "./system/DevManageDlg.h"
#include "./system/SystemSetDlg.h"
#include "./system/DataCount.h"
//#include "./common/CreateDump.h"
#include "./database/IniHelper.h"

//static UINT WM_DELETE_UPDATE = RegisterWindowMessage(_T("User"));
extern LONG CrashHandler(EXCEPTION_POINTERS *pException);
extern CDataBaseT*  g_pDBT;
extern CIOCPServer *g_server;
extern int g_currentDoorNum;
extern BYTE g_address[];
extern BYTE  g_curDoorId;
extern ClientInfo *g_CurClientInfo;
extern bool g_setIpPort;
extern int g_port;
extern CString g_ip;
//extern CString GetExePath();

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

#define IDC_BUTTON_DATAQUERY	WM_USER+0x032
#define IDC_BUTTON_USERMANAGE	WM_USER+0x033
#define IDC_BUTTON_DEVMANAGE	WM_USER+0x034
#define IDC_BUTTON_SYSTEMSET	WM_USER+0x035
#define IDC_BUTTON_DATACOUNT	WM_USER+0x036
#define IDC_BUTTON_EXIT_SYSTEM	WM_USER+0x037
#define IDC_BUTTON_EXPORT		WM_USER+0x038

#define TIMER_UPDATA     10
#define POLL_TIME 1000


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CBaseDialog
{
    public:
        CAboutDlg();
        
// 对话框数据
        enum { IDD = IDD_ABOUTBOX };
        
    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
        
// 实现
    protected:
        DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CBaseDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CBaseDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CBaseDialog)
END_MESSAGE_MAP()


// CSDMDlg 对话框


CSDMDlg::CSDMDlg(CWnd* pParent /*=NULL*/)
    : CBaseDialog(CSDMDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_strTitle = "Security Door Management Platform";
    m_clientNum = 0;
    m_totalPassedToday = 0;
    m_totalAlarmedToday = 0;
    m_alarmStatus[0] = 0;
    m_alarmStatus[1] = 0;
    m_alarmStatus[2] = 0;
    m_alarmStatus[3] = 00;
    m_alarmStatus[4] = 0;
    m_alarmStatus[5] = 00;
    m_alarmStatus[6] = 0;
    m_alarmStatus[7] = 00;
    m_alarmStatus[8] = 0;
    m_alarmStatus[9] = 00;
    m_alarmStatus[10] = 0;
    m_alarmStatus[11] = 00;
    m_alarmStatus[12] = 0;
    m_alarmStatus[13] = 0;
    m_alarmStatus[14] = 0;
    m_alarmStatus[15] = 0;
    m_areaCnt = 0;
    m_isFirstEnter = TRUE;
}

void CSDMDlg::DoDataExchange(CDataExchange* pDX)
{
    CBaseDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LOG_LIST, m_LogListBox);
    DDX_Control(pDX, IDC_STATIC_SDDATA, m_SDDataCtrl);
    DDX_Control(pDX, IDC_LIST_DEVINFO, m_DevInfoList);
    DDX_Control(pDX, IDC_DEV_TREE, m_DevTree);
}

BEGIN_MESSAGE_MAP(CSDMDlg, CBaseDialog)
    ON_WM_CLOSE()
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_TIMER()
    ON_WM_QUERYDRAGICON()
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_BUTTON_DATAQUERY, OnBnClickedDataQuery)
    ON_BN_CLICKED(IDC_BUTTON_USERMANAGE, OnBnClickedUserManage)
    ON_BN_CLICKED(IDC_BUTTON_DEVMANAGE, OnBnClickedDevManage)
    ON_BN_CLICKED(IDC_BUTTON_SYSTEMSET, OnBnClickedSystemSet)
	ON_BN_CLICKED(IDC_BUTTON_DATACOUNT, OnBnClickedCountInfo)
    ON_BN_CLICKED(IDC_BUTTON_EXIT_SYSTEM, OnBnClickedExit)
    ON_BN_CLICKED(IDC_BUTTON_EXPORT, OnBnClickedExport)
    ON_NOTIFY(NM_CLICK, IDC_LIST_DEVINFO, &CSDMDlg::OnNMClickListDevinfo)
    ON_NOTIFY(TVN_SELCHANGED, IDC_DEV_TREE, &CSDMDlg::OnTvnSelchangedDevTree)
    ON_MESSAGE(WM_ALARM_DATA_RECEIVED, OnUpdateTable)
    ON_MESSAGE(WM_DEV_INFO_RECEIVED, OnGetDoorId)
    ON_MESSAGE(WM_DELETE_UPDATE, OnDeleteUpdate)
    //ON_REGISTERED_MESSAGE(WM_DELETE_UPDATE, OnDeleteUpdate)
END_MESSAGE_MAP()


// CSDMDlg 消息处理程序

BOOL CSDMDlg::OnInitDialog()
{
    CBaseDialog::OnInitDialog();
    // 将“关于...”菜单项添加到系统菜单中。
    // IDM_ABOUTBOX 必须在系统命令范围内。
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);
    CMenu* pSysMenu = GetSystemMenu(FALSE);
    
    if (pSysMenu != NULL)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }
    
    // 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
    SetIcon(m_hIcon, TRUE);         // 设置大图标
    SetIcon(m_hIcon, FALSE);        // 设置小图标
    InitNetWork();
    InitStyle();
    InitTreeCtrl();
    InitDevinfoList();
    GetDataFormDB();
    ReadCountInfo();
	GetDataFormCountTb();
    //DeviceSync(); //同步处理
    SetTimer(TIMER_UPDATA, POLL_TIME, NULL);    //60ms轮询一次
    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CSDMDlg::ReadCountInfo()
{
    CTime curTime;
    curTime = CTime::GetCurrentTime();             //获取当前时间日期
    CString strDate = curTime.Format(_T("%Y%m%d"));
    CString rdate, rpassed, ralarmed;
    rdate = IniHelper::Instance()->GetIniStringValue("Count", "Date");
    rpassed = IniHelper::Instance()->GetIniStringValue("Count", "Passed");
    ralarmed = IniHelper::Instance()->GetIniStringValue("Count", "Alarmed");
    int curDate = atoi(strDate);
    int rDate = atoi(rdate);
    
    if (rDate == curDate)
    {
        m_totalPassedToday = atoi(rpassed);
        m_totalAlarmedToday = atoi(ralarmed);   //启动上位机时先读取当天已经通过的及报警次数

        GetDlgItem(IDC_STATIC_PASSED_SHOW)->SetWindowText(rpassed);
        GetDlgItem(IDC_STATIC_ALARMCNT_SHOW)->SetWindowText(ralarmed);
    }
}

void CSDMDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CBaseDialog::OnSysCommand(nID, lParam);
    }
}

void CSDMDlg::OnClose(/*UINT nID, LPARAM lParam*/)
{

}

HBRUSH CSDMDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    //HBRUSH brush = CreateSolidBrush(RGB(127, 0, 0));
    CFont font;
    font.CreatePointFont(500, _T("宋体"));
    
    // TODO:  在此更改 DC 的任何属性
    if (pWnd->GetDlgCtrlID() == IDC_STATIC_PASSED_SHOW)
    {
        pDC->SelectObject(&font);
        pDC->SetBkColor(RGB(127, 0, 0));
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(RGB(0, 255, 0));
        //return brush;
    }
    
    if (pWnd->GetDlgCtrlID() == IDC_STATIC_ALARMCNT_SHOW)
    {
        pDC->SelectObject(&font);
        pDC->SetBkColor(RGB(127, 0, 0));
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(RGB(255, 0, 0));
        //return brush;
    }
    
    return hbr;
}

//设备同步处理函数
void CSDMDlg::DeviceSync()
{
    vector<ClientInfo*> allCliect = theApp.m_SDCommData.GetAllClients();
    int clientNum = allCliect.size();
    
    if (clientNum <= 0)
    {
        return; //判断当前是否有连接客户端，没有则直接返回，没必要再走轮询逻辑
    }
    
    vector<ClientInfo*>::iterator iter = allCliect.begin();
    
    while (iter != allCliect.end())
    {
        BYTE byte[4] = { 0x80, 0x04, 0x00, 0x7F };
        ClientInfo *client = *iter;
        //byte[0] = client->doorID | 0x80;
        WSABUF wsaBuf;
        wsaBuf.len = 4;
        wsaBuf.buf = (char*)byte;
        g_server->SendData(client->ipAddress, client->nPort, wsaBuf, 4);
        iter++;
        Sleep(20);
    }
}

LRESULT CSDMDlg::OnGetDoorId(WPARAM wParam, LPARAM lParam)
{
    LPBYTE data = (BYTE*)wParam;
    CString id;
    id.Format("%d", data[0] & 0x7F);    //获取设备id
    char ip[25] = { 0 };
    memcpy(ip, data + 4, (25)*sizeof(BYTE));
    vector<ClientInfo*> allCliect = theApp.m_SDCommData.GetAllClients();
    int clientNum = allCliect.size();
    
    if (clientNum <= 0)
    {
        return 0L; //判断当前是否有连接客户端，没有则直接返回，没必要再走轮询逻辑
    }
    
    vector<ClientInfo*>::iterator iter = allCliect.begin();
    
    while (iter != allCliect.end())
    {
        ClientInfo *client = *iter;
        
        if (strcmp(client->ipAddress, ip) == 0)
        {
            client->doorID = data[0] & 0xff;
        }
        
        iter++;
    }
    
    CString name;
    CString remark;
    g_pDBT->AddNewDev(id, name, remark);
    UpdateData(FALSE);
    
    return 0L;
}

LRESULT CSDMDlg::OnDeleteUpdate(WPARAM wParam, LPARAM lParam)
{
    GetDataFormDB();
    return 0L;
}


//自定义消息响应函数——更新表格数据
LRESULT CSDMDlg::OnUpdateTable(WPARAM wParam, LPARAM lParam)
{
    CTime curTime;
    curTime = CTime::GetCurrentTime();             //获取当前时间日期
    CString strDate = curTime.Format(_T("%Y%m%d"));          //格式化日期
    //CString strTime = curTime.Format(_T("%X"));          //格式化时间
    //CString strDateTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S %A"));   //格式化日期时间
    LPBYTE data = (BYTE*)wParam;
    m_alarmStatus[0] = data[9] & 0x01;
    m_alarmStatus[1] = data[9] >> 1 & 0x01;
    m_alarmStatus[2] = data[9] >> 2 & 0x01;
    m_alarmStatus[3] = data[9] >> 3 & 0x01;
    m_alarmStatus[4] = data[9] >> 4 & 0x01;
    m_alarmStatus[5] = data[9] >> 5 & 0x01;
    m_alarmStatus[6] = data[10] & 0x01;
    m_alarmStatus[7] = data[10] >> 1 & 0x01;
    m_alarmStatus[8] = data[10] >> 2 & 0x01;
    m_alarmStatus[9] = data[10] >> 3 & 0x01;
    m_alarmStatus[10] = data[10] >> 4 & 0x01;
    m_alarmStatus[11] = data[10] >> 5 & 0x01;
    m_alarmStatus[12] = data[11] & 0x01;
    m_alarmStatus[13] = data[11] >> 1 & 0x01;
    m_alarmStatus[14] = data[11] >> 2 & 0x01;
    m_alarmStatus[15] = data[11] >> 3 & 0x01;
    //CRect rect;
    //GetDlgItem(IDC_STATIC_SDDATA)->GetWindowRect(&rect);
    //ScreenToClient(&rect);
    //InvalidateRect(&rect, TRUE);
    LoadSecurityDoor();
    CString test, text2;
    test.Format("%d%d  %d%d  %d%d", m_alarmStatus[0], m_alarmStatus[1], m_alarmStatus[2], m_alarmStatus[3], m_alarmStatus[4], m_alarmStatus[5]);
    text2.Format("  %d%d  %d%d  %d%d", m_alarmStatus[6], m_alarmStatus[7], m_alarmStatus[8], m_alarmStatus[9], m_alarmStatus[10], m_alarmStatus[11]);
    CString doorId;
    int doorSn;
    int forwardPass;
    int backPass;
    int warningCnt;
    int areaCnt;
    CString Sdomain;
    CString Sdate;
    doorSn = data[0] & 0x7F;
    doorId.Format("%d", data[0] & 0x7F);
    //获取设备ID号
    BOOL isFindSameId = FALSE;  //是否已有相同的设备
    int devNum = 0; //当前所连入的设备总数
    
    for (int i = 0; i < 100; i++)   //是否有相同的设备id
    {
        if (g_address[i] == 0)
        {
            break;  //没有设备了
        }
        
        if (g_address[i] == data[0])
        {
            isFindSameId = TRUE;
            break;  //在缓存中找到相同id的设备，则跳出循环
        }
        
        devNum++;
    }
    
    if (!isFindSameId)  //判断是否是新设备
    {
        g_address[devNum] = data[0];
        g_pDBT->AddNewDev(doorId, "", "");
    }
    
    if (devNum >= 127)
    {
        devNum = 0;
    }
    
    //根据当前接收到的客户端ID,更新复位缓存中已有的连接设备断线判断次数
    int curId = data[0] & 0xff;
    vector<ClientInfo*> allCliect = theApp.m_SDCommData.GetAllClients();
    vector<ClientInfo*>::iterator iter = allCliect.begin();
    
    while (iter != allCliect.end())
    {
        ClientInfo *client = *iter;
        
        if (curId == client->doorID)
        {
            client->cntOfOnlineJudge = 0;   //计数清零
            client->isOfflineFlag = 0;      //掉线标志复位
            break;
        }
        
        iter++;
    }
    
    bool findId = false;

    forwardPass = data[3] * 128 + data[4];
    backPass = data[5] * 128 + data[6];
    warningCnt = data[7] * 128 + data[8];
    areaCnt = data[11] >> 4 & 0x7;
    int passed = forwardPass + backPass;
    POSITION prePos;
    POSITION pos = m_cntList.GetHeadPosition();
    
    while (pos != NULL)
    {
        prePos = pos;
        UnitCountSt unit = m_cntList.GetNext(pos);
        
        if (unit.id == doorSn)
        {
            findId = true;
            
			if (unit.todayDate != strDate)
			{
				unit.pass = 0;
				unit.alarm = 0;
				unit.todayPass = 0;
				unit.todayAlarm = 0;  //找到了，但记录日期不同则当天记录数要先清零
			}

            //已有，则要判断该ID号的安检门是否有过清零操作，清空了则安检门的统计要小于上位机的中间统计
            if ((unit.pass > passed) || (unit.alarm > warningCnt) || (passed == 1 && unit.pass == 1))
            {
                unit.todayPass += passed;
                unit.todayAlarm += warningCnt;  //记录相关总数
				unit.totalPass += passed;
				unit.totalAlarm += warningCnt;
            }
            else
            {
                unit.todayPass += passed - unit.pass;
                unit.todayAlarm += warningCnt - unit.alarm;
				unit.totalPass += passed - unit.pass;
				unit.totalAlarm += warningCnt - unit.alarm;
            }
            
            unit.pass = passed;
            unit.alarm = warningCnt;    //判断完后更新中间值
            unit.todayDate = strDate;
            m_cntList.SetAt(prePos, unit);
            //break;
        }
		else
		{
			if (unit.todayDate != strDate)
			{
				unit.pass = 0;
				unit.alarm = 0;
				unit.todayPass = 0;
				unit.todayAlarm = 0;  //找到了，但记录日期不同则当天记录数要先清零
				m_cntList.SetAt(prePos, unit);
			}
		}
    }
    
    if (findId)
    {
    }
    else
    {
        //没有找到则添加这个id的数据到链表中去
        UnitCountSt tem;
        tem.id = doorSn;
        tem.pass = passed;
        tem.alarm = warningCnt;
        tem.todayPass = passed;
        tem.todayAlarm = warningCnt;
        tem.totalPass = passed;
        tem.totalAlarm = warningCnt;
        tem.todayDate = strDate;
        m_cntList.AddTail(tem);
    }
    
    m_totalPassedToday = 0;
    m_totalAlarmedToday = 0;
    pos = m_cntList.GetHeadPosition();
    
    while (pos != NULL)
    {
		//计算当天所有安检门下通过的人数及报警次数
        UnitCountSt unit = m_cntList.GetNext(pos);
        m_totalPassedToday += unit.todayPass;
        m_totalAlarmedToday += unit.todayAlarm;

		//更新每一安检门记录的数据到数据库中去
		g_pDBT->AddOrUpdateCountTb(unit.id, unit.totalPass, unit.totalAlarm, unit.todayPass, unit.todayAlarm, unit.todayDate, unit.pass, unit.alarm);
    }
    
    CString passedCntStr;
    CString alarmCntStr;
    passedCntStr.Format("%d", m_totalPassedToday);
    alarmCntStr.Format("%d", m_totalAlarmedToday);
    IniHelper::Instance()->SetIniStringValue("Count", "Date", strDate);
    IniHelper::Instance()->SetIniStringValue("Count", "Passed", passedCntStr);
    IniHelper::Instance()->SetIniStringValue("Count", "Alarmed", alarmCntStr);
    GetDlgItem(IDC_STATIC_PASSED_SHOW)->SetWindowText(passedCntStr);
    GetDlgItem(IDC_STATIC_ALARMCNT_SHOW)->SetWindowText(alarmCntStr);
    //Sdomain.Format("%d%d%d",data[9],data[10],data[11]);
    //test.Format("%x%x%x",data[9],data[10],data[11]);
    Sdomain = test + text2;
    COleDateTime datetime = COleDateTime::GetCurrentTime();
    //将新来的数据添加到数据库中
    g_pDBT->AddNewData(doorId, forwardPass, backPass, warningCnt, Sdomain, datetime);
    //再从新从数据库中加载以刷新数据
    GetDataFormDB();
    return 0L;
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。
void CSDMDlg::OnPaint()
{
    if (m_isFirstEnter)
    {
        m_isFirstEnter = FALSE;
        LoadSecurityDoor();
    }
    
    CDialog::OnPaint();
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSDMDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CSDMDlg::InitStyle(void)
{
    int nFullWidth = GetFullWidth(); //1920
    int nFullHeight = GetFullHeight(); //1080
    nFullWidth = GetFullWidth() * 5 / 6; //1920
    nFullHeight = GetFullHeight() * 5 / 6; //1080
    HINSTANCE hInstance = AfxGetInstanceHandle();
    LoadBackSkin(hInstance, TEXT("MAIN_DLG_BG"));
    SetWindowPos(NULL, 0, 0, nFullWidth, nFullHeight, SWP_NOMOVE);
    //::SetWindowPos(m_hWnd,HWND_TOPMOST,0,0,nFullWidth,nFullHeight,SWP_NOMOVE);   //TODO 仅发布时放开
    m_titleImage.LoadImage(hInstance, TEXT("MAIN_DLG_TITLE"));

	m_btnCountInfo.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(nFullWidth - 110 * 5 - 200, 3, 0, 0), this, IDC_BUTTON_DATACOUNT);
	m_btnCountInfo.SetButtonImage(hInstance, TEXT("BTN_MAIN"));
	m_btnCountInfo.SetWindowText("数据统计");//DataCount
    m_btnDataQuery.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(nFullWidth - 110 * 4 - 200, 3, 0, 0), this, IDC_BUTTON_DATAQUERY);
    m_btnDataQuery.SetButtonImage(hInstance, TEXT("BTN_MAIN"));
    m_btnDataQuery.SetWindowText("数据操作");//DataQuery
    m_btnUserManage.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(nFullWidth - 110 * 3 - 200, 3, 0, 0), this, IDC_BUTTON_USERMANAGE);
    m_btnUserManage.SetButtonImage(hInstance, TEXT("BTN_MAIN"));
    m_btnUserManage.SetWindowText("账户管理");//UserManage
    m_btnDevManage.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(nFullWidth - 110 * 2 - 200, 3, 0, 0), this, IDC_BUTTON_DEVMANAGE);
    m_btnDevManage.SetButtonImage(hInstance, TEXT("BTN_MAIN"));
    m_btnDevManage.SetWindowText("设备管理");//SystemSet
    m_btnSystemSet.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(nFullWidth - 110 * 1 - 200, 3, 0, 0), this, IDC_BUTTON_SYSTEMSET);
    m_btnSystemSet.SetButtonImage(hInstance, TEXT("BTN_MAIN"));
    m_btnSystemSet.SetWindowText("系统设置");//SystemSet
    m_btnExit.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(nFullWidth - 200, 3, 0, 0), this, IDC_BUTTON_EXIT_SYSTEM);
    m_btnExit.SetButtonImage(hInstance, TEXT("BTN_MAIN"));
    m_btnExit.SetWindowText("退出系统");//ExitSystem
    int w_unit = nFullWidth / 6;
    int h_unit = nFullHeight / 20;
    int w_space = 5;
    int h_space = 5;
    GetDlgItem(IDC_STATIC_CFG)->MoveWindow(w_unit + 2 * w_space, h_unit + h_space, w_unit * 5 - w_space * 3, h_unit * 2 - 2 * h_space);
    RECT cfgRect;
    GetDlgItem(IDC_STATIC_CFG)->GetWindowRect(&cfgRect);
    int nLeft = cfgRect.left + 10;
    int nTop = cfgRect.top + 23;
    int nWidth = 100;
    int nHeight = 20;
    GetDlgItem(IDC_STATIC_IP)->MoveWindow(nLeft, nTop, nWidth, nHeight);
    nLeft = nLeft + nWidth;
    GetDlgItem(IDC_STATIC_SERVERIP)->MoveWindow(nLeft, nTop, nWidth, nHeight);
    nLeft = nLeft + nWidth + 10;
    GetDlgItem(IDC_STATIC_PORT)->MoveWindow(nLeft, nTop, nWidth, nHeight);
    nLeft = nLeft + nWidth;
    GetDlgItem(IDC_LABEL_PORT)->MoveWindow(nLeft, nTop, nWidth, nHeight);//IDC_EDIT_PORT
    nLeft = nLeft + nWidth + 10;
    GetDlgItem(IDC_STATIC_NUM)->MoveWindow(nLeft, nTop, nWidth, nHeight);
    nLeft = nLeft + nWidth;
    GetDlgItem(IDC_LABEL_CLIENT_NUM)->MoveWindow(nLeft, nTop, nWidth, nHeight);
    nLeft = nLeft + nWidth + 20;
    m_btnExport.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(nLeft, nTop - 5, 0, 0), this, IDC_BUTTON_EXPORT);
    m_btnExport.SetButtonImage(hInstance, TEXT("BTN_REGISTER"));
    m_btnExport.SetWindowText("数据导出");
    m_DevTree.MoveWindow(w_space, h_unit + 2 * h_space, w_unit, h_unit * 19 - 3 * h_space);
    m_SDDataCtrl.MoveWindow(w_unit + 2 * w_space, h_unit * 3, w_unit, h_unit * 13 - h_space);
    m_DevInfoList.MoveWindow(w_unit * 2 + 15, h_unit * 3, w_unit * 3, h_unit * 13 - h_space);
    //m_DevInfoList.MoveWindow(w_unit * 2 + 15, h_unit * 3, w_unit * 3 - 4 * w_space, h_unit * 13 - h_space);
    GetDlgItem(IDC_STATIC_COUNT)->MoveWindow(w_unit * 5 + 20, h_unit * 3, w_unit - 5 * w_space, h_unit * 13 - h_space);
    GetDlgItem(IDC_STATIC_COUNT)->GetWindowRect(&cfgRect);
    nLeft = cfgRect.left + 5;
    nTop = cfgRect.top + 10;
    nWidth = cfgRect.right - cfgRect.left - 10;
    nHeight = (cfgRect.bottom - cfgRect.top - 10) / 20;
    GetDlgItem(IDC_STATIC_PASSED)->MoveWindow(nLeft, nTop, nWidth, nHeight);
    nTop = nTop + nHeight;
    GetDlgItem(IDC_STATIC_PASSED_SHOW)->MoveWindow(nLeft, nTop, nWidth, nHeight * 9);
    nTop = nTop + 9 * nHeight;
    GetDlgItem(IDC_STATIC_ALARMCNT)->MoveWindow(nLeft, nTop, nWidth, nHeight);
    nTop = nTop + nHeight;
    GetDlgItem(IDC_STATIC_ALARMCNT_SHOW)->MoveWindow(nLeft, nTop, nWidth, nHeight * 9);
    m_LogListBox.MoveWindow(w_unit + 2 * w_space, h_unit * 16, w_unit * 5 - 3 * w_space, h_unit * 4 - h_space);
}

void CSDMDlg::OnClientDraw(CDC*pDC, INT nWidth, INT nHeight)
{
    m_titleImage.DrawImage(pDC, 5, 4, 24, 24);
    pDC->SetTextColor(RGB(255, 255, 255));
    CRect rcText(35, 10, 300, 26);
    DrawText(pDC, rcText, m_strTitle, DT_LEFT | DT_END_ELLIPSIS);
}

void CSDMDlg::DrawText(CDC* pDC, CRect rcText, LPCTSTR lpszText, UINT uFormat, INT nHeight/*=18*/, bool bBold/*=true*/, LPCTSTR lpszName/*=TEXT("微软雅黑")*/)
{
    CFont font;
    font.CreateFont(nHeight, 0, 0, 0, bBold ? FW_BOLD : FW_NORMAL, 0, FALSE, 0, 0, 0, 0, 0, 0, lpszName);
    CFont*  pOldFont = pDC->SelectObject(&font);
    pDC->DrawText(lpszText, &rcText, uFormat);
    pDC->SelectObject(pOldFont);
}

void CSDMDlg::OnBnClickedDataQuery()
{
    CDataQueryDlg dlg(this);
    dlg.DoModal();
}

void CSDMDlg::OnBnClickedUserManage()
{
    CUserManageDlg dlg;
    dlg.DoModal();
}

void CSDMDlg::OnBnClickedDevManage()
{
    CDevManageDlg dlg;
    dlg.DoModal();
}

void CSDMDlg::OnBnClickedSystemSet()
{
    OpenOrCloseTime(FALSE);
    CSystemSetDlg dlg;
    dlg.DoModal();
    dlg.SetMainDlg(this);
    OpenOrCloseTime(TRUE);
}

void CSDMDlg::OnBnClickedCountInfo()
{
	CDataCountDlg dlg;
	dlg.DoModal();
}

void CSDMDlg::OnBnClickedExit()
{
    DeInitializeNetWork();
    CDialog::OnCancel();
}

void CSDMDlg::OnBnClickedExport()
{
    CString strReturnPath = DataExport::GetExePath();
    //CString strReturnPath = GetExePath();
    CString filename = "20180515.csv";
    filename.Format("%s\\20180515.csv", strReturnPath);
    //TRUE为open对话框，false 为save as 对话框。
    CFileDialog fg(false, _T("*.csv"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                   _T("Excel CSV File(*.csv)|*.csv|All File(*.*)|*.*||"), NULL);
    //      _T("Excel CSV File(*.csv)|Text File(*.txt)|*.txt|All File(*.*)|*.*||"), NULL);
    
    if (fg.DoModal() == IDOK)
    {
        CString name = fg.GetFileName();
        
        if (name != "")
        {
            filename = fg.GetPathName();
        }
        
        if (DataExport::ExportToCsv(filename) == 1)
        {
            this->MessageBox("文件打开失败！");
            return;
        }
        else
        {
            this->MessageBox("导出成功！");
        }
    }
}

void CSDMDlg::AddClient()
{
    m_clientNum ++;
    CString num;
    num.Format(_T("%d"), m_clientNum);
    GetDlgItem(IDC_LABEL_CLIENT_NUM)->SetWindowText(num);
}

//减少客户端数量
void CSDMDlg::DecreaseCliect()
{
    m_clientNum--;
    CString num;
    
    if (m_clientNum == 0)
    {
        num = _T("");
    }
    else
    {
        num.Format(_T("%d"), m_clientNum);
    }
    
    GetDlgItem(IDC_LABEL_CLIENT_NUM)->SetWindowText(num);
}

// CMyServerApp 消息处理程序
//初始化网络信息
BOOL CSDMDlg::InitNetWork()
{
    //1.加载套接字库
    if (!m_IOCPServer.LoadSocketLib())
    {
        return FALSE;
    }
    
    // 设置本机IP地址
    CString ip = m_IOCPServer.GetLocalIP();
    int port = DEFAULT_PORT;
    //ip = _T("127.0.0.1"); //for  test
    
    if (ip == _T("127.0.0.1"))
    {
        IpSetDlg ipDlg;
        ipDlg.SetTipContent(_T("温馨提示：\n\n        由于本次自动获取本机 IP 地址出错，因而需要手动设置："));
        
        if (ipDlg.DoModal() == IDOK)
        {
            ip = ipDlg.m_ip;
            port = atoi(ipDlg.m_port);
        }
    }
    
    if (g_setIpPort)
    {
        ip = g_ip;
        port = g_port;
    }
    
    m_IOCPServer.SetServerIpPort(ip, port); //设置服务器ip及其端口号
    SetDlgItemText(IDC_STATIC_SERVERIP, ip);//m_IOCPServer.GetLocalIP()
    CString sPort;
    sPort.Format(_T("%d"), port);
    SetDlgItemText(IDC_LABEL_PORT, sPort);
    //SetDlgItemInt(IDC_EDIT_PORT, port);
    
    //2.开启服务器
    if (!m_IOCPServer.Start())
    {
        m_LogListBox.AddString("服务器启动失败\n");
        return FALSE;
    }
    
    m_IOCPServer.SetMainDlg(this);
    theApp.m_SDCommData.InitUI(this);
    theApp.m_SDCommData.SetCurCtrlWnd(m_hWnd);
    m_LogListBox.AddString("服务器启动成功\n");
    g_server = &m_IOCPServer;
    return TRUE;
}
//退出系统时回收网络资源
void CSDMDlg::DeInitializeNetWork()
{
    //1.通知客户端退出
    theApp.m_SDCommData.LoginOutInform();
    Sleep(1000);
    //2.停止服务
    m_IOCPServer.Stop();
    //3.清除加载的套接字库
    m_IOCPServer.UnloadSocketLib();
}

//将宽字符转为多字节
char* CSDMDlg::WCharToChar(const wchar_t* pwch, char* pch, size_t* pnewlen)
{
    _ASSERTE(pwch);
    _ASSERTE(pch);
    size_t clen = WideCharToMultiByte(CP_ACP, 0, pwch, -1, NULL, 0, NULL, NULL);
    WideCharToMultiByte(CP_ACP, NULL, pwch, -1, (LPSTR)pch, clen, NULL, NULL);
    
    if (pnewlen)
    {
        *pnewlen = clen;
    }
    
    pch[clen] = '\0';
    return pch;
}

//将多字节转为宽字符
wchar_t* CSDMDlg::CharToWChar(const char* pch, wchar_t* pwch, size_t* pnewlen)
{
    _ASSERTE(pwch);
    _ASSERTE(pch);
    size_t len = strlen(pch);
    size_t wlen = MultiByteToWideChar(CP_ACP, 0, pch, len, NULL, 0);
    MultiByteToWideChar(CP_ACP, 0, pch, len, pwch, wlen);
    
    if (pnewlen)
    {
        *pnewlen = wlen;
    }
    
    pwch[wlen] = '\0';
    return pwch;
}

void CSDMDlg::LoadBitmap()
{
}

void  CSDMDlg::LoadSecurityDoor()
{
    CRect rect;
    GetDlgItem(IDC_STATIC_SDDATA)->GetWindowRect(&rect);
    ScreenToClient(&rect);
    InvalidateRect(&rect, TRUE);
    CDC *pDC = GetDlgItem(IDC_STATIC_SDDATA)->GetDC();
    //CClientDC dc(GetDlgItem(IDC_STATIC_SDDATA));
    CPaintDC dc(this);
    //加载bmp文件   xxx.bmp 就是要加载的图片名字
    CBitmap bitmap;
    bitmap.LoadBitmap(IDB_BITMAP1);
    CDC dcimage;
    dcimage.CreateCompatibleDC(pDC); //内存
    BITMAP bmp;
    bitmap.GetBitmap(&bmp); //获取位图信息
    dcimage.SelectObject(&bitmap); //选取位图对象
    GetDlgItem(IDC_STATIC_SDDATA)->GetWindowRect(&rect);
    int left = rect.left;
    int top = rect.top;
    ScreenToClient(&rect);
    left = rect.left;
    top = rect.top;
    int width = (rect.right - rect.left);
    int height = rect.bottom - rect.top;
    int m_nWidth = bmp.bmWidth;
    int m_nHeight = bmp.bmHeight;
    float rateXY = (float)height / (float)width;
    int partX = width / 10;
    int partY = height * 23 / 120;
    int interX = width - partX * 5;
    int interY = (height - partY) / 6;
    CBitmap *oldBmp = dcimage.SelectObject(&bitmap);
    pDC->SetStretchBltMode(HALFTONE);
    ::StretchBlt(pDC->m_hDC, 0, 0, width, height, dcimage, 0, 0, m_nWidth, m_nHeight, SRCCOPY);
    ::SelectObject(dcimage, oldBmp);
    DeleteObject(oldBmp);
    CPen redPen;
    redPen.CreatePen(PS_SOLID, 6, RGB(255, 0, 0));
    CBrush redBrush;
    redBrush.CreateSolidBrush(RGB(255, 0, 0));
    
    for (int i = 0; i < 12; i++)
    {
        if (m_alarmStatus[i] == 1)
        {
            rect.left = left + 2 * partX + interX * (i % 2);
            rect.top = top + partY + (i / 2) * interY;
            rect.right = rect.left + partX;
            rect.bottom = rect.top + partX * 2;
            //m_SDDataCtrl.MoveWindow(280,110,400,800);
            dc.SelectObject(&redBrush);
            dc.SelectObject(&redPen);
            dc.FillRect(&rect, &redBrush);
        }
    }
    
    //UpdateData(FALSE);
    ReleaseDC(pDC);
}

void CSDMDlg::InitTreeCtrl()
{
    m_ImageList.Create(24, 24, ILC_COLOR32 | ILC_MASK, 1, 1);
    m_ImageList.SetBkColor(RGB(255, 255, 255));
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_SDOOR_AREA));
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_NETWORK_BUSY));
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_NETWORK_DISCONNECT));
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_NETWORK_IDLE));
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_SDOOR_AREA));
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_NETWORK_BUSY));
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_NETWORK_DISCONNECT));
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_NETWORK_IDLE));
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_SDOOR_AREA));
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_NETWORK_BUSY));
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_NETWORK_DISCONNECT));
    m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_NETWORK_IDLE));
    m_DevTree.SetTextColor(RGB(0, 0, 255));
    m_DevTree.SetImageList(&m_ImageList, TVSIL_NORMAL);
    DWORD dwStyle = TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;
    DWORD dwStyles = m_DevTree.GetStyle(); //获取树控制原风格
    dwStyles |= dwStyle;
    m_DevTree.ModifyStyle(0, dwStyles);
    
    if (0)
    {
        CTreeCtrl* pTreeCtl = &m_DevTree;
        //测试数据的添加
        ASSERT(pTreeCtl);
        pTreeCtl->DeleteAllItems();
        HTREEITEM hItem;
        hItem = pTreeCtl->InsertItem("一区", 0, 0, TVI_ROOT);
        pTreeCtl->InsertItem("192.168.0.10", 1, 1, hItem);
        pTreeCtl->InsertItem("192.168.0.11", 2, 2, hItem);
        pTreeCtl->InsertItem("192.168.0.12", 3, 3, hItem);
        hItem = pTreeCtl->InsertItem("二区", 4, 4, TVI_ROOT);
        pTreeCtl->InsertItem("192.168.0.20", 5, 5, hItem);
        pTreeCtl->InsertItem("192.168.0.21", 6, 6, hItem);
        pTreeCtl->InsertItem("192.168.0.22", 7, 7, hItem);
        hItem = pTreeCtl->InsertItem("三区", 8, 8, TVI_ROOT);
        pTreeCtl->InsertItem("192.168.0.30", 9, 9, hItem);
        pTreeCtl->InsertItem("192.168.0.31", 10, 10, hItem);
        pTreeCtl->InsertItem("192.168.0.32", 11, 11, hItem);
    }
}

int CSDMDlg::GetFullWidth()
{
    int nFullWidth = GetSystemMetrics(SM_CXSCREEN);
    nFullWidth  = (nFullWidth < 0x400)  ? 0x400 : nFullWidth;
    return nFullWidth;
}

int CSDMDlg::GetFullHeight()
{
    int nFullHeight = GetSystemMetrics(SM_CYSCREEN);
    nFullHeight = (nFullHeight < 0x300) ? 0x300 : nFullHeight;
    return nFullHeight;
}

void CSDMDlg::OnDestroy()
{
    CDialog::OnDestroy();
}

void CSDMDlg::OnNMClickListDevinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    POSITION ps;
    int nIndex;
    ps = m_DevInfoList.GetFirstSelectedItemPosition();
    nIndex = m_DevInfoList.GetNextSelectedItem(ps);
    m_DevInfoList.SetItemState(nIndex, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
    CString str;
    str = str + m_DevInfoList.GetItemText(nIndex, 4);
    m_alarmStatus[0] = atoi(str.Mid(0, 1)) ;
    m_alarmStatus[1] = atoi(str.Mid(1, 1));
    m_alarmStatus[2] = atoi(str.Mid(4, 1));
    m_alarmStatus[3] = atoi(str.Mid(5, 1));
    m_alarmStatus[4] = atoi(str.Mid(8, 1));
    m_alarmStatus[5] = atoi(str.Mid(9, 1));
    m_alarmStatus[6] = atoi(str.Mid(12, 1));
    m_alarmStatus[7] = atoi(str.Mid(13, 1));
    m_alarmStatus[8] = atoi(str.Mid(16, 1));
    m_alarmStatus[9] = atoi(str.Mid(17, 1));
    m_alarmStatus[10] = atoi(str.Mid(20, 1));
    m_alarmStatus[11] = atoi(str.Mid(21, 1));
    //CRect rect;
    //GetDlgItem(IDC_STATIC_SDDATA)->GetWindowRect(&rect);
    //ScreenToClient(&rect);
    //InvalidateRect(&rect, TRUE);
    LoadSecurityDoor();
}

void CSDMDlg::InitDevinfoList(void)
{
    m_DevInfoList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_DevInfoList.SetBkColor(RGB(247, 247, 255));
    m_DevInfoList.SetTextColor(RGB(0, 0, 255));
    m_DevInfoList.SetTextBkColor(RGB(247, 247, 255));
    CString str[] = { _T(""), _T("门ID"), _T("前通过人数"), _T("后通过人数"), _T("报警次数"), _T("报警区域"), _T("时间") };
    RECT listRect;
    GetDlgItem(IDC_LIST_DEVINFO)->GetWindowRect(&listRect);
    //this->ScreenToClient(&listRect);
    int listWidth = listRect.right - listRect.left;
    //listWidth = listRect.Width();
    int colsNum = sizeof(str) / sizeof(str[0]);
    int colWidth = listWidth / (4 * colsNum);
    int widthSum = 0;
    
    for (int i = 0; i < colsNum; i++)
    {
        int len = 2 * colWidth;
        
        if (i <= 1)
        {
            len = 2 * colWidth;
        }
        else if (i <= 4)
        {
            len = 4 * colWidth;
        }
        else
        {
            len = 6 * colWidth;
        }
        
        widthSum += len;
        m_DevInfoList.InsertColumn(i, str[i], LVCFMT_CENTER, len);
        //m_dataList.InsertItem(i, _T(""));
        //m_dataList.SetItemText(i, 0, _T("AAA"));
    }
    
    m_DevInfoList.DeleteColumn(0);
    //m_DevInfoList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);// |//LVS_EX_FULLROWSELECT |LVS_EX_GRIDLINES //
    ////LVS_EX_HEADERDRAGDROP | LVS_EX_ONECLICKACTIVATE );
    //m_DevInfoList.SetBkColor(RGB(247, 247, 255));
    //m_DevInfoList.SetTextColor(RGB(0, 0, 255));
    //m_DevInfoList.SetTextBkColor(RGB(247, 247, 255));
    //m_DevInfoList.InsertColumn(0, "门ID", LVCFMT_CENTER, 100);
    //m_DevInfoList.InsertColumn(1, "前通过人数", LVCFMT_CENTER, 150);
    //m_DevInfoList.InsertColumn(2, "后通过人数", LVCFMT_CENTER, 150);
    //m_DevInfoList.InsertColumn(3, "报警次数", LVCFMT_CENTER, 150);
    //m_DevInfoList.InsertColumn(4, "报警区域", LVCFMT_CENTER, 150);
    //m_DevInfoList.InsertColumn(5, "时间", LVCFMT_CENTER, 150);
}

void CSDMDlg::GetDataFormDB()
{
    int iRow = 1;
    m_DevInfoList.DeleteAllItems();
    char pWhere[150] = "";
    //char pWhere[150] = "where DOORID='33'";
    char pOrder[30] = "";
    LPCTSTR TableName = "ANNUNCIATOR";
    
//  int total;
    if (g_pDBT->OpenSql(TableName/*cnt_TableName[Tb_ANNUNCIATOR]*/, "*", pWhere, pOrder))
    {
        int nItem = g_pDBT->GetDBRecordSet()->GetRecordCount();
        
        //allreocd = nItem;
        if (nItem > 0)
        {
            g_pDBT->GetDBRecordSet()->MoveFirst();
            //int ID;
            CString ID, doorID, passForwd, passBack, waring, data, time;
            m_DevInfoList.SetRedraw(FALSE);
            
            //更新内容
            while (!g_pDBT->GetDBRecordSet()->IsEOF())
            {
                g_pDBT->GetDBRecordSet()->GetValueString(ID, cnt_Enannunciator[0]);
                g_pDBT->GetDBRecordSet()->GetValueString(doorID, cnt_Enannunciator[1]);
                g_pDBT->GetDBRecordSet()->GetValueString(passForwd, cnt_Enannunciator[2]);
                g_pDBT->GetDBRecordSet()->GetValueString(passBack, cnt_Enannunciator[3]);
                g_pDBT->GetDBRecordSet()->GetValueString(waring, cnt_Enannunciator[4]);
                g_pDBT->GetDBRecordSet()->GetValueString(data, cnt_Enannunciator[5]);
                g_pDBT->GetDBRecordSet()->GetValueString(time, cnt_Enannunciator[6]);
                int nRow = m_DevInfoList.InsertItem(0, doorID); //插入行
                m_DevInfoList.SetItemText(nRow, 1, passForwd); //插入行
                m_DevInfoList.SetItemText(nRow, 2, passBack);
                m_DevInfoList.SetItemText(nRow, 3, waring); //插入行
                m_DevInfoList.SetItemText(nRow, 4, data);
                m_DevInfoList.SetItemText(nRow, 5, time); //插入行
                g_pDBT->GetDBRecordSet()->MoveNext();
            }
        }
        
        m_DevInfoList.SetRedraw(TRUE);
        m_DevInfoList.UpdateWindow();
        g_pDBT->GetDBRecordSet()->Close();
    }
    
    UpdateData(FALSE);
}

void CSDMDlg::GetDataFormDB(CString doorID)
{
    int iRow = 1;
    m_DevInfoList.DeleteAllItems();
    char pWhere[150] = "";
    char pOrder[30] = "";
    LPCTSTR TableName = "ANNUNCIATOR";
    sprintf(pWhere, "where DOORID='%s'", doorID);
    
    if (g_pDBT->OpenSql(TableName/*cnt_TableName[Tb_ANNUNCIATOR]*/, "*", pWhere, pOrder))
    {
        int nItem = g_pDBT->GetDBRecordSet()->GetRecordCount();
        
        //allreocd = nItem;
        if (nItem > 0)
        {
            g_pDBT->GetDBRecordSet()->MoveFirst();
            //int ID;
            CString ID, doorID, passForwd, passBack, waring, data, time;
            m_DevInfoList.SetRedraw(FALSE);
            
            //更新内容
            while (!g_pDBT->GetDBRecordSet()->IsEOF())
            {
                g_pDBT->GetDBRecordSet()->GetValueString(ID, cnt_Enannunciator[0]);
                g_pDBT->GetDBRecordSet()->GetValueString(doorID, cnt_Enannunciator[1]);
                g_pDBT->GetDBRecordSet()->GetValueString(passForwd, cnt_Enannunciator[2]);
                g_pDBT->GetDBRecordSet()->GetValueString(passBack, cnt_Enannunciator[3]);
                g_pDBT->GetDBRecordSet()->GetValueString(waring, cnt_Enannunciator[4]);
                g_pDBT->GetDBRecordSet()->GetValueString(data, cnt_Enannunciator[5]);
                g_pDBT->GetDBRecordSet()->GetValueString(time, cnt_Enannunciator[6]);
                int nRow = m_DevInfoList.InsertItem(0, doorID); //插入行
                m_DevInfoList.SetItemText(nRow, 1, passForwd); //插入行
                m_DevInfoList.SetItemText(nRow, 2, passBack);
                m_DevInfoList.SetItemText(nRow, 3, waring); //插入行
                m_DevInfoList.SetItemText(nRow, 4, data);
                m_DevInfoList.SetItemText(nRow, 5, time); //插入行
                g_pDBT->GetDBRecordSet()->MoveNext();
            }
        }
        
        m_DevInfoList.SetRedraw(TRUE);
        m_DevInfoList.UpdateWindow();
        g_pDBT->GetDBRecordSet()->Close();
    }
    
    UpdateData(FALSE);
}

void CSDMDlg::GetDataFormCountTb()
{
    int iRow = 1;
    char pWhere[150] = "";
    char pOrder[30] = "";
    LPCTSTR TableName = "CountTb";
    m_cntList.RemoveAll();
    
    if (g_pDBT->OpenSql(TableName, "*", pWhere, pOrder))
    {
        int nItem = g_pDBT->GetDBRecordSet()->GetRecordCount();
        
        if (nItem > 0)
        {
            g_pDBT->GetDBRecordSet()->MoveFirst();
            UnitCountSt unit;
			unit.id = 0;
			unit.totalPass = 0;
			unit.totalAlarm = 0;
			unit.todayPass = 0;
			unit.todayAlarm = 0;
			unit.pass = 0;
			unit.alarm = 0;
			unit.todayDate = _T("");
            //更新内容
            while (!g_pDBT->GetDBRecordSet()->IsEOF())
            {
                g_pDBT->GetDBRecordSet()->GetValueInt(unit.id, cnt_CountTb[1]);
                g_pDBT->GetDBRecordSet()->GetValueInt(unit.totalPass, cnt_CountTb[2]);
                g_pDBT->GetDBRecordSet()->GetValueInt(unit.totalAlarm, cnt_CountTb[3]);
                g_pDBT->GetDBRecordSet()->GetValueInt(unit.todayPass, cnt_CountTb[4]);
                g_pDBT->GetDBRecordSet()->GetValueInt(unit.todayAlarm, cnt_CountTb[5]);
                g_pDBT->GetDBRecordSet()->GetValueString(unit.todayDate, cnt_CountTb[6]);
				g_pDBT->GetDBRecordSet()->GetValueInt(unit.pass, cnt_CountTb[7]);
				g_pDBT->GetDBRecordSet()->GetValueInt(unit.alarm, cnt_CountTb[8]);
                m_cntList.AddTail(unit);
                g_pDBT->GetDBRecordSet()->MoveNext();
            }
        }
        
        g_pDBT->GetDBRecordSet()->Close();
    }
}

UnitCountSt CSDMDlg::GetDataFormCountTb(CString doorID)
{
    int iRow = 1;
    char pWhere[150] = "";
    char pOrder[30] = "";
    LPCTSTR TableName = "CountTb";
    sprintf(pWhere, "where DOORID='%s'", doorID);
    UnitCountSt unit;
    unit.id = 0;
    unit.totalPass = 0;
    unit.totalAlarm = 0;
    unit.todayPass = 0;
    unit.todayAlarm = 0;
    unit.pass = 0;
    unit.alarm = 0;
    unit.todayDate = _T("");
    
    if (g_pDBT->OpenSql(TableName, "*", pWhere, pOrder))
    {
        int nItem = g_pDBT->GetDBRecordSet()->GetRecordCount();
        
        if (nItem > 0)
        {
            g_pDBT->GetDBRecordSet()->MoveFirst();
            
            //更新内容
            while (!g_pDBT->GetDBRecordSet()->IsEOF())
            {
                g_pDBT->GetDBRecordSet()->GetValueInt(unit.id, cnt_CountTb[1]);
                g_pDBT->GetDBRecordSet()->GetValueInt(unit.totalPass, cnt_CountTb[2]);
                g_pDBT->GetDBRecordSet()->GetValueInt(unit.totalAlarm, cnt_CountTb[3]);
                g_pDBT->GetDBRecordSet()->GetValueInt(unit.todayPass, cnt_CountTb[4]);
                g_pDBT->GetDBRecordSet()->GetValueInt(unit.todayAlarm, cnt_CountTb[5]);
                g_pDBT->GetDBRecordSet()->GetValueString(unit.todayDate, cnt_CountTb[6]);
				g_pDBT->GetDBRecordSet()->GetValueInt(unit.pass, cnt_CountTb[7]);
				g_pDBT->GetDBRecordSet()->GetValueInt(unit.alarm, cnt_CountTb[8]);
                break;

            }
        }
        
        g_pDBT->GetDBRecordSet()->Close();
    }
    
    return unit;
}

//树形设备项的选择事件处理函数
void CSDMDlg::OnTvnSelchangedDevTree(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
    int nGetSel = 0;
    char pTemp[MAX_PATH] = "";
    memset(pTemp, 0, MAX_PATH);
    CString strName = "-1";
    strName = m_DevTree.GetItemText(m_DevTree.GetSelectedItem());
    int mod = strName.Find(":");
    int comma = strName.Find(",");
    
    if (mod == -1)
    {
    }
    else
    {
        CString idStr = strName.Mid(mod + 1);
        g_curDoorId = atoi(idStr);
        int id = atoi(idStr);
        CString ipStr = strName.Left(comma);
        char *ipChar = (char*)((LPCTSTR)ipStr);
        
        if (id == 0)
        {
            g_CurClientInfo = theApp.m_SDCommData.GetCurClientInfo(ipChar);
        }
        else
        {
            g_CurClientInfo = theApp.m_SDCommData.GetCurClientInfo(id | 0x80);
        }
        
        /*g_CurClientInfo = theApp.m_SDCommData.GetCurClientInfo(g_curDoorId);*/
        GetDataFormDB(idStr);
        //MessageBox(strName);
    }
}

void CSDMDlg::GetDevFormDB(CString strName, CString &returnName)
{
    char pWhere[150] = "";
    char pOrder[30] = "";
    LPCTSTR TableName = "DEVICES";
    sprintf(pWhere, "where COMID='%s'", strName);
    //m_ListboxDevlist.ResetContent();
    CString strvalue;
    
    if (g_pDBT->OpenSql(TableName, "*", pWhere, pOrder))
    {
        int nItem = g_pDBT->GetDBRecordSet()->GetRecordCount();
        
        //allreocd = nItem;
        if (nItem > 0)
        {
            g_pDBT->GetDBRecordSet()->MoveFirst();
            //int ID;
            CString ID, strBeizhu, passForwd, passBack, waring, data, time;
            m_DevTree.SetRedraw(FALSE);
            
            //更新内容
            while (!g_pDBT->GetDBRecordSet()->IsEOF())
            {
                g_pDBT->GetDBRecordSet()->GetValueString(ID, cnt_Endevices[0]);
                g_pDBT->GetDBRecordSet()->GetValueString(strBeizhu, cnt_Endevices[1]);
                
                if (ID.IsEmpty())
                {
                    //return;
                }
                else
                {
                    if (strBeizhu == "NULL" || strBeizhu == "")
                    {
                        strvalue = ID;
                    }
                    else
                    {
                        strvalue.Format("%s=%s", ID, strBeizhu);
                    }
                    
                    returnName = strvalue;
                    //                  int findName = m_ListboxDevlist.FindString(i,strName);
                    //                  if (findName)
                    //                  {
                    //                      ::SendMessage(m_hWndDoorList, LB_ADDSTRING, 0, (LPARAM)strvalue.GetBuffer());
                    //                  }
                }
                
                g_pDBT->GetDBRecordSet()->MoveNext();
            }
        }
        
        g_pDBT->GetDBRecordSet()->Close();
    }
    
    UpdateData(FALSE);
}

//定时轮询安检门设备
void CSDMDlg::OnTimer(UINT nIDEvent)
{
    if (nIDEvent == TIMER_UPDATA)
    {
        //vector<ClientInfo*> offCliect;
        //offCliect.clear();
        vector<ClientInfo*> allCliect = theApp.m_SDCommData.GetAllClients();
        int clientNum = allCliect.size();
        
        if (clientNum <= 0)
        {
            return; //判断当前是否有连接客户端，没有则直接返回，没必要再走轮询逻辑
        }
        
        vector<ClientInfo*>::iterator iter = allCliect.begin();
        m_LogListBox.ResetContent();
        
        while (iter != allCliect.end())
        {
            BYTE byte[4] = { 0x00, 0x04, 0x01, 0x7F };
            ClientInfo *client = *iter;
            WSABUF wsaBuf;
            wsaBuf.len = 4;
            byte[0] = client->doorID & 0xff;
            
            if (client->doorID == 0)    //同步处理
            {
                //DeviceSync();
                byte[0] = 0x80;//{ 0x80, 0x04, 0x00, 0x7F };
                byte[2] = 0x00;
                wsaBuf.buf = (char*)byte;
                g_server->SendData(client->ipAddress, client->nPort, wsaBuf, 4);
                iter++;
                Sleep(20);
                continue;
            }
            
            client->cntOfOnlineJudge++;
            
            if (client->cntOfOnlineJudge >= 100)    //100 * 60ms = 6s
            {
                client->cntOfOnlineJudge = 100;
                client->isOfflineFlag = 1;
                //当前客户端掉线
                CString msg;
                msg.Format("客户端：%s 没有返回数据！", client->ipAddress);
                m_LogListBox.AddString(msg);
                //offCliect.push_back(client);
                //allCliect.erase(iter); //不能放在这里删除客户端，会崩溃
                //allCliect.pop_back(&client);
            }
            
            wsaBuf.buf = (char*)byte;
            g_server->SendData(client->ipAddress, client->nPort, wsaBuf, 4);
            iter++;
            //Sleep(50);
        }
        
        //     vector<ClientInfo*>::iterator it = offCliect.begin();
        //
        //     while (it != offCliect.end())
        //     {
        //ClientInfo *client = *it;
        //theApp.m_SDCommData.DeleteClient(client->ipAddress, client->nPort);
        //it++;
        //     }
    }
    
    CDialog::OnTimer(nIDEvent);
}

void CSDMDlg::OpenOrCloseTime(BOOL res)
{
    if (res)
    {
        SetTimer(TIMER_UPDATA, POLL_TIME, NULL);
    }
    else
    {
        KillTimer(TIMER_UPDATA);
    }
    
    // TODO: 在此处添加消息处理程序代码
}
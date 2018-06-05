// SDServer.cpp : 定义应用程序的类行为。
#include "stdafx.h"
#include "SDM.h"
#include "SDMDlg.h"
#include "SDM.h"
#include "Log.h"
#include "./system/LoginDlg.h"
#include "./system/RegisterDlg.h"
#include <string>
#include <sstream>
#include "./Des/DES.h"
#include "./Des/CRC32.h"
#include "./common/DataDefine.h"
#include "./common/CreateDump.h"
#include "./Des/TaskKeyMgr.h"
#include "./network/IOCPServer.h"

#define  MAX_PATH 260

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CSDMApp, CWinApp)
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


CSDMApp::CSDMApp()	// CSDMApp 构造
{
}

int g_dayRemain;
int g_currentDoorNum = 0;// 当前接入门的数量

bool g_bIsTimeExpired;		//判断软件是不是过期
bool g_bIsRegistered;		//判断软件是不是已经注册
bool g_autoStartApp;
bool g_autoLogin;

bool g_setIpPort = false;
int g_port = 12345;
CString g_ip = _T("127.0.0.1");

CString g_strCurrentLanguage;	//当前的语言类型
CString g_strIniFilePath;		//ini文件的路径
CString g_strImageRootPath;	//图像保存根目录
CString g_exePath;

CLog g_log;
CSDMApp theApp;	// 唯一的一个 CSDMApp 对象

BYTE g_curDoorId;
BYTE g_address[200] = { 0 };
ClientInfo* g_CurClientInfo;
CDataBaseT*	g_pDBT = NULL;			//数据库操作
CIOCPServer* g_server = NULL;

CString GetIniStringValue(CString strSection, CString strSectionKey, CString strFileName);

BOOL CSDMApp::InitInstance()
{
    // 如果一个运行在 Windows XP 上的应用程序清单指定要
    // 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
    //则需要 InitCommonControls()。否则，将无法创建窗口。
    InitCommonControls();
    GdiplusStartup(&m_pGdiToken, &m_gdiplusStartupInput, NULL);
    CWinApp::InitInstance();
    //skinppLoadSkin(_T("Longhorn.ssk"));
    AfxEnableControlContainer();
    // 标准初始化
    // 如果未使用这些功能并希望减小
    // 最终可执行文件的大小，则应移除下列
    // 不需要的特定初始化例程
    // 更改用于存储设置的注册表项
    // TODO: 应适当修改该字符串，
    // 例如修改为公司或组织名
    SetRegistryKey(_T("安检门管理平台"));
    //system("rd /s/q D:\\SDServerLogFile");
    //初始化日志
    CTime cTime = CTime::GetCurrentTime();
    CString logFileName(_T(""));
    logFileName.Format("SDServer%d%.2d%.2d.log", cTime.GetYear(), cTime.GetMonth(), cTime.GetDay());
    g_log.Init(logFileName, 0);
    g_log.PrintTime(TRUE);
    g_log.PrintAppName(FALSE);
    #if(!_DEBUG)
    CTaskKeyMgr::Disable(CTaskKeyMgr::TASKKEYS, !CTaskKeyMgr::AreTaskKeysDisabled(), TRUE);
    #endif
    LOGMESSAGE("CSDMApp::InitInstance", "Start the application!");
    //获取初始化的语言类型
    CString strReturnPath = GetExePath();
    g_exePath = strReturnPath;
    g_strIniFilePath.Format("%s\\ini\\", strReturnPath);
    g_strCurrentLanguage = GetIniStringValue("Setting", "Language", "Setting");
    //获取图像保存根目录
    g_strImageRootPath = GetIniStringValue("Image", "SavePath", "CCSystem");
    
    if("" == g_strImageRootPath)
    {
        //默认路径
        g_strImageRootPath = "D:\\SDServerImage\\";
    }
    
    CString strClosePC = GetIniStringValue("FunctionItem", "ClosePC", "InsideConfig");
    g_autoStartApp = (0 == strClosePC.CompareNoCase("0")) ? false : true;
    
    /*	//英文系统下自启动失败
    if(g_autoStartApp)
    	AutoRunApp();
    */
    //保证一台机器只能启动一个实例
    if(!OnlyInstance())
    {
        LOGMESSAGE("CSDMApp::InitInstance", "The application has started,you can't repeat start!");
        exit(0);
    }
    
    //设置系统回调函数，当系统崩溃时，由操作系统调用，生成Dump文件
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)CrashHandler);
    CString strAutoLogin = GetIniStringValue("FunctionItem", "AutoLogin", "InsideConfig");
    g_autoLogin = (0 == strAutoLogin.CompareNoCase("0")) ? false : true;
    LOGMESSAGE("CSDMApp::InitInstance", "Enter query valid period module!");
    //获取注册信息
    //m_dayRemain = 100;
    CPublicFunction::GetRegValue();
    
    if(g_bIsRegistered)//已经注册则验证有效期
    {
        //判断是否已经过期
        g_bIsTimeExpired = true;
        //GetLastExitTime();
        CString strLicensePath = strReturnPath + "\\License.lic";
        OnQueryValidPeriod(_T(strLicensePath));
        
        if(g_bIsTimeExpired)
        {
            AfxMessageBox(GetIniStringValue("MainDialog", "SoftwareExpiredPrompt", g_strCurrentLanguage)); //软件有效期结束，请重新获取授权码注册
            LOGMESSAGE("CSDMApp::InitInstance", "Enter the register module!");
            CRegisterDlg dlg;
            
            //m_dayRemain = 100;
            if(IDCANCEL == dlg.DoModal())
            {
                LOGMESSAGE("CSDMApp::InitInstance", "Register failure!");
                exit(0);
            }
            
            LOGMESSAGE("CSDMApp::InitInstance", "Register success!");
        }
    }
    else//注册
    {
        LOGMESSAGE("CSDMApp::InitInstance", "Enter the register module!");
        CRegisterDlg dlg;
        
        if(IDCANCEL == dlg.DoModal())
        {
            LOGMESSAGE("CSDMApp::InitInstance", "Register failure!");
            exit(0);
        }
        
        LOGMESSAGE("CSDMApp::InitInstance", "Register success!");
    }
    
    LOGMESSAGE("CSDMApp::InitInstance", "End of the query valid period module and the software is effective!");
    CString strlogpath = strReturnPath + "\\Logs";
    DeleteExpiredLog(strlogpath);
    
    //改变显示器分辨率
    if(!ChangeDisMode())
    {
        return FALSE;
    }
    
    //初始化数据库连接
    LOGMESSAGE("CSDMApp::InitInstance", "Initial connect the database!");
    InitDBConnect(strReturnPath);
    LOGMESSAGE("CSDMApp::InitInstance", "Connect the database OK");
    #if(!NEEDREGISTER)
    g_dayRemain = 3650;
    #endif
    
    if(!g_autoLogin)
    {
        LOGMESSAGE("CSDMApp::InitInstance", "Enter the login module!");
        CLoginDlg login;
        login.Init(g_dayRemain);
        
        if(IDOK != login.DoModal())
        {
            LOGMESSAGE("CSDMApp::InitInstance", "Login failure!");
            FreeResource();
            //CPublicFunction::WriteExitTimeReg();
            exit(1);
        }
        
        LOGMESSAGE("CSDMApp::InitInstance", "Login success!");
    }
    else
    {
        LOGMESSAGE("CSDMApp::InitInstance", "The ini file disabled the login module!");
    }
    
    LOGMESSAGE("CSDMApp::InitInstance", "Start DelExpiredImage thread");
    AfxBeginThread(DelExpiredImage, 0, THREAD_PRIORITY_ABOVE_NORMAL, 0, 0, NULL);
    LOGMESSAGE("CSDMApp::InitInstance", "Begin initial the main module controls!");
    CSDMDlg dlg;
    m_pMainWnd = &dlg;
    INT_PTR nResponse = dlg.DoModal();
    
    if(nResponse == IDOK)
    {
    }
    else if(nResponse == IDCANCEL)
    {
    }
    
    // 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
    // 而不是启动应用程序的消息泵。
    return FALSE;
}

void CSDMApp::AutoRunApp()
{
    HKEY hNewKey;
    UINT retVal;
    LPCSTR rgsKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    CString strexePath = GetExePath() + "\\SDServer.exe";
    retVal = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, rgsKey, 0, KEY_WRITE, &hNewKey);
    
    if(retVal)
    {
        return;
    }
    
    LPSTR exepathName = strexePath.GetBuffer();
    strexePath.ReleaseBuffer();
    retVal =::RegSetValueEx(hNewKey, "THSDServer", 0, REG_SZ, (const  unsigned  char *)exepathName, MAX_PATH);
    
    if(retVal)
    {
        RegCloseKey(hNewKey);
        return;
    }
}

CString GetIniStringValue(CString strSection, CString strSectionKey, CString strFileName)
{
    CString strFile;
    CString strValue;
    char m_Value[500] = "";
    memset(m_Value, 0, 500);
    strFile.Format("%s%s%s", g_strIniFilePath, strFileName, ".ini") ;
    GetPrivateProfileString(strSection, strSectionKey, NULL, m_Value, 500, strFile);
    strValue = m_Value;
    return strValue;
}

int CSDMApp::ExitInstance()
{
    // TODO: 在此添加专用代码和/或调用基类
    g_log.Uninit();
    FreeResource();
    GdiplusShutdown(m_pGdiToken);
    return CWinApp::ExitInstance();
}

void CSDMApp::FreeResource()
{
    try
    {
        if(NULL != g_pDBT)
        {
            delete g_pDBT;
            g_pDBT = NULL;
        }
        
        CPublicFunction::WriteExitTimeReg();
    }
    catch(...) {}
}

//唯一app实例
BOOL CSDMApp::OnlyInstance()
{
    m_hMutex = CreateMutex(NULL, FALSE, "SDM.EXE");
    
    if(GetLastError() == ERROR_ALREADY_EXISTS)
    {
        CloseHandle(m_hMutex);
        m_hMutex = NULL;
        //MessageBox(GetIniStringValue("MainDialog","PromptMsgOne",g_strCurrentLanguage));
        return FALSE;
    }
    
    return TRUE;
}

BOOL CSDMApp::ChangeDisMode()
{
    //改变显示器分辨率支持1280*1024（17，9方屏），1440*900（19宽屏），1600*900(20宽屏)
    //1680*1050（22寸），1920*1080（21.5宽屏及更大宽屏）这几个主流的分辨率
    m_nFullWidth = GetSystemMetrics(SM_CXSCREEN);
    m_nFullHeight = GetSystemMetrics(SM_CYSCREEN);
    DEVMODE lpDevMode;
    lpDevMode.dmPelsWidth = m_nFullWidth;
    lpDevMode.dmPelsHeight = m_nFullHeight;
    
    if(lpDevMode.dmPelsWidth < 0x400)
    {
        lpDevMode.dmPelsWidth = 0x400;
    }
    
    if(lpDevMode.dmPelsHeight < 0x300)
    {
        lpDevMode.dmPelsHeight = 0x300;
    }
    
    lpDevMode.dmSize = sizeof(DEVMODE);
    lpDevMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
    
    if(ChangeDisplaySettings(&lpDevMode, NULL))
    {
        return FALSE;
    }
    
    return TRUE;
}

void CSDMApp::InitDBConnect(const CString &strReturnPath)
{
    //windows自带数据库
    CString pCurrentPath;
    pCurrentPath.Format("%s\\", strReturnPath);
    CString m_strDBPath;
    CString strConnect = "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=";
    strConnect.AppendFormat("%s%s", pCurrentPath, "SDM.mdb");
    strConnect.Append(";Jet OLEDB:Database Password=car2012");
    m_strDBPath.Format("%s%s", pCurrentPath, "SDServer.mdb");
    //SQL数据库
    //CString strConnect = "Provider=SQLOLEDB.1;Password=launch*0112;Persist Security Info=False;User ID=sa;Initial Catalog=SDM;Data Source=GOS-01708081054";
    //CString m_strDBPath = "D:\\DataNew\\SDM.mdf";
    g_pDBT = new CDataBaseT;
    
    //g_pDBT->CompactDB(m_strDBPath);          //数据库压缩
    if(!g_pDBT->InitDB(strConnect, m_strDBPath))
    {
        AfxMessageBox(GetIniStringValue("MainDialog", "ConnDatabaseFailPrompt", g_strCurrentLanguage)); //连接数据库失败,无法使用程序,请检查数据库连接!
        exit(0);
    }
}

void CSDMApp::DeleteExpiredLog(const CString& strFilepath)
{
    CString inipath = GetExePath() + "\\ini\\CCSystem.ini";
    char days[10] = "";
    memset(days, 0, 10);
    GetPrivateProfileString("Image", "SaveLogDays", "1", days, 80, inipath);
    int nday = atoi(days);
    WIN32_FIND_DATA findData;
    HANDLE hError = INVALID_HANDLE_VALUE;
    char filePathName[1024] = "";
    char fullLogName[1024] = "";
    strcpy(filePathName, strFilepath);
    strcat(filePathName, "\\*.*");
    hError = FindFirstFile(filePathName, &findData);
    CTime   curTime;
    
    if(hError == INVALID_HANDLE_VALUE)
    {
        return;
    }
    
    while(::FindNextFile(hError, &findData))
    {
        if(strcmp(findData.cFileName, ".") == 0
                || strcmp(findData.cFileName, "..") == 0
                /*|| (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)*/)
        {
            continue;
        }
        
        wsprintf(fullLogName, "%s\\%s", strFilepath, findData.cFileName);
        
        if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            DeleteExpiredLog(fullLogName);
        }
        
        CString strFileName(fullLogName);
        CFileStatus rStatus;
        
        if(CFile::GetStatus(strFileName, rStatus))
        {
            CTime now = CTime::GetCurrentTime();
            CTimeSpan ts = now - rStatus.m_ctime;
            
            if(ts.GetDays() > nday)
            {
                DeleteFile(strFileName);
            }
        }
    }
    
    if(hError != INVALID_HANDLE_VALUE)
    {
        FindClose(hError);
        hError = INVALID_HANDLE_VALUE;
    }
}

UINT CSDMApp::DelExpiredImage(LPVOID lParam)
{
    CString days = GetIniStringValue("Image", "ImageSaveDays", "CCSystem");
    int iDays = (days.IsEmpty()) ? 30 : atoi(days);
    WIN32_FIND_DATA findData;
    HANDLE hError = INVALID_HANDLE_VALUE;
    char filePathName[1024] = "";
    char fullLogName[1024] = "";
    strcpy(filePathName, g_strImageRootPath);
    strcat(filePathName, "\\*.*");
    hError = FindFirstFile(filePathName, &findData);
    CTime   curTime;
    
    if(hError == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
    
    while(::FindNextFile(hError, &findData))
    {
        if(strcmp(findData.cFileName, ".") == 0
                || strcmp(findData.cFileName, "..") == 0
                /*|| (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)*/)
        {
            continue;
        }
        
        wsprintf(fullLogName, "%s\\%s", g_strImageRootPath, findData.cFileName);
        
        if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            CTime now = CTime::GetCurrentTime();
            CTimeSpan ts = now - findData.ftCreationTime;
            
            if(ts.GetDays() > iDays)
            {
                CPublicFunction::DeleteDir(fullLogName);
            }
        }
    }
    
    if(hError != INVALID_HANDLE_VALUE)
    {
        FindClose(hError);
        hError = INVALID_HANDLE_VALUE;
    }
    
    return 1;
}

void CSDMApp::OnQueryValidPeriod(CString &strpath)
{
    LOGMESSAGE("CSDMApp::OnQueryValidPeriod", "Enter the check the license file module!");
    //读取文件中的机器码
    CFile fFile;
    
    if(!CPublicFunction::IsFileExist(strpath))
    {
        LOGMESSAGE("CSDMApp::OnQueryValidPeriod", "The license file is missing!");
        CPublicFunction::LostRegInfoPrompt();
        return;
    }
    
    //判断打开文件是否成功
    /*if(!fFile.Open(strpath,CFile::modeRead))
    {
    	CPublicFunction::LostRegInfoPrompt();
    }
    CString strLicense = "";
    fFile.Read(strLicense.GetBufferSetLength(fFile.GetLength()),fFile.GetLength());
    fFile.Close();
    strLicense.ReleaseBuffer();*/
    CString strLicense = CPublicFunction::ReadCodeFromFile(strpath);
    
    if(strLicense.IsEmpty())
    {
        LOGMESSAGE("CSDMApp::OnQueryValidPeriod", "The license file is invalid!");
        CPublicFunction::LostRegInfoPrompt();
        return;
    }
    
    LOGMESSAGE("CSDMApp::OnQueryValidPeriod", "The license file is valid and enter the check regcode module!");
    //CString strLicense = "9FEA569AC6DC9A554DB1C88366E288B1ADDF61C65AFBCFAD";
    unsigned long number = GetHardwareNumber();
    std::ostringstream stm;
    std::string strResult;
    stm << number;
    strResult = stm.str();
    TDES des;
    des.MakeKey(strResult);
    strLicense.Trim();
    int len = strLicense.GetLength();
    TMemoryStream stream;
    stream.SetSize(len);
    stream.SetPosition(0);
    stream.Write(strLicense.GetString(), len);
    TMemoryStream dest;
    
    if(des.DecryptNumber(stream, dest) == true)
    {
        std::string decry(dest.Memory());
        int last = decry.find_first_of(':');
        
        if(last < 4)
        {
            g_bIsTimeExpired = true;
            LOGMESSAGE("CSDMApp::OnQueryValidPeriod", " The RegCode is invalid!");
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
            LOGMESSAGE("CSDMApp::OnQueryValidPeriod", " The RegCode is invalid!");
            return;
        }
        
        char* pData = (char*)dest.Memory();
        pData = pData + posHead;
        TModuleData* pModuleData = (TModuleData*)pData;
        TDateTime minDate = TDateTime::OriginDateTime();
        TDateTime nowDateTime = TDateTime::GetCurrentTime();
        TDateTime startTime = minDate + (int)pModuleData->StartTime;
        pModuleData->Day += 1;
        TDateTime endTime = startTime + (int)pModuleData->Day;
        int CheckDay = (int)pModuleData->Day;
        
        if(CheckDay > 3600)
        {
            g_bIsTimeExpired = false;
            g_dayRemain = 50;
            LOGMESSAGE("CSDMApp::OnQueryValidPeriod", " The RegCode is Permanent invalid!");
            return;
        }
        
        GetLastExitTime();
        g_dayRemain = static_cast<int>(endTime) - static_cast<int>(nowDateTime);
        int year = endTime.GetYear() - minDate.GetYear();
        
        if(year > 30)
        {
            g_bIsTimeExpired = true;
            LOGMESSAGE("CSDMApp::OnQueryValidPeriod", " The RegCode is invalid!");
            return;
        }
        
        if(nowDateTime > endTime)
        {
            g_bIsTimeExpired = true;
            LOGMESSAGE("CSDMApp::OnQueryValidPeriod", " The RegCode is invalid!");
            return;
        }
        
        if(nowDateTime < startTime)
        {
            g_bIsTimeExpired = true;
            LOGMESSAGE("CSDMApp::OnQueryValidPeriod", " The RegCode is invalid!");
            return;
        }
        
        if(CheckDay <= 0 || g_dayRemain <= 0)
        {
            g_bIsTimeExpired = true;
            LOGMESSAGE("CSDMApp::OnQueryValidPeriod", " The RegCode is invalid!");
            return;
        }
        
        g_bIsTimeExpired = false;
        LOGMESSAGE("CSDMApp::OnQueryValidPeriod", " The RegCode is valid!");
    }
    else
    {
        g_bIsTimeExpired = true;
        LOGMESSAGE("CSDMApp::OnQueryValidPeriod", " The RegCode is invalid!");
        return;
    }
}

void CSDMApp::GetLastExitTime()
{
    //从注册表中获取上次登录时间,保证系统没有非法改变时间
    HKEY hKey;
    DWORD dwSize = 20, dwType = REG_SZ;
    char owner_Get[20];
    memset(owner_Get, 0, 20);
    LOGMESSAGE("CSDMApp::GetLastExitTime", "Enter open the regedit module!");
    
    if(ERROR_SUCCESS ==::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\CSecureCD"), NULL, KEY_READ, &hKey))
    {
        LOGMESSAGE("CSDMApp::GetLastExitTime", "Open the regedit success!");
        
        if(ERROR_SUCCESS ==::RegQueryValueEx(hKey, _T("ExitTime"), NULL, &dwType, (BYTE *)owner_Get, &dwSize))
        {
            //获取注册表中记录的登录时间
            char *pPos = strstr(owner_Get, "-");
            int iYear = atoi(owner_Get);
            int iMonth = atoi(pPos + 1);
            pPos = strstr(pPos + 1, "-");
            int iDay = atoi(pPos + 1);
            pPos = strstr(pPos + 1, "-");
            int iHour = atoi(pPos + 1);
            pPos = strstr(pPos + 1, "-");
            int iMin = atoi(pPos + 1);
            pPos = strstr(pPos + 1, "-");
            int iSec = atoi(pPos + 1);
            struct tm sourcedate;
            memset((void*)&sourcedate, 0, sizeof(sourcedate));
            sourcedate.tm_sec = iSec;
            sourcedate.tm_min = iMin;
            sourcedate.tm_hour = iHour;
            sourcedate.tm_mday = iDay;
            sourcedate.tm_mon = iMonth - 1;
            sourcedate.tm_year = iYear - 1900;
            time_t nLastLoginTime = mktime(&sourcedate);
            //跟当前时间进行比较
            time_t nRemainTime = 0;
            time_t nlongCurTime;
            time(&nlongCurTime);
            nRemainTime = nlongCurTime - nLastLoginTime;
            
            if(nRemainTime > 0)
            {
                CPublicFunction::WriteExitTimeReg();
            }
            else
            {
                if(abs((double)nRemainTime) > 60)
                {
                    AfxMessageBox(GetIniStringValue("MainDialog", "SystemTimeisModPrompt", g_strCurrentLanguage)); //系统时间与当前时间不一致，无法启动软件
                    LOGMESSAGE("CSDMApp::GetLastExitTime", "The system time is inconsistent with the current time,so the software can't start!");
                    ::RegCloseKey(hKey);
                    exit(0);
                }
            }
            
            LOGMESSAGE("CSDMApp::GetLastExitTime", "Get valid time success!");
        }
        else
        {
            LOGMESSAGE("CSDMApp::GetLastExitTime", "Get valid time failure!");
            ::RegCloseKey(hKey);
            CPublicFunction::LostRegInfoPrompt();
        }
    }
    else//打开注册表失败
    {
        LOGMESSAGE("CSDMApp::GetLastExitTime", "Open the regedit failure!");
        ::RegCloseKey(hKey);
        CPublicFunction::LostRegInfoPrompt();
    }
    
    ::RegCloseKey(hKey);
}
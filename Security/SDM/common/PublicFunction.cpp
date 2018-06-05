#include "stdafx.h"
#include "atlwin.h"
#include "PublicFunction.h"
#include "../Des/DESEx.h"
#include "../Des/XFolderSize.h"
#include "../system/RegisterDlg.h"


extern bool g_bIsRegistered;
extern bool g_bIsTimeExpired;
extern CString g_strImageRootPath;
extern CString g_strCurrentLanguage;
extern CString g_strIniFilePath;
extern CLog g_log;
extern CDataBaseT*	g_pDBT;

extern CString GetIniStringValue(CString strSection, CString strSectionKey, CString strFileName);

CPublicFunction::CPublicFunction(void)
{
}

CPublicFunction::~CPublicFunction(void)
{
}

bool CPublicFunction::IsFileExist(const CString& strName)
{
    bool bResult = false;
    char chFile[MAX_PATH * 2];
    memset(chFile, 0, MAX_PATH * 2);
    sprintf(chFile, "%s", strName);
    WIN32_FIND_DATA FindFileData;
    HANDLE hFindFile = FindFirstFile(chFile, &FindFileData);
    
    if(hFindFile != INVALID_HANDLE_VALUE
            && FindFileData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
    {
        bResult = true;
    }
    
    FindClose(hFindFile);
    return bResult;
}

CString CPublicFunction::GetImagePath(const CString &path, const CString &fileName, const CString &date)
{
    if(fileName.IsEmpty() || date.IsEmpty())
    {
        return "";
    }
    
    CString retVal = path;
    retVal += date.Left(10);
    retVal += "\\" + fileName;
    return retVal;
}

IPicture* CPublicFunction::GetPictureFromName(const char *path)
{
    try
    {
        IPicture *pPic = NULL;
        CFile file;
        
        if(!file.Open(path, CFile::modeRead))
        {
            return false;
        }
        
        DWORD m_nFileLen = file.GetLength();
        //分配内存空间
        HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, m_nFileLen);
        LPVOID lpBuf = ::GlobalLock(hMem);
        
        if(m_nFileLen == file.Read(lpBuf, m_nFileLen))
        {
            IStream *pStream = NULL;
            
            //从内存空间中创建流
            if(CreateStreamOnHGlobal(hMem, TRUE, &pStream) == S_OK)
            {
                //从流中创建图片
                if(OleLoadPicture(pStream, m_nFileLen, TRUE, IID_IPicture, (LPVOID *)&pPic) != S_OK)
                {
                    pPic = NULL;
                }
            }
        }
        
        file.Close();
        ::GlobalUnlock(hMem);
        GlobalFree(hMem);
        return pPic;
    }
    catch(...)
    {
        return NULL;
    }
}

void CPublicFunction::ShowImage(IPicture *pPicture, RECT rect, CDC *dc, long nPicWidth, long nPicHeight, int rgb,
                                int zoomStep, int zoomLeft, int zoomTop)
{
    if(NULL == pPicture)
    {
        return ;
    }
    
    HDC hdcDest = dc->m_hDC;
    //按窗口尺寸显示
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    CPen pen(PS_SOLID, 1, rgb);
    dc->SelectObject(&pen);
    dc->Rectangle(0, 0, width, height);
    long jpgWidth = 0;
    long jpgHeight = 0;
    pPicture->get_Width(&jpgWidth);
    pPicture->get_Height(&jpgHeight);
    
    if((nPicWidth < width) && (nPicHeight < height))
    {
        width = nPicWidth;
        height = nPicHeight;
    }
    
    long nLeft = (zoomStep - zoomLeft) * jpgWidth / 32;
    long nBottom = jpgHeight - (zoomStep - zoomTop) * jpgHeight / 32;
    long nWidth = jpgWidth - jpgWidth * zoomStep / 16;
    long nHeight = jpgHeight - jpgHeight * zoomStep / 16;
    //保持图片按正常比例显示而增加
    double jpgRatio = ((double)jpgWidth) / ((double)jpgHeight); //图片的宽长比
    double rRatio = ((double)width) / ((double)height);      //矩形区的宽长比
    double pRatio = 0;
    
    if(jpgRatio > rRatio)
    {
        pRatio = ((double)jpgWidth) / ((double)jpgHeight);
        
        if(pRatio >= rRatio)
        {
            nHeight = jpgHeight;
            nBottom = jpgHeight;
            height  = width / pRatio;
        }
        else
        {
            height  = rect.bottom - rect.top;
            nHeight = nWidth / rRatio;
            nBottom = jpgHeight - (jpgHeight - nHeight) * (zoomStep - zoomTop) / 32;
        }
    }
    else
    {
        pRatio = ((double)jpgWidth) / ((double)nHeight);
        
        if(pRatio <= rRatio) //图像宽度未超出显示区
        {
            width = height * pRatio;
            nLeft = 0;
            nWidth = jpgWidth;
        }
        else//图像宽度超出了显示区
        {
            width  = rect.right - rect.left;
            nWidth = jpgWidth * rRatio / pRatio;
            nLeft  = (jpgWidth - nWidth) * (zoomStep - zoomLeft) / 16;
        }
    }
    
    //实现图片居中显示
    int left = ((rect.right - rect.left) - width) / 2;
    int top = ((rect.bottom - rect.top) - height) / 2;
    pPicture->Render(hdcDest, left + 1, top + 1, width - 2, height - 2, nLeft, nBottom, nWidth, -nHeight, NULL);
}

BMPImageInfo CPublicFunction::GetBmpImageInfoByName(const char *dibFileName)
{
    char m_fileName[256] = {0};
    BITMAPFILEHEADER bitmapFileHeader;
    BMPImageInfo bmpImageInfo;
    bmpImageInfo.pBmpData = NULL;
    bmpImageInfo.dSize = 0;
    strcpy(m_fileName, dibFileName);
    CFile dibFile(m_fileName, CFile::modeRead);
    dibFile.Read((void*)&bitmapFileHeader, sizeof(BITMAPFILEHEADER));
    
    if(bitmapFileHeader.bfType == 0x4d42)
    {
        DWORD fileLength = dibFile.GetLength();
        bmpImageInfo.dSize = fileLength - sizeof(BITMAPFILEHEADER);
        bmpImageInfo.pBmpData = (BYTE*)GlobalAlloc(GMEM_MOVEABLE, bmpImageInfo.dSize);
        dibFile.Read((void*)bmpImageInfo.pBmpData, bmpImageInfo.dSize);
        bmpImageInfo.nWidth = ((LPBITMAPINFOHEADER)&bitmapFileHeader)->biWidth;
        bmpImageInfo.mHeight = ((LPBITMAPINFOHEADER)&bitmapFileHeader)->biHeight;
        dibFile.Close();
    }
    
    return bmpImageInfo;
}


double CPublicFunction::GetShowRadio(int clickCount)
{
    double retVal = 1;
    
    switch(clickCount)
    {
        case 1:
            retVal = 1.1;
            break;
            
        case 2:
            retVal = 1.2;
            break;
            
        case 3:
            retVal = 1.3;
            break;
            
        case 4:
            retVal = 1.4;
            break;
            
        case 5:
            retVal = 1.5;
            break;
            
        case 6:
            retVal = 1.6;
            break;
            
        case 7:
            retVal = 1.7;
            break;
            
        case 8:
            retVal = 1.8;
            break;
            
        case 9:
            retVal = 1.9;
            break;
            
        case 10:
            retVal = 2;
            break;
            
        case 11:
            retVal = 2.2;
            break;
            
        case 12:
            retVal = 2.4;
            break;
            
        case 13:
            retVal = 2.6;
            break;
            
        case 14:
            retVal = 2.8;
            break;
            
        case 15:
            retVal = 3;
            break;
            
        case 16:
            retVal = 3.3;
            break;
            
        case 17:
            retVal = 3.6;
            break;
            
        case 18:
            retVal = 3.9;
            break;
            
        case 19:
            retVal = 4.2;
            break;
            
        case 20:
            retVal = 4.6;
            break;
            
        case 21:
            retVal = 5;
            break;
            
        case 22:
            retVal = 5.5;
            break;
            
        case 23:
            retVal = 6;
            break;
            
        case 24:
            retVal = 6.5;
            break;
            
        case 25:
            retVal = 7;
            break;
            
        case 26:
            retVal = 7.5;
            break;
            
        case 27:
            retVal = 8;
            break;
            
        case 28:
            retVal = 8.5;
            break;
            
        case 29:
            retVal = 9;
            break;
            
        case 30:
            retVal = 9.5;
            break;
            
        case 31:
            retVal = 10;
            break;
            
        case 32:
            retVal = 11;
            break;
            
        case 33:
            retVal = 12;
            break;
            
        case 34:
            retVal = 13;
            break;
            
        case 35:
            retVal = 14;
            break;
            
        case 36:
            retVal = 15;
            break;
            
        case 37:
            retVal = 16;
            break;
            
        default:
            retVal = 1;
            break;
    }
    
    return retVal;
}

int CPublicFunction::GetClickCount(double radio)
{
    int m_realSizeEquClickCount = 0;
    
    if(radio <= 1)
    {
        m_realSizeEquClickCount = 0;
    }
    else if(radio > 1 && radio <= 1.1)
    {
        m_realSizeEquClickCount = 1;
    }
    else if(radio > 1.1 && radio <= 1.2)
    {
        m_realSizeEquClickCount = 2;
    }
    else if(radio > 1.2 && radio <= 1.3)
    {
        m_realSizeEquClickCount = 3;
    }
    else if(radio > 1.3 && radio <= 1.4)
    {
        m_realSizeEquClickCount = 4;
    }
    else if(radio > 1.4 && radio <= 1.5)
    {
        m_realSizeEquClickCount = 5;
    }
    else if(radio > 1.5 && radio <= 1.6)
    {
        m_realSizeEquClickCount = 6;
    }
    else if(radio > 1.6 && radio <= 1.7)
    {
        m_realSizeEquClickCount = 7;
    }
    else if(radio > 1.7 && radio <= 1.8)
    {
        m_realSizeEquClickCount = 8;
    }
    else if(radio > 1.8 && radio <= 1.9)
    {
        m_realSizeEquClickCount = 9;
    }
    else if(radio > 1.9 && radio <= 2)
    {
        m_realSizeEquClickCount = 10;
    }
    else if(radio > 2 && radio <= 2.2)
    {
        m_realSizeEquClickCount = 11;
    }
    else if(radio > 2.2 && radio <= 2.4)
    {
        m_realSizeEquClickCount = 12;
    }
    else if(radio > 2.4 && radio <= 2.6)
    {
        m_realSizeEquClickCount = 13;
    }
    else if(radio > 2.6 && radio <= 2.8)
    {
        m_realSizeEquClickCount = 14;
    }
    else if(radio > 2.8 && radio <= 3)
    {
        m_realSizeEquClickCount = 15;
    }
    else if(radio > 3 && radio <= 3.3)
    {
        m_realSizeEquClickCount = 16;
    }
    else if(radio > 3.3 && radio <= 3.6)
    {
        m_realSizeEquClickCount = 17;
    }
    else if(radio > 3.6 && radio <= 3.9)
    {
        m_realSizeEquClickCount = 18;
    }
    else if(radio > 3.9 && radio <= 4.2)
    {
        m_realSizeEquClickCount = 19;
    }
    else if(radio > 4.2 && radio <= 4.6)
    {
        m_realSizeEquClickCount = 20;
    }
    else if(radio > 4.6 && radio <= 5)
    {
        m_realSizeEquClickCount = 21;
    }
    else if(radio > 5 && radio <= 5.5)
    {
        m_realSizeEquClickCount = 22;
    }
    else if(radio > 5.5 && radio <= 6)
    {
        m_realSizeEquClickCount = 23;
    }
    else if(radio > 6 && radio <= 6.5)
    {
        m_realSizeEquClickCount = 24;
    }
    else if(radio > 6.5 && radio <= 7)
    {
        m_realSizeEquClickCount = 25;
    }
    else if(radio > 7 && radio <= 7.5)
    {
        m_realSizeEquClickCount = 26;
    }
    else if(radio > 7.5 && radio <= 8)
    {
        m_realSizeEquClickCount = 27;
    }
    else if(radio > 8 && radio <= 8.5)
    {
        m_realSizeEquClickCount = 28;
    }
    else if(radio > 8.5 && radio <= 9)
    {
        m_realSizeEquClickCount = 29;
    }
    else if(radio > 9 && radio <= 9.5)
    {
        m_realSizeEquClickCount = 30;
    }
    else if(radio > 9.5 && radio <= 10)
    {
        m_realSizeEquClickCount = 31;
    }
    else if(radio > 10 && radio <= 11)
    {
        m_realSizeEquClickCount = 32;
    }
    else if(radio > 11 && radio <= 12)
    {
        m_realSizeEquClickCount = 33;
    }
    else if(radio > 12 && radio <= 13)
    {
        m_realSizeEquClickCount = 34;
    }
    else if(radio > 13 && radio <= 14)
    {
        m_realSizeEquClickCount = 35;
    }
    else if(radio > 14 && radio <= 15)
    {
        m_realSizeEquClickCount = 36;
    }
    else if(radio > 15 && radio <= 16)
    {
        m_realSizeEquClickCount = 37;
    }
    
    return m_realSizeEquClickCount;
}

void CPublicFunction::DrawRectangeSide(CDC *pDc, CRect rect, int nWidth, COLORREF crColor)
{
    //DC由调用者释放
    CPen pen(PS_SOLID, nWidth, crColor);
    pDc->SelectObject(&pen);
    pDc->MoveTo(rect.left, rect.top);
    pDc->LineTo(rect.right, rect.top);
    pDc->LineTo(rect.right, rect.bottom);
    pDc->LineTo(rect.left, rect.bottom);
    pDc->LineTo(rect.left, rect.top);
}

CString CPublicFunction::GetCurrentDateTime()
{
    CTime cTime = CTime::GetCurrentTime();
    CString strCurrentTime(_T(""));
    strCurrentTime.Format("%04d-%.2d-%.2d %.2d:%.2d:%.2d",
                          cTime.GetYear(), cTime.GetMonth(), cTime.GetDay(),
                          cTime.GetHour(), cTime.GetMinute(), cTime.GetSecond());
    return strCurrentTime;
}

CString CPublicFunction::GetImageNameTimeSpan(CString &curDT)
{
    CTime cTime = CTime::GetCurrentTime();
    CString strCurrentTime(_T(""));
    strCurrentTime.Format("%04d%.2d%.2d%.2d%.2d%.2d",
                          cTime.GetYear(), cTime.GetMonth(), cTime.GetDay(),
                          cTime.GetHour(), cTime.GetMinute(), cTime.GetSecond());
    curDT.Format("%04d-%.2d-%.2d %.2d:%.2d:%.2d",
                 cTime.GetYear(), cTime.GetMonth(), cTime.GetDay(),
                 cTime.GetHour(), cTime.GetMinute(), cTime.GetSecond());
    return strCurrentTime;
}

CString CPublicFunction::GetDate()
{
    CTime cTime = CTime::GetCurrentTime();
    CString strCurrentTime(_T(""));
    strCurrentTime.Format("%04d-%.2d-%.2d",
                          cTime.GetYear(), cTime.GetMonth(), cTime.GetDay());
    return strCurrentTime;
}

CString CPublicFunction::CreateCurrentDTPath()
{
    CString strParentDirectory = g_strImageRootPath; //一级目录文件名，"D:\\CarCheckImage\\";
    CString strChildDirectory = GetDate();//二级目录文件名
    CString strtemp = strParentDirectory + strChildDirectory + "\\";
    
    if(PathIsDirectory(strParentDirectory))//判断目录是否存在，不存在，返回FALSE
    {
        if(PathIsDirectory(strtemp))
        {
            return strtemp;
        }
        else
        {
            if(CreateDirectory(strtemp, NULL)) //返回零，表示失败，非零，成功
            {
                return strtemp;
            }
            else
            {
                return "";
            }
        }
    }
    else
    {
        if(CreateDirectory(strParentDirectory, NULL))
        {
            if(PathIsDirectory(strtemp))
            {
                return strtemp;
            }
            else
            {
                if(CreateDirectory(strtemp, NULL))
                {
                    return strtemp;
                }
                else
                {
                    return "";
                }
            }
        }
        else
        {
            return "";
        }
    }
}

void CPublicFunction::WriteRunLogFile(CString Source, CString Message)
{
    g_log.InsertMsg("", 3, Source + "------" + Message);
}

void CPublicFunction::WriteLog()
{
    LOGMESSAGE("CPublicFunction::WriteLog", "Exception");
}

int CPublicFunction::SetRegValue(LPCSTR KeyValueName, DWORD &dwVal)
{
    HKEY m_hKey;
    DWORD dwDisposition;
    int nValue = 0;
    //打开注册表
    LONG ret =::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\CSecureCD"), 0, KEY_WRITE | KEY_READ, &m_hKey);
    
    if(ret != ERROR_SUCCESS)
    {
        ret =::RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\CSecureCD"), 0L, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &m_hKey, &dwDisposition);
                              
        if(ret != ERROR_SUCCESS)
        {
            nValue = 1;
            return nValue;
        }
        
        nValue = 0;
    }
    
    //写入特征值
    //CString str = KeyValueName;
    ret = ::RegSetValueEx(m_hKey, KeyValueName, NULL, REG_DWORD, (CONST BYTE*)&dwVal, sizeof(DWORD));
    
    if(ret != ERROR_SUCCESS)
    {
        nValue = 2;
    }
    
    //关闭注册表
    ::RegCloseKey(m_hKey);
    return nValue;
}

//获取注册信息
void CPublicFunction::GetRegValue()
{
    g_bIsRegistered = false;
    HKEY m_hKey;
    //打开注册表
    LONG ret =::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\CSecureCD"), 0, KEY_READ, &m_hKey);
    
    if(ret != ERROR_SUCCESS)
    {
        return;
    }
    
    //从注册表中读入特征值
    DWORD dwVal = 0, dwType = 0, dwLen = 20;
    ret = ::RegQueryValueEx(m_hKey, _T("RegisterData"), NULL, &dwType, (unsigned char*)&dwVal, &dwLen);
    
    if(ret != ERROR_SUCCESS)	//读取数据失败
    {
        return;
    }
    
    //检查读到的特征值是否为1，如果是，则表明软件已经注册
    g_bIsRegistered = (dwVal == 1);
    //关闭注册表
    ::RegCloseKey(m_hKey);
}

//注册表中写入本次登录时间
void CPublicFunction::WriteExitTimeReg()
{
    HKEY hKey;
    DWORD dwSize = 20;
    DWORD dwDisposition;
    time_t nlongCurTime;
    time(&nlongCurTime);
    char chTmp[20];
    memset(chTmp, 0, 20);
    struct tm *p;
    p = localtime(&nlongCurTime);
    p->tm_year = p->tm_year + 1900;
    p->tm_mon = p->tm_mon + 1;
    sprintf(chTmp, "%04d-%02d-%02d-%02d-%02d-%02d", p->tm_year, p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    
    if(ERROR_SUCCESS !=::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\CSecureCD\\"), NULL, KEY_WRITE, &hKey))
    {
        //创建相应的Key,如果创建失败，给出提示信息，直接返回
        if(ERROR_SUCCESS !=::RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\CSecureCD"), 0L, NULL,
                                            REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition))
        {
            LOGMESSAGE("CPublicFunction::WriteExitTimeReg", "When writing,open the regedit failed!");
            return ;
        }
    }
    
    if(ERROR_SUCCESS ==::RegSetValueEx(hKey, _T("ExitTime"), NULL, REG_SZ, (BYTE *)chTmp, dwSize))
    {
        LOGMESSAGE("CPublicFunction::WriteExitTimeReg", "Write the regedit success!");
    }
    else
    {
        LOGMESSAGE("CPublicFunction::WriteExitTimeReg", "Write the regedit failure!");
    }
    
    ::RegCloseKey(hKey);
}

//软件信息丢失时提示重新注册
void CPublicFunction::LostRegInfoPrompt()
{
    AfxMessageBox(GetIniStringValue("PublicMsg", "SoftInfoLostPrompt", g_strCurrentLanguage)); //软件相关信息被破坏，请重新注册
    LOGMESSAGE("CPublicFunction::LostRegInfoPrompt", "Enter the register module!");
    CRegisterDlg dlg;
    
    if(IDCANCEL == dlg.DoModal())
    {
        LOGMESSAGE("CPublicFunction::LostRegInfoPrompt", "Register failure!");
        DWORD dwtemp = 0;
        SetRegValue("RegisterData", dwtemp);
        exit(0);
    }
    
    LOGMESSAGE("CPublicFunction::LostRegInfoPrompt", "Register success!");
}

BOOL CPublicFunction::WriteCodeToFile(const CString& code, const CString &path)
{
    des_ctx dc;
    CString m_szEnString = code;
    int iLen = m_szEnString.GetLength();
    
    if(iLen < 16)
    {
        AfxMessageBox(GetIniStringValue("PublicMsg", "WriteCodeFileFailPrompt", g_strCurrentLanguage)); //写注册码失败
        return FALSE;
    }
    
    CString szRes;
    CString szKey(_T("SZ@$%2013THSD!%^&*"));
    des_key(&dc, (unsigned char*)(LPCTSTR)szKey);
    des_enc(&dc, (unsigned char*)(LPCTSTR)m_szEnString, 2);
    CFile mFile;
    
    if(!mFile.Open(path, CFile::modeWrite | CFile::modeCreate))
    {
        AfxMessageBox(GetIniStringValue("PublicMsg", "WriteCodeFileFailPrompt", g_strCurrentLanguage)); //写注册码失败
        return FALSE;
    }
    
    mFile.Write(m_szEnString, iLen);
    mFile.Close();
    return TRUE;
}

CString CPublicFunction::ReadCodeFromFile(const CString& path)
{
    CFile mFile;
    
    if(!mFile.Open(_T(path), CFile::modeRead))
    {
        AfxMessageBox(GetIniStringValue("PublicMsg", "OpenCodeFileFailPrompt", g_strCurrentLanguage)); //打开注册码文件失败
        return "";
    }
    
    char c[500] = {0};
    UINT iSize = mFile.Read(c, 500);
    
    if(iSize <= 0)
    {
        return "";
    }
    
    int iReadLen = iSize;
    mFile.Close();
    CString szKey(_T("SZ@$%2013THSD!%^&*"));
    des_ctx dc;
    des_key(&dc, (unsigned char*)(LPCTSTR)szKey);
    des_dec(&dc, (unsigned char*)c, 2);
    CString szRead(c);
    CString strDecode(_T(""));
    strDecode = szRead.Mid(0, iReadLen);
    return strDecode;
}

BOOL CPublicFunction::ContainSpecialChar(const CString &str)
{
    if((-1 != str.Find('\'')) ||
            (-1 != str.Find('\"')) ||
            (-1 != str.Find('<'))  ||
            (-1 != str.Find('>'))  ||
            (-1 != str.Find('\\')) ||
            (-1 != str.Find(':'))  ||
            (-1 != str.Find('?'))  ||
            (-1 != str.Find('/')) ||
            (-1 != str.Find('*'))
      )
    {
        return TRUE;
    }
    
    return FALSE;
}

BOOL CPublicFunction::DiskHasEnoughSpace(const CString &szPath, DWORD &crisisValue)
{
    DWORD dwFreeDiskSpace;
    BOOL retVal = TRUE;
    ULARGE_INTEGER uiFreeBytesAvailableToCaller;
    ULARGE_INTEGER uiTotalNumberOfBytes;
    ULARGE_INTEGER uiTotalNumberOfFreeBytes;
    
    if(GetDiskFreeSpaceEx(szPath, &uiFreeBytesAvailableToCaller,
                          &uiTotalNumberOfBytes,
                          &uiTotalNumberOfFreeBytes))
    {
        dwFreeDiskSpace  = (DWORD)(uiFreeBytesAvailableToCaller.QuadPart >> 20);
        
        if(dwFreeDiskSpace < crisisValue)
        {
            crisisValue = dwFreeDiskSpace;
            retVal = FALSE;
        }
    }
    
    return retVal;
}

/*
* 路径串str的最后不要加'/'
*/
void CPublicFunction::DeleteDir(CString str)
{
    CFileFind finder; //文件查找类
    CString strdel, strdir; //strdir:要删除的目录，strdel：要删除的文件
    strdir.Format(_T("%s\\*.*"), str);//删除文件夹，先要清空文件夹,加上路径,注意加"//"
    BOOL b_finded = (BOOL)finder.FindFile(strdir);
    
    while(b_finded)
    {
        b_finded = (BOOL)finder.FindNextFile();
        
        if(finder.IsDots()) { continue; }//找到的是当前目录或上级目录则跳过
        
        strdel = finder.GetFileName(); //获取找到的文件名
        
        if(finder.IsDirectory())   //如果是文件夹
        {
            strdel = str + "\\" + strdel; //加上路径,注意加"//"
            DeleteDir(strdel); //递归删除文件夹
        }
        else //不是文件夹
        {
            strdel = str + "\\" + strdel;
            
            if(finder.IsReadOnly())//清除只读属性
            {
                SetFileAttributes(strdel, GetFileAttributes(strdel) & (~FILE_ATTRIBUTE_READONLY));
            }
            
            DeleteFile(strdel); //删除文件(API)
        }
    }
    
    finder.Close();
    BOOL ret = RemoveDirectory(str); //删除文件夹(API)
    
    if(!ret)
    {
        DelFolder(str);
        //DWORD eCode=GetLastError();
        //ShowMessageBox(_T("删除文件夹失败"));
    }
}

double CPublicFunction::GetFolderSizeEx(const CString &szPath)
{
    //return 0;
    LARGE_INTEGER li;
    DWORD dwFolderCount = 0;
    DWORD dwFileCount = 0;
    double retVal = 0;
    CXFolderSize fs;
    BOOL rc = fs.GetFolderSize(szPath, TRUE, TRUE, &li, &dwFolderCount, &dwFileCount);
    
    if(!rc)
    {
        return 0;
    }
    
    if(li.LowPart >= 0)
    {
        retVal = static_cast<double>((static_cast<double>(li.LowPart) / 1024) / 1024);
    }
    
    return retVal;
}

BOOL CPublicFunction::DelFolder(const CString &szPath)
{
    SHFILEOPSTRUCT FileOp;
    FileOp.fFlags = FOF_NOCONFIRMATION;
    FileOp.hNameMappings = NULL;
    FileOp.hwnd = NULL;
    FileOp.lpszProgressTitle = NULL;
    FileOp.pFrom = szPath;
    FileOp.pTo = NULL;
    FileOp.wFunc = FO_DELETE;
    return SHFileOperation(&FileOp) == 0;
}

BOOL CPublicFunction::CreateDir(const CString& dir)
{
    try
    {
        CFileFind   aFind;
        
        if((aFind.FindFile(dir, 0)))
        {
            return TRUE;
        }
        
        //创建文件夹
        return CreateDirectory(dir, NULL);
    }
    catch(...)
    {
        return FALSE;
    }
}

vector<CString> CPublicFunction::SplitString(const CString& stringToSplit, const char split)
{
    vector<CString> vStrings;
    
    if(stringToSplit.IsEmpty())
    {
        return vStrings;
    }
    
    int pos = 0;
    int len = 0;
    CString str(_T(""));
    CString tmp = stringToSplit;
    
    while((pos = tmp.Find(split)) > 0)
    {
        str = tmp.Left(pos);
        len = tmp.GetLength();
        tmp = tmp.Right(len - pos - 1);
        vStrings.push_back(str);
    }
    
    //加入最后一个元素
    vStrings.push_back(tmp);
    return vStrings;
}

bool CPublicFunction::ThePlateNoIsExsit(const CString &plateNo)
{
    //修改了车牌号码
    char pWhere[150] = "";
    sprintf(pWhere, " where LiencePlate = '%s'", plateNo);
    BOOL retVal = g_pDBT->OpenSql("CheckInfo", "*", pWhere, "Order by ID DESC ");
    
    if(retVal && g_pDBT->GetDBRecordSet()->GetRecordCount() > 0)
    {
        return true;
    }
    
    return false;
}

CheckInfo CPublicFunction::GetCheckInfo()
{
    CheckInfo ckeckInfo;
    g_pDBT->GetDBRecordSet()->GetValueString(ckeckInfo.SDoorID, "SDoorID");
    g_pDBT->GetDBRecordSet()->GetValueString(ckeckInfo.checkTime, "CheckTime");
    g_pDBT->GetDBRecordSet()->GetValueString(ckeckInfo.autoJudged, "autoJudged");
    g_pDBT->GetDBRecordSet()->GetValueString(ckeckInfo.userName, "userName");
    g_pDBT->GetDBRecordSet()->GetValueString(ckeckInfo.userID, "userID");
    g_pDBT->GetDBRecordSet()->GetValueString(ckeckInfo.captureImageName, "captureImageName");
    g_pDBT->GetDBRecordSet()->GetValueString(ckeckInfo.existHisRecord, "ExistHisRecord");
    g_pDBT->GetDBRecordSet()->GetValueString(ckeckInfo.checkOperator, "checkOperator");
    g_pDBT->GetDBRecordSet()->GetValueString(ckeckInfo.describe, "describe");
    g_pDBT->GetDBRecordSet()->GetValueString(ckeckInfo.BlackWhiteList, "BlackWhiteList");
    return ckeckInfo;
}

BOOL CPublicFunction::ModifyCheckResult(const CString& id, int checkResult, const CString& field)
{
    //修改数据库
    BOOL retVal = FALSE;
    char pWhere[150] = "";
    sprintf(pWhere, " where ID = %d ", atoi(id));
    retVal = g_pDBT->OpenSql("CheckInfo", "*", pWhere);
    
    if(retVal)
    {
        if(g_pDBT->GetDBRecordSet()->GetRecordCount() > 0)
        {
            retVal = g_pDBT->GetDBRecordSet()->PutCollect(field, checkResult);
            retVal = g_pDBT->GetDBRecordSet()->Update();
            retVal = g_pDBT->GetDBRecordSet()->Close();
        }
        else
        {
            retVal = FALSE;
        }
    }
    
    return retVal;
}

BOOL CPublicFunction::UpdateDB(CheckInfo &checkInfo, const CString& plateNo, int vpId, const CString& saveName, const CString& plateNoImageName,
                               const CString &captureImageName, const CString& exist)
{
    //修改数据库
    BOOL retVal = FALSE;
    char pWhere[150] = "";
    
    if(checkInfo.Id.IsEmpty())
    {
        //修改最新一条记录（当向CheckInfo表插入第一条记录时）
        retVal = g_pDBT->OpenSql("CheckInfo", "*", "", " order by CheckTime ", 1);
    }
    else
    {
        sprintf(pWhere, " where ID = %d ", atoi(checkInfo.Id));
        retVal = g_pDBT->OpenSql("CheckInfo", "*", pWhere);
    }
    
    if(retVal)
    {
        if(g_pDBT->GetDBRecordSet()->GetRecordCount() > 0)
        {
            retVal = g_pDBT->GetDBRecordSet()->PutCollect("LiencePlate", (LPCSTR)plateNo);
            retVal = g_pDBT->GetDBRecordSet()->PutCollect("VPID", vpId);
            retVal = g_pDBT->GetDBRecordSet()->PutCollect("SaveName", saveName);
            retVal = g_pDBT->GetDBRecordSet()->PutCollect("PlateNoImageName", plateNoImageName);
            retVal = g_pDBT->GetDBRecordSet()->PutCollect("CaptureImageName", captureImageName);
            retVal = g_pDBT->GetDBRecordSet()->PutCollect("ExistHisRecord", atoi(exist));
            retVal = g_pDBT->GetDBRecordSet()->Update();
            retVal = g_pDBT->GetDBRecordSet()->Close();
        }
        else
        {
            retVal = FALSE;
        }
    }
    
    return retVal;
}

void CPublicFunction::AdjustCroodForZoomIn(int& destX, int& destY, int& displayW, int& displayH, int& clientW, int& clientH)
{
    //X方向
    if(displayW >= clientW)
    {
        //图像的显示大小超过了显示屏幕
        if(destX >= 0)
        {
            destX = 0;
        }
        else
        {
            if((destX + displayW) < clientW)
            {
                destX = clientW - displayW;
            }
        }
    }
    else if(displayW < clientW)
    {
        //图像的显示大小没有超过显示屏幕
        if(destX < 0)
        {
            destX = 0;
        }
        else
        {
            if((destX + displayW) >= clientW)
            {
                destX = (clientW - displayW) / 2;
            }
        }
    }
    
    //Y方向
    if(displayH >= clientH)
    {
        //图像的显示大小超过了显示屏幕
        if(destY >= 0)
        {
            destY = 0;
        }
        else
        {
            if((destY + displayH) < clientH)
            {
                destY = clientH - displayH;
            }
        }
    }
    else if(displayH < clientH)
    {
        //图像的显示大小没有超过显示屏幕
        if(destY < 0)
        {
            destY = 0;
        }
        else
        {
            //往下移动
            if((destY + displayH) >= clientH)
            {
                destY = (clientH - displayH) / 2;
            }
        }
    }
}

void CPublicFunction::AdjustCroodForDrag(int& destX, int& destY, int& displayW, int& displayH, int& clientW, int& clientH, int moveW, int moveH)
{
    //X方向
    if(displayW >= clientW)
    {
        //图像的显示大小超过了显示屏幕
        if(moveW < 0)
        {
            //往左移动
            if((destX + displayW) < clientW)
            {
                destX = clientW - displayW;
            }
        }
        else
        {
            //往右移动
            if(destX > 0)
            {
                destX = 0;
            }
        }
    }
    else if(displayW < clientW)
    {
        //图像的显示大小没有超过显示屏幕
        if(moveW < 0)
        {
            //往左移动
            if(destX < 0)
            {
                destX = 0;
            }
        }
        else
        {
            //往右移动
            if((destX + displayW) >= clientW)
            {
                destX = clientW - displayW;
            }
        }
    }
    
    //Y方向
    if(displayH >= clientH)
    {
        //图像的显示大小超过了显示屏幕
        if(moveH < 0)
        {
            //往上移动
            if((destY + displayH) < clientH)
            {
                destY = clientH - displayH;
            }
        }
        else
        {
            //往下移动
            if(destY > 0)
            {
                destY = 0;
            }
        }
    }
    else if(displayH < clientH)
    {
        //图像的显示大小没有超过显示屏幕
        if(moveH < 0)
        {
            //向上移动
            if(destY < 0)
            {
                destY = 0;
            }
        }
        else
        {
            //往下移动
            if((destY + displayH) >= clientH)
            {
                destY = clientH - displayH;
            }
        }
    }
}

void CPublicFunction::AdjustShowSnapRect(CRect rect, int nImageWidth, int nImageHeight, int& destX, int& destY, int& destW, int& destH)
{
    if(((double)rect.Width() / (double)rect.Height()) > ((double)nImageWidth / nImageHeight))
    {
        destH = rect.Height();
        destW = (int)((double)rect.Height() / (double)nImageHeight * nImageWidth);
        destX = (rect.Width() - destW) / 2;
        destY = 0;
    }
    else
    {
        destW = rect.Width();
        destH = (int)((double)rect.Width() / (double)nImageWidth * nImageHeight);
        destY = (rect.Height() - destH) / 2;
        destX = 0;
    }
}

void CPublicFunction::ShowImage(CDC *dc, HDC hdc, CString &fileName, int ctrIID, RECT rect, int rgb)
{
    try
    {
        IPicture *pPicture = GetPictureFromName(fileName.GetBuffer());
        
        if(NULL == pPicture)
        {
            return ;
        }
        
        CBrush brush(RGB(0, 0, 0));
        CBrush *oldBrush = dc->SelectObject(&brush);
        long jpgWidth = 0;
        long jpgHeight = 0;
        pPicture->get_Width(&jpgWidth);
        pPicture->get_Height(&jpgHeight);
        //图片的实际长宽
        long nPicWidth = MulDiv(jpgWidth, GetDeviceCaps(hdc, LOGPIXELSX), HIMETRIC_PER_INCH);
        long nPicHeight = MulDiv(jpgHeight, GetDeviceCaps(hdc, LOGPIXELSY), HIMETRIC_PER_INCH);
        ShowImage(pPicture, rect, dc, nPicWidth, nPicHeight, rgb, 0, 0, 0);
        dc->SelectObject(oldBrush);
        pPicture->Release();
    }
    catch(...) {}
}

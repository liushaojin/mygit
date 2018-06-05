#ifndef CREATEDUMP_TH_H_
#define CREATEDUMP_TH_H_

#include "PublicFunction.h"

/************************************************************
 生成Dump文件 获取程序运行的路径
 ************************************************************/
CString GetExePath()
{
    CString strFullPath;
    char m_Buff[MAX_PATH];
    memset(m_Buff, 0, MAX_PATH);
    GetModuleFileName(NULL, m_Buff, MAX_PATH);
    strFullPath = m_Buff;
    int nReturnLen = strFullPath.ReverseFind('\\');
    CString strReturnPath = strFullPath.Left(nReturnLen);
    return strReturnPath;
}

//得到当前时间
CString GetPresentTime()
{
    CTime cTime = CTime::GetCurrentTime();
    CString strCurrentTime(_T(""));
    strCurrentTime.Format("%d%.2d%.2d_%.2d%.2d%.2d",
                          cTime.GetYear(), cTime.GetMonth(), cTime.GetDay(),
                          cTime.GetHour(), cTime.GetMinute(), cTime.GetSecond());
    return strCurrentTime;
}

//创建Dump文件
void CreateDumpFile(LPCSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException)
{
    // 创建Dump文件
    HANDLE hDumpFile = CreateFile(lpstrDumpFilePathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    // Dump信息
    MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
    dumpInfo.ExceptionPointers = pException;
    dumpInfo.ThreadId = GetCurrentThreadId();
    dumpInfo.ClientPointers = TRUE;
    // 写入Dump文件内容
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
    CloseHandle(hDumpFile);
}

// 处理异常的回调函数
LONG CrashHandler(EXCEPTION_POINTERS *pException)
{
    // 以当前时间为文件名
    CString path = GetExePath();
    path += "\\Dump\\";
    
    if(!PathFileExists(path))
    {
        CreateDirectory(path, NULL);
    }
    
    CString strDumpFileName = path;
    strDumpFileName += GetPresentTime() + _T(".dmp");
    // 创建Dump文件
    CreateDumpFile(strDumpFileName, pException);
    static  bool showed = false;
    
    if(!showed)
    {
        showed = true;
        AfxMessageBox(GetIniStringValue("MainDialog", "AppErrorMsg", g_strCurrentLanguage));
    }
    
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif
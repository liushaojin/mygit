// Log.cpp: implementation of the CLog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include < iostream>
#include < io.h>
#include < sys\stat.h>
#include < afx.h>
using namespace std;

#include "Log.h"

#ifdef _DEBUG
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
    #define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLog::CLog()
{
    m_bEnable = FALSE;
    m_bPrintTime = FALSE;
    m_csFileName = "";
    m_hasData = FALSE;
    m_csAppName = "";
    m_pThread = NULL;
    // we'll make sure only one call uses the critical stuff at a time
    //InitializeCriticalSection(&m_crit);
}

CLog::~CLog()
{
    if(m_bEnable)
    {
        Uninit();
    }
}

void CLog::Uninit()
{
    while(!m_logMsgMap.empty())
    {
        Sleep(100);
        continue;
    }
    
    m_bEnable = FALSE;
    
    try
    {
        if(NULL != m_pThread)
        {
            WaitForSingleObject(m_pThread->m_hThread, 3000);
            delete m_pThread;
            m_pThread = NULL;
        }
    }
    catch(...)
    {
    }
}

/*
 参数说明：int iLogLevel写日志等级:＝0时，全部写日志；
                               ＝1时，写Info、Warn、Err;
                               ＝2时，写Warn、Err;
							   ＝3时，写Err

*/
void CLog::Init(const char *pOutputFilename, int iLogLevel)
{
    ///////////TODO
    m_bEnable = TRUE;
    m_csFileName = pOutputFilename;
    
    if(m_pThread = AfxBeginThread(WriteThread, (LPVOID)this, THREAD_PRIORITY_ABOVE_NORMAL, 0, CREATE_SUSPENDED, NULL))
    {
        m_pThread->m_bAutoDelete = FALSE;
        m_pThread->ResumeThread();
    }
    
    m_iLogLevelFlag = iLogLevel;
}

void CLog::Enable(BOOL bEnable)
{
    m_bEnable = bEnable;
}

CString CLog::ChangeLogName(const CString &filename)
{
    CTime cTime = CTime::GetCurrentTime();
    CString logFileName(_T(""));
    logFileName.Format("CarCheck%d%.2d%.2d.log", cTime.GetYear(), cTime.GetMonth(), cTime.GetDay());
    
    if(filename == logFileName)
    {
        return filename;
    }
    
    return logFileName;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

CString	CLog::GetBaseName(const CString &path)
{
    CString out = path;
    int iSlashPos = path.ReverseFind('\\');
    
    if(iSlashPos != -1)
    {
        out = out.Mid(iSlashPos + 1);
    }
    else
    {
        iSlashPos = path.ReverseFind('/');
        
        if(iSlashPos != -1)
        {
            out = out.Mid(iSlashPos + 1);
        }
    }
    
    int iDotPos = out.ReverseFind('.');
    
    if(iDotPos > 0)
    {
        out = out.Left(iDotPos);
    }
    
    return out;
}

/////////////////////////////////////////////////////////////////////////////

CString CLog::GetBaseDir(const CString & path)
{
    CString out = "";
    int iSlashPos = path.ReverseFind('\\');
    
    if(iSlashPos != -1)
    {
        out = path.Left(iSlashPos);
    }
    else
    {
        iSlashPos = path.ReverseFind('/');
        
        if(iSlashPos != -1)
        {
            out = path.Left(iSlashPos);
        }
    }
    
    return out;
}

void CLog::InsertMsg(const CString &logFileName, int iLogLevel, const CString &msg)
{
    try
    {
        CScopedLocker cscLocker(&m_critSect);
        LogMsg logMsg;
        logMsg.level = iLogLevel;
        logMsg.msg = msg;
        m_logMsgMap.insert(make_pair(logFileName, logMsg));
        m_hasData = TRUE;
    }
    catch(...)
    {
    }
}

///////////////////////////////////
UINT CLog::WriteThread(LPVOID lParam)
{
    CLog *sThis = (CLog*)(lParam);
    LogMsg logMsg;
    CString key;
    CString fileName("");
    char buf[MAX_PATH + 1];
    multimap<CString, LogMsg>::iterator itr;
    
    try
    {
        DWORD res = GetModuleFileName(AfxGetInstanceHandle(), buf, MAX_PATH);
        CString appDir = sThis->GetBaseDir(buf);
        CTime cTime = CTime::GetCurrentTime();
        CString path(_T(""));
        path.Format("%s\\Logs\\", appDir);
        CFileFind   aFind;
        
        if(!(aFind.FindFile(path, 0)))
        {
            //创建文件夹
            CreateDirectory(path, NULL);
        }//if
        
        while(sThis->m_bEnable)
        {
            if(!sThis->m_hasData || sThis->m_logMsgMap.empty())
            {
                Sleep(500);
                continue;
            }
            
            sThis->m_csFileName = sThis->ChangeLogName(sThis->m_csFileName);
            fileName = path + "\\" + sThis->m_csFileName;
            FILE *fp = fopen(fileName, "a+");
            
            if(!fp)
            {
                continue;
            }
            
            CScopedLocker cscLocker(&sThis->m_critSect);
            
            for(itr = sThis->m_logMsgMap.begin(); itr != sThis->m_logMsgMap.end(); ++itr)
            {
                key = (*itr).first;
                logMsg = (*itr).second;
                
                if(sThis->m_bPrintTime)
                {
                    CTime ct ;
                    ct = CTime::GetCurrentTime();
                    fprintf(fp, "DateTime:%s", ct.Format("%Y-%m-%d %H:%M:%S"));
                }
                
                fprintf(fp, "      Log Msg:%s\n", logMsg.msg);
            }
            
            fclose(fp);
            sThis->m_logMsgMap.clear();
            sThis->m_hasData = FALSE;
        }
    }
    catch(...)
    {
        return -1;
    }
    
    return 0;
}

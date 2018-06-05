#include "StdAfx.h"
#include "AppInit.h"
#include <afxwin.h>


CAppInit::CAppInit(void)
{
}


CAppInit::~CAppInit(void)
{
}

CString CAppInit::DataSource()
{
    return "192.168.0.188";
}

CString CAppInit::DataBase()
{
    return "SDMDB";
}

CString CAppInit::UserID()
{
    return "sa";
}

CString CAppInit::UserPwd()
{
    return "sa";
}

UINT CAppInit::DatabaseType()
{
    return 1;
}

CString CAppInit::GetAppPath()
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

bool CAppInit::CreateFolder(CString strDir)
{
    if(strDir.IsEmpty())
    {
        return false;
    }
    
    bool bRet = PathIsDirectory(strDir);
    
    if(!bRet)
    {
        bRet = CreateDirectory(strDir, NULL);
        return bRet;
    }
    
    return false;
}

#include "StdAfx.h"
#include "IniHelper.h"
//#include "../common/CreateDump.h"

IniHelper* IniHelper::m_instance = 0;

IniHelper::IniHelper(void)
{
    Init();
	m_szFileName = _T("Count");
}

IniHelper::IniHelper(LPCTSTR szFileName)
{
    // (1) 绝对路径，需检验路径是否存在
    // (2) 以"./"开头，则需检验后续路径是否存在
    // (3) 以"../"开头，则涉及相对路径的解析
    Init();
    // 相对路径
    m_szFileName.Format(".//%s", szFileName);
}

IniHelper::~IniHelper(void)
{
}

IniHelper* IniHelper::Instance()
{
	if (m_instance == 0)
	{
		m_instance = new IniHelper();
	}
	return m_instance;
}

void IniHelper::Init()
{
	
    m_unMaxSection = 512;
    m_unSectionNameMaxSize = 33; // 32位UID串
}


void IniHelper::SetFileName(LPCTSTR szFileName)
{
    m_szFileName.Format(".//%s", szFileName);
}

CString IniHelper::GetAppPath()
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

CString IniHelper::GetIniStringValue(CString strSection, CString strSectionKey)
{
	CString strFile;
	CString strValue;
	char m_Value[500] = "";
	memset(m_Value, 0, 500);
	CString iniFilePath;
	iniFilePath.Format("%s\\ini\\", GetAppPath());
	strFile.Format("%s%s%s", iniFilePath, m_szFileName, ".ini");
	GetPrivateProfileString(strSection, strSectionKey, NULL, m_Value, 500, strFile);
	strValue = m_Value;
	return strValue;
}

CString IniHelper::GetIniStringValue(CString strSection, CString strSectionKey, CString strFileName)
{
	CString strFile;
	CString strValue;
	char m_Value[500] = "";
	memset(m_Value, 0, 500);
	CString iniFilePath;
	iniFilePath.Format("%s\\ini\\", GetAppPath());
	strFile.Format("%s%s%s", iniFilePath, strFileName, ".ini");
	GetPrivateProfileString(strSection, strSectionKey, NULL, m_Value, 500, strFile);
	strValue = m_Value;
	return strValue;
}

void IniHelper::SetIniStringValue(CString strSection, CString strSectionKey, CString strSectionKeyVal)
{
	CString strFile;
	CString iniFilePath;
	iniFilePath.Format("%s\\ini\\", GetAppPath());
	strFile.Format("%s%s%s", iniFilePath, m_szFileName, ".ini");
	WritePrivateProfileString(strSection, strSectionKey, strSectionKeyVal, strFile);
}
void IniHelper::SetIniStringValue(CString strSection, CString strSectionKey, CString strSectionKeyVal, CString strFileName)
{
	CString strFile;
	CString iniFilePath;
	iniFilePath.Format("%s\\ini\\", GetAppPath());
	strFile.Format("%s%s%s", iniFilePath, strFileName, ".ini");
	WritePrivateProfileString(strSection, strSectionKey, strSectionKeyVal, strFile);
}

DWORD IniHelper::GetProfileSectionNames(CStringArray &strArray)
{
    int nAllSectionNamesMaxSize = m_unMaxSection * m_unSectionNameMaxSize + 1;
    char *pszSectionNames = new char[nAllSectionNamesMaxSize];
    DWORD dwCopied = 0;
    dwCopied = ::GetPrivateProfileSectionNames(pszSectionNames, nAllSectionNamesMaxSize, m_szFileName);
    strArray.RemoveAll();
    char *pSection = pszSectionNames;
    
    do
    {
        CString szSection(pSection);
        
        if (szSection.GetLength() < 1)
        {
            delete[] pszSectionNames;
            return dwCopied;
        }
        
        strArray.Add(szSection);
        pSection = pSection + szSection.GetLength() + 1; // next section name
    }
    while (pSection && pSection < pszSectionNames + nAllSectionNamesMaxSize);
    
    delete[] pszSectionNames;
    return dwCopied;
}

DWORD IniHelper::GetProfileString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, CString& szKeyValue)
{
    DWORD dwCopied = 0;
    dwCopied = ::GetPrivateProfileString(lpszSectionName, lpszKeyName, "",
                                         szKeyValue.GetBuffer(MAX_PATH), MAX_PATH, m_szFileName);
    szKeyValue.ReleaseBuffer();
    return dwCopied;
}

int IniHelper::GetProfileInt(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName)
{
    int nKeyValue = ::GetPrivateProfileInt(lpszSectionName, lpszKeyName, 0, m_szFileName);
    return nKeyValue;
}

BOOL IniHelper::SetProfileString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, LPCTSTR lpszKeyValue)
{
    return ::WritePrivateProfileString(lpszSectionName, lpszKeyName, lpszKeyValue, m_szFileName);
}

BOOL IniHelper::SetProfileInt(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, int nKeyValue)
{
    CString szKeyValue;
    szKeyValue.Format("%d", nKeyValue);
    return ::WritePrivateProfileString(lpszSectionName, lpszKeyName, szKeyValue, m_szFileName);
}

BOOL IniHelper::DeleteSection(LPCTSTR lpszSectionName)
{
    return ::WritePrivateProfileSection(lpszSectionName, NULL, m_szFileName);
}

BOOL IniHelper::DeleteKey(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName)
{
    return ::WritePrivateProfileString(lpszSectionName, lpszKeyName, NULL, m_szFileName);
}
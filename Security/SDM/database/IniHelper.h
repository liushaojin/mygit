#pragma once

#include "afxcmn.h"
#include "afxwin.h"

class IniHelper
{
public:
	static IniHelper* Instance();
	virtual ~IniHelper(void);

	void SetFileName(LPCTSTR szFileName);

	// Operations     
	BOOL SetProfileInt(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, int nKeyValue);
	BOOL SetProfileString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, LPCTSTR lpszKeyValue);

	DWORD GetProfileSectionNames(CStringArray& strArray); // 返回section数量     

	int GetProfileInt(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName);
	DWORD GetProfileString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, CString& szKeyValue);

	BOOL DeleteSection(LPCTSTR lpszSectionName);
	BOOL DeleteKey(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName);

	CString GetIniStringValue(CString strSection, CString strSectionKey);
	CString GetIniStringValue(CString strSection, CString strSectionKey, CString strFileName);
	void SetIniStringValue(CString strSection, CString strSectionKey, CString strSectionKeyVal);
	void SetIniStringValue(CString strSection, CString strSectionKey, CString strSectionKeyVal, CString strFileName);
private:
	IniHelper(void);
	IniHelper(LPCTSTR szFileName);


	void Init();
	CString GetAppPath();
	
	CString  m_szFileName; // .//Config.ini, 如果该文件不存在，则exe第一次试图Write时将创建该文件     

	UINT m_unMaxSection; // 最多支持的section数(256)     
	UINT m_unSectionNameMaxSize; // section名称长度，这里设为32(Null-terminated)     

	static IniHelper* m_instance;
};

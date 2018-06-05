#pragma once
class CAppInit
{
public:
	CAppInit(void);
	~CAppInit(void);

	static CString DataSource();
	static CString DataBase();
	static CString UserID();
	static CString UserPwd();
	static UINT DatabaseType();
	static CString GetAppPath();
	static bool CreateFolder(CString strDir);
};


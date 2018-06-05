#pragma once
#include "./database/DBTable.h"
#include "./database/Ado.h"
#include "./common/DataDefine.h"

class CDataBaseT
{
public:
	CDataBaseT(void);
	~CDataBaseT(void);
protected:
	// 连接对象
	CAdoConnection* m_pAdoConnect;	
	//CAdoRecordSet m_pAdoRecordSet[5];
	CAdoRecordSet* m_pRecordset;
	const int m_nRdsNumCnt;

	CAdoRecordSet* m_pRecordsetFileDel;

	CString m_strSQL;
	CString m_strDBName;
public:
	// 初始化数据库
	BOOL InitDB(CString strConnect, CString strpath);
	void CompactDB(CString strsource);

	// 返回数据库连接
	CAdoConnection* GetDBConnection(void);
	CAdoRecordSet* GetDBRecordSet(void);

	BOOL ExecSql(LPCSTR SqlStr, int& nResult);
	BOOL OpenSql(LPCTSTR TblName, LPCTSTR FldList = "*", LPCTSTR Constr = "", LPCSTR OrderBy = "", int nItemLimited = 0);

	// 向数据库中添加新的数据
	BOOL AddNewData(CString DoorId, int forwardPass, int backPass, int Warning, CString Sdomain, COleDateTime  &pCurrentTime);
	// 打开指定的表
	void OpenRecordSet(DBTable dbTblSwitch, char* pTblInfo = NULL);
	BOOL AddDataToCheckInfo(const CheckInfo &checkinfo);
	BOOL ExecSql(LPCSTR SqlStr);
	BOOL AddNewBeiZhu(CString name, CString remark, CString beizhu);
	BOOL AddNewDev(CString name, CString remark, CString beizhu);
	BOOL DelDevFromDb(CString name);	//从数据库中删除设备
	//删除数据库记录
	BOOL DeleteRecord(DBTable dbTblSwitch, CString strWhere);
	BOOL AddOrUpdateCountTb(int doorId, int totalPass, int totalAlarm, int todayPass, int todayAlarm, CString todayDate, int lastPass, int lastAlarm);
};

#include "StdAfx.h"
#include "DataBaseT.h"
#include <assert.h>

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

#pragma warning (disable:4996)

CDataBaseT::CDataBaseT(void)
    : m_pAdoConnect(NULL)
    , m_nRdsNumCnt(5)
    , m_strDBName(_T(""))
{
    m_pRecordset = NULL;
    m_strSQL = _T("select * from ");
    m_pRecordsetFileDel = NULL;
}

CDataBaseT::~CDataBaseT(void)
{
    if(m_pRecordset != NULL)
    {
        delete m_pRecordset;
    }
    
    m_pRecordset = NULL;
    
    if(m_pRecordsetFileDel != NULL)
    {
        if(m_pRecordsetFileDel->IsOpen())
        {
            m_pRecordsetFileDel->Close();
        }
        
        delete m_pRecordsetFileDel;
    }
    
    m_pRecordsetFileDel = NULL;
    
    if(m_pAdoConnect != NULL)
    {
        delete m_pAdoConnect;
    }
    
    m_pAdoConnect = NULL;
    CoUninitialize();
}

// 初始化数据库
BOOL CDataBaseT::InitDB(CString strConnect, CString strpath)
{
    ::CoInitialize(NULL);
    BOOL retVal = FALSE;
    
    if(m_pAdoConnect == NULL)
    {
        m_pAdoConnect = new CAdoConnection;
    }
    
    retVal = m_pAdoConnect->Connect(strConnect);
    
    //for(int i = 0; i < m_nRdsNumCnt; i++)
    //	m_pAdoRecordSet[i].SetAdoConnection(m_pAdoConnect);
    if(m_pRecordset == NULL)
    {
        m_pRecordset = new CAdoRecordSet(m_pAdoConnect);
    }
    
    m_strDBName = strpath;
    
    //文件删除记录
    if(m_pRecordsetFileDel == NULL)
    {
        m_pRecordsetFileDel = new CAdoRecordSet(m_pAdoConnect);
    }
    
    return retVal;
}

//必须在connect数据库前调用
void CDataBaseT::CompactDB(CString strsource)
{
    char pTemp[100] = "";
    int nItem = 0, nbreak = 0;
    //while (nbreak != -1)
    //{
    //	nItem = nbreak;
    //	nbreak = strsource.Find('\\',nbreak + 1);
    //};
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = FindFirstFile(strsource, &FindFileData);
    
    if(hFind == INVALID_HANDLE_VALUE)
    {
        return;
    }
    else if(FindFileData.nFileSizeHigh > 0 || FindFileData.nFileSizeLow <= 0x8000000)
    {
        FindClose(hFind);
        return;
    }
    
    FindClose(hFind);
    nItem = strsource.ReverseFind('\\');
    
    if(nItem > 0)
    {
        strncpy(pTemp, strsource.GetBuffer(strsource.GetLength()), nItem + 1);
        strcat(pTemp, "abbc.mdb\0");
    }
    else
    {
        strcpy(pTemp, "d:\\abbc.mdb\0");
    }
    
    CString strcmpsource, strdest;
    strcmpsource.Format("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%s;Jet OLEDB:LDDatabase Password=ld2012", strsource);
    strdest.Format("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%s;Jet OLEDB:Engine Type=5;Jet OLEDB:LDDatabase Password=ld2012", pTemp);
    
    try
    {
        ::CoInitialize(NULL);
        IJetEnginePtr jet(__uuidof(JetEngine));
        
        if(jet->CompactDatabase((_bstr_t)strcmpsource, (_bstr_t)strdest) == S_OK)
        {
            if(DeleteFile(strsource))
            {
                rename(pTemp, strsource);
            }
        }
        
        jet.Release();
        ::CoUninitialize();
        //jet->CompactDatabase( "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=d:\\nwind2.mdb;Jet OLEDB:Database Password=test",
        //	"Provider=Microsoft.Jet.OLEDB.4.0;Data Source=d:\\abbc.mdb;Jet OLEDB:Engine Type=4;Jet OLEDB:Database Password=test");
    }
    catch(_com_error &e)
    {
        ::MessageBox(NULL, (LPCTSTR)e.Description(), "", MB_OK) ;
    }
}

// 返回数据库连接
CAdoConnection* CDataBaseT::GetDBConnection(void)
{
    return m_pAdoConnect;
}

// 返回数据库记录
CAdoRecordSet* CDataBaseT::GetDBRecordSet(void)
{
    return m_pRecordset;
}

// 打开指定的表
void CDataBaseT::OpenRecordSet(DBTable dbTblSwitch, char* pTblInfo)
{
    CString strSql = "";
    //switch(dbTblSwitch)
    //{
    //case Tb_Class:
    //	strSql = m_strSQL + cnt_TableName[0];
    //	break;
    //case Tb_Teacher:
    //	strSql = m_strSQL + cnt_TableName[1];
    //	break;
    //case Tb_File:
    //	strSql = m_strSQL + cnt_TableName[2];
    //	break;
    //case Tb_Student:
    //	strSql = m_strSQL + cnt_TableName[3];
    //	break;
    //case Tb_Score:
    //	strSql = m_strSQL + cnt_TableName[4];
    //	strSql.Append(pTblInfo);
    //}
    //int i = (int)dbTblSwitch;
    //m_pAdoRecordSet[i].Open(strSql);
}

BOOL CDataBaseT::OpenSql(LPCTSTR TblName, LPCTSTR FldList, LPCTSTR Constr, LPCSTR OrderBy, int nItemLimited)
{
    assert(m_pAdoConnect != NULL);
    char sqlstr[1024] = "";
    
    if(nItemLimited > 0)
    {
        sprintf(sqlstr, "select top %d ", nItemLimited);
    }
    else
    {
        sprintf(sqlstr, "select ");
    }
    
    strcat(sqlstr, FldList);
    strcat(sqlstr, " from ");
    strcat(sqlstr, TblName);
    strcat(sqlstr, "  ");
    strcat(sqlstr, Constr);
    strcat(sqlstr, "  ");
    strcat(sqlstr, OrderBy);
    
    if(m_pRecordset->IsOpen())
    {
        m_pRecordset->Close();
    }
    
    int retVal = m_pRecordset->Open(sqlstr);
    return (retVal >= 0) ? TRUE : FALSE;
}


BOOL CDataBaseT::ExecSql(LPCSTR SqlStr, int& nResult)
{
    CAdoRecordSet Recordset(m_pAdoConnect);
    
    if(Recordset.ExecuteSql(SqlStr))
    {
        _variant_t vIndex = (long)0;
        _variant_t vCount = Recordset.GetRecordset()->GetCollect(vIndex);
        nResult = vCount.lVal;
        Recordset.Close();
        return true;
    }
    
    return false;
}

BOOL CDataBaseT::AddDataToCheckInfo(const CheckInfo &checkinfo)
{
    BOOL retVal = FALSE;
    
    try
    {
        retVal = OpenSql("Checkinfo");
        
        if(!retVal)
        {
            return retVal;
        }
        
        if(retVal = m_pRecordset->AddNew())
        {
            m_pRecordset->PutCollect("DoorID", checkinfo.SDoorID);
            m_pRecordset->PutCollect("autoJudged", checkinfo.autoJudged);
            m_pRecordset->PutCollect("userName", checkinfo.userName);
            m_pRecordset->PutCollect("userID", checkinfo.userID);
            //TODO
            //COleDateTime datetime = COleDateTime::GetCurrentTime();
            COleVariant varTime(checkinfo.checkTime);
            varTime.ChangeType(VT_DATE);
            COleDateTime datetime(varTime);
            m_pRecordset->PutCollect("CheckTime", datetime);
            m_pRecordset->PutCollect("CheckOperator", checkinfo.checkOperator);
            m_pRecordset->PutCollect("Describe", checkinfo.describe);
            m_pRecordset->PutCollect("ExistHisRecord", atoi(checkinfo.existHisRecord));
            m_pRecordset->PutCollect("CaptureImageName", checkinfo.captureImageName);
            m_pRecordset->PutCollect("BlackWhiteList", checkinfo.BlackWhiteList);
            retVal = m_pRecordset->Update();
        }
        
        return retVal;
    }
    catch(_com_error e)
    {
        return FALSE;
    }
}

BOOL CDataBaseT::ExecSql(LPCSTR SqlStr)
{
    assert(m_pAdoConnect != NULL);
    
    if(m_pRecordset->IsOpen())
    {
        m_pRecordset->Close();
    }
    
    return m_pRecordset->ExecuteSql(SqlStr);
}

BOOL CDataBaseT::AddNewData(CString DoorId, int forwardPass, int backPass, int Warning, CString Sdomain, COleDateTime  &pCurrentTime)
{
    BOOL bResult = FALSE;
    LPCSTR TableName = "ANNUNCIATOR";
    
    if(OpenSql(TableName))
    {
        long lnnum = m_pRecordset->GetRecordCount();
        {
            if(m_pRecordset->AddNew())
            {
                //m_pRecordset->PutCollect(cnt_Enannunciator[0], (int)(lnnum + 1));
                m_pRecordset->PutCollect(cnt_Enannunciator[1], DoorId);
                //m_pRecordset->GetField(2)->Value = _variant_t(DetectResult);
                m_pRecordset->PutCollect(cnt_Enannunciator[2], forwardPass);
                m_pRecordset->PutCollect(cnt_Enannunciator[3], backPass);
                m_pRecordset->PutCollect(cnt_Enannunciator[4], Warning);
                m_pRecordset->PutCollect(cnt_Enannunciator[5], Sdomain);
                m_pRecordset->PutCollect(cnt_Enannunciator[6], pCurrentTime);
                m_pRecordset->Update();
                bResult = TRUE;
            }
            
            m_pRecordset->Close();
        }
    }
    
    return bResult;
}


BOOL CDataBaseT::AddOrUpdateCountTb(int doorId, int totalPass, int totalAlarm, int todayPass, int todayAlarm, CString todayDate, int lastPass, int lastAlarm)
{
	BOOL bResult = FALSE;
	LPCSTR TableName = "CountTb";
	char pWhereTemp[100] = "";
	sprintf(pWhereTemp, "where DOORID=%d", doorId);

	if (OpenSql(TableName, "*", pWhereTemp))
	{
		long lnnum = m_pRecordset->GetRecordCount();

		if (lnnum == 0)
		{
			if (m_pRecordset->AddNew())
			{
				m_pRecordset->PutCollect(cnt_CountTb[1], doorId);
				m_pRecordset->PutCollect(cnt_CountTb[2], totalPass);
				m_pRecordset->PutCollect(cnt_CountTb[3], totalAlarm);
				m_pRecordset->PutCollect(cnt_CountTb[4], todayPass);
				m_pRecordset->PutCollect(cnt_CountTb[5], todayAlarm);
				m_pRecordset->PutCollect(cnt_CountTb[6], todayDate);
				m_pRecordset->PutCollect(cnt_CountTb[7], lastPass);
				m_pRecordset->PutCollect(cnt_CountTb[8], lastAlarm);
				m_pRecordset->Update();
				bResult = TRUE;
			}
		}
		else
		{
			m_pRecordset->GetField(2)->Value = _variant_t(totalPass);
			m_pRecordset->GetField(3)->Value = _variant_t(totalAlarm);
			m_pRecordset->GetField(4)->Value = _variant_t(todayPass);
			m_pRecordset->GetField(5)->Value = _variant_t(todayAlarm);
			m_pRecordset->GetField(6)->Value = _variant_t(todayDate);
			m_pRecordset->GetField(7)->Value = _variant_t(lastPass);
			m_pRecordset->GetField(8)->Value = _variant_t(lastAlarm);

			m_pRecordset->Update();
			bResult = TRUE;
		}

		m_pRecordset->Close();
	}

	return bResult;
}

BOOL CDataBaseT::AddNewBeiZhu(CString name, CString remark, CString beizhu)
{
    //(int ReadTimes, int DetectResult, LPCSTR pFullName,COleDateTime  &pCurrentTime,
    //	LPCSTR pCaption, LPCSTR DeviceNum)
    BOOL bResult = FALSE;
    LPCSTR TableName = "DEVICES";
    char pWhereTemp[100] = "";
    sprintf(pWhereTemp, "where COMID='%s'", name);
    
    if(OpenSql(TableName, "*", pWhereTemp))
    {
        long lnnum = m_pRecordset->GetRecordCount();
        
        if(lnnum == 0)
        {
            if(m_pRecordset->AddNew())
            {
                m_pRecordset->PutCollect(cnt_Endevices[0], name);
                m_pRecordset->PutCollect(cnt_Endevices[1], remark);
                m_pRecordset->PutCollect(cnt_Endevices[2], beizhu);
                m_pRecordset->Update();
            }
        }
        else
        {
            m_pRecordset->GetField(1)->Value = _variant_t(remark);
            m_pRecordset->GetField(2)->Value = _variant_t(beizhu);
            m_pRecordset->Update();
            bResult = TRUE;
        }
        
        m_pRecordset->Close();
    }
    
    return bResult;
}

BOOL CDataBaseT::AddNewDev(CString name, CString remark, CString beizhu)
{
    //(int ReadTimes, int DetectResult, LPCSTR pFullName,COleDateTime  &pCurrentTime,
    //	LPCSTR pCaption, LPCSTR DeviceNum)
    BOOL bResult = FALSE;
    LPCSTR TableName = "DEVICES";
    char pWhereTemp[100] = "";
    sprintf(pWhereTemp, "where COMID='%s'", name);
    
    if(OpenSql(TableName, "*", pWhereTemp))
    {
        long lnnum = m_pRecordset->GetRecordCount();
        
        if(lnnum == 0)
        {
            if(m_pRecordset->AddNew())
            {
                m_pRecordset->PutCollect(cnt_Endevices[0], name);
                m_pRecordset->PutCollect(cnt_Endevices[1], remark);
                m_pRecordset->PutCollect(cnt_Endevices[2], beizhu);
                m_pRecordset->Update();
            }
        }
        
        // 		else
        // 		{
        // 			m_pRecordset->GetField(1)->Value=_variant_t(remark);
        // 			m_pRecordset->GetField(2)->Value=_variant_t(beizhu);
        // 			m_pRecordset->Update();
        // 			bResult = TRUE;
        // 		}
        m_pRecordset->Close();
    }
    
    return bResult;
}

BOOL CDataBaseT::DelDevFromDb(CString name)
{
    BOOL bResult = FALSE;
    LPCSTR TableName = "DEVICES";
    char pWhereTemp[100] = "";
    sprintf(pWhereTemp, "where COMID='%s'", name);
    
    if(OpenSql(TableName, "*", pWhereTemp))
    {
        BOOL bResult = m_pRecordset->DeleteCurrentRecord();
        m_pRecordset->Close();
    }
    
    return bResult;
}

// 删除记录
BOOL CDataBaseT::DeleteRecord(DBTable dbTblSwitch, CString strWhere)
{
    BOOL nResult = false;
    CString strUserIDtemp = "";
    CString strSqlTemp = "";
    
    switch(dbTblSwitch)
    {
        case Tb_ANNUNCIATOR:
            {
                LPCTSTR TableName = "ANNUNCIATOR";
                
                if(OpenSql(TableName, "*", strWhere))
                {
                    BOOL res = m_pRecordset->DeleteAllRecord();
                    m_pRecordset->Close();
                    nResult = true;
                }
                
            }
            break;
            
        case Tb_DEVICES:
            {
                LPCTSTR TableName = "DEVICES";
                
                if(OpenSql(TableName, "*", strWhere))
                {
                    BOOL res = m_pRecordset->DeleteAllRecord();
                    m_pRecordset->Close();
                    nResult = true;
                }
            }
            break;
            
        case Tb_UserInfo://删除用户时，直接删除用户的做法不合适，改为将用户已删除标记设为1
            //if(OpenSql(cnt_TableName[Tb_UserInfo],"*",strWhere))
            //{
            //	m_pRecordset->GetValueString(strUserIDtemp,0);
            //	m_pRecordset->DeleteAllRecord();
            //	m_pRecordset->Close();
            //}
            //strSqlTemp.Format(" where ID = %s",strUserIDtemp);
            //if(OpenSql(cnt_TableName[Tb_UserPermission],"*",strSqlTemp))
            //{
            //	m_pRecordset->DeleteAllRecord();
            //	m_pRecordset->Close();
            //	nResult = true;
            //}
            {
                if(OpenSql(cnt_TableName[Tb_UserInfo], "*", strWhere))
                {
                    if(m_pRecordset->GetRecordCount() > 0)
                    {
                        m_pRecordset->GetField(3)->Value = _variant_t(1);
                        m_pRecordset->Update();
                    }
                    
                    m_pRecordset->Close();
                    nResult = true;
                }
            }
            break;
            
        case Tb_CUser:
            if(OpenSql(cnt_TableName[Tb_CUser], "*", strWhere))
            {
                m_pRecordset->DeleteAllRecord();
                m_pRecordset->Close();
                nResult = true;
            }
            
            break;

		case Tb_DATACOUNT:
		{
			LPCTSTR TableName = "CountTb";

			if (OpenSql(TableName, "*", strWhere))
			{
				BOOL res = m_pRecordset->DeleteAllRecord();
				m_pRecordset->Close();
				nResult = true;
			}

		}
			break;
    }
    
    return nResult;
}
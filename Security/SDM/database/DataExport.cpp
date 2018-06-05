#include "StdAfx.h"
#include "DataExport.h"
#include "DBTable.h"
#include "DataBaseT.h"

extern CDataBaseT* g_pDBT;

DataExport::DataExport(void)
{
}

DataExport::~DataExport(void)
{
}

CString DataExport::GetExePath()
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

int DataExport::ExportToCsv(CString fileName, CString tableName)
{
    CStdioFile file;
    
    if (!file.Open(fileName, CFile::modeCreate | CFile::modeWrite))
    {
        //this->MessageBox("Opening CSV is failed！");
        return OpenFailed;
    }
    
    LPCTSTR TableName = "ANNUNCIATOR";
    CString str = _T("序号,门ID,前通过人数,后通过人数,报警次数,报警区域,时间\n");

    file.WriteString(str);
    char pWhere[150] = "";
    char pOrder[30] = "";
    
    if (g_pDBT->OpenSql(TableName/*cnt_TableName[Tb_ANNUNCIATOR]*/, "*", pWhere, pOrder))
    {
        int nItem = g_pDBT->GetDBRecordSet()->GetRecordCount();
        
        if (nItem > 0)
        {
            g_pDBT->GetDBRecordSet()->MoveFirst();
            CString ID, doorID, passForwd, passBack, waring, data, time;
            
            //更新内容
            while (!g_pDBT->GetDBRecordSet()->IsEOF())
            {
                g_pDBT->GetDBRecordSet()->GetValueString(ID, cnt_Enannunciator[0]);
                g_pDBT->GetDBRecordSet()->GetValueString(doorID, cnt_Enannunciator[1]);
                g_pDBT->GetDBRecordSet()->GetValueString(passForwd, cnt_Enannunciator[2]);
                g_pDBT->GetDBRecordSet()->GetValueString(passBack, cnt_Enannunciator[3]);
                g_pDBT->GetDBRecordSet()->GetValueString(waring, cnt_Enannunciator[4]);
                g_pDBT->GetDBRecordSet()->GetValueString(data, cnt_Enannunciator[5]);
                g_pDBT->GetDBRecordSet()->GetValueString(time, cnt_Enannunciator[6]);
                str = ID + ',' + doorID + ',' + passForwd + ',' + passBack + ',' + waring + ',' + data + ',' + time + "\r";  // + "\r\n"
                file.WriteString(str);
                g_pDBT->GetDBRecordSet()->MoveNext();
            }
        }
        
        g_pDBT->GetDBRecordSet()->Close();
    }
    
    file.Close();

	return 0;
}

int DataExport::ExportCountToCsv(CString fileName)
{
	CStdioFile file;

	if (!file.Open(fileName, CFile::modeCreate | CFile::modeWrite))
	{
		//this->MessageBox("Opening CSV is failed！");
		return OpenFailed;
	}

	LPCTSTR tableName = "CountTb";
	CString str = _T("序号,门ID,总通过数,总报警数,最近一天通过数,最近一天报警数,最近一天日期\n");

	file.WriteString(str);
	char pWhere[150] = "";
	char pOrder[30] = "";

	if (g_pDBT->OpenSql(tableName/*cnt_TableName[Tb_ANNUNCIATOR]*/, "*", pWhere, pOrder))
	{
		int nItem = g_pDBT->GetDBRecordSet()->GetRecordCount();

		if (nItem > 0)
		{
			g_pDBT->GetDBRecordSet()->MoveFirst();
			CString ID, doorID, passForwd, passBack, waring, data, time;

			//更新内容
			while (!g_pDBT->GetDBRecordSet()->IsEOF())
			{
				g_pDBT->GetDBRecordSet()->GetValueString(ID, cnt_CountTb[0]);
				g_pDBT->GetDBRecordSet()->GetValueString(doorID, cnt_CountTb[1]);
				g_pDBT->GetDBRecordSet()->GetValueString(passForwd, cnt_CountTb[2]);
				g_pDBT->GetDBRecordSet()->GetValueString(passBack, cnt_CountTb[3]);
				g_pDBT->GetDBRecordSet()->GetValueString(waring, cnt_CountTb[4]);
				g_pDBT->GetDBRecordSet()->GetValueString(data, cnt_CountTb[5]);
				g_pDBT->GetDBRecordSet()->GetValueString(time, cnt_CountTb[6]);
				str = ID + ',' + doorID + ',' + passForwd + ',' + passBack + ',' + waring + ',' + data + ',' + time + "\r";  // + "\r\n"
				file.WriteString(str);
				g_pDBT->GetDBRecordSet()->MoveNext();
			}
		}

		g_pDBT->GetDBRecordSet()->Close();
	}

	file.Close();

	return 0;
}
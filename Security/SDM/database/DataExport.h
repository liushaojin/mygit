#pragma once

#include "afxcmn.h"
#include "afxwin.h"

class DataExport
{
    public:
        enum ExportErr {None, OpenFailed};
        DataExport(void);
        virtual ~DataExport(void);
        static CString GetExePath();
        static int ExportToCsv(CString fileName, CString tableName = _T(""));
		static int ExportCountToCsv(CString fileName);
};

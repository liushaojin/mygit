#pragma once

#ifndef __AFXWIN_H__
    #error 在包含用于 PCH 的此文件之前包含“stdafx.h”
#endif

#include "resource.h"		// 主符号
#include "predefine.h"
#include "./database/DataBaseT.h"
#include "./network/SDCommData.h"

#define WM_DELETE_UPDATE (WM_USER + 0x100)

class CSDMApp : public CWinApp
{
    public:
        CSDMApp();
        
        virtual BOOL InitInstance();
        virtual int ExitInstance();
        
        void DeleteExpiredLog(const CString& strFilepath);
        void OnQueryValidPeriod(CString &strpath);	//查询有效期
        void GetLastExitTime();
        void AutoRunApp();	//开机自动运行
        static UINT DelExpiredImage(LPVOID lParam);	//删除到期图片信息
        
        
        CSDCommData m_SDCommData;
        int m_nFullWidth;
        int m_nFullHeight;
        
        DECLARE_MESSAGE_MAP()
        
    private:
        HANDLE m_hMutex;//互斥锁对象，防止软件被多次启动
        GdiplusStartupInput m_gdiplusStartupInput;
        ULONG_PTR m_pGdiToken;
        
        BOOL OnlyInstance();                                //保证一台电脑只能启动一个实例
        BOOL ChangeDisMode();                               //改变屏幕分辨率
        void InitDBConnect(const CString &strReturnPath);   //初始化数据库连接
        void FreeResource();
};

extern CSDMApp theApp;

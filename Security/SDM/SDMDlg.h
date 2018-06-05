
// SDMDlg.h : 头文件
//

#pragma once
#include "./controls/BaseDialog.h"
#include "./network/IOCPServer.h"
#include "afxwin.h"
#include "afxcmn.h"

typedef struct UnitCount
{
	int id;				//记录单个安检门的id号
	int pass;			//记录单个安检门上传的通过数
	int alarm;			//记录单个安检门上传的报警数
	int todayPass;		//记录单个安检门当天的通过总数
	int todayAlarm;		//记录单个安检门当天的报警总数
	int totalPass;		//记录单个安检门历史上传的通过数
	int totalAlarm;		//记录单个安检门历史上传的报警数
	CString todayDate;	//记录单个安检门数据更新最近日期
}UnitCountSt;

// CSDMDlg 对话框
class CSDMDlg : public CBaseDialog
{
// 构造
    public:
        CSDMDlg(CWnd* pParent = NULL);  // 标准构造函数
        
// 对话框数据
        enum { IDD = IDD_SDM_DIALOG };
        
        afx_msg void OnBnClickedDataQuery();
        afx_msg void OnBnClickedUserManage();
        afx_msg void OnBnClickedDevManage();
        afx_msg void OnBnClickedSystemSet();
		afx_msg void OnBnClickedCountInfo();
        afx_msg void OnBnClickedExit();
        afx_msg void OnBnClickedExport();
        afx_msg void OnPaint();
        afx_msg void OnNMClickListDevinfo(NMHDR *pNMHDR, LRESULT *pResult);
        afx_msg void OnTvnSelchangedDevTree(NMHDR *pNMHDR, LRESULT *pResult);
        
        void InitStyle(void);
        void AddClient();
        void DecreaseCliect();
        
        CIOCPServer m_IOCPServer;
        CListBox m_LogListBox;
        CStatic m_SDDataCtrl;
        CImageList m_ImageList;
        
        DetectData m_data;  //数据更新用缓存
        CTreeCtrl m_DevTree;
        CListCtrl m_DevInfoList;
        BYTE m_alarmStatus[16]; //6个区域的状态

        int m_clientNum;
        int m_areaCnt;		//区域个数
		int m_totalPassedToday;	//开机后的通过人数
		int m_totalAlarmedToday;		//开机后的报警次数
        
    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
        virtual void OnClientDraw(CDC*pDC, INT nWidth, INT nHeight);
        virtual BOOL OnInitDialog();
        void DrawText(CDC* pDC, CRect rcText, LPCTSTR lpszText, UINT uFormat, INT nHeight = 18, bool bBold = true, LPCTSTR lpszName = TEXT("微软雅黑"));
        
        
        // 生成的消息映射函数
		afx_msg void OnClose(/*UINT nID, LPARAM lParam*/);
        afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
		afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
        //afx_msg LRESULT MyMessage(WPARAM wParam, LPARAM lParam);
        afx_msg LRESULT OnUpdateTable(WPARAM wParam, LPARAM lParam);    //声明自定义消息处理函数
        afx_msg LRESULT OnGetDoorId(WPARAM wParam, LPARAM lParam);
        afx_msg LRESULT OnDeleteUpdate(WPARAM wParam, LPARAM lParam);
        //afx_msg void OnPaint();
        afx_msg HCURSOR OnQueryDragIcon();
        afx_msg void OnDestroy();
        afx_msg void OnTimer(UINT nIDEvent);
        
        void DeviceSync();
        void OpenOrCloseTime(BOOL res);
        
        HICON m_hIcon;
        CImageEx m_titleImage;
        CString  m_strTitle;
        CButtonEx m_btnDataQuery;
        CButtonEx m_btnUserManage;
        CButtonEx m_btnDevManage;
        CButtonEx m_btnSystemSet;
		CButtonEx m_btnCountInfo;
        CButtonEx m_btnExit;
        CButtonEx m_btnExport;
        
        DECLARE_MESSAGE_MAP()
        
        
    private:
        int  GetFullWidth();
        int  GetFullHeight();
        BOOL InitNetWork();
        void DeInitializeNetWork();
        char* WCharToChar(const wchar_t* pwch, char* pch,
                          size_t* pnewlen = NULL);
        wchar_t* CharToWChar(const char* pch, wchar_t* pwch,
                             size_t* pnewlen = NULL);
        void InitTreeCtrl();
        void InitDevinfoList();
        void LoadSecurityDoor();
        void LoadBitmap();
        void GetDataFormDB();
		void ReadCountInfo();
        void GetDataFormDB(CString doorID);
        void GetDevFormDB(CString strName, CString &returnName);
		void GetDataFormCountTb();
		UnitCountSt GetDataFormCountTb(CString doorID);	//从数据库中读取各个门的统计信息
        
        CRect       m_clientRect;       //程序界面区域位置
        CRect       m_screenRect;       //储存屏幕区域的位置
        CDC			m_dcMem;//定义内存DC指针
        BOOL		m_isFirstEnter;
		CList<UnitCountSt, UnitCountSt&> m_cntList;
        
};

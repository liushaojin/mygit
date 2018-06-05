#pragma once
#include "afxdtctl.h"
#include "../controls/BaseDialog.h"
#include "afxcmn.h"
#include "afxwin.h"


class CDataQueryDlg : public CBaseDialog
{
        DECLARE_DYNAMIC(CDataQueryDlg)
        
    public:
        CDataQueryDlg(CWnd* pParent = NULL);   // 标准构造函数
        virtual ~CDataQueryDlg();     
        virtual BOOL OnInitDialog();

        void InitStyle(void);
        void InitListData(void);
        void GetDataFormDB(CString requireStr = "");
        void SetDevIdFormDB();
        
        afx_msg void OnBnClickedBtnQuery();
        afx_msg void OnBnClickedBtnQueryall();
        afx_msg void OnBnClickedBtnDel();
		afx_msg void OnBnClickedBtnDelall();
        afx_msg void OnBnClickedCheckTime();
        afx_msg void OnBnClickedCheckDoorid();
        afx_msg void OnCbnSelchangeCmbDoorid();
        afx_msg void OnBnClickedBtnClr();
		afx_msg void OnBnClickedBtnExport();
        
        CListCtrl m_dataList;
        CStatic m_requireGrp;
        CStatic m_resultGrp;
        BOOL m_findById;
        BOOL m_findByTime;
        CComboBox m_idCombo;
// 对话框数据
        enum { IDD = IDD_DATAQUERY_DIALOG };
        
    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
        virtual void OnClientDraw(CDC*pDC, INT nWidth, INT nHeight);
        void DrawText(CDC* pDC, CRect rcText, LPCTSTR lpszText, UINT uFormat, INT nHeight = 18, bool bBold = true, LPCTSTR lpszName = TEXT("微软雅黑"));
        afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
        
        CImageEx m_titleImage;
        CString  m_strTitle;
        CButtonEx m_btnQuery;
        CButtonEx m_btnQueryAll;
        CButtonEx m_btnDel;
		CButtonEx m_btnDelAll;
        CButtonEx m_btnClear;
		CButtonEx m_btnExport;

        DECLARE_MESSAGE_MAP()
        
};

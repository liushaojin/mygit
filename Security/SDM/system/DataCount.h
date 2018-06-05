#pragma once
#include "afxdtctl.h"
#include "../controls/BaseDialog.h"
#include "afxcmn.h"
#include "afxwin.h"


class CDataCountDlg : public CBaseDialog
{
        DECLARE_DYNAMIC(CDataCountDlg)
        
    public:
        CDataCountDlg(CWnd* pParent = NULL);   // 标准构造函数
        virtual ~CDataCountDlg();
        virtual BOOL OnInitDialog();
        
        void InitStyle(void);
        void InitListData(void);
        void GetDataFormDB(CString requireStr = "");
        void SetDevIdFormDB();
		void CountDataShow();
		void UpdateCountDataShow(int tdpass, int tdalarm, int ttpass, int ttalarm);
        
        afx_msg void OnBnClickedBtnCount();
        afx_msg void OnBnClickedBtnCountall();
        afx_msg void OnBnClickedBtnDel();
        afx_msg void OnBnClickedBtnDelall();

        afx_msg void OnBnClickedCheckDoorid();
        afx_msg void OnCbnSelchangeCmbDoorid();
        afx_msg void OnBnClickedBtnClr();
        afx_msg void OnBnClickedBtnExport();
        
        CListCtrl m_countList;
		BOOL m_idChecked;
        CComboBox m_idCombo;
// 对话框数据
        enum { IDD = IDD_DATACOUNT_DIALOG };
        
    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
        virtual void OnClientDraw(CDC*pDC, INT nWidth, INT nHeight);
        void DrawText(CDC* pDC, CRect rcText, LPCTSTR lpszText, UINT uFormat, INT nHeight = 18, bool bBold = true, LPCTSTR lpszName = TEXT("微软雅黑"));
        afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
        
        CImageEx m_titleImage;
        CString  m_strTitle;
        CButtonEx m_btnCount;
        CButtonEx m_btnCountAll;
        CButtonEx m_btnDel;
        CButtonEx m_btnDelAll;
        CButtonEx m_btnExport;
        
        DECLARE_MESSAGE_MAP()
        
        
public:
	
	afx_msg void OnNMClickCountList(NMHDR *pNMHDR, LRESULT *pResult);
};
#pragma once

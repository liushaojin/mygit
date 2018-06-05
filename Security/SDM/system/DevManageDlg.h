#pragma once

#include "../controls/BaseDialog.h"
#include "afxwin.h"
#include "afxcmn.h"


class CDevManageDlg : public CBaseDialog
{
	DECLARE_DYNAMIC(CDevManageDlg)

public:
	CDevManageDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDevManageDlg();
	virtual BOOL OnInitDialog();
	void InitStyle(void);
	void InitData();

	afx_msg void OnCbnSelchangeIdCombo();
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedModify();
	afx_msg void OnBnClickedDel();
	afx_msg void OnBnClickedQuit();
	CButton m_btnAdd;
	CButton m_btnModify;
	CButton m_btnDelete;

	CStatic m_idLbl;
	CComboBox m_idCombo;
	CStatic m_nameLbl;
	CEdit m_nameEdit;
	CStatic m_remarkGrp;
	CEdit m_remarkEdit;
	CButton m_btnQuit;

	enum { IDD = IDD_DEVMANAGE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual void OnClientDraw(CDC*pDC,INT nWidth,INT nHeight);
	void DrawText( CDC* pDC, CRect rcText,LPCTSTR lpszText,UINT uFormat,INT nHeight=18,bool bBold=true,LPCTSTR lpszName=TEXT("微软雅黑") );
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	CImageEx m_titleImage;
	CString  m_strTitle;
	CButtonEx	m_btnQuery;
	CButtonEx	m_btnQueryAll;
	CButtonEx	m_btnDel;
	CButtonEx	m_btnClear;

	DECLARE_MESSAGE_MAP()

};

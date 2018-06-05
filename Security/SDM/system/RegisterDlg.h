#pragma once
#include "afxwin.h"
#include "../controls/BaseDialog.h"
#include "HyperLink.h"

// CRegisterDlg 对话框
class CRegisterDlg : public CBaseDialog
{
	DECLARE_DYNAMIC(CRegisterDlg)

public:
	CRegisterDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRegisterDlg();
	virtual BOOL OnInitDialog();
	enum { IDD = IDD_REGISTER_DIALOG };

	void GetSNByNIC();  //获取机器码
	void WriteLicenseInformation(CString License);
	void WriteExitTimeReg();
	void initRegStatus(void);
	void InitStyle(void);

	afx_msg void OnBnClickedOk();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedCancel();
	CString getMachineCode(void);
	CString getCurrentDateTime();
// 对话框数据

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual void OnClientDraw(CDC*pDC,INT nWidth,INT nHeight);
	void DrawText( CDC* pDC, CRect rcText,LPCTSTR lpszText,UINT uFormat,INT nHeight=18,bool bBold=true,LPCTSTR lpszName=TEXT("微软雅黑") );

	CImageEx m_titleImage;
	CString  m_strTitle;
	CButtonEx m_btnReg;
	CButtonEx m_btExit;

	DECLARE_MESSAGE_MAP()

private:
	CEdit m_strMCode;
	CString m_strEditSN;

};

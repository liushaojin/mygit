#pragma once
#include "afxwin.h"
#include "controls/BaseDialog.h"
#include "common/DataDefine.h"
#include "afxcmn.h"

// IpSetDlg 对话框

class IpSetDlg : public CBaseDialog
{
	DECLARE_DYNAMIC(IpSetDlg)

public:
	IpSetDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~IpSetDlg();
	virtual BOOL OnInitDialog();


	void InitStyle(void);
	void SetTipContent(CString tip);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();

	CString m_ip;

	enum { IDD = IDD_IPSET_DIALOG };


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual void OnClientDraw(CDC*pDC, INT nWidth, INT nHeight);
	void DrawText(CDC* pDC, CRect rcText, LPCTSTR lpszText, UINT uFormat, INT nHeight = 18, bool bBold = true, LPCTSTR lpszName = TEXT("微软雅黑"));
	bool IsOk(string str, int num);
	bool IsIpAddressValid(const char* pszIPAddr);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);


	CImageEx m_titleImage;
	CString  m_strTitle;
	CButtonEx	m_btnLogin;
	CButtonEx	m_btExit;
	CString m_tip;

	DECLARE_MESSAGE_MAP()
public:
	CIPAddressCtrl m_ipCtrl;
	CEdit m_portEdit;
	CString m_port;
};

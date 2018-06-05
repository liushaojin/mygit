#pragma once
#include "../controls/BaseDialog.h"
#include "../SDMDlg.h"
#include "../network/IOCPServer.h"


class CSystemSetDlg : public CBaseDialog
{
	DECLARE_DYNAMIC(CSystemSetDlg)

public:
	CSystemSetDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSystemSetDlg();
	virtual BOOL OnInitDialog();
	void InitStyle(void);
	void SetMainDlg(CSDMDlg* p) { m_pDlg = p; }
	void Setallparameter(ClientInfo *client);
	void Setallparameter(int nGetSel, BOOL isSendAll);

	LRESULT OnUpdateParams(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedBtnAlldev();
	afx_msg void OnBnClickedBtnSetdev();
	afx_msg void OnBnClickedBtnGetpara();

	afx_msg void OnRadioAutoBnClicked();
	afx_msg void OnRadioLanguageBnClicked();
	afx_msg void OnRadioRingModeBnClicked();
	afx_msg void OnRadioAlarmModeBnClicked();
	afx_msg void OnRadioAreaModeBnClicked();
	afx_msg void OnRadioCheckModeBnClicked();
	afx_msg void OnRadioAlarmAreaBnClicked();
	afx_msg void OnRadioInfrateModeBnClicked();
	afx_msg void OnRadioFilterModeBnClicked();

	CIOCPServer m_server;
	CSDMDlg  *m_pDlg;

	//各区的灵敏度
	int m_sensitivityArea1;
	int m_sensitivityArea2;
	int m_sensitivityArea3;
	int m_sensitivityArea4;
	int m_sensitivityArea5;
	int m_sensitivityArea6;
	int m_sensitivityArea7;
	int m_sensitivityArea8;
	int m_sensitivityAll;
	int m_totalSensitivity;
	int m_frequency;

	int m_radioAutoFrequencyVal;
	int m_radioLanguageVal;
	int m_radioRingModeVal;
	int m_radioAlarmModeVal;
	int m_radioAreaTypeVal;
	int m_radioCheckModeVal;
	int m_radioAlarmAreaVal;
	int m_radioInfrateModeVal;
	int m_radioFilterModeVal;
// 对话框数据
	enum { IDD = IDD_SYSTEMSET_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual void OnClientDraw(CDC*pDC,INT nWidth,INT nHeight);
	void DrawText( CDC* pDC, CRect rcText,LPCTSTR lpszText,UINT uFormat,INT nHeight=18,bool bBold=true,LPCTSTR lpszName=TEXT("微软雅黑") );
	
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	CImageEx m_titleImage;
	CString  m_strTitle;
	CButtonEx m_btnQuery;
	CButtonEx m_btnQueryAll;
	CButtonEx m_btnDel;

	CComboBox m_totalSenseCombo;
	CComboBox m_frequencyCombo;
	CComboBox m_alarmRingCombo;
	CComboBox m_alarmVoiceCombo;
	CComboBox m_alarmTimeCombo;

	DECLARE_MESSAGE_MAP()

};

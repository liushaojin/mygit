#pragma once
#include "../controls/BaseDialog.h"
#include "afxwin.h"
#include "afxcmn.h"


class CUserManageDlg : public CBaseDialog
{
        DECLARE_DYNAMIC(CUserManageDlg)
        
    public:
        CUserManageDlg(CWnd* pParent = NULL);   // 标准构造函数
        virtual ~CUserManageDlg();
        virtual BOOL OnInitDialog();
        void InitStyle(void);
        void InitCtrlDisplayText(void);
        void BeautifyBtn(void);
        void InitUserListCtrl(void);
        void Init(CString username, CString pwd);
        void FillDataAndSetFun();
        void UpdateUserList(LPCTSTR sqlstr);
        void SuperModify();
        bool OrdinaryModify();
        void ChangeCtlLocation();
        
        afx_msg void OnBnClickedAdd();
        afx_msg void OnBnClickedModify();
        afx_msg void OnBnClickedDel();
        afx_msg void OnBnClickedQuit();
        afx_msg void OnNMClickListUserinfo(NMHDR *pNMHDR, LRESULT *pResult);
        
        CButton m_btnAdd;
        CButton m_btnModify;
        CButton m_btnDelete;
        CString m_strEditUserName;
        CString m_strEditPassword;
        CString m_strEditNewPwd;
        CString m_strEditConfirmNewPwd;
        CComboBox m_cmbUserRole;
        
        CMap<int, int, CString, LPCSTR> m_CtrlUserRoleMap;  //用户角色
        CMap<CString, LPCSTR, CString, LPCSTR> m_ListUserRoleMap;  //列表控件角色
        BOOL GetCtrlUserRole(int id, CString &userRole);
        BOOL GetListCtrlUserRole(CString listRole, CString &cmbRole);
        CListCtrl m_lstUserInfo;
        
        int m_nRoleID;
        
// 对话框数据
        enum { IDD = IDD_USERMANAGE_DIALOG };
        
    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
        virtual void OnClientDraw(CDC*pDC, INT nWidth, INT nHeight);
        void DrawText(CDC* pDC, CRect rcText, LPCTSTR lpszText, UINT uFormat, INT nHeight = 18, bool bBold = true, LPCTSTR lpszName = TEXT("微软雅黑"));
        afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
        
        
        CImageEx m_titleImage;
        CString  m_strTitle;
        CButtonEx	m_btnQuery;
        CButtonEx	m_btnQueryAll;
        CButtonEx	m_btnDel;
        CButtonEx	m_btnClear;
        
        DECLARE_MESSAGE_MAP()
        
};

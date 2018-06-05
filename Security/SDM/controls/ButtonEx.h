#pragma once
#include "ImageEx.h"

//按钮的状态
enum
{
    CTRL_NOFOCUS = 0x01,            //普通
    CTRL_FOCUS,                     //mousemove
    CTRL_SELECTED,                  //buttondown
    CTRL_DISABLE,                   //无效
};

//图片形式
enum
{
    BTN_IMG_1 = 1,                  //
    BTN_IMG_3 = 3,                  //3分图（1个图片内有3小图，下同）
    BTN_IMG_4 = 4,                  //4分图
};

//按钮类型
enum
{
    BTN_TYPE_NORMAL = 0x10,         //普通BTN
    BTN_TYPE_MENU,                  //菜单类型的BTN
    BTN_TYPE_STATIC,                //静态类型的BTN
};

class CButtonEx :
    public CButton
{
        DECLARE_DYNAMIC(CButtonEx)
        
    public:
        CButtonEx(void);
        virtual ~CButtonEx(void);
        
        bool FixButtonSize();
        bool SetTextColor(COLORREF crTextColor);
        bool SetButtonImage(LPCTSTR pszFileName, bool bExpandImage = false);
        bool SetButtonImage(HINSTANCE hInstance, LPCTSTR pszResourceName, bool bExpandImage = false);
        void PaintParent();
        
        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
        
    protected:
        virtual void PreSubclassWindow();
        virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
        
        afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg LRESULT OnMouseLeave(WPARAM wparam, LPARAM lparam);
        afx_msg LRESULT OnMouseHover(WPARAM wparam, LPARAM lparam);
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        
        bool m_bExpand;
        bool m_bHovering;
        COLORREF m_crTextColor;
        CImageEx m_ImageBack;
        
        UINT m_nState;
        UINT m_nBtnType;
        BOOL m_bMenuOn;     //BTN类型为BTN_TYPE_MENU时，是否处于按下的状态
        
        DECLARE_MESSAGE_MAP()
};


#pragma once

class CListBoxEx :
    public CListBox
{
        DECLARE_DYNAMIC(CListBoxEx)
        
        
    public:
        CListBoxEx(void);
        virtual ~CListBoxEx(void);
        
        void SetItemHeight(int nItemHeight);
        virtual void MeasureItem(LPMEASUREITEMSTRUCT /*lpMeasureItemStruct*/);
        virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
        virtual int CompareItem(LPCOMPAREITEMSTRUCT /*lpCompareItemStruct*/);
        
    protected:
        DECLARE_MESSAGE_MAP()
        
        INT m_nItemHeight;
        
};


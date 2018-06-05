#include "StdAfx.h"
#include "TaskKeyMgr.h"

#define HKCU HKEY_CURRENT_USER
LPCTSTR KEY_DisableTaskMgr = "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System";
LPCTSTR VAL_DisableTaskMgr = "DisableTaskMgr";

/******************************************************************
 - 功能描述：Disable task-key related stuff.
 - 隶属模块：windows平台
 - 函数属性：内部系统调用
 - 参数说明：
			dwFlags  = what to disable
			bDisable = disable (TRUE) or enable (FALSE)
			bBeep    = whether to beep for illegal keys (TASKKEYS only)
 - 返回说明：无
 ******************************************************************/

void CTaskKeyMgr::Disable(DWORD dwFlags, BOOL bDisable, BOOL bBeep)
{
    // task manager (Ctrl+Alt+Del)
    if(dwFlags & TASKMGR)
    {
        HKEY hk;
        
        if(RegOpenKey(HKCU, KEY_DisableTaskMgr, &hk) != ERROR_SUCCESS)
        {
            RegCreateKey(HKCU, KEY_DisableTaskMgr, &hk);
        }
        
        if(bDisable)
        {
            // disable TM: set policy = 1
            DWORD val = 1;
            RegSetValueEx(hk, VAL_DisableTaskMgr, NULL, REG_DWORD, (BYTE*)&val, sizeof(val));
        }
        else
        {
            // enable TM: remove policy
            RegDeleteValue(hk, VAL_DisableTaskMgr);
        }
    }
    
    // task keys (Alt-TAB etc)
    if(dwFlags & TASKKEYS)
    {
        ::DisableTaskKeys(bDisable, bBeep); // install keyboard hook
    }
    
    // task bar
    if(dwFlags & TASKBAR)
    {
        HWND hwnd = FindWindow("Shell_traywnd", NULL);
        EnableWindow(hwnd, !bDisable);
    }
}

BOOL CTaskKeyMgr::IsTaskBarDisabled()
{
    HWND hwnd = FindWindow("Shell_traywnd", NULL);
    return IsWindow(hwnd) ? !IsWindowEnabled(hwnd) : TRUE;
}

BOOL CTaskKeyMgr::IsTaskMgrDisabled()
{
    HKEY hk;
    
    if(RegOpenKey(HKCU, KEY_DisableTaskMgr, &hk) != ERROR_SUCCESS)
    {
        return FALSE;    // no key ==> not disabled
    }
    
    DWORD val = 0;
    DWORD len = 4;
    return RegQueryValueEx(hk, VAL_DisableTaskMgr, NULL, NULL, (BYTE*)&val, &len) == ERROR_SUCCESS && val == 1;
}

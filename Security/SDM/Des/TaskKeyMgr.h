#pragma once
#include "TaskKeyHook.h"

// Use this class to disable task keys, task manager and/or the taskbar.
// Call Disable with flags for items you want to disable;
//for example: CTaskMgrKeys::Disable(CTaskMgrKeys::ALL);

class CTaskKeyMgr
{
    public:
        enum
        {
            TASKMGR  = 0x01,	// disable task manager (Ctrl+Alt+Del)
            TASKKEYS = 0x02,	// disable task keys (Alt-TAB, etc)
            TASKBAR  = 0x04,	// disable task bar
            ALL = 0xFFFF			// disable everything :
        };
        static void Disable(DWORD dwItem, BOOL bDisable, BOOL bBeep = FALSE);
        
        static BOOL IsTaskMgrDisabled();
        static BOOL IsTaskBarDisabled();
        static BOOL AreTaskKeysDisabled()// call DLL
        {
            return ::AreTaskKeysDisabled();
        }
};

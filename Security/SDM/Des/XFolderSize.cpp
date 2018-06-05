#include <windows.h>
#include <stdio.h>
#include <crtdbg.h>
#include <tchar.h>
#include "XFolderSize.h"


#pragma warning(disable : 4127)		// conditional expression is constant
#pragma warning(disable : 4996)		// bogus deprecation warning

///////////////////////////////////////////////////////////////////////////////
//
// GetFileSize64()
//
// Purpose:     Get 64-bit file size
//
// Parameters:  lpszPath   - fully qualified path to file
//              lpFileSize - 64-bit file size
//
// Returns:     BOOL - Returns nonzero if successful or zero otherwise.
//
BOOL CXFolderSize::GetFileSize64(LPCTSTR lpszPath, PLARGE_INTEGER lpFileSize)
{
    BOOL rc = FALSE;
    _ASSERTE(lpszPath);
    _ASSERTE(lpFileSize);
    
    if(lpszPath && lpFileSize)
    {
        lpFileSize->QuadPart = 0;
        HANDLE hFile = NULL;
        hFile = CreateFile(lpszPath, READ_CONTROL, 0, NULL,
                           OPEN_EXISTING, 0, NULL);
                           
        if(hFile != INVALID_HANDLE_VALUE)
        {
            rc = GetFileSizeEx(hFile, lpFileSize);
            CloseHandle(hFile);
        }
    }
    
    return rc;
}

///////////////////////////////////////////////////////////////////////////////
//
// GetFolderSize()
//
// Purpose:     Get folder size
//
// Parameters:  lpszStartFolder - fully qualified starting folder path
//              bRecurse        - TRUE = recurse into subdirectories
//              bQuickSize      - TRUE = if lpszStartFolder is a drive, use
//                                SHGetDiskFreeSpace() to get size for entire
//                                disk, instead of recursing (folder count
//                                and file count will be 0).
//              lpFolderSize    - pointer to 64-bit folder size
//              lpFolderCount   - pointer to 32-bit folder count (optional,
//                                may be NULL)
//              lpFileCount     - pointer to 64-bit file count (optional,
//                                may be NULL)
//
// Returns:     BOOL - Returns nonzero if successful or zero otherwise.
//
BOOL CXFolderSize::GetFolderSize(LPCTSTR lpszStartFolder,
                                 BOOL bRecurse,
                                 BOOL bQuickSize,
                                 PLARGE_INTEGER lpFolderSize,
                                 LPDWORD lpFolderCount /*= NULL*/,
                                 LPDWORD lpFileCount /*= NULL*/)
{
    BOOL rc = FALSE;
    _ASSERTE(lpszStartFolder);
    _ASSERTE(lpFolderSize);
    
    if(lpszStartFolder && lpFolderSize)
    {
        lpFolderSize->QuadPart = 0;
        
        if(lpFolderCount)
        {
            *lpFolderCount = 0;
        }
        
        if(lpFileCount)
        {
            *lpFileCount = 0;
        }
        
        if(bQuickSize && (_tcslen(lpszStartFolder) < 4))
        {
            // get used space for entire volume
            ULARGE_INTEGER ulFreeCaller;
            ulFreeCaller.QuadPart = 0;
            ULARGE_INTEGER ulTotal;
            ulTotal.QuadPart = 0;
            ULARGE_INTEGER ulFree;
            ulFree.QuadPart = 0;
            rc = SHGetDiskFreeSpace(lpszStartFolder, &ulFreeCaller, &ulTotal, &ulFree);
            lpFolderSize->QuadPart = ulTotal.QuadPart - ulFree.QuadPart;
            return rc;
        }
        
        HANDLE hFF = 0;
        WIN32_FIND_DATA wfd = { 0 };
        TCHAR szPath[2000];
        LARGE_INTEGER li_total;
        li_total.QuadPart = 0;
        DWORD dwTotalFolderCount = 0;
        DWORD dwTotalFileCount = 0;
        _tcsncpy(szPath, lpszStartFolder, sizeof(szPath) / sizeof(TCHAR) - 1);
        _tcscat(szPath, _T("\\*"));
        hFF = FindFirstFile(szPath, &wfd);
        
        if(hFF != INVALID_HANDLE_VALUE)
        {
            rc = TRUE;
            
            do
            {
                _tcsncpy(szPath, lpszStartFolder, sizeof(szPath) / sizeof(TCHAR) - 1);
                _tcscat(szPath, _T("\\"));
                _tcscat(szPath, wfd.cFileName);
                
                if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if(_tcscmp(wfd.cFileName, _T(".")) &&
                            _tcscmp(wfd.cFileName, _T("..")) &&
                            bRecurse)
                    {
                        // recurse into the subdirectory
                        LARGE_INTEGER li;
                        li.QuadPart = 0;
                        DWORD dwFolderCount = 0;
                        DWORD dwFileCount = 0;
                        rc = GetFolderSize(szPath, bRecurse, bQuickSize, &li,
                                           &dwFolderCount, &dwFileCount);
                        li_total.QuadPart += li.QuadPart;
                        dwTotalFolderCount += dwFolderCount + 1;
                        dwTotalFileCount += dwFileCount;
                    }
                }
                else
                {
                    LARGE_INTEGER li;
                    li.QuadPart = 0;
                    rc = GetFileSize64(szPath, &li);
                    li_total.QuadPart += li.QuadPart;
                    dwTotalFileCount += 1;
                }
            }
            while(FindNextFile(hFF, &wfd));
        }
        
        *lpFolderSize = li_total;
        
        if(lpFolderCount)
        {
            *lpFolderCount = dwTotalFolderCount;
        }
        
        if(lpFileCount)
        {
            *lpFileCount = dwTotalFileCount;
        }
        
        if(hFF != INVALID_HANDLE_VALUE)
        {
            FindClose(hFF);
        }
    }
    
    return rc;
}

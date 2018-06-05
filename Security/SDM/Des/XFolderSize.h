#ifndef XFOLDERSIZE_H
#define XFOLDERSIZE_H

class CXFolderSize
{
// Constructor
    public:
        CXFolderSize() {};
        
        
        BOOL GetFileSize64(LPCTSTR lpszPath, PLARGE_INTEGER lpFileSize);
        BOOL GetFolderSize(LPCTSTR lpszStartFolder,
                           BOOL bRecurse,
                           BOOL bQuickSize,
                           PLARGE_INTEGER lpFolderSize,
                           LPDWORD lpFolderCount = NULL,
                           LPDWORD lpFileCount = NULL);
};

#endif //XFOLDERSIZE_H

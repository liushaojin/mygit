#pragma once
#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <vector>
#include "SDM.h"
#include "./common/DataDefine.h"
#include "Log.h"

using namespace std;

extern CString g_strCurrentLanguage;
extern CString GetIniStringValue(CString strSection, CString strSectionKey, CString strFileName);
class CPublicFunction
{
    public:
        CPublicFunction(void);
        ~CPublicFunction(void);
        
    public:
        //判断给定的文件是否操作
        static bool IsFileExist(const CString& strName);
        //改造图片文件全路径
        static CString GetImagePath(const CString &path, const CString &fileName, const CString &date);
        //显示图片
        static void ShowImage(IPicture *pPicture, RECT rect, CDC *dc, long nPicWidth, long nPicHeight, int rgb, int zoomStep, int zoomLeft, int zoomTop);
        //根据文件名获取图片流
        static IPicture* GetPictureFromName(const char *path);
        //根据名称获取BMP图像信息
        static BMPImageInfo GetBmpImageInfoByName(const char *dibFileName);
        
        static double GetShowRadio(int clickCount);
        static int GetClickCount(double radio);
        
        //static BOOL   ShowMessageBox(const CString &msg,MsgBoxType msgType=eWarning,const CString &title=GetIniStringValue("MainDialog","PromptTitle",g_strCurrentLanguage));
        static void DrawRectangeSide(CDC *pDc, CRect rect, int nWidth, COLORREF crColor); //画矩形边框
        static CString GetCurrentDateTime();
        static CString GetImageNameTimeSpan(CString &curDT);
        static CString GetDate();
        static CString CreateCurrentDTPath();//创建保存目录
        static void WriteRunLogFile(CString Source, CString Message); //记录运行日志
        
        static void DrawGradient(HDC pDC, const RECT &rect,
                                 COLORREF beginColor = RGB(160, 160, 160), COLORREF endColor = RGB(236, 236, 236),
                                 bool fromUpToDown = false, bool drawWhole = true, int drawHeight = 0);
                                 
        static int SetRegValue(LPCSTR KeyValueName, DWORD &dwVal);
        static void GetRegValue();
        static void WriteExitTimeReg();
        static void LostRegInfoPrompt();
        
        static BOOL WriteCodeToFile(const CString& code, const CString &path);
        static CString ReadCodeFromFile(const CString& path);
        
        static BOOL ContainSpecialChar(const CString &str);
        
        //指定磁盘（例如：C:）是否有足够的空间可供使用，crisisValue为临界值，单位为MB
        static BOOL DiskHasEnoughSpace(const CString &szPath, DWORD &crisisValue);
        
        static void DeleteDir(CString str);
        static BOOL DelFolder(const CString &szPath);
        //返回文件夹总大小（单位：MB）
        static double GetFolderSizeEx(const CString &szPath);
        
        static BOOL CreateDir(const CString& dir);
        
        static vector<CString> SplitString(const CString& stringToSplit, const char split);
        
        static bool ThePlateNoIsExsit(const CString& plateNo);
        
        static CheckInfo GetCheckInfo();
        
        static BOOL ModifyCheckResult(const CString& id, int checkResult, const CString& field);
        
        static BOOL UpdateDB(CheckInfo &checkInfo, const CString& plateNo, int vpId, const CString& saveName, const CString& plateNoImageName, const CString &captureImageName, const CString& exist);
        
        //缩小坐标时调整
        static void AdjustCroodForZoomIn(int& destX, int& destY, int& displayW, int& displayH, int& clientW, int& clientH);
        //移动图像时限制图像移出边界
        static void AdjustCroodForDrag(int& destX, int& destY, int& displayW, int& displayH, int& clientW, int& clientH, int moveW, int moveH);
        
        //显示抓拍图像时调整
        static void AdjustShowSnapRect(CRect rect, int nImageWidth, int nImageHeight, int& destX, int& destY, int& destW, int& destH);
        
        static CheckInfo GetTcpBackupMsgCheckInfo();
        
        static void ShowImage(CDC *dc, HDC hdc, CString &fileName, int ctrIID, RECT rect, int rgb);
    private:
        static void WriteLog();
};

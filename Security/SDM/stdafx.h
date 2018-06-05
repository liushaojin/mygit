
// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef _SECURE_ATL
    #define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
    #define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展


#include <afxdisp.h>        // MFC 自动化类
#include <conio.h>


#ifndef _AFX_NO_OLE_SUPPORT
    #include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
    #include <afxcmn.h>             // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // 功能区和控件条的 MFC 支持
#include "Resource.h"

#define GOOGLE_GLOG_DLL_DECL
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <Windows.h>
#include "glog/logging.h"
#include "SkinPPWTL.h"   //在这个地方加

#ifdef _DEBUG
    #pragma comment(lib,"glog/libglog_staticd.lib")
#else
    #pragma comment(lib,"glog/libglog_static.lib")
#endif

#define SAFE_DELETE_ARRAYP(p)   {if(p) { delete[] (p); (p) = NULL; } }
#define SAFE_DELETE_CLASS(p)    {if(p) {delete (p); (p) = NULL;}}



//数组维数
#define CountArray(Array) (sizeof(Array)/sizeof(Array[0]))
//接口释放
#define SafeRelease(pObject) { if (pObject!=NULL) { pObject->Release(); pObject=NULL; } }

#include <GdiPlus.h>
using namespace Gdiplus;

#pragma comment(lib,"Gdiplus.lib")

#pragma warning(disable:4099)

#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")
using namespace std;

//定义客户端的数据结构
typedef struct _ClientInfo
{
    char ipAddress[25];
    int  nPort;
    WSABUF wsaBuf;
    int doorID;
	int cntOfOnlineJudge;	//掉线判断次数
	int isOfflineFlag;		//掉线标志0：在线；1：掉线
} ClientInfo;

//定义安检门检测数据结构
typedef struct _DetectData
{
	int incId;			//自增序号
	CString doorId;		//安检门编号
	int passForward;	//前向通过人数
	int passBack;		//后向通过人数
	int warnCnt;		//报警次数
	CString warnArea;	//报警区域
	CString warnTime;	//报警发生时间
} DetectData;


#include "common/PublicFunction.h"
#define LOGMESSAGE(src, msg) CPublicFunction::WriteRunLogFile(src, msg)


#ifdef _UNICODE
    #if defined _M_IX86
        #pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
    #elif defined _M_X64
        #pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
    #else
        #pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
    #endif
#endif

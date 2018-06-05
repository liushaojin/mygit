#ifndef _DATADEFINE_H_
#define _DATADEFINE_H_

enum UserAuthority
{
	uaSuperAdministrator=1,       //超级管理员
	uaCommonlyAdministrator=2,    //一般管理员
	uaSysMaintenancePersonnel=3,  //系统维护人员
	uaOrdinaryOperator=4,          //普通操作员
	uaNone=5
};

typedef struct _tagUser
{
	int id;                          //用户ID
	CString username;                //用户名
	CString pwd;                     //密码
	int     roleID;                  //权限ID
	UserAuthority userAuthority;     //权限
	CString       des;               //描述信息

	_tagUser()
	{
		id=0;
		username="";
		pwd="";
		roleID=4;
		userAuthority=uaNone;
		des="";
	}
}User;

enum AutoJudged
{
	ajNormal=0,
	ajContraband=1,
	ajDangerous=2,
	ajCustom=3,
	ajDifficultToPenetrate=4
};

typedef struct _tagCheckInfo
{
	CString   Id;                   //ID, by automatic generation system
	CString   SDoorID;			    //门ID
	CString   checkTime;            //检查时间
	CString   autoJudged;           //检查结果
	CString   userName;             //顾客姓名
	CString   userID;               //顾客身份证ID
	CString   captureImageName;     //抓拍图像名称
	CString   existHisRecord;       //是否存在历史记录
	CString   checkOperator;        //操作人员
	CString   describe;             //顾客描述信息
	CString   BlackWhiteList ;      //黑白名单

} CheckInfo;

//提示消息类型
enum MsgBoxType
{
	eInformation = 0,     //信息
	eQuestion    = 1,     //疑问
	eWarning     = 2,     //警告
	eError       =3       //错误
};

//BMP图像信息
typedef struct _tagBMPImageInfo
{
	BYTE  *pBmpData;   //BMP图像的数据
	DWORD dSize;       //大小
	int   nWidth;      //宽度
	int   mHeight;     //高度
} BMPImageInfo;

struct TModuleData
{
	WORD Code;//模块编号
	WORD StartTime;
	WORD Day;//使用天数
	WORD Custom;//模块自定义信息
};

#endif
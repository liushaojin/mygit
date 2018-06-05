#pragma once
#include <atlstr.h>
//表的枚举
enum DBTable {Tb_ANNUNCIATOR, Tb_BASECONFIG, Tb_DEVICES, Tb_TLANGUAGE,
              Tb_DATACOUNT, Tb_CUser, Tb_UserID, Tb_Logon, Tb_Info,
              Tb_UserInfo, Tb_UserPermission, Tb_LogFile, Tb_CheckInfo
             };

//数据库中的表名
const CString cnt_TableName[22] = { "LOG_ID", "LOGON", "TBLE_INFO",
                                    "USER_INFO", "USER_PERMISSION", "LOG_FILE", "CheckInfo"
                                  };

const CString cnt_UserIDLog[9] = {"NUMBER", "ID", "PASSWORD", "CLASS", "SUPERIOR_ID",
                                  "ID_VALID", "LOGON_DATE", "LOGOUT_DATE", "RE_PASSWORD_DATE"
                                 };

const CString cnt_Logon[5] = {"AUTO_NUMBER", "LOG_ID", "LOGIN_TIME", "LOGOUT_TIME", "REMARK"};
const CString cnt_Info[3] = {"NUM", "CONTEXT", "TIME"};

//用户信息表
const CString cnt_UserInfo[5] = {"ID", "USER_NAME", "USER_PASSWORD", "USER_DELETED", "USER_CLASS"};
//用户权限表
const CString cnt_UserPermission[20] = {"ID", "PICTURE_SET", "CTRLPANEL_QUERY", "CTRLPANEL_COMSET", "POINT_SET",
                                        "XRAY_SET", "FUNCTION_SET", "TIME_CURVE", "FILE_MANAGE",
                                        "ERROR_INFO", "TIP_MANAGE", "TIP_REPORT", "ALARM_SET", "USER_MANAGE",
                                        "PERMISSION_MANAGE", "DATA_COUNT", "ABOUT", "SOFT_REGIST", "NET_SET", "USER_RANK"
                                       };
//日志表
const CString cnt_LogFile[5] = {"AUTO_NUMBER", "USER_NAME", "TIME", "OPERATE", "BAG_COUNT"};

const CString cnt_CarCheckInfo[9] = {"ID", "CheckTime", "SaveName", "LiencePlate", "DriverName", "CheckResult", "PlateNoImageName", "Passage", "CheckOperator"};
//安检门信息表
const CString cnt_Enannunciator[7] = {"SNO", "DOORID", "PASSFORWARD", "PASSBACK", "WARNING", "SDOMAIN", "SDATE"};
//安检门统计信息表
const CString cnt_CountTb[9] = { "SN", "DOORID", "TOTALPASS", "TOTALALARM", "TODAYPASS", "TODAYALARM", "UPDATE", "LASTPASS", "LASTALARM" };
//安检门英文
//const CString cnt_LogFile[5] = {"AUTO_NUMBER","USER_NAME","TIME","OPERATE","BAG_COUNT"};
const CString cnt_Entlanguage[4] = { "id", "remark", "chinese", "enlish" }; //jmj add
const CString cnt_Enbaseconfig[5] = { "code", "stype", "name", "high", "low" };
const CString cnt_Endevices[3] = { "COMID", "NAME", "REMARK" };

//安检门中文
//const CString cnt_LogFile[5] = {"AUTO_NUMBER","USER_NAME","TIME","OPERATE","BAG_COUNT"};
//const CString cnt_CHtlanguage[4] = { "编号","说明","中文","英文" };//jmj add
//const CString cnt_CHEnannunciator[6] = { "自动编号", "门编号", "通过人数", "报警人数", "区域情况", "时间" };
//const CString cnt_CHEnbaseconfig[5] = { "编码", "类型", "名称", "上限", "下限" };
//const CString cnt_CHEndevices[3] = { "设备编号", "名称", "备注" };
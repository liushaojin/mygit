#pragma once
//图像效果

//成像流程宏定义
#define STEP            5
#define SKIPLINE        10
#define DETECTWIDTH     1400
#define VALIDTHREOLD    10
#define EXPONENTIAL     400
//相机相关宏定义
#define TRMODE          0//触发模式
#define TRSOURCE        1//
#define WORKMODE        4
#define TESTMODE        0 
#define DEFAULTLINE     2000


#define MAXLINE         10000
#define MINLINE         2000
#define MOBILE          0
#define LANG_ENG        1 //中文设为0，英文设为1

//成都火眼臻睛再固定式上冲突，不能使用
#define LPRSDKTYPE      4//车牌识别SDK：0文通，1亚洲视觉，2成都火眼臻睛，3佳都,4中安
#define MAXHEIGHT		10000//最大高度
#define NORMALHEIGHT    6000

#define MAXWIDTH		2096//最大的宽度


#define UGANDA          0  //乌干达

#define PRISON          0  //监狱版本，默认6路视频
#define DOUBLEGE        0 //连接车底数

typedef WORD* DataByte;
typedef WORD DateFormat;

#define DATABIT			0//0:mid 1:high

//调试相关宏定义
//#define NO_HARDWARE    1//无硬件时进行调试设为1，有硬件时设为0

#define NEEDREGISTER     1//需要注册设为1;2加密狗硬件ID注册并写入加密狗

#define DVRNINECHANEL    0  //9路视频显示

#define CVRSCANNER		 0//身份证扫描

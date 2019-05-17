#pragma once

wstring string2wstring(string str);  
string wstring2string(wstring wstr);  
void GetAppPath(CString &strPath);
BOOL CreateMultiDirs(CString xFilePath);
BOOL DeleteDirectory(CString &pFilePath);
LONG CrashHandler(EXCEPTION_POINTERS *pException);
void Delay(HANDLE hEvent);
int string_replase(string &s1, const string &s2, const string &s3);//add by zhuxy


//add by zhuxy20190315 一些宏定义
/*返回值枚举类型*/
enum RET_VALUE{
	RET_OK = 0,
	RET_ERR
};
/*采集工作方式，手动和自动*/
enum WORK_WAYS{
	WAY_AUTO=0,//自动采集中
	WAY_MANUAL,//手动采集中
	WAY_STOP//当前没有进行采集动作
};
/*当前采集是否就绪*/
enum WORK_STATE{
	STATE_UNKNOWN=-1,
	STATE_BUSY=0,
	STATE_READY=1
};
/*权限*/
enum ROOT{
	ROOT_ADMIN=0,
	ROOT_GEUST,
	ROOT_RESERVE
};
#if CLOSE_IF
/*主备*/
enum MAIN_SPARE{
	MAIN_CONTROL=1,
	SPARE_CONTROL
};
#endif
//add by zhuxy20190319 配置文件的key个数
#define CFG_KEY_COUNT 19
#define CAM_KEY_COUNT 27
//add by zhuxy20190319 时间的字节数，年月日时分秒
#define TIME_LENGTH 14 
//add by zhuxy20190322 tcp socket接收的数组大小
#define RCV_BUF_SIZE 256
//add by zhuxy20190322 tcp中过车信息的包大小
#define TRAIN_INFO_SIZE 8
//add by zhuxy20190325 if块的打开关闭
#define CLOSE_IF 0
#define OPEN_IF 1
//add by zhuxy 行高宽度
#define LINE_HIGHT 30
#define LINE_WIGHT 180
//add by zhuxy20190327	图像显示的大小
#define IMAGE_SHOW_SIZE 1000
//开始采集\停止采集\上传数据按钮初始ID
#define START_BTN_ID 100
#define STOP_BTN_ID 200
#define UPLOAD_BTN_ID 300
#define BTN_ID_FOR_ALL_CAM 400

//IP
//#define Control_IP _T("192.168.3.22")
#define Control_IP _T("127.0.0.1")
#define Control_UDP_Port 8280

#define Main_Gather_IP _T("127.0.0.1")
#define Main_Gather_Server_Port 20001
#define Main_Gather_UDP_PORT 20002


#define Spare_Gather_Server_Port 20001
#define Spare_Gather_UDP_PORT 20002


/*返回宏*/
#define PRINTF_LOCATION() TRACE("ret in %s at %d",__FILE__,__LINE__)

#define RET_VALUE_IF_NOT_EAQU(a,b,c)  \
	do {  \
	if(a!=b) \
{        \
	PRINTF_LOCATION();\
	return c; \
}        \
	} while (false)

#define RET_VALUE_IF_EAQU(a,b,c)  \
	do {  \
	if(a==b) \
{        \
	PRINTF_LOCATION();\
	return c; \
}        \
	} while (false)

#define RET_IF_NOT_EAQU(a,b)  \
	do {  \
	if(a!=b) \
{        \
	PRINTF_LOCATION();\
	return; \
}        \
	} while (false)

#define RET_IF_EAQU(a,b)  \
	do {  \
	if(a==b) \
{        \
	PRINTF_LOCATION();\
	return; \
}        \
	} while (false)

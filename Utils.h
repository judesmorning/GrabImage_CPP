#pragma once

wstring string2wstring(string str);  
string wstring2string(wstring wstr);  
void GetAppPath(CString &strPath);
BOOL CreateMultiDirs(CString xFilePath);
BOOL DeleteDirectory(CString &pFilePath);
LONG CrashHandler(EXCEPTION_POINTERS *pException);
void Delay(HANDLE hEvent);
int string_replase(string &s1, const string &s2, const string &s3);//add by zhuxy


//add by zhuxy20190315 һЩ�궨��
/*����ֵö������*/
enum RET_VALUE{
	RET_OK = 0,
	RET_ERR
};
/*�ɼ�������ʽ���ֶ����Զ�*/
enum WORK_WAYS{
	WAY_AUTO=0,//�Զ��ɼ���
	WAY_MANUAL,//�ֶ��ɼ���
	WAY_STOP//��ǰû�н��вɼ�����
};
/*��ǰ�ɼ��Ƿ����*/
enum WORK_STATE{
	STATE_UNKNOWN=-1,
	STATE_BUSY=0,
	STATE_READY=1
};
/*Ȩ��*/
enum ROOT{
	ROOT_ADMIN=0,
	ROOT_GEUST,
	ROOT_RESERVE
};
#if CLOSE_IF
/*����*/
enum MAIN_SPARE{
	MAIN_CONTROL=1,
	SPARE_CONTROL
};
#endif
//add by zhuxy20190319 �����ļ���key����
#define CFG_KEY_COUNT 19
#define CAM_KEY_COUNT 27
//add by zhuxy20190319 ʱ����ֽ�����������ʱ����
#define TIME_LENGTH 14 
//add by zhuxy20190322 tcp socket���յ������С
#define RCV_BUF_SIZE 256
//add by zhuxy20190322 tcp�й�����Ϣ�İ���С
#define TRAIN_INFO_SIZE 8
//add by zhuxy20190325 if��Ĵ򿪹ر�
#define CLOSE_IF 0
#define OPEN_IF 1
//add by zhuxy �и߿��
#define LINE_HIGHT 30
#define LINE_WIGHT 180
//add by zhuxy20190327	ͼ����ʾ�Ĵ�С
#define IMAGE_SHOW_SIZE 1000
//��ʼ�ɼ�\ֹͣ�ɼ�\�ϴ����ݰ�ť��ʼID
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


/*���غ�*/
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

#include "StdAfx.h"
#include "Myserial.h"


/*************************************串口定义***************************************************/
HANDLE hCom;
unsigned char d[8];
DWORD dwBytesWritten = 9;
DWORD dwErrorFlags=0;
COMSTAT ComStat;
OVERLAPPED m_osWrite;
BOOL bWriteStat;
bool a;
int i;
int flag=1;
int serialflag=0;
int receiveSign;
BOOL bRead = TRUE;
BOOL bResult = TRUE;
DWORD dwError = 0;
DWORD BytesRead = 0;
char RXBuff;
BYTE aaaa[4096];


/*****************************************数据定义**************************/



/*****************************************串口类定义**************************/


//函数名称:构造函数
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
ComAsy::ComAsy()://构造函数
m_hCom(INVALID_HANDLE_VALUE),
	m_IsOpen(false),
	m_Thread(NULL)
{
	memset(&m_ovWait, 0, sizeof(m_ovWait));
	memset(&m_ovWrite, 0, sizeof(m_ovWrite));
	memset(&m_ovRead, 0, sizeof(m_ovRead));
}

//函数名称:析构函数
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
ComAsy::~ComAsy()//析构函数
{
	UninitCOM();//串口对象被释放时关闭串口
}

//函数名称:初始化串口
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
bool ComAsy::InitCOM(LPCTSTR Port)//配置串口
{
	m_hCom = CreateFile(Port, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED|FILE_ATTRIBUTE_NORMAL,//设置异步标识
		NULL);
	if (INVALID_HANDLE_VALUE == m_hCom)
	{
		return false;
	}
	SetupComm(m_hCom, 4096, 4096);//设置发送接收缓存

	DCB dcb;//96n81模式
	GetCommState(m_hCom, &dcb);
	dcb.DCBlength = sizeof(dcb);
	dcb.BaudRate = 9600;//波特率
	dcb.StopBits = ONESTOPBIT;//停止位数为1位
	dcb.Parity = 0;//校验方式为无校验
	dcb.ByteSize = 8;//数据位为8位
	SetCommState(m_hCom, &dcb);//配置串口

	PurgeComm(m_hCom, PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR|PURGE_TXABORT);

	COMMTIMEOUTS ct;
	ct.ReadIntervalTimeout = MAXDWORD;//两字符之间最大的延时，读取无延时，因为有WaitCommEvent等待数据
	ct.ReadTotalTimeoutConstant = 0;  //读时间常量
	ct.ReadTotalTimeoutMultiplier = 0;// 读时间系数

	ct.WriteTotalTimeoutMultiplier = 500;// 写时间系数
	ct.WriteTotalTimeoutConstant = 5000;// 写时间常量

	SetCommTimeouts(m_hCom, &ct);//配置读写超时

	//创建事件对象
	m_ovRead.hEvent = CreateEvent(NULL, false, false, NULL);
	m_ovWrite.hEvent = CreateEvent(NULL, false, false, NULL);
	m_ovWait.hEvent = CreateEvent(NULL, false, false, NULL);

	SetCommMask(m_hCom, EV_ERR | EV_RXCHAR);//设置接受事件

	//创建读取线程
	m_Thread = (HANDLE)_beginthreadex(NULL, 0, &ComAsy::OnRecv, this, 0, NULL);
	m_IsOpen = true;
	return true;
}

//函数名称:关闭串口
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void ComAsy::UninitCOM()//关闭串口
{
	m_IsOpen = false;
	if (INVALID_HANDLE_VALUE != m_hCom)
	{
		CloseHandle(m_hCom);
		m_hCom = INVALID_HANDLE_VALUE;
	}
	if (NULL != m_ovRead.hEvent)
	{
		CloseHandle(m_ovRead.hEvent);
		m_ovRead.hEvent = NULL;
	}
	if (NULL != m_ovWrite.hEvent)
	{
		CloseHandle(m_ovWrite.hEvent);
		m_ovWrite.hEvent = NULL;
	}
	if (NULL != m_ovWait.hEvent)
	{
		CloseHandle(m_ovWait.hEvent);
		m_ovWait.hEvent = NULL;
	}
	if (NULL != m_Thread)
	{
		WaitForSingleObject(m_Thread, 5000);//等待线程结束
		CloseHandle(m_Thread);
		m_Thread = NULL;
	}
}

//函数名称:发送数据
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
bool ComAsy::ComWrite(LPBYTE buf, int &len)//发送数据
{
	BOOL rtn = FALSE;
	DWORD WriteSize = 0;

	PurgeComm(m_hCom, PURGE_TXCLEAR|PURGE_TXABORT);
	m_ovWait.Offset = 0;
	rtn = WriteFile(m_hCom, buf, len, &WriteSize, &m_ovWrite);

	len = 0;
	if (FALSE == rtn && GetLastError() == ERROR_IO_PENDING)//后台读取
	{
		//等待数据写入完成
		if (FALSE == ::GetOverlappedResult(m_hCom, &m_ovWrite, &WriteSize, TRUE))
		{
			return false;
		}
	}

	len = WriteSize;
	return rtn != FALSE;

}

//函数名称:接收数据
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
unsigned int __stdcall ComAsy::OnRecv( void* LPParam)//接收数据
{
	ComAsy *obj = static_cast<ComAsy*>(LPParam);

	DWORD WaitEvent = 0, Bytes = 0;
	BOOL Status = FALSE;
	BYTE ReadBuf[4096];
	DWORD Error;
	COMSTAT cs = {0};

	while(obj->m_IsOpen)
	{
		WaitEvent = 0;
		obj->m_ovWait.Offset = 0;
		Status = WaitCommEvent(obj->m_hCom,&WaitEvent, &obj->m_ovWait );
		//WaitCommEvent也是一个异步命令，所以需要等待
		if (FALSE == Status && GetLastError() == ERROR_IO_PENDING)//
		{
			//如果缓存中无数据线程会停在此，如果hCom关闭会立即返回False
			Status = GetOverlappedResult(obj->m_hCom, &obj->m_ovWait,  &Bytes, TRUE);
		}
		ClearCommError(obj->m_hCom, &Error, &cs);
		if (TRUE == Status //等待事件成功
			&& WaitEvent&EV_RXCHAR//缓存中有数据到达
			&& cs.cbInQue > 0)//有数据
		{
			Bytes = 0;
			obj->m_ovRead.Offset = 0;
			memset(ReadBuf, 0, sizeof(ReadBuf));
			//数据已经到达缓存区，ReadFile不会当成异步命令，而是立即读取并返回True
			Status = ReadFile(obj->m_hCom, ReadBuf, sizeof(ReadBuf), &Bytes, &obj->m_ovRead);
			PurgeComm(obj->m_hCom, PURGE_RXCLEAR|PURGE_RXABORT);
			//for(int i=0;i<4096;i++)
			//	aaaa[i]=ReadBuf[i];
			//这里给trainNumStr赋值
		}

	}
	return 0;
}
// Myudp.cpp : 实现文件
//

#include "stdafx.h"
#include "ImageGrab.h"
#include "Myudp.h"

UINT udpSendThread(LPVOID param);

// Myudp
//函数名称:构造函数
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
Myudp::Myudp()
{
	AfxSocketInit();
	stopSend=false;
	spareCount=1;
	spareGatherIPS.resize(spareCount);
	spareGatherIPS[0] = _T("127.0.0.1");//备控的ip定义
}

Myudp::~Myudp()
{
	stopSend=true;
}


// Myudp 成员函数



//函数名称:接收数据
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:beginTime:%Y%m%d%H%M%S
void Myudp::OnReceive(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类
	char info[1024];CString ip;uint port;
	int rcvNum = ReceiveFrom(info,1*1024,ip,port);
	if(1 != GlobalParameter::GetInstance()->m_tCfg.nSpare)//如果是备控则进行存储过车时间
	{
		if(rcvNum == 24)//beginTime:20190415101056
		{
			char cTime[15];
			memset(cTime,0,15);
			memcpy(cTime,info+10,14);
			trainNumStr = CString(cTime);
		}
	}
	CSocket::OnReceive(nErrorCode);
}

//函数名称:开始工作
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
void Myudp::work()
{
	if(0!=Create(20002,SOCK_DGRAM,NULL))
	{
		gLogFile.AddAppRecord(_T("UDP端口初始化成功,端口号20002"));
		AfxBeginThread(udpSendThread,this);
	}
	else
	{
		gLogFile.AddAppRecord(_T("UDP端口初始化失败,请检查网络"));
	}
}

//函数名称:UDP发送生命信号函数
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
UINT udpSendThread(LPVOID param)
{
	gLogFile.AddAppRecord(_T("UDP通信线程开启"));
	Myudp* pThis = (Myudp*)param;
	std::string lifePacket;
	bool mainControl;//是否是主控标志位
	if(1==GlobalParameter::GetInstance()->m_tCfg.nSpare)
	{
		lifePacket = "main_control:alive";
		mainControl = true;
		gLogFile.AddAppRecord(_T("当前采集软件为主控"));
	}
	else
	{
		lifePacket = std::to_string(_ULonglong(GlobalParameter::GetInstance()->m_tCfg.nSpare))+"_spare_control:alive";
		mainControl = false;
		gLogFile.AddAppRecord(_T("当前采集软件为备控"));
	}
	
	while (!(pThis->stopSend))
	{
		pThis->SendTo(lifePacket.c_str(),lifePacket.size(),Control_UDP_Port,Control_IP);//发送生命报文
		if(mainControl)//如果是主控采集软件,则发送校准报文给备控
		{
			if (trainTimeStr != _T(""))//如果过车时间不为空才发送
			{
				USES_CONVERSION;
				char* pInfo = W2A(trainTimeStr);
				for(int i=0;i<pThis->spareCount;i++)
				{
					pThis->SendTo(pInfo,CStringA(trainTimeStr).GetLength(),Spare_Gather_UDP_PORT,pThis->spareGatherIPS[i]);
				}
			}
		}
		Sleep(1000);
	}
	return 0;
}
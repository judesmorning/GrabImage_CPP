// ManagerThread.cpp : implementation file
//

#include "stdafx.h"
#include "ImageGrab.h"
#include "ManagerThread.h"
#include "MainFrm.h"

/////////////////////////////////////////////////////////////////////////////

BOOL bExit = FALSE;
void InitSocket();
UINT NewSocket(LPVOID param);
UINT ListenSocketForStartGrab(LPVOID param);

IMPLEMENT_DYNCREATE(CManagerThread, CWinThread)

BEGIN_MESSAGE_MAP(CManagerThread, CWinThread)
	ON_THREAD_MESSAGE(ID_PREPAREGRAB, OnPrepareGrab)
	ON_THREAD_MESSAGE(ID_STOPGRAB, OnStopGrab)
	ON_THREAD_MESSAGE(ID_STARTGRAB, OnStartGrab)
END_MESSAGE_MAP()

CManagerThread::CManagerThread()
{
	m_pListenThread = NULL;
}

CManagerThread::~CManagerThread()
{
}

//函数名称:初始化
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
BOOL CManagerThread::InitInstance()
{
	gCamMgr.Init();
	gLogFile.AddAppRecord(_T("完成gCamMgr初始化"));
	gDataMgr.Init();
	gLogFile.AddAppRecord(_T("完成gDataMgr初始化"));
	InitSocket();
	gLogFile.AddAppRecord(_T("初始化网络套接字,WinSock环境"));
	m_pListenThread = AfxBeginThread(ListenSocketForStartGrab, this);
	GlobalParameter::GetInstance()->m_tTrainInfo.workState = STATE_READY;//fix by zhuxy 1->STATE_READY
	gLogFile.AddAppRecord(_T("修改工作状态:STATE_READY"));
	return TRUE;
}

//函数名称:退出
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
int CManagerThread::ExitInstance()
{
	bExit = TRUE;
	WSACleanup();

	gDataMgr.Destory();
	gCamMgr.Destory();

	if (m_pListenThread)
	{
		WaitForSingleObject(m_pListenThread->m_hThread, 5000);
	}

	return CWinThread::ExitInstance();
}

//函数名称:准备采集
//函数作用:NULL
//函数参数:wParam：如果=0则针对所有摄像机 否则为操作的相机通道,从1开始
//		  lParam: 如果=0则时自动模式 =1是手动模式
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void CManagerThread::OnPrepareGrab(WPARAM wParam, LPARAM lParam)//0,1
{
	SYSTEMTIME stLocal;
	CString strInfo;

	GetLocalTime(&stLocal);
	
	//add by zhuxy 20190315
	if (lParam == WAY_MANUAL) //手动触发
	{
		theApp.m_strCurrentImgSavePath = GlobalParameter::GetInstance()->m_tCfg.strLocalImgSavePath.c_str();//  192.168.1.100//GrabImg//
		CTime curTime = CTime::GetCurrentTime();
		CString strTime = curTime.Format(_T("%Y%m"));
		theApp.m_strCurrentImgSavePath += strTime;// D://GrabImg//201903	
		strTime = curTime.Format(_T("%Y%m%d%H%M%S"));
		theApp.m_strTrainId = strTime;
		workWay = WAY_MANUAL;//add by zhuxy 20190321 设置标志位为手动模式
		gLogFile.AddAppRecord(_T("初始化数据,修改工作模式:WAY_MANUAL"));
	}
	else//自动触发
	{
		theApp.m_strCurrentImgSavePath = GlobalParameter::GetInstance()->m_tCfg.strImgSavePath.c_str();//  D://GrabImg//
		CTime curTime = CTime::GetCurrentTime();
		CString strTime = curTime.Format(_T("%Y%m")); 
		theApp.m_strCurrentImgSavePath += strTime;// D://GrabImg//201903	
		strTime = curTime.Format(_T("%Y%m%d%H%M%S"));
		theApp.m_strTrainId = strTime;
		workWay = WAY_AUTO;
		gLogFile.AddAppRecord(_T("初始化数据,修改工作模式:WAY_AUTO"));
	}
#if OPEN_IF
	this->makeDirForAllImageChanel(wParam);//add by zhuxy 20190315 使用新的创建文件夹方法
	gLogFile.AddAppRecord(_T("创建指定通道图片存储文件夹"));
#endif

	gCamMgr.PrepareGrab(wParam);//复位图片数量计数
	gLogFile.AddAppRecord(_T("复位图片计数"));
	gDataMgr.StartDataMgr(wParam);//处理全局buff对象的数据【通过多个线程】
	gLogFile.AddAppRecord(_T("数据管理线程启动"));
	GetLocalTime(&stLocal);
	strInfo.Format(_T("完成数据初始化"));
	gLogFile.AddAppRecord(strInfo);
}

//函数名称:停止采集
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void CManagerThread::OnStopGrab(WPARAM wParam, LPARAM lParam) 
{
	if (0!=wParam)
	{
		gCamMgr.StopGrab(wParam-1);
		gLogFile.AddAppRecord(_T("停止指定通道采集"));
	}
	else
	{
		gCamMgr.StopGrab();
		gLogFile.AddAppRecord(_T("停止采集"));
	}
#if CLOSE_IF
	for (int i=0;i<gCamMgr.m_vCamCtrl.size();i++)//add by zhuxy20190402如果有至少一台相机在工作都不停止落盘线程
	{
		if (gCamMgr.m_vCamCtrl[i]->m_nCamStatus==CAM_STATUS_RUNNING)
		{
			return;
		}
	}
#endif
	gDataMgr.StopDataMgr(wParam);
	gLogFile.AddAppRecord(_T("数据管理线程停止"));
}

//函数名称:开始采集
//函数作用:NULL
//函数参数:wParam:某路摄像机 lPraam:手动还是自动
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void CManagerThread::OnStartGrab(WPARAM wParam, LPARAM lParam) //0,1
{
	int nCtlMsgNumber = GlobalParameter::GetInstance()->m_tCfg.nCtlMsgNumber;
	CString strTemp = _T("");
	if (lParam == WAY_MANUAL)//手动
	{
		gLogFile.AddAppRecord(_T("手动采集,准备数据"));
		OnPrepareGrab(wParam, WAY_MANUAL);
	}
	else//自动情况下应全部开始采集
	{
		gLogFile.AddAppRecord(_T("自动采集,准备数据"));
		OnPrepareGrab(0,WAY_AUTO);
	}
	gCamMgr.StartGrab(wParam);
}

//函数名称:初始化网络套接字
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void InitSocket()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );

	err = WSAStartup( wVersionRequested, &wsaData );
	if(err != 0)
	{
		printf("Could not find a usable WinSock DLL\n");
		return;
	}

	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */

	if ( LOBYTE( wsaData.wVersion ) != 2 ||
		HIBYTE( wsaData.wVersion ) != 2 ) {
			WSACleanup( );
			printf("Could not find a usable WinSock DLL\n");
			return; 
	}
}

//函数名称:监听服务器
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
UINT ListenSocketForStartGrab(LPVOID param)
{
	try 
	{
		WSAEVENT EventArray[1];
		WSANETWORKEVENTS NetworkEvents;

		SOCKET RecvSocket = WSASocket(AF_INET, 
			SOCK_STREAM, 
			IPPROTO_TCP, 
			NULL, 
			0, 
			WSA_FLAG_OVERLAPPED);
		if (INVALID_SOCKET == RecvSocket)
		{
			return 0;
		}
		sockaddr_in service;
		service.sin_family = AF_INET;
		service.sin_addr.s_addr = ADDR_ANY;
		service.sin_port = htons(GlobalParameter::GetInstance()->m_tCfg.nCtrlPort);		

		CString strLog;
		strLog.Format(_T("服务器端口号%d,准备初始化TCP服务器"), GlobalParameter::GetInstance()->m_tCfg.nCtrlPort);
		gLogFile.AddAppRecord(strLog);
		//----------------------
		// Bind the socket.
		if (bind( RecvSocket, (SOCKADDR*) &service, sizeof(service)) == SOCKET_ERROR) 
		{
			//gLogFile.AddRecord(1,"警告：绑定端口失败！");
			closesocket(RecvSocket);

			AfxMessageBox(_T("绑定端口失败,请重新启动程序或者改变网络端口号"));
			AfxGetMainWnd()->PostMessage(WM_CLOSE);
			gLogFile.AddAppRecord(_T("端口绑定失败"));
			return 0;
		}
		gLogFile.AddAppRecord(_T("端口绑定成功"));
		//gLogFile.AddRecord(3,"绑定通讯端口成功");

		//-------------------------
		// Create new event
		EventArray[0] = WSACreateEvent();

		//-------------------------
		// Associate event types FD_ACCEPT and FD_CLOSE
		// with the listening socket and NewEvent
		WSAEventSelect( RecvSocket, EventArray[0], FD_ACCEPT);

		if (listen( RecvSocket,5 ) == SOCKET_ERROR)
		{
			CString strLog;
			strLog.Format(_T("监听失败,错误码:%d"), GetLastError());
			AfxMessageBox(strLog);
			AfxGetMainWnd()->PostMessage(WM_CLOSE);
			gLogFile.AddAppRecord(_T("网络异常,tcp端口监听失败"));
			return 0;
		}
		gLogFile.AddAppRecord(_T("端口监听成功"));
		SOCKET AcceptSocket;
		//----------------------
		// Accept the connection.
		while(bExit == FALSE) 
		{
			if (WSAWaitForMultipleEvents(1,EventArray,TRUE,5000,FALSE)==WAIT_TIMEOUT)
			{
				//printf("timeout\n");
				//gLogFile.AddAppRecord(_T("超时"));//delete by zhuxy 20190320	如果没有客户端连接服务器则什么也不做继续等待
			}
			else
			{
				WSAEnumNetworkEvents(RecvSocket,EventArray[0],&NetworkEvents);
				WSAResetEvent(EventArray[0]);
				if (NetworkEvents.lNetworkEvents&FD_ACCEPT)
				{
					gLogFile.AddAppRecord(_T("有新的客户端连接服务器"));
					AcceptSocket = accept( RecvSocket, NULL, NULL );
					AfxBeginThread(NewSocket,(LPVOID)AcceptSocket);
				}
			}
		} 

		closesocket(RecvSocket);
		gLogFile.AddAppRecord(_T("关闭tcp连接"));
	}
	catch (...) 
	{
		gLogFile.AddAppRecord(_T("ListenSocketForStartGrab函数异常")); 
	}

	return 0;
}

//函数名称:接收数据回调函数
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
bool CManagerThread::CtlMsgHandleTwo(SOCKET socket)//运达应该只使用这个函数来解析
{
	//add by zhuxy 20190322 根据协议修改正确的接收消息代码
	char rcvBuf[RCV_BUF_SIZE];
	memset(rcvBuf,0,RCV_BUF_SIZE);
	int rcvSize = recv(socket,rcvBuf,RCV_BUF_SIZE,0);
	CString logStr;
	logStr.Format(_T("服务器接收到%d个字节,数据:%s"),rcvSize,rcvBuf);
	gLogFile.AddAppRecord(logStr);
	if(rcvSize == TRAIN_INFO_SIZE)//过车信息包
	{
		gLogFile.AddAppRecord(_T("接收到过车信息包"));
		CString trainInfoStr(rcvBuf);
		if (0==trainInfoStr.Find(_T("21")))//如果是21号线
		{
			
			CString numStr = trainInfoStr.Mid(2,3);
			int num = _wtoi(numStr);
			if (0 == (num%2))
			{
				trainNumStr.Format(_T("%s%.3d%.3d%s"),_T("21B08"),num-1,num,trainInfoStr.Mid(5,2));
			}
			else
			{
				trainNumStr.Format(_T("%s%.3d%.3d%s"),_T("21B08"),num,num+1,trainInfoStr.Mid(5,2));
			}
			logStr.Format(_T("21号线车,车号:%s"),trainNumStr);
			gLogFile.AddAppRecord(logStr);
		}
		else if(0==trainInfoStr.Find(_T("14")))//14号线
		{
			CString numStr = trainInfoStr.Mid(2,3);
			int num = _wtoi(numStr);
			if (0 == (num%2))
			{
				trainNumStr.Format(_T("%s%.3d%.3d%s"),_T("14B07"),num-1,num,trainInfoStr.Mid(5,2));
			}
			else
			{
				trainNumStr.Format(_T("%s%.3d%.3d%s"),_T("14B07"),num,num+1,trainInfoStr.Mid(5,2));
			}
			logStr.Format(_T("14号线车,车号:%s"),trainNumStr);
			gLogFile.AddAppRecord(logStr);
		}
		else//未知线路号
		{
			trainNumStr = trainInfoStr;
			gLogFile.AddAppRecord(_T("未知线路号")+trainInfoStr);
		}
	}
	else if (rcvSize == 9)//控制包 in_out: 1
	{
		HRBMSG msg;
		memset(&msg,0,sizeof(msg));
		CString rcvStr(rcvBuf);
		msg.m_nDelay = _wtoi(rcvStr.Right(1));
		CString strTmp = _T("");
		strTmp.Format(_T("收到过车控制数据,过车状态:%d"),msg.m_nDelay);
		gLogFile.AddAppRecord(strTmp);

		if (msg.m_nDelay == 1)//接收到开始采集指令
		{	
			if(WAY_STOP==workWay)//如果当前没有进行采集动作则开始采集
			{
				gLogFile.AddAppRecord(_T("收到来车指令,初始化数据"));
				GlobalParameter::GetInstance()->m_tTrainInfo.workState = STATE_BUSY;
				gLogFile.AddAppRecord(_T("修改工作状态:STATE_BUSY"));
				::PostThreadMessage(theApp.m_pMgrThread->m_nThreadID, ID_STARTGRAB, 0, 0);
			}
			else if(WAY_MANUAL==workWay)//如果当前正在进行手动采集则停止再自动采集
			{
				gLogFile.AddAppRecord(_T("收到来车指令,停止手动采集"));
				::PostThreadMessage(theApp.m_pMgrThread->m_nThreadID, ID_STOPGRAB, 0, 0);
				while (workWay==WAY_STOP)
				{
					Sleep(500);
				}
				Sleep(5000);//等待压缩线程结束，否则计数会出错
				gCamMgr.PrepareGrab();//计数清零
				
				GlobalParameter::GetInstance()->m_tTrainInfo.workState = STATE_BUSY;
				::PostThreadMessage(theApp.m_pMgrThread->m_nThreadID, ID_STARTGRAB, 0, 0);
			}
			else if(WAY_AUTO==workWay)
			{
				gLogFile.AddAppRecord(_T("网络数据异常,上次过车尚未完成"));
			}
		}
		else if(msg.m_nDelay == 2)//收到停止采集指令
		{
			gLogFile.AddAppRecord(_T("收到过车结束信号,关闭相机"));
			GlobalParameter::GetInstance()->m_tTrainInfo.workState = STATE_UNKNOWN;
			::PostThreadMessage(theApp.m_pMgrThread->m_nThreadID, ID_STOPGRAB, 0, 0);
		}
		else
		{
			gLogFile.AddAppRecord(_T("网络收到错误的控制信息"));
		}
	}
	else if(rcvSize == 14)//激光状态包 light_state: 1
	{
		CString rcvStr(rcvBuf);
		if(1 == _wtoi(rcvStr.Right(1)))
		{
			lightStateStr = _T("开");
			gLogFile.AddAppRecord(_T("激光状态:开"));
		}
		else if(2 == _wtoi(rcvStr.Right(1)))
		{
			lightStateStr = _T("关");
			gLogFile.AddAppRecord(_T("激光状态:关"));
		}
		else
		{
			lightStateStr = _T("未知");
			gLogFile.AddAppRecord(_T("激光状态:未知"));
		}
	}
	else
	{
		gLogFile.AddAppRecord(_T("网络收到意料之外的指令"));
		return false;
	}
	return true;
}

//函数名称:为tcp客户端建立连接
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
UINT NewSocket(LPVOID param)
{
	try{
		gLogFile.AddAppRecord(_T("正在为tcp客户端分配空间"));
		
		SOCKET socket = (SOCKET)param;
		WSAEVENT EventArray[1];
		char nAck = 'p';
		int nRecvSize = 0;

		WSANETWORKEVENTS NetworkEvents;
		EventArray[0] = WSACreateEvent();
		if (WSAEventSelect( socket, EventArray[0], FD_READ)!=0)
		{
			//printf("Event Select failed\n");
		}
		gLogFile.AddAppRecord(_T("tcp客户端分配空间成功"));
		while (1)
		{
			if (WSAWaitForMultipleEvents(1,EventArray,TRUE,10000,FALSE)==WAIT_TIMEOUT)
			{
				WSACloseEvent(EventArray[0]);
				closesocket(socket);
				gLogFile.AddAppRecord(_T("tcp连接关闭"));
				break;
			}

			WSAEnumNetworkEvents(socket,EventArray[0],&NetworkEvents);
			WSAResetEvent(EventArray[0]);
			if (NetworkEvents.lNetworkEvents&FD_READ)
			{
				if(1 == GlobalParameter::GetInstance()->m_tCfg.nCtlMsgNumber)
				{
					theApp.m_pMgrThread->CtlMsgHandleTwo(socket);
				}
				else
				{
					gLogFile.AddAppRecord(_T("错误的tcp类型"));
				}
				send(socket,&nAck,1,0);gLogFile.AddAppRecord(_T("向tcp客户端发送响应"));		
				closesocket(socket);
				WSACloseEvent(EventArray[0]);
				Sleep(5000);
				gLogFile.AddAppRecord(_T("tcp连接关闭"));
				break;
			}
		}
	}
	catch(...)
	{
		gLogFile.AddAppRecord(_T("NewSocket函数异常"));
	}

	return 0;
}

//函数名称:根据需求新建文件夹
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
BOOL CManagerThread::makeDirForAllImageChanel(int camCount)
{	
	BOOL ret = true;
	int carNum = GlobalParameter::GetInstance()->m_tCfg.nCamNum;//摄像头个数
	ret = CreateDirectory(theApp.m_strCurrentImgSavePath, NULL);//D:\\GrabImg\\201901
	//RET_VALUE_IF_NOT_EAQU(ret,true,false);

	CTime curTime = CTime::GetCurrentTime();
	CString strTimeCarNum = curTime.Format(_T("\\\\%Y%m%d%H%M%S"));//过车时间
	if(1==GlobalParameter::GetInstance()->m_tCfg.nSpare)//如果是主控,记录过车时间,用于备控采集软件校准
	{
		trainTimeStr = curTime.Format(_T("beginTime:%Y%m%d%H%M%S"));
	}
	CString tempCarNumStr;
	tempCarNumStr.Format(_T("_%s"),theApp.m_strTrainId);
	strTimeCarNum += tempCarNumStr;

	ret = CreateDirectory(theApp.m_strCurrentImgSavePath+strTimeCarNum, NULL);//D:\\GrabImg\\201901\\2019031859_和谐号
	if(0!=camCount)
	{
		carNum = 1;
	}
	for(int i=0;i<carNum;i++)
	{
		CString tmpStr;
		if(0==camCount)
		{
			tmpStr.Format(_T("\\Camera_0%d"),i);
		}
		else
		{
			tmpStr.Format(_T("\\Camera_0%d"),camCount-1);
		}
		ret = CreateDirectory(theApp.m_strCurrentImgSavePath+strTimeCarNum+tmpStr, NULL);
		switch (GlobalParameter::GetInstance()->m_tCfg.nSaveImageFlag)
		{
		case 1:
			{
				ret = CreateDirectory(theApp.m_strCurrentImgSavePath+strTimeCarNum+tmpStr+"\\jpg", NULL);
				break;
			}
		case 2:
			{
				ret = CreateDirectory(theApp.m_strCurrentImgSavePath+strTimeCarNum+tmpStr+"\\bmp", NULL);
				break;
			}
		case 3:
			{
				ret = CreateDirectory(theApp.m_strCurrentImgSavePath+strTimeCarNum+tmpStr+"\\jpg", NULL);
				ret = CreateDirectory(theApp.m_strCurrentImgSavePath+strTimeCarNum+tmpStr+"\\bmp", NULL);
				break;
			}
		default:
			ret = false;
			break;
		}
	}
	if(!ret)TRACE("err in %s at %d\n",__FILE__,__LINE__);
	theApp.m_strCurrentImgSavePath += strTimeCarNum; //D:\\GrabImg\\201903\\20190318180034_20190318180034
	if (camCount!=0)//如果是单独开启某路采集则保存当前路径
	{
		GlobalParameter::GetInstance()->imgPaths[camCount-1]=theApp.m_strCurrentImgSavePath;
	}
	return ret;
}


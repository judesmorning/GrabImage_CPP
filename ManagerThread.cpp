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

//��������:��ʼ��
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
BOOL CManagerThread::InitInstance()
{
	gCamMgr.Init();
	gLogFile.AddAppRecord(_T("���gCamMgr��ʼ��"));
	gDataMgr.Init();
	gLogFile.AddAppRecord(_T("���gDataMgr��ʼ��"));
	InitSocket();
	gLogFile.AddAppRecord(_T("��ʼ�������׽���,WinSock����"));
	m_pListenThread = AfxBeginThread(ListenSocketForStartGrab, this);
	GlobalParameter::GetInstance()->m_tTrainInfo.workState = STATE_READY;//fix by zhuxy 1->STATE_READY
	gLogFile.AddAppRecord(_T("�޸Ĺ���״̬:STATE_READY"));
	return TRUE;
}

//��������:�˳�
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
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

//��������:׼���ɼ�
//��������:NULL
//��������:wParam�����=0�������������� ����Ϊ���������ͨ��,��1��ʼ
//		  lParam: ���=0��ʱ�Զ�ģʽ =1���ֶ�ģʽ
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void CManagerThread::OnPrepareGrab(WPARAM wParam, LPARAM lParam)//0,1
{
	SYSTEMTIME stLocal;
	CString strInfo;

	GetLocalTime(&stLocal);
	
	//add by zhuxy 20190315
	if (lParam == WAY_MANUAL) //�ֶ�����
	{
		theApp.m_strCurrentImgSavePath = GlobalParameter::GetInstance()->m_tCfg.strLocalImgSavePath.c_str();//  192.168.1.100//GrabImg//
		CTime curTime = CTime::GetCurrentTime();
		CString strTime = curTime.Format(_T("%Y%m"));
		theApp.m_strCurrentImgSavePath += strTime;// D://GrabImg//201903	
		strTime = curTime.Format(_T("%Y%m%d%H%M%S"));
		theApp.m_strTrainId = strTime;
		workWay = WAY_MANUAL;//add by zhuxy 20190321 ���ñ�־λΪ�ֶ�ģʽ
		gLogFile.AddAppRecord(_T("��ʼ������,�޸Ĺ���ģʽ:WAY_MANUAL"));
	}
	else//�Զ�����
	{
		theApp.m_strCurrentImgSavePath = GlobalParameter::GetInstance()->m_tCfg.strImgSavePath.c_str();//  D://GrabImg//
		CTime curTime = CTime::GetCurrentTime();
		CString strTime = curTime.Format(_T("%Y%m")); 
		theApp.m_strCurrentImgSavePath += strTime;// D://GrabImg//201903	
		strTime = curTime.Format(_T("%Y%m%d%H%M%S"));
		theApp.m_strTrainId = strTime;
		workWay = WAY_AUTO;
		gLogFile.AddAppRecord(_T("��ʼ������,�޸Ĺ���ģʽ:WAY_AUTO"));
	}
#if OPEN_IF
	this->makeDirForAllImageChanel(wParam);//add by zhuxy 20190315 ʹ���µĴ����ļ��з���
	gLogFile.AddAppRecord(_T("����ָ��ͨ��ͼƬ�洢�ļ���"));
#endif

	gCamMgr.PrepareGrab(wParam);//��λͼƬ��������
	gLogFile.AddAppRecord(_T("��λͼƬ����"));
	gDataMgr.StartDataMgr(wParam);//����ȫ��buff��������ݡ�ͨ������̡߳�
	gLogFile.AddAppRecord(_T("���ݹ����߳�����"));
	GetLocalTime(&stLocal);
	strInfo.Format(_T("������ݳ�ʼ��"));
	gLogFile.AddAppRecord(strInfo);
}

//��������:ֹͣ�ɼ�
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void CManagerThread::OnStopGrab(WPARAM wParam, LPARAM lParam) 
{
	if (0!=wParam)
	{
		gCamMgr.StopGrab(wParam-1);
		gLogFile.AddAppRecord(_T("ָֹͣ��ͨ���ɼ�"));
	}
	else
	{
		gCamMgr.StopGrab();
		gLogFile.AddAppRecord(_T("ֹͣ�ɼ�"));
	}
#if CLOSE_IF
	for (int i=0;i<gCamMgr.m_vCamCtrl.size();i++)//add by zhuxy20190402���������һ̨����ڹ�������ֹͣ�����߳�
	{
		if (gCamMgr.m_vCamCtrl[i]->m_nCamStatus==CAM_STATUS_RUNNING)
		{
			return;
		}
	}
#endif
	gDataMgr.StopDataMgr(wParam);
	gLogFile.AddAppRecord(_T("���ݹ����߳�ֹͣ"));
}

//��������:��ʼ�ɼ�
//��������:NULL
//��������:wParam:ĳ·����� lPraam:�ֶ������Զ�
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void CManagerThread::OnStartGrab(WPARAM wParam, LPARAM lParam) //0,1
{
	int nCtlMsgNumber = GlobalParameter::GetInstance()->m_tCfg.nCtlMsgNumber;
	CString strTemp = _T("");
	if (lParam == WAY_MANUAL)//�ֶ�
	{
		gLogFile.AddAppRecord(_T("�ֶ��ɼ�,׼������"));
		OnPrepareGrab(wParam, WAY_MANUAL);
	}
	else//�Զ������Ӧȫ����ʼ�ɼ�
	{
		gLogFile.AddAppRecord(_T("�Զ��ɼ�,׼������"));
		OnPrepareGrab(0,WAY_AUTO);
	}
	gCamMgr.StartGrab(wParam);
}

//��������:��ʼ�������׽���
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
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

//��������:����������
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
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
		strLog.Format(_T("�������˿ں�%d,׼����ʼ��TCP������"), GlobalParameter::GetInstance()->m_tCfg.nCtrlPort);
		gLogFile.AddAppRecord(strLog);
		//----------------------
		// Bind the socket.
		if (bind( RecvSocket, (SOCKADDR*) &service, sizeof(service)) == SOCKET_ERROR) 
		{
			//gLogFile.AddRecord(1,"���棺�󶨶˿�ʧ�ܣ�");
			closesocket(RecvSocket);

			AfxMessageBox(_T("�󶨶˿�ʧ��,����������������߸ı�����˿ں�"));
			AfxGetMainWnd()->PostMessage(WM_CLOSE);
			gLogFile.AddAppRecord(_T("�˿ڰ�ʧ��"));
			return 0;
		}
		gLogFile.AddAppRecord(_T("�˿ڰ󶨳ɹ�"));
		//gLogFile.AddRecord(3,"��ͨѶ�˿ڳɹ�");

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
			strLog.Format(_T("����ʧ��,������:%d"), GetLastError());
			AfxMessageBox(strLog);
			AfxGetMainWnd()->PostMessage(WM_CLOSE);
			gLogFile.AddAppRecord(_T("�����쳣,tcp�˿ڼ���ʧ��"));
			return 0;
		}
		gLogFile.AddAppRecord(_T("�˿ڼ����ɹ�"));
		SOCKET AcceptSocket;
		//----------------------
		// Accept the connection.
		while(bExit == FALSE) 
		{
			if (WSAWaitForMultipleEvents(1,EventArray,TRUE,5000,FALSE)==WAIT_TIMEOUT)
			{
				//printf("timeout\n");
				//gLogFile.AddAppRecord(_T("��ʱ"));//delete by zhuxy 20190320	���û�пͻ������ӷ�������ʲôҲ���������ȴ�
			}
			else
			{
				WSAEnumNetworkEvents(RecvSocket,EventArray[0],&NetworkEvents);
				WSAResetEvent(EventArray[0]);
				if (NetworkEvents.lNetworkEvents&FD_ACCEPT)
				{
					gLogFile.AddAppRecord(_T("���µĿͻ������ӷ�����"));
					AcceptSocket = accept( RecvSocket, NULL, NULL );
					AfxBeginThread(NewSocket,(LPVOID)AcceptSocket);
				}
			}
		} 

		closesocket(RecvSocket);
		gLogFile.AddAppRecord(_T("�ر�tcp����"));
	}
	catch (...) 
	{
		gLogFile.AddAppRecord(_T("ListenSocketForStartGrab�����쳣")); 
	}

	return 0;
}

//��������:�������ݻص�����
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
bool CManagerThread::CtlMsgHandleTwo(SOCKET socket)//�˴�Ӧ��ֻʹ���������������
{
	//add by zhuxy 20190322 ����Э���޸���ȷ�Ľ�����Ϣ����
	char rcvBuf[RCV_BUF_SIZE];
	memset(rcvBuf,0,RCV_BUF_SIZE);
	int rcvSize = recv(socket,rcvBuf,RCV_BUF_SIZE,0);
	CString logStr;
	logStr.Format(_T("���������յ�%d���ֽ�,����:%s"),rcvSize,rcvBuf);
	gLogFile.AddAppRecord(logStr);
	if(rcvSize == TRAIN_INFO_SIZE)//������Ϣ��
	{
		gLogFile.AddAppRecord(_T("���յ�������Ϣ��"));
		CString trainInfoStr(rcvBuf);
		if (0==trainInfoStr.Find(_T("21")))//�����21����
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
			logStr.Format(_T("21���߳�,����:%s"),trainNumStr);
			gLogFile.AddAppRecord(logStr);
		}
		else if(0==trainInfoStr.Find(_T("14")))//14����
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
			logStr.Format(_T("14���߳�,����:%s"),trainNumStr);
			gLogFile.AddAppRecord(logStr);
		}
		else//δ֪��·��
		{
			trainNumStr = trainInfoStr;
			gLogFile.AddAppRecord(_T("δ֪��·��")+trainInfoStr);
		}
	}
	else if (rcvSize == 9)//���ư� in_out: 1
	{
		HRBMSG msg;
		memset(&msg,0,sizeof(msg));
		CString rcvStr(rcvBuf);
		msg.m_nDelay = _wtoi(rcvStr.Right(1));
		CString strTmp = _T("");
		strTmp.Format(_T("�յ�������������,����״̬:%d"),msg.m_nDelay);
		gLogFile.AddAppRecord(strTmp);

		if (msg.m_nDelay == 1)//���յ���ʼ�ɼ�ָ��
		{	
			if(WAY_STOP==workWay)//�����ǰû�н��вɼ�������ʼ�ɼ�
			{
				gLogFile.AddAppRecord(_T("�յ�����ָ��,��ʼ������"));
				GlobalParameter::GetInstance()->m_tTrainInfo.workState = STATE_BUSY;
				gLogFile.AddAppRecord(_T("�޸Ĺ���״̬:STATE_BUSY"));
				::PostThreadMessage(theApp.m_pMgrThread->m_nThreadID, ID_STARTGRAB, 0, 0);
			}
			else if(WAY_MANUAL==workWay)//�����ǰ���ڽ����ֶ��ɼ���ֹͣ���Զ��ɼ�
			{
				gLogFile.AddAppRecord(_T("�յ�����ָ��,ֹͣ�ֶ��ɼ�"));
				::PostThreadMessage(theApp.m_pMgrThread->m_nThreadID, ID_STOPGRAB, 0, 0);
				while (workWay==WAY_STOP)
				{
					Sleep(500);
				}
				Sleep(5000);//�ȴ�ѹ���߳̽�����������������
				gCamMgr.PrepareGrab();//��������
				
				GlobalParameter::GetInstance()->m_tTrainInfo.workState = STATE_BUSY;
				::PostThreadMessage(theApp.m_pMgrThread->m_nThreadID, ID_STARTGRAB, 0, 0);
			}
			else if(WAY_AUTO==workWay)
			{
				gLogFile.AddAppRecord(_T("���������쳣,�ϴι�����δ���"));
			}
		}
		else if(msg.m_nDelay == 2)//�յ�ֹͣ�ɼ�ָ��
		{
			gLogFile.AddAppRecord(_T("�յ����������ź�,�ر����"));
			GlobalParameter::GetInstance()->m_tTrainInfo.workState = STATE_UNKNOWN;
			::PostThreadMessage(theApp.m_pMgrThread->m_nThreadID, ID_STOPGRAB, 0, 0);
		}
		else
		{
			gLogFile.AddAppRecord(_T("�����յ�����Ŀ�����Ϣ"));
		}
	}
	else if(rcvSize == 14)//����״̬�� light_state: 1
	{
		CString rcvStr(rcvBuf);
		if(1 == _wtoi(rcvStr.Right(1)))
		{
			lightStateStr = _T("��");
			gLogFile.AddAppRecord(_T("����״̬:��"));
		}
		else if(2 == _wtoi(rcvStr.Right(1)))
		{
			lightStateStr = _T("��");
			gLogFile.AddAppRecord(_T("����״̬:��"));
		}
		else
		{
			lightStateStr = _T("δ֪");
			gLogFile.AddAppRecord(_T("����״̬:δ֪"));
		}
	}
	else
	{
		gLogFile.AddAppRecord(_T("�����յ�����֮���ָ��"));
		return false;
	}
	return true;
}

//��������:Ϊtcp�ͻ��˽�������
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
UINT NewSocket(LPVOID param)
{
	try{
		gLogFile.AddAppRecord(_T("����Ϊtcp�ͻ��˷���ռ�"));
		
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
		gLogFile.AddAppRecord(_T("tcp�ͻ��˷���ռ�ɹ�"));
		while (1)
		{
			if (WSAWaitForMultipleEvents(1,EventArray,TRUE,10000,FALSE)==WAIT_TIMEOUT)
			{
				WSACloseEvent(EventArray[0]);
				closesocket(socket);
				gLogFile.AddAppRecord(_T("tcp���ӹر�"));
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
					gLogFile.AddAppRecord(_T("�����tcp����"));
				}
				send(socket,&nAck,1,0);gLogFile.AddAppRecord(_T("��tcp�ͻ��˷�����Ӧ"));		
				closesocket(socket);
				WSACloseEvent(EventArray[0]);
				Sleep(5000);
				gLogFile.AddAppRecord(_T("tcp���ӹر�"));
				break;
			}
		}
	}
	catch(...)
	{
		gLogFile.AddAppRecord(_T("NewSocket�����쳣"));
	}

	return 0;
}

//��������:���������½��ļ���
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
BOOL CManagerThread::makeDirForAllImageChanel(int camCount)
{	
	BOOL ret = true;
	int carNum = GlobalParameter::GetInstance()->m_tCfg.nCamNum;//����ͷ����
	ret = CreateDirectory(theApp.m_strCurrentImgSavePath, NULL);//D:\\GrabImg\\201901
	//RET_VALUE_IF_NOT_EAQU(ret,true,false);

	CTime curTime = CTime::GetCurrentTime();
	CString strTimeCarNum = curTime.Format(_T("\\\\%Y%m%d%H%M%S"));//����ʱ��
	if(1==GlobalParameter::GetInstance()->m_tCfg.nSpare)//���������,��¼����ʱ��,���ڱ��زɼ����У׼
	{
		trainTimeStr = curTime.Format(_T("beginTime:%Y%m%d%H%M%S"));
	}
	CString tempCarNumStr;
	tempCarNumStr.Format(_T("_%s"),theApp.m_strTrainId);
	strTimeCarNum += tempCarNumStr;

	ret = CreateDirectory(theApp.m_strCurrentImgSavePath+strTimeCarNum, NULL);//D:\\GrabImg\\201901\\2019031859_��г��
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
	if (camCount!=0)//����ǵ�������ĳ·�ɼ��򱣴浱ǰ·��
	{
		GlobalParameter::GetInstance()->imgPaths[camCount-1]=theApp.m_strCurrentImgSavePath;
	}
	return ret;
}


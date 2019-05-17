// Myudp.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ImageGrab.h"
#include "Myudp.h"

UINT udpSendThread(LPVOID param);

// Myudp
//��������:���캯��
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
Myudp::Myudp()
{
	AfxSocketInit();
	stopSend=false;
	spareCount=1;
	spareGatherIPS.resize(spareCount);
	spareGatherIPS[0] = _T("127.0.0.1");//���ص�ip����
}

Myudp::~Myudp()
{
	stopSend=true;
}


// Myudp ��Ա����



//��������:��������
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:beginTime:%Y%m%d%H%M%S
void Myudp::OnReceive(int nErrorCode)
{
	// TODO: �ڴ����ר�ô����/����û���
	char info[1024];CString ip;uint port;
	int rcvNum = ReceiveFrom(info,1*1024,ip,port);
	if(1 != GlobalParameter::GetInstance()->m_tCfg.nSpare)//����Ǳ�������д洢����ʱ��
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

//��������:��ʼ����
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
void Myudp::work()
{
	if(0!=Create(20002,SOCK_DGRAM,NULL))
	{
		gLogFile.AddAppRecord(_T("UDP�˿ڳ�ʼ���ɹ�,�˿ں�20002"));
		AfxBeginThread(udpSendThread,this);
	}
	else
	{
		gLogFile.AddAppRecord(_T("UDP�˿ڳ�ʼ��ʧ��,��������"));
	}
}

//��������:UDP���������źź���
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
UINT udpSendThread(LPVOID param)
{
	gLogFile.AddAppRecord(_T("UDPͨ���߳̿���"));
	Myudp* pThis = (Myudp*)param;
	std::string lifePacket;
	bool mainControl;//�Ƿ������ر�־λ
	if(1==GlobalParameter::GetInstance()->m_tCfg.nSpare)
	{
		lifePacket = "main_control:alive";
		mainControl = true;
		gLogFile.AddAppRecord(_T("��ǰ�ɼ����Ϊ����"));
	}
	else
	{
		lifePacket = std::to_string(_ULonglong(GlobalParameter::GetInstance()->m_tCfg.nSpare))+"_spare_control:alive";
		mainControl = false;
		gLogFile.AddAppRecord(_T("��ǰ�ɼ����Ϊ����"));
	}
	
	while (!(pThis->stopSend))
	{
		pThis->SendTo(lifePacket.c_str(),lifePacket.size(),Control_UDP_Port,Control_IP);//������������
		if(mainControl)//��������زɼ����,����У׼���ĸ�����
		{
			if (trainTimeStr != _T(""))//�������ʱ�䲻Ϊ�ղŷ���
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
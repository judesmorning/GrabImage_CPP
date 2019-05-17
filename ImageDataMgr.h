#pragma once

#define BUFFERSIZEFORJPG	5000

#include "ImageBuff.h"
#include "SafeList.h"

class CImageDataMgr
{
public:
	CImageDataMgr(void);
	~CImageDataMgr(void);
public:
	bool isOpenAllCams;
	int camChanel;
	bool stopFlag[4];
public:
	void Init();
	void Destory();
	void StartDataMgr(int camChanel=0);
	void StopDataMgr(int camChanel=0);

	int GetJPGData(BYTE ** pData,int *nIndex,int *nCameraIndex,int *nCameraIndexInConf,int nTimeOut);

public:
	vector<CImageBuffer*> m_vImgBuf;
	
	bool m_bStopGrabFlag;  //ֹͣ�ɼ���ʾ ��Ҫ��Ϊƴ���˳��Ŀ���
	bool m_bStopJointFlag;    //ֹͣƴ�ӱ�ʾ ��Ҫ��������ѹ�������̵߳��˳�
	bool m_bStopCompressMgrFlag;  //ѹ�������ʾ ��Ҫ�������ƴ����̵߳��˳�
	bool m_bStopSendMgrFlag;      //���͹����־  ��Ҫ�������Ʒ�����Դ���ͷ�
	int m_nSendWaitTime;

	HANDLE m_hCompressThreadCountSemaphore;
	int m_nCompressThreadCount;

	HANDLE m_hJointThreadCountSemaphore;
	int m_nJointThreadCount;

	HANDLE m_hSendThreadCountSemaphore;	
	int m_nSendThreadCount;

	HANDLE m_hCompressCountSemaphore;
	HANDLE m_hUploadCountSemaphore;

	Enhance_Distribute_Moudle *pModule[CAMNUM];

public:
	//// ѹ�����������Ϣ	
	//BYTE *m_pJPGBuffer[BUFFERSIZEFORJPG];
	//int m_nSizeJPGBuffer[BUFFERSIZEFORJPG];
	//int m_nIndexJPG[BUFFERSIZEFORJPG];
	//int m_nCameraIndexJPG[BUFFERSIZEFORJPG];
	//int m_nCameraIndexInConf[BUFFERSIZEFORJPG];
	
	//deque<pUploadParam> m_dUploadParam;
	vector<pUploadParam> m_dUploadParam;

	int m_nFirstJPG;
	int m_nLastJPG;

	HANDLE m_hEventJPGReady;	
	HANDLE m_hSemaphoreJPGBufferUsed; 
	HANDLE m_hSemaphoreJPGBufferInUsed; 

public:
	train::CSafeList<JointImgInfo*> m_listJointInfo;
};


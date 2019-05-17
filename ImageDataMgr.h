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
	
	bool m_bStopGrabFlag;  //停止采集标示 主要作为拼接退出的开关
	bool m_bStopJointFlag;    //停止拼接标示 主要用来控制压缩管理线程的退出
	bool m_bStopCompressMgrFlag;  //压缩管理标示 主要用来控制传输线程的退出
	bool m_bStopSendMgrFlag;      //发送管理标志  主要用来控制发送资源的释放
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
	//// 压缩后的数据信息	
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


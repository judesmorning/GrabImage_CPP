#include "StdAfx.h"
#include "ImageGrab.h"
#include "ImageDataMgr.h"


UINT RemoveOverdueFile(LPVOID pParam);  //�Ƴ�����ͼ��
void DataJointThread(LPVOID param);   //ͼ��ƴ���߳�
void DataCompressMgrThread(void * param);   //����ѹ�������߳�
void DataCompressThread(void* param);    //����ѹ���߳�
void SendImageThread(LPVOID param);     //ͼ�����߳�
void SendImageMgrThread(LPVOID param);   //ͼ���͹����߳�

//��������:���캯��
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
CImageDataMgr::CImageDataMgr(void)
{
	m_bStopGrabFlag = 0;
	m_nCompressThreadCount = 0;
	m_nSendThreadCount = 0;
	m_nJointThreadCount = 0;
	m_nFirstJPG = 0;
	m_nLastJPG = 0;
	m_hEventJPGReady = NULL;
}


CImageDataMgr::~CImageDataMgr(void)
{

}

//��������:��ʼ��
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void CImageDataMgr::Init()
{
	int nCamNum = GlobalParameter::GetInstance()->m_tCfg.nCamNum;
	
	for (int i=0; i<nCamNum; i++)
	{
		CameraCfg camCfg = GlobalParameter::GetInstance()->m_vCamCfg[i];

		CImageBuffer *pImgBuf = new CImageBuffer;
		pImgBuf->InitBuffer(camCfg.nBufferNum, camCfg.nWidth * camCfg.nHeight, i);
		m_vImgBuf.push_back(pImgBuf);

		Enhance_Distribute_Params param;
		param.clah_wblock_num = camCfg.tEnhanceParam.tClaneParam.clah_wblock_num;
		param.clah_hblock_num = camCfg.tEnhanceParam.tClaneParam.clah_hblock_num;
		param.clip_limit = camCfg.tEnhanceParam.tClaneParam.clip_limit;

		pModule[i] = initialize_enhance_distri();
		configure_enhance_distri(pModule[i], param);
	}

	m_hEventJPGReady = CreateSemaphore(NULL,0,10000,NULL);
	m_hSemaphoreJPGBufferUsed = CreateSemaphore(NULL,1,1,NULL);
	m_hSemaphoreJPGBufferInUsed = CreateSemaphore(NULL,1,1,NULL);

    m_hCompressThreadCountSemaphore = CreateSemaphore(NULL,1,1,NULL);
	m_hSendThreadCountSemaphore = CreateSemaphore(NULL,1,1,NULL);
	m_hCompressCountSemaphore = CreateSemaphore(NULL,1,1,NULL);
	m_hUploadCountSemaphore = CreateSemaphore(NULL,1,1,NULL);
	m_hJointThreadCountSemaphore = CreateSemaphore(NULL,1,1,NULL);

	m_dUploadParam.clear();

#if OPEN_IF//�ѱ�־λ��ʼ����start����ĵ�����
	m_bStopGrabFlag = false;//δʹ��
	m_bStopJointFlag = false;//δʹ��
	m_bStopCompressMgrFlag = false;  //δʹ��
	m_bStopSendMgrFlag = false;

	m_nCompressThreadCount = 0; 
	m_nJointThreadCount = 0;
	m_nSendThreadCount = 0;
	m_nSendWaitTime = 0;

	long lCount;
	ReleaseSemaphore(m_hCompressThreadCountSemaphore, 1, &lCount);
	ReleaseSemaphore(m_hJointThreadCountSemaphore, 1, &lCount);
	ReleaseSemaphore(m_hSendThreadCountSemaphore, 1, &lCount);
	ReleaseSemaphore(m_hCompressCountSemaphore, 1, &lCount);
	ReleaseSemaphore(m_hUploadCountSemaphore, 1, &lCount);
#endif
}

//��������:�ڴ����
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void CImageDataMgr::Destory()
{
	vector<CImageBuffer*>::iterator iter = m_vImgBuf.begin();
	while (iter != m_vImgBuf.end())
	{
		CImageBuffer *pImgBuf = *iter;
		pImgBuf->ClearBuffer();
		delete pImgBuf;
		pImgBuf = NULL;

		iter = m_vImgBuf.erase(iter);
	}

	CloseHandle(m_hEventJPGReady);
	CloseHandle(m_hSemaphoreJPGBufferUsed);
	CloseHandle(m_hSemaphoreJPGBufferInUsed);
	CloseHandle(m_hJointThreadCountSemaphore);
	CloseHandle(m_hCompressThreadCountSemaphore);
    CloseHandle(m_hSendThreadCountSemaphore);
	CloseHandle(m_hCompressCountSemaphore);
	CloseHandle(m_hUploadCountSemaphore);

	int nCamNum = GlobalParameter::GetInstance()->m_vCamCfg.size();
	for (int i=0; i<nCamNum; i++)
	{
		terminate_enhance_distri(pModule[i]);
	}
}


/*
������StartDataMgr()
�������ܣ�������ͼ��������ص�ѹ���������̣߳�
�������壺
NULL
����ֵ��NULL
*/
void CImageDataMgr::StartDataMgr(int camChanel/* =0 */)
{
#if CLOSE_IF//����֧��ͬʱ�ɼ��Ĵ���
	m_bStopGrabFlag = false;
	m_bStopJointFlag = false;
	m_bStopCompressMgrFlag = false;  
	m_bStopSendMgrFlag = false;

	m_nCompressThreadCount = 0; 
	m_nJointThreadCount = 0;
	m_nSendThreadCount = 0;
	m_nSendWaitTime = 0;
	//m_nJointThreadCount = 0;

	long lCount;
	ReleaseSemaphore(m_hCompressThreadCountSemaphore, 1, &lCount);
	ReleaseSemaphore(m_hJointThreadCountSemaphore, 1, &lCount);
	ReleaseSemaphore(m_hSendThreadCountSemaphore, 1, &lCount);
	ReleaseSemaphore(m_hCompressCountSemaphore, 1, &lCount);
	ReleaseSemaphore(m_hUploadCountSemaphore, 1, &lCount);

	int nCamNum = GlobalParameter::GetInstance()->m_tCfg.nCamNum;
	for (int i=0; i<nCamNum; i++)
	{
		//��ʼ����ƴ��ѹ��
		_beginthread(DataJointThread, 0, this);
	}
	if (nCamNum > 0)
	{
		//��ʼ����ѹ�������߳�
		_beginthread(DataCompressMgrThread, 0, this);

		if (GlobalParameter::GetInstance()->m_tCfg.nSendImageFlag == 1)//add by zhuxy 20190321	���͵����
		{
			//�������͹����߳�
			_beginthread(SendImageMgrThread, 0, this);
		}
	}
#endif

#if OPEN_IF
	if (camChanel==0)//�������������ƴ��
	{
		this->isOpenAllCams=true;
		int nCamNum = GlobalParameter::GetInstance()->m_tCfg.nCamNum;
		this->m_nJointThreadCount = 0;
		this->camChanel = 0;
		for (int i=0; i<nCamNum; i++)
		{
			//��ʼ����ƴ��ѹ��
			stopFlag[i] = false;
			_beginthread(DataJointThread, 0, this);
		}

		CString infoStr;
		infoStr.Format(_T("����ƴ���߳�,�߳���:%d"),nCamNum);
		gLogFile.AddAppRecord(infoStr);
	}
	else//��ָ���������ƴ��
	{
		this->isOpenAllCams=false;		
		this->camChanel = camChanel-1;
		stopFlag[this->camChanel] = false;
		_beginthread(DataJointThread, 0, this);

		CString infoStr;
		infoStr.Format(_T("����ƴ���߳�,�߳���:1"));
		gLogFile.AddAppRecord(infoStr);
	}
	//��ʼ����ѹ�������߳�
	_beginthread(DataCompressMgrThread, 0, this);
	gLogFile.AddAppRecord(_T("��������ѹ�������߳�"));
#endif
}


/*
������StopDataMgr()
�������ܣ�ͨ����λ����m_bStopGrabFlag��ֹͣ����ɼ���
�������壺
NULL
*/
void CImageDataMgr::StopDataMgr(int camChanel/* =0 */)
{
#if CLOSE_IF
	m_bStopGrabFlag = true;
#endif
	if(camChanel!=0)
	{
		stopFlag[camChanel-1]=true;
	}
	else
	{
		for(int i=0;i<4;i++)
		{
			stopFlag[i]=true;
		}
	}
}

//��������:��¼�ɼ���Ϣ���߳�
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void RecordGrabInfoThread(void * param)
{
	CImageDataMgr *pThisObject = (CImageDataMgr*)param;
	try
	{
		while(pThisObject->m_nCompressThreadCount > 0)
		{
			Sleep(500);
		}

		gLogFile.AddAppRecord(_T("ѹ���߳��˳�"));

		GlobalParameter *pGlobalParam = GlobalParameter::GetInstance();
		if (pGlobalParam->m_tCfg.nSendImageFlag == 1)
		{
			while (!pThisObject->m_bStopSendMgrFlag /*|| pThisObject->m_nSendThreadCount > 0*/)
			{
				pThisObject->m_nSendWaitTime++;
				Sleep(1000);
			}

			gLogFile.AddAppRecord(_T("�����߳��˳�"));
			for (auto iter = pThisObject->m_dUploadParam.begin(); iter != pThisObject->m_dUploadParam.end(); iter++ )
			{
				if ((*iter)->pJPGBuffer != NULL)
				{
					delete [] (*iter)->pJPGBuffer;
					(*iter)->pJPGBuffer = NULL;
				}

				(*iter)->nSizeJPGBuffer = -1;
                delete *iter;
			}

			pThisObject->m_dUploadParam.clear();
			pThisObject->m_nFirstJPG = 0;
			pThisObject->m_nLastJPG = 0;

			if (pThisObject->m_hEventJPGReady)
			{
				CloseHandle(pThisObject->m_hEventJPGReady);
			}

			pThisObject->m_hEventJPGReady = CreateSemaphore(NULL,0,10000,NULL);
		}

		int nCamNum = pGlobalParam->m_vCamCfg.size();
		for (int i=0; i<nCamNum; i++)
		{
			CString strInfo;
			if(_T("") == trainNumStr)
			{
				strInfo.Format(_T("������ţ�%s, �ɼ�����%d, ѹ������%d, ��ʧ����%d"),
					_T("UnknownTrainId"),\
					pGlobalParam->m_vImgGrabInfo[i].nGrabNum, \
					pGlobalParam->m_vImgGrabInfo[i].nCompressNum, \
					//pGlobalParam->m_vImgGrabInfo[i].nUploadNum,\//delete by zhuxy20190422
					//pGlobalParam->m_vImgGrabInfo[i].nLostNum);//delete by zhuxy20190422
					pGlobalParam->m_vImgGrabInfo[i].nGrabNum - \
					pGlobalParam->m_vImgGrabInfo[i].nCompressNum);
			}
			else
			{
				strInfo.Format(_T("������ţ�%s, �ɼ�����%d, ѹ������%d, ��ʧ����%d"),
					string2wstring(pGlobalParam->m_tTrainInfo.strTrainId).c_str(),\
					pGlobalParam->m_vImgGrabInfo[i].nGrabNum, \
					pGlobalParam->m_vImgGrabInfo[i].nCompressNum, \
					//pGlobalParam->m_vImgGrabInfo[i].nUploadNum,\//delete by zhuxy20190422
					//pGlobalParam->m_vImgGrabInfo[i].nLostNum);//delete by zhuxy20190422
					pGlobalParam->m_vImgGrabInfo[i].nGrabNum - \
					pGlobalParam->m_vImgGrabInfo[i].nCompressNum);
			}
			gLogFile.AddTrainInfoRecord(i, strInfo);

			//if (0 == pGlobalParam->m_vImgGrabInfo[i].nGrabNum)
			//{
			//	gCamMgr.CheckCamStatus(i);
			//}
			
			pThisObject->m_vImgBuf[i]->Reset();   //����������
		}

		if (GlobalParameter::GetInstance()->m_tCfg.nSaveImageFlag == 1)
		{
//			AfxBeginThread(RemoveOverdueFile, NULL);
		}

		//GlobalParameter::GetInstance()->m_tTrainInfo.workState = 3;//delete by zhuxy20190322
	}
	catch (...)
	{
		gLogFile.AddAppRecord(_T("RecordGrabInfoThread�߳��쳣"));
	}
}


/*
������DataCompressThread(void* param)
�������ܣ�����ѹ���̺߳��������������ڶ��߳��У���Ϊȫ�ֺ�����
�������壺
param:JointImgInfo�ṹ�壬��������̺߳����ڴ�����Ϣ
����ֵ��NULL
*/
void DataCompressThread(void* param)
{
	JointImgInfo *pJointInfo = (JointImgInfo*)param;
	int nJointIndex = pJointInfo->nJointIndex;
	int nImgIndex = pJointInfo->nImgIndex;
	int nCamIndex = pJointInfo->nCamIndex;
	int nEnhanceIndex = pJointInfo->nEnHanceIndex;
	BYTE *pJointBuf = pJointInfo->pImgData;           //��ȡԭʼ����
	CImageDataMgr *pThisObject = (CImageDataMgr*)pJointInfo->pObject;

	GlobalParameter *pGlobalParam = GlobalParameter::GetInstance();
	CameraCfg camCfg = pGlobalParam->m_vCamCfg[nCamIndex];
	int nWidth = camCfg.nWidth;
	int nHeight = camCfg.nHeight;
	int nJointWidth = nWidth;
	int nJointHeight = nHeight * nJointIndex;
	
	BYTE *pOutBuffer = NULL;
	size_t  outsize;
	long lCount;

	try
	{
		for (int i=nJointIndex; i>0; i--)
		{
			bool bRtn = GetJpgCompressData(nWidth, nHeight, pJointBuf + (nJointIndex - i) * nWidth * nHeight,  &pOutBuffer, &outsize, camCfg.nImgQuality);
			if (bRtn)
			{
				WaitForSingleObject(pThisObject->m_hCompressCountSemaphore, INFINITE);
				pGlobalParam->m_vImgGrabInfo[nCamIndex].nCompressNum++;//add ע��by zhuxy ѹ������1
				ReleaseSemaphore(pThisObject->m_hCompressCountSemaphore, 1, &lCount);

				if ((pGlobalParam->m_tCfg.nSendImageFlag == 1) && (WaitForSingleObject(pThisObject->m_hSemaphoreJPGBufferInUsed,INFINITE) == WAIT_OBJECT_0))
				{
					while ((pThisObject->m_nFirstJPG+1)%BUFFERSIZEFORJPG == pThisObject->m_nLastJPG)
					{
						Sleep(100);
					}
					pUploadParam uploadParam = new UploadParam();
					uploadParam->pJPGBuffer = new BYTE[outsize]; 
					memcpy(uploadParam->pJPGBuffer, pOutBuffer, outsize);
					uploadParam->nSizeJPGBuffer = outsize;
					uploadParam->nIndexJPG = nImgIndex-i;
					uploadParam->nCameraIndexJPG = camCfg.nChannelNum;
					uploadParam->nCameraIndexInConf = nCamIndex;
					
					pThisObject->m_dUploadParam.push_back(uploadParam);
					pThisObject->m_nFirstJPG = (pThisObject->m_nFirstJPG+1)%BUFFERSIZEFORJPG;

					ReleaseSemaphore(pThisObject->m_hEventJPGReady, 1, &lCount);
					ReleaseSemaphore(pThisObject->m_hSemaphoreJPGBufferInUsed, 1, &lCount);
				}

				if (pGlobalParam->m_tCfg.nSaveImageFlag == 1)
				{
#if OPEN_IF//�����ṩ�µķ�ʽ����jpg8λ���
					//����ͼ��jpg
					wchar_t cDestFileName[255] = {0};
					wsprintfW(cDestFileName, _T("%s\\Camera_0%d\\jpg\\%d.jpg"), theApp.m_strCurrentImgSavePath, camCfg.nChannelNum, nImgIndex - i);
					FILE *fp = fopen(wstring2string(cDestFileName).c_str(), "wb");
					if (fp != NULL)
					{
						fwrite(pOutBuffer, outsize, 1, fp);
						fclose(fp);
						//gLogFile.AddCameraRecord(nCamIndex, cDestFileName);
					}
					else
					{
						CString strInfo;
						strInfo.Format(_T("�ļ���%s д��ʧ��"), cDestFileName);
						gLogFile.AddCameraRecord(nCamIndex, strInfo);
					}
#endif

#if CLOSE_IF //CImage����ͼƬ������ζ���24λ��
					//����ͼ��jpg
					wchar_t cDestFileName[255] = {0};
					wsprintfW(cDestFileName, _T("%s\\Camera_0%d\\jpg\\%d.jpg"), theApp.m_strCurrentImgSavePath, camCfg.nChannelNum, nImgIndex - i);//add by zhuxy 20190318
					CImage img;
					img.Create(nWidth, nHeight, 8, 0);
					//�ҵ�ͼ������ָ�����ʼ��ַ
					BYTE *p = (BYTE *)img.GetBits() + (img.GetPitch()*(img.GetHeight() - 1));
					//��������
					memcpy(p, pJointInfo->pImgData, nWidth*nHeight);
					if (img.GetBPP() == 8)
					{
						//8λ�Ҷ�ͼ�����õ�ɫ��
						RGBQUAD colors[256];
						img.GetColorTable(0, img.GetMaxColorTableEntries(), colors);
						for (int i = 0; i < 256; i++)
						{
							colors[i].rgbBlue = (BYTE)i;
							colors[i].rgbGreen = (BYTE)i;
							colors[i].rgbRed = (BYTE)i;
							colors[i].rgbReserved = 0;
						}
						img.SetColorTable(0, img.GetMaxColorTableEntries(), colors);
					}
					//����ͼƬ
					long a = img.Save(cDestFileName);
					if(E_OUTOFMEMORY == a)//add by zhuxy
					{
						gLogFile.AddAppRecord(_T("�洢�쳣,�ڴ治��"));
					}
					else if(E_FAIL == a)
					{
						gLogFile.AddAppRecord(_T("�洢�쳣,�洢ʧ��"));
					}
#endif
				}
				else if (pGlobalParam->m_tCfg.nSaveImageFlag == 2)
				{
					//bmp
					wchar_t cDestFileName[255] = {0};
					//wsprintfW(cDestFileName, _T("%s\\bmp\\%d-%d.bmp"), theApp.m_strCurrentImgSavePath, camCfg.nChannelNum, nImgIndex - i);//delete by zhuxy 20190318
					wsprintfW(cDestFileName, _T("%s\\Camera_0%d\\bmp\\%d.bmp"), theApp.m_strCurrentImgSavePath, camCfg.nChannelNum, nImgIndex - i);//add by zhuxy 20190318

					CImage img;
					img.Create(nWidth, nHeight, 8, 0);
					//�ҵ�ͼ������ָ�����ʼ��ַ
					BYTE *p = (BYTE *)img.GetBits() + (img.GetPitch()*(img.GetHeight() - 1));
					//��������
					memcpy(p, pJointInfo->pImgData, nWidth*nHeight);
					if (img.GetBPP() == 8)
					{
						//8λ�Ҷ�ͼ�����õ�ɫ��
						RGBQUAD colors[256];
						img.GetColorTable(0, img.GetMaxColorTableEntries(), colors);
						for (int i = 0; i < 256; i++)
						{
							colors[i].rgbBlue = (BYTE)i;
							colors[i].rgbGreen = (BYTE)i;
							colors[i].rgbRed = (BYTE)i;
							colors[i].rgbReserved = 0;
						}
						img.SetColorTable(0, img.GetMaxColorTableEntries(), colors);
					}
					//����ͼƬ
					long a = img.Save(cDestFileName);
					if(E_OUTOFMEMORY == a)//add by zhuxy
					{
						gLogFile.AddAppRecord(_T("�洢�쳣,�ڴ治��"));
					}
					else if(E_FAIL == a)
					{
						gLogFile.AddAppRecord(_T("�洢�쳣,�洢ʧ��"));
					}
				}
				else if (pGlobalParam->m_tCfg.nSaveImageFlag == 3)
				{
					//����ͼ��jpg bmp
					wchar_t cDestFileName[255] = {0};
					wsprintfW(cDestFileName, _T("%s\\Camera_0%d\\jpg\\%d.jpg"), theApp.m_strCurrentImgSavePath, camCfg.nChannelNum, nImgIndex - i);//add by zhuxy 20190318
					CImage img;
					img.Create(nWidth, nHeight, 8, 0);
					//�ҵ�ͼ������ָ�����ʼ��ַ
					BYTE *p = (BYTE *)img.GetBits() + (img.GetPitch()*(img.GetHeight() - 1));
					//��������
					memcpy(p, pJointInfo->pImgData, nWidth*nHeight);
					if (img.GetBPP() == 8)
					{
						//8λ�Ҷ�ͼ�����õ�ɫ��
						RGBQUAD colors[256];
						img.GetColorTable(0, img.GetMaxColorTableEntries(), colors);
						for (int i = 0; i < 256; i++)
						{
							colors[i].rgbBlue = (BYTE)i;
							colors[i].rgbGreen = (BYTE)i;
							colors[i].rgbRed = (BYTE)i;
							colors[i].rgbReserved = 0;
						}
						img.SetColorTable(0, img.GetMaxColorTableEntries(), colors);
					}
					//����ͼƬjpg
					long a = img.Save(cDestFileName);
					if(E_OUTOFMEMORY == a)//add by zhuxy
					{
						gLogFile.AddAppRecord(_T("�洢�쳣,�ڴ治��"));
					}
					else if(E_FAIL == a)
					{
						gLogFile.AddAppRecord(_T("�洢�쳣,�洢ʧ��"));
					}
					
					wsprintfW(cDestFileName, _T("%s\\Camera_0%d\\bmp\\%d.bmp"), theApp.m_strCurrentImgSavePath, camCfg.nChannelNum, nImgIndex - i);//add by zhuxy 20190318
				
					//����ͼƬbmp
					a = img.Save(cDestFileName);
					if(E_OUTOFMEMORY == a)//add by zhuxy
					{
						gLogFile.AddAppRecord(_T("�洢�쳣,�ڴ治��"));
					}
					else if(E_FAIL == a)
					{
						gLogFile.AddAppRecord(_T("�洢�쳣,�洢ʧ��"));
					}
				}
				free(pOutBuffer);
				pOutBuffer = NULL;
			}
			else
			{
				CString strInfo;
				strInfo.Format(_T("ѹ����%��ͼƬʧ��"), nImgIndex - i);
				gLogFile.AddCameraRecord(nCamIndex, strInfo);
			}
		}
	}
	catch (...)
	{
		gLogFile.AddAppRecord( _T("DataCompressThread�߳��쳣"));
	}

	/*��������ڴ�*/
	delete pJointInfo;//ɾ������ͼƬ�Ķ��ڴ棬�Ǹ��ṹ��
	delete [] pJointBuf;//ɾ��ͼƬ���ݵ��ڴ棬�Ǹ��ֽ�����

	WaitForSingleObject(pThisObject->m_hCompressThreadCountSemaphore, INFINITE);
	pThisObject->m_nCompressThreadCount--;
	ReleaseSemaphore(pThisObject->m_hCompressThreadCountSemaphore, 1, &lCount);
}


/*
������DataCompressMgrThread(void * param)
�������ܣ�����ѹ���̺߳��������������趨������ѹ���̣߳�
�������壺
param:CImageDataMgr�ṹ�壬���ڽ����̵߳����ݽṹ���ݽ�������߳�
����ֵ��NULL
*/
void DataCompressMgrThread(void * param)
{
	CImageDataMgr *pThisObject = (CImageDataMgr*)param;
	long lCount;
	int nCompressThreadCount = GlobalParameter::GetInstance()->m_tCfg.nCompressThreadCount;

	try
	{
		while(0 == pThisObject->m_nJointThreadCount)
		{
			Sleep(100);
#if CLOSE_IF
			TRACE(_T("����ѹ���̺߳���������:%d"),GetCurrentThreadId());
#endif
		}

		while (true)
		{
			if (!pThisObject->m_listJointInfo.IsEmpty())
			{
				if (pThisObject->m_nCompressThreadCount >= nCompressThreadCount)//add
				{
					continue;
				}

				WaitForSingleObject(pThisObject->m_hCompressThreadCountSemaphore,INFINITE);
				pThisObject->m_nCompressThreadCount++;
				ReleaseSemaphore(pThisObject->m_hCompressThreadCountSemaphore,1, &lCount);

				//ѹ���߳�
				_beginthread( DataCompressThread, 0, pThisObject->m_listJointInfo.GetHead());

				pThisObject->m_listJointInfo.RemoveHead();
			}
			else
			{
				Sleep(100);
			}
			//else if (pThisObject->m_bStopJointFlag)
			//if (0 == pThisObject->m_nJointThreadCount)//fix by zhuxy20190426
			if ((0==pThisObject->m_nJointThreadCount)&&(pThisObject->m_listJointInfo.IsEmpty()))//�����жϷ����е�ͼƬ�����ᱻ����
			{
				break;
			}
		}
	}
	catch (...)
	{
		gLogFile.AddAppRecord( _T("DataCompressMgrThread�߳��쳣"));
	}

	_beginthread( RecordGrabInfoThread, 0, param);//addע�� �����߳�д�뱾�ι������
	pThisObject->m_bStopCompressMgrFlag = true;
	gLogFile.AddAppRecord(_T("����ѹ�������߳��˳�"));	

	//add by zhuxy 20190321 ����������ɼ�����
	if(workWay==WAY_AUTO)//�Զ��ɼ��²��ڷ��������ɶ�Ӧ�Ĺ�����Ϣ
	{
		CString A_B_Str;
		if (trainNumStr==_T(""))//�Զ�ģʽ��ȴû���յ�������Ϣ��ʶ��Ϊδ֪����
		{
			theApp.m_strTrainId = _T("UnknownTrainId");
			A_B_Str=_T("UnknownDirection");//ab��
		}
		else
		{
			//21B08031032A1
			if (TRAIN_INFO_SIZE!=trainNumStr.GetLength())
			{
				theApp.m_strTrainId = trainNumStr.Mid(0,trainNumStr.GetLength()-2);//21B08031032
				A_B_Str = trainNumStr.Right(2);//A1
			}
			else
			{
				theApp.m_strTrainId = trainNumStr.Mid(0,5);//21B08031032
				A_B_Str = trainNumStr.Mid(5,2);//A1
			}
		}
		CString oldPath = theApp.m_strCurrentImgSavePath;//�������ļ���,�ѳ��Ŵ���ʱ��
		CString newPath = theApp.m_strCurrentImgSavePath.Mid(0,theApp.m_strCurrentImgSavePath.Find(_T("_"))+1)+theApp.m_strTrainId;
		if(1 != GlobalParameter::GetInstance()->m_tCfg.nSpare)//����Ǳ������������ÿ�ʼ����ʱ��
		{
			newPath = theApp.m_strCurrentImgSavePath.Mid(0,theApp.m_strCurrentImgSavePath.Find(_T("_"))-14)+trainTimeStr+_T("_")+theApp.m_strTrainId;
		}
		BOOL ret = MoveFileW(oldPath,newPath);
		if(ret)
		{
			theApp.m_strCurrentImgSavePath = newPath;
		}
		SYSTEMTIME stLocal; 
		GetLocalTime(&stLocal);
		CString strPath = GlobalParameter::GetInstance()->m_tCfg.strImgSavePath.c_str();
		CString strTime;
		strTime.Format(_T("%04u-%02u-%02u %02u:%02u:%02u"),
			stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour,
			stLocal.wMinute, stLocal.wSecond);
		theApp.m_strCamName = theApp.m_strCamName.Left(theApp.m_strCamName.GetLength() - 1);
		CString strLogPath = strPath + _T("������Ϣ.txt");
		CString tmpStr;

		for (int i =0;i<gCamMgr.m_vCamCtrl.size();i++)
		{
			if(0==GlobalParameter::GetInstance()->m_vImgGrabInfo[i].nGrabNum)
			{
				continue;
			}
			CString tmpPathStr = theApp.m_strCurrentImgSavePath;
			if(0==tmpPathStr.Find(_T("\\")))//����Ƿ������ĵ�ַ��ȥ��˫����
			{
				tmpPathStr.Replace(_T("\\\\"),_T("\\"));tmpPathStr = _T("\\")+tmpPathStr;
			}
			tmpStr.Format(_T("%s&%s&%s&0%d&%s\\Camera_0%d"),strTime, theApp.m_strTrainId,A_B_Str,i,tmpPathStr,i);//D:\\GrabImg\\201903\\20190318180034_��г��			
			gLogFile.AddAppRecord(strLogPath, tmpStr);
			CString tempString;

			tempString.Format(_T("���%dֹͣ�ɼ�,ͼ���С:%dx%d"),i+1,GlobalParameter::GetInstance()->m_vCamCfg[i].nWidth,GlobalParameter::GetInstance()->m_vCamCfg[i].nHeight);
			gLogFile.AddAppRecord(tempString);
		}
	}
	GlobalParameter::GetInstance()->m_tTrainInfo.strTrainId="";//��ԭ��ʼ״̬
	GlobalParameter::GetInstance()->m_tTrainInfo.strTrainTime="";
	GlobalParameter::GetInstance()->m_tTrainInfo.workState=STATE_READY;
	trainNumStr = _T("");
	trainTimeStr = _T("");
	workWay = WAY_STOP;
}

void LogEnhance(BYTE* imageData,int nHeight,int nWidth,float fGamma,int nThreshold)
{
	// ����oldPixel:[1,256],������ȱ���һ�����ұ�
	uchar lut[256] ={0};

	double temp =0.0f;    
	for ( int i =0; i<= 255; i++)  
	{  
		temp = pow((float)(i/255.0),fGamma)*255.0f;
		lut[i] = (uchar)(temp);
	} 

	for( int row =0; row < nHeight; row++)
	{
		uchar *data = (uchar*)imageData+ row*nWidth;
		for ( int col = 0; col< nWidth; col++)
		{
			if(data[col] > nThreshold)
			{
				data[col] = lut[data[col]];
			}
		}
	}
	//gLogFile.AddAppRecord(_T("Log��ǿͼ��"));
}


/*
������DataJointThread(LPVOID param)
��������: ͼ��ƴ���̺߳��������ڽ����ͼ��ƴ�ӳ�һ��ͼ��Ϊ���̺߳�����
�������壺
LPVOID param���������ڽ����̲߳���ȫ�����ݽ�ƴ���߳�
����ֵ��NULL
*/
void DataJointThread(LPVOID param)
{
	CImageDataMgr *pThisObject = (CImageDataMgr*)param;

	long lCount = 0;
	WaitForSingleObject(pThisObject->m_hJointThreadCountSemaphore, INFINITE);
#if CLOSE_IF
	int nCamIndex = pThisObject->m_nJointThreadCount++;
#endif
#if OPEN_IF
	int nCamIndex = pThisObject->camChanel;
	if(pThisObject->isOpenAllCams)
	{
		pThisObject->camChanel++;
	}	
	pThisObject->m_nJointThreadCount++;
#endif
	TRACE(_T("camIndex:%d\n"),nCamIndex);
	ReleaseSemaphore(pThisObject->m_hJointThreadCountSemaphore, 1, &lCount);

	JointImgInfo *pTempJointInfo = NULL;
	BYTE *pOriData = NULL;
	int nTimeOut = 0;
	bool bRtn = false;

	int nThreadIndex = 0;
	int nJointIndex = 0; //ƴ������

	GlobalParameter *pGlobalParam = GlobalParameter::GetInstance();
	int nStopWaitTime = pGlobalParam->m_tCfg.nStopGrabWaitTime;
	int nWidth = pGlobalParam->m_vCamCfg[nCamIndex].nWidth;
	int nHeight = pGlobalParam->m_vCamCfg[nCamIndex].nHeight;
	int nJointNum = pGlobalParam->m_vCamCfg[nCamIndex].nJointNum;
	int nJointWidth = nWidth;
	int nJointHeight = nHeight * nJointNum;

	//����ƴ�ӻ���
	BYTE *pJointBuf = new BYTE[nJointWidth * nJointHeight];
	memset(pJointBuf, 0, nJointWidth * nJointHeight);

	try
	{
		while (true)
		{
			bRtn = pThisObject->m_vImgBuf[nCamIndex]->BufferOut((void **)&pOriData);
			if (bRtn)
			{
				//��ͼ�����ݴ���ƴ�ӻ���
				memcpy(pJointBuf + (nJointIndex++ * nWidth * nHeight), pOriData, nWidth * nHeight);

				//ͼ����������ƴ���� ����ǿ
				if (nJointIndex == nJointNum)
				{
					BYTE *pImgData = new BYTE[nJointWidth * nJointHeight];
					memcpy(pImgData, pJointBuf, nJointWidth * nJointHeight);

					JointImgInfo *pJointInfo = new JointImgInfo;
					pJointInfo->nJointIndex = nJointIndex;
					pJointInfo->nImgIndex = nThreadIndex * nJointNum + nJointIndex;
					pJointInfo->nCamIndex = nCamIndex;
					pJointInfo->pImgData = pImgData;
					pJointInfo->pObject = pThisObject;

					//2017 6/16 ��ȡ��һ��ƴ��ͼ��lut��
					//if ((nThreadIndex == 0) && ((pGlobalParam->m_vCamCfg[nCamIndex].nEnhanceType == 2) || (pGlobalParam->m_vCamCfg[nCamIndex].nEnhanceType == 3)))
					//{
					//	createlut(pJointBuf, pGlobalParam->m_vCamCfg[nCamIndex].tEnhanceParam.nSaturated, nJointWidth, nJointHeight, true, nCamIndex);
					//}
					////2017 11/16 ͼ����ǿ������ͼƬ��
					if (pGlobalParam->m_vCamCfg[nCamIndex].tEnhanceParam.fLogEnhance > 0)//ʵ�ʹ����в�û��������
					{
						LogEnhance(pImgData,
							nJointHeight,
							nJointWidth, pGlobalParam->m_vCamCfg[nCamIndex].tEnhanceParam.fLogEnhance,
							pGlobalParam->m_vCamCfg[nCamIndex].tEnhanceParam.nLogEnhanceThreshold);
					}
#if CLOSE_IF //��������ԭ���Ĵ���ʽ���ᵼ��ѹ����С�ڲɼ���
					//���ɫ������
					if (pTempJointInfo != NULL)
					{
						pThisObject->m_listJointInfo.AddTail(pTempJointInfo);
					}
					else
					{
						int nHeadInvaildImgCount = pGlobalParam->m_vCamCfg[nCamIndex].nHeadInvaildImgCount;
						if ((nHeadInvaildImgCount>0) && (nHeadInvaildImgCount < nJointNum))
						{
							pJointInfo->nEnHanceIndex = 1;
						}
					}
					nThreadIndex++;
					nJointIndex = 0;
					pTempJointInfo = pJointInfo;
					pThisObject->m_listJointInfo.AddTail(pTempJointInfo);
#endif

#if OPEN_IF//add by zhuxyѹ�������ڲɼ���
					nThreadIndex++;
					nJointIndex = 0;
					pThisObject->m_listJointInfo.AddTail(pJointInfo);
#endif
					memset(pJointBuf, 0, nJointWidth * nJointHeight);
				}
			}
			else
			{
				//if ((!pThisObject->m_bStopGrabFlag) && (nTimeOut < nStopWaitTime))//addע�� ���û��ֹͣ��ÿ�ﵽ��ʱ�ȴ�������ȴ�
				if((!pThisObject->stopFlag[nCamIndex]) && (nTimeOut < nStopWaitTime))
				{
					++nTimeOut;
#if CLOSE_IF//�Ƿ���Ҫ�ȴ�һ��
					Sleep(100);
#endif
				}
				else
				{
					int nGrabNum = pGlobalParam->m_vImgGrabInfo[nCamIndex].nGrabNum;
					int nJonitNum = nThreadIndex * nJointNum + nJointIndex;

					if (pThisObject->stopFlag[nCamIndex])  //�յ�ֹͣ�ɼ��ź�
					{
						CString strInfo;
						strInfo.Format(_T("����ƴ���߳������˳�: �ɼ���%d, ƴ����%d, ��ʱ�ȴ�����%d"), nGrabNum, nJonitNum, nTimeOut);
						gLogFile.AddCameraRecord(nCamIndex, strInfo);
					}
					else  //û���յ�ֹͣ�ɼ��ź�
					{
						GlobalParameter::GetInstance()->m_tTrainInfo.workState = -1;
						gCamMgr.StopGrab(nCamIndex);

						CString strInfo;
						strInfo.Format(_T("����ƴ���̳߳�ʱ�˳�: �ɼ���%d, ƴ����%d, ��ʱ�ȴ�����%d"), nGrabNum, nJonitNum, nTimeOut);
						gLogFile.AddCameraRecord(nCamIndex, strInfo);
					}

					break;
				}
			}
		}

		//��ǿʣ��ͼƬ
		if ((pTempJointInfo != NULL) && (nJointIndex > 0))
		{
			BYTE *pImgData = new BYTE[nJointWidth * (nJointNum + nJointIndex) * nHeight];
			memcpy(pImgData, pTempJointInfo->pImgData, nJointWidth * nJointHeight);
			memcpy(pImgData + (nJointWidth * nJointHeight), pJointBuf, nJointWidth * nJointIndex * nHeight);

			JointImgInfo *pJointInfo = new JointImgInfo;
			pJointInfo->nJointIndex = nJointNum + nJointIndex;
			pJointInfo->nImgIndex = nThreadIndex * nJointNum + nJointIndex;
			pJointInfo->nCamIndex = nCamIndex;
			pJointInfo->pImgData = pImgData;
			pJointInfo->pObject = pThisObject;

			int nTailInvaildImgCount = pGlobalParam->m_vCamCfg[nCamIndex].nTailInvaildImgCount;
			if ((nTailInvaildImgCount>0) && (nTailInvaildImgCount<(nJointNum + nJointIndex)))
			{
				pJointInfo->nEnHanceIndex = 2;
			}
			pThisObject->m_listJointInfo.AddTail(pJointInfo);
			delete[] pTempJointInfo->pImgData;
			delete pTempJointInfo;
		}
		else if (pTempJointInfo != NULL)
		{
			int nTailInvaildImgCount = pGlobalParam->m_vCamCfg[nCamIndex].nTailInvaildImgCount;
			if ((nTailInvaildImgCount>0) && (nTailInvaildImgCount<nJointNum))
			{
				pTempJointInfo->nEnHanceIndex = 2;
			}
			pThisObject->m_listJointInfo.AddTail(pTempJointInfo);
		}
		else if (nJointIndex > 0)
		{
			BYTE *pImgData = new BYTE[nJointWidth * nJointIndex * nHeight];
			memcpy(pImgData, pJointBuf, nJointWidth * nJointIndex * nHeight);

			JointImgInfo *pJointInfo = new JointImgInfo;
			pJointInfo->nJointIndex = nJointIndex;
			pJointInfo->nImgIndex = nThreadIndex * nJointNum + nJointIndex;
			pJointInfo->nCamIndex = nCamIndex;
			pJointInfo->pImgData = pImgData;
			pJointInfo->pObject = pThisObject;
			pThisObject->m_listJointInfo.AddTail(pJointInfo);
		}
	}
	catch (...)
	{
		gLogFile.AddAppRecord(_T("DataJointThread�߳��쳣"));
	}

	delete[] pJointBuf;
	pJointBuf = NULL;
	//pThisObject->m_bStopJointFlag = true;

	WaitForSingleObject(pThisObject->m_hJointThreadCountSemaphore, INFINITE);
	pThisObject->m_nJointThreadCount--;
	ReleaseSemaphore(pThisObject->m_hJointThreadCountSemaphore, 1, &lCount);
}


/*
������SendImageThread(LPVOID param)
��������: ͼ�����̺߳��������ڽ�ͼ�����ݷְ�������ָ���ķ�������
�������壺
LPVOID param���������ڽ����̲߳���ȫ�����ݷ��ͽ��߳�
����ֵ��NULL
*/
void SendImageThread(LPVOID param)
{
	CImageDataMgr *pThisObject = (CImageDataMgr *)param;
	long count;

	HANDLE hThisThread = GetCurrentThread();
	::SetThreadPriority(hThisThread,THREAD_PRIORITY_NORMAL);

	BYTE *pData = NULL;
	int nSize = 0;
	int nSizeTemp = 0;
	int nIndex = 0;
	int nCameraIndex = 0;
	int nCameraIndexInConf = 0;

	try{		
		wchar_t strError[255];
		//char buf[1000000];    //ͼ���ܳ���1��
		char *buf = (char*)malloc(2048*1400);
		char *pbuf=NULL;
		char nTemp=0;	
		int nTimeOut = 0;
		SOCKET RecvSocket=INVALID_SOCKET;
		int bResult;
		nTimeOut=10;

		while ((INVALID_SOCKET == RecvSocket)&&(nTimeOut>0)){
			RecvSocket = WSASocket(AF_INET, 
				SOCK_STREAM, 
				IPPROTO_TCP, 
				NULL, 
				0, 
				WSA_FLAG_OVERLAPPED);
			if (INVALID_SOCKET == RecvSocket) {
				Sleep(1000);
				nTimeOut--;
			}
		}

		if (INVALID_SOCKET == RecvSocket)
		{
			wsprintfW(strError, _T("�޷�����ͨѶ,���뿪������ϵ��������룺%d\n"), WSAGetLastError());
			gLogFile.AddAppRecord(strError);

			WaitForSingleObject(pThisObject->m_hSendThreadCountSemaphore,INFINITE);
			pThisObject->m_nSendThreadCount--;
			ReleaseSemaphore(pThisObject->m_hSendThreadCountSemaphore,1,&count);

			free(buf);
			return;
			//_endthread();
		}

		sockaddr_in clientService; 
		clientService.sin_family = AF_INET;
		clientService.sin_addr.s_addr = inet_addr(wstring2string(GlobalParameter::GetInstance()->m_tCfg.strSerIp).c_str());
		clientService.sin_port = htons(GlobalParameter::GetInstance()->m_tCfg.nSerPort);

		//----------------------
		// Connect to server.
		bResult = SOCKET_ERROR;
		bResult = connect( RecvSocket, (SOCKADDR*) &clientService, sizeof(clientService) );

		if (bResult == SOCKET_ERROR) 
		{
			wsprintfW(strError, _T("�޷����ӵ����������������磬������룺%d\n"), WSAGetLastError());
			gLogFile.AddAppRecord(strError);

			WaitForSingleObject(pThisObject->m_hSendThreadCountSemaphore,INFINITE);
			pThisObject->m_nSendThreadCount--;
			ReleaseSemaphore(pThisObject->m_hSendThreadCountSemaphore,1,&count);
			
			free(buf);
			return;
			//_endthread();	
		}

		WSAEVENT EventArray[1];
		EventArray[0] = WSACreateEvent();

		if (WSAEventSelect( RecvSocket, EventArray[0], FD_READ | FD_CLOSE)!=0)
		{
			WSACloseEvent(EventArray[0]);
			closesocket(RecvSocket);

			wsprintfW(strError, _T("�޷������¼�����,���뿪������ϵ���������: %x\n"), WSAGetLastError());
			gLogFile.AddAppRecord(strError);

			WaitForSingleObject(pThisObject->m_hSendThreadCountSemaphore,INFINITE);
			pThisObject->m_nSendThreadCount--;
			ReleaseSemaphore(pThisObject->m_hSendThreadCountSemaphore,1,&count);
			
			free(buf);
			return;
			//_endthread();
		}

		while (1)
		{
			nSize = pThisObject->GetJPGData(&pData,&nIndex,&nCameraIndex,&nCameraIndexInConf,100);
			if (nSize==-1)
			{
				if (!pThisObject->m_bStopCompressMgrFlag || pThisObject->m_nCompressThreadCount>0)
					continue;
				else
					break;
			}else if (nSize==0)
			{
				continue;
			}

			memset(buf, 0, 2048*1400);
			memcpy(buf+sizeof(PICTUREINFO),pData,nSize);

			nSizeTemp = nSize;
			nSize+=sizeof(PICTUREINFO);
			
			strncpy_s(((PICTUREINFO *)buf)->m_strTrainID, GlobalParameter::GetInstance()->m_tTrainInfo.strTrainId.c_str(), 11);	
			((PICTUREINFO *)buf)->m_nImageIndex = nIndex;  //pnImagesMap[global_nFinishedImageCount];
			((PICTUREINFO *)buf)->m_nCamPos = nCameraIndex;  //pThisObject->m_nCameraPosition;
			((PICTUREINFO *)buf)->m_nTotalSize = nSize;

			// ��ʼ����
			pbuf = buf;
			while (nSize>=1000)
			{
				send(RecvSocket,pbuf,1000,0);
				if (WSAWaitForMultipleEvents(1,EventArray,TRUE,1000,FALSE)==WAIT_TIMEOUT)
				{
					WSACloseEvent(EventArray[0]);
					closesocket(RecvSocket);

					if (WaitForSingleObject(pThisObject->m_hSemaphoreJPGBufferInUsed,INFINITE)==WAIT_OBJECT_0)
					{

						while ((pThisObject->m_nFirstJPG+1)%BUFFERSIZEFORJPG==pThisObject->m_nLastJPG){
							Sleep(100);
						}

						//pThisObject->m_pJPGBuffer[pThisObject->m_nFirstJPG] = pData; // FreeJPGData()������delete
						//pThisObject->m_nSizeJPGBuffer[pThisObject->m_nFirstJPG] = nSizeTemp;
						//pThisObject->m_nIndexJPG[pThisObject->m_nFirstJPG] = nIndex;
						//pThisObject->m_nCameraIndexJPG[pThisObject->m_nFirstJPG] = nCameraIndex;
						//pThisObject->m_nCameraIndexInConf[pThisObject->m_nFirstJPG] = nCameraIndexInConf;
						//pThisObject->m_nFirstJPG = (pThisObject->m_nFirstJPG+1)%BUFFERSIZEFORJPG;

						pUploadParam uploadParam = new UploadParam();
						uploadParam->pJPGBuffer = pData; // FreeJPGData()������delete
						uploadParam->nSizeJPGBuffer = nSizeTemp;
						uploadParam->nIndexJPG = nIndex;
						uploadParam->nCameraIndexJPG = nCameraIndex;
						uploadParam->nCameraIndexInConf = nCameraIndexInConf;

						//pThisObject->m_dUploadParam.push_front(uploadParam);
						pThisObject->m_dUploadParam.insert(pThisObject->m_dUploadParam.begin(), uploadParam);
						pThisObject->m_nFirstJPG = (pThisObject->m_nFirstJPG+1)%BUFFERSIZEFORJPG;

						ReleaseSemaphore(pThisObject->m_hEventJPGReady,1,&count);
						ReleaseSemaphore(pThisObject->m_hSemaphoreJPGBufferInUsed,1,&count);
					}

					CString strInfo;
				    strInfo.Format(_T("ͼ������:%d, ���ӳ�ʱ,�������� -1"), nIndex);
					gLogFile.AddCameraRecord(nCameraIndexInConf, strInfo);

					WaitForSingleObject(pThisObject->m_hSendThreadCountSemaphore,INFINITE);
					pThisObject->m_nSendThreadCount--;
					ReleaseSemaphore(pThisObject->m_hSendThreadCountSemaphore,1,&count);

					free(buf);
					return;
					//_endthread();
				}

				WSAResetEvent(EventArray[0]);
				recv(RecvSocket,&nTemp,1,0);
				nSize-=1000;
				pbuf+=1000;
			}

			if (nSize>0)
			{
				send(RecvSocket,pbuf,nSize,0);
				if (WSAWaitForMultipleEvents(1,EventArray,TRUE,10000,FALSE)==WAIT_TIMEOUT)
				{
					WSACloseEvent(EventArray[0]);
					closesocket(RecvSocket);

					if (WaitForSingleObject(pThisObject->m_hSemaphoreJPGBufferInUsed,INFINITE)==WAIT_OBJECT_0)
					{

						while ((pThisObject->m_nFirstJPG+1)%BUFFERSIZEFORJPG==pThisObject->m_nLastJPG)
						{
							Sleep(100);
						}

						//pThisObject->m_pJPGBuffer[pThisObject->m_nFirstJPG] = pData; // FreeJPGData()������delete
						//pThisObject->m_nSizeJPGBuffer[pThisObject->m_nFirstJPG] = nSizeTemp;
						//pThisObject->m_nIndexJPG[pThisObject->m_nFirstJPG] = nIndex;
						//pThisObject->m_nCameraIndexJPG[pThisObject->m_nFirstJPG] = nCameraIndex;
						//pThisObject->m_nCameraIndexInConf[pThisObject->m_nFirstJPG] = nCameraIndexInConf;
						//pThisObject->m_nFirstJPG = (pThisObject->m_nFirstJPG+1)%BUFFERSIZEFORJPG;

						pUploadParam uploadParam = new UploadParam();
						uploadParam->pJPGBuffer = pData; // FreeJPGData()������delete
						uploadParam->nSizeJPGBuffer = nSizeTemp;
						uploadParam->nIndexJPG = nIndex;
						uploadParam->nCameraIndexJPG = nCameraIndex;
						uploadParam->nCameraIndexInConf = nCameraIndexInConf;

						//pThisObject->m_dUploadParam.push_front(uploadParam);
						pThisObject->m_dUploadParam.insert(pThisObject->m_dUploadParam.begin(), uploadParam);
						pThisObject->m_nFirstJPG = (pThisObject->m_nFirstJPG+1)%BUFFERSIZEFORJPG;

						ReleaseSemaphore(pThisObject->m_hEventJPGReady,1,&count);
						ReleaseSemaphore(pThisObject->m_hSemaphoreJPGBufferInUsed,1,&count);
					}

					CString strInfo;
					strInfo.Format(_T("ͼ������:%d, ���ӳ�ʱ,�������� -2"), nIndex);
					gLogFile.AddCameraRecord(nCameraIndexInConf, strInfo);

					WaitForSingleObject(pThisObject->m_hSendThreadCountSemaphore,INFINITE);
					pThisObject->m_nSendThreadCount--;
					ReleaseSemaphore(pThisObject->m_hSendThreadCountSemaphore,1,&count);
					//_endthread();

					free(buf);
                    return;
				}

				WSAResetEvent(EventArray[0]);
				recv(RecvSocket,&nTemp,1,0);
				nSize-=nSize;
				pbuf+=nSize;
			}	

			delete [] pData;
			pData=NULL;

			WaitForSingleObject(pThisObject->m_hUploadCountSemaphore,INFINITE);
			GlobalParameter::GetInstance()->m_vImgGrabInfo[nCameraIndexInConf].nUploadNum++;
			ReleaseSemaphore(pThisObject->m_hUploadCountSemaphore,1,&count);
		}

		// �ͷ�������Դ
		WSACloseEvent(EventArray[0]);
		closesocket(RecvSocket);	

		free(buf);
	}
	catch (...)
	{		
		gLogFile.AddAppRecord(_T("SendImageThread�߳��쳣"));
	}

	WaitForSingleObject(pThisObject->m_hSendThreadCountSemaphore,INFINITE);
	pThisObject->m_nSendThreadCount--;
	ReleaseSemaphore(pThisObject->m_hSendThreadCountSemaphore,1,&count);

	gLogFile.AddAppRecord(_T("�����߳��˳�"));
}


/*
������SendImageMgrThread(LPVOID param)
��������: ����ͼ�����̺߳�������������ָ��������ͼ�����̣߳�
�������壺
LPVOID param���������ڽ����̲߳���ȫ�����ݷ��ͽ��߳�
����ֵ��NULL
*/
void SendImageMgrThread(LPVOID param)
{
	CImageDataMgr *pThisObject = (CImageDataMgr*)param;
	long lCount;

	try
	{	
	    int nSendThreadCount = GlobalParameter::GetInstance()->m_tCfg.nSendThreadCount;
		int nSendWaitTime = GlobalParameter::GetInstance()->m_tCfg.nSendWaitTime;

		while(true)
		{
			if (pThisObject->m_nSendWaitTime >= nSendWaitTime)
			{
				gLogFile.AddAppRecord(_T("��������̳߳�ʱ�˳�"));
				break;
			}
			else if ((pThisObject->m_nFirstJPG!=pThisObject->m_nLastJPG) && (pThisObject->m_nSendThreadCount<nSendThreadCount))
			{
				WaitForSingleObject(pThisObject->m_hSendThreadCountSemaphore,INFINITE);
				pThisObject->m_nSendThreadCount++;	
				ReleaseSemaphore(pThisObject->m_hSendThreadCountSemaphore,1,&lCount);

				_beginthread(SendImageThread, 0, param);

			}
			else if (pThisObject->m_bStopCompressMgrFlag && (pThisObject->m_nCompressThreadCount==0) && 
				     (pThisObject->m_nFirstJPG==pThisObject->m_nLastJPG) && (pThisObject->m_nSendThreadCount==0))
			{
				gLogFile.AddAppRecord(_T("��������߳�����˳�"));
				break;
			}

			Sleep(200);
		}
	}
	catch(...)
	{
		gLogFile.AddAppRecord(_T("SendImageMgrThread�߳��쳣"));
	}

	pThisObject->m_bStopSendMgrFlag = true;
}


/*
������GetJPGData(BYTE ** pData,int *nIndex,int *nCameraIndex,int *nCameraIndexInConf,int nTimeOut)
��������: ��ѹ����������л�ȡѹ�������ݣ�
�������壺
BYTE ** pData��ѹ����ͼ������
int *nIndex��ͼ���ڶ����б��
int *nCameraIndex��������
int *nCameraIndexInConf:����������ļ��еı��
int nTimeOut �����û�ȡѹ�����ݵȴ���ʱʱ��
����ֵ��NULL
*/
int CImageDataMgr::GetJPGData(BYTE ** pData,int *nIndex,int *nCameraIndex,int *nCameraIndexInConf,int nTimeOut)
{
	long count;
	int nSize=0;
	try{
		if (WaitForSingleObject(m_hSemaphoreJPGBufferInUsed,nTimeOut)==WAIT_OBJECT_0)
		{
			if (WaitForSingleObject(m_hEventJPGReady,nTimeOut)==WAIT_OBJECT_0)
			{
				//*pData = m_pJPGBuffer[m_nLastJPG];
				//nSize = m_nSizeJPGBuffer[m_nLastJPG];
				//*nIndex = m_nIndexJPG[m_nLastJPG];
				//*nCameraIndex = m_nCameraIndexJPG[m_nLastJPG];
				//*nCameraIndexInConf = m_nCameraIndexInConf[m_nLastJPG];
				//m_pJPGBuffer[m_nLastJPG] = NULL;
				//m_nLastJPG = (m_nLastJPG+1)%BUFFERSIZEFORJPG;

				if (!m_dUploadParam.empty())
				{
					*pData = m_dUploadParam[0]->pJPGBuffer;
					nSize = m_dUploadParam[0]->nSizeJPGBuffer;
					*nIndex = m_dUploadParam[0]->nIndexJPG;
					*nCameraIndex = m_dUploadParam[0]->nCameraIndexJPG;
					*nCameraIndexInConf = m_dUploadParam[0]->nCameraIndexInConf;

					delete m_dUploadParam[0];
					m_dUploadParam.erase(m_dUploadParam.begin());
					//m_dUploadParam.pop_front();
				}
				else
				{
					nSize = -1;
				}

				m_nLastJPG = (m_nLastJPG+1)%BUFFERSIZEFORJPG;
			}
			else
			{
				nSize = -1;
			}

			ReleaseSemaphore(m_hSemaphoreJPGBufferInUsed,1,&count);
		}
		else
		{
			nSize = 0;
		}
	}catch(...)
	{
		gLogFile.AddAppRecord(_T("CImageCompresser::GetJPGData �����쳣"));
	}

	return nSize;
}


/*
������RemoveOverdueFile(LPVOID pParam)
��������: ��������·���ڵĴ������ݣ��ҵ�����·���ڳ�������ʱ�������ݲ�ɾ����
�������壺
LPVOID param���������ڽ����̲߳���ȫ�����ݷ��ͽ��߳�
����ֵ��NULL
*/
UINT RemoveOverdueFile(LPVOID pParam)
{
	CString strImgSavePath = GlobalParameter::GetInstance()->m_tCfg.strImgSavePath.c_str();
	if (!PathFileExists(strImgSavePath))
	{
		return 0;
	}

	WIN32_FIND_DATA FindFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
  
	CString DirSpec;
	DirSpec = strImgSavePath + _T("/*");

	map<LONGLONG,CString, less<LONGLONG>> vFileInfo;

    hFind = FindFirstFile(DirSpec, &FindFileData);
	while (FindNextFile(hFind, &FindFileData) != 0)
    {
		if(FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY
			&& _tcscmp(FindFileData.cFileName, _T(".")) != 0
			&& _tcscmp(FindFileData.cFileName, _T("..")) != 0)
		{ //�ҵ�Ŀ¼			
            CFileTime fileTime(FindFileData.ftLastAccessTime);
			LONGLONG  nTime = fileTime.GetTime();

			CString strFileName = FindFileData.cFileName;
			vFileInfo[nTime] = strFileName;
		}
	}
	FindClose(hFind);

	int nSaveTrainCount = GlobalParameter::GetInstance()->m_tCfg.nSaveTrainCount;
	int nCount = vFileInfo.size();
	if (nCount <= nSaveTrainCount)
	{
		return 0;
	}

	gLogFile.AddAppRecord(_T("ɾ��ͼ��"));
	nCount = nCount - nSaveTrainCount;

	map<LONGLONG,CString>::iterator iter = vFileInfo.begin();
	for (; iter != vFileInfo.end(); iter++)
	{
		CString strTemp = strImgSavePath + _T("\\") + iter->second;
		DeleteDirectory(strTemp);
		
		if (--nCount <=0)
		{
			break;
		}
	}

	return 1;
}
#include "StdAfx.h"
#include "ImageGrab.h"
#include "ImageDataMgr.h"


UINT RemoveOverdueFile(LPVOID pParam);  //移除过期图像
void DataJointThread(LPVOID param);   //图像拼接线程
void DataCompressMgrThread(void * param);   //数据压缩管理线程
void DataCompressThread(void* param);    //数据压缩线程
void SendImageThread(LPVOID param);     //图像发送线程
void SendImageMgrThread(LPVOID param);   //图像发送管理线程

//函数名称:构造函数
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
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

//函数名称:初始化
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
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

#if OPEN_IF//把标志位初始化从start那里改到这里
	m_bStopGrabFlag = false;//未使用
	m_bStopJointFlag = false;//未使用
	m_bStopCompressMgrFlag = false;  //未使用
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

//函数名称:内存回收
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
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
函数：StartDataMgr()
函数功能：启动和图像数据相关的压缩、发送线程；
参数含义：
NULL
返回值：NULL
*/
void CImageDataMgr::StartDataMgr(int camChanel/* =0 */)
{
#if CLOSE_IF//凌云支持同时采集的代码
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
		//开始数据拼接压缩
		_beginthread(DataJointThread, 0, this);
	}
	if (nCamNum > 0)
	{
		//开始数据压缩管理线程
		_beginthread(DataCompressMgrThread, 0, this);

		if (GlobalParameter::GetInstance()->m_tCfg.nSendImageFlag == 1)//add by zhuxy 20190321	发送到哪里？
		{
			//开启发送管理线程
			_beginthread(SendImageMgrThread, 0, this);
		}
	}
#endif

#if OPEN_IF
	if (camChanel==0)//让所有相机进行拼接
	{
		this->isOpenAllCams=true;
		int nCamNum = GlobalParameter::GetInstance()->m_tCfg.nCamNum;
		this->m_nJointThreadCount = 0;
		this->camChanel = 0;
		for (int i=0; i<nCamNum; i++)
		{
			//开始数据拼接压缩
			stopFlag[i] = false;
			_beginthread(DataJointThread, 0, this);
		}

		CString infoStr;
		infoStr.Format(_T("开启拼接线程,线程数:%d"),nCamNum);
		gLogFile.AddAppRecord(infoStr);
	}
	else//让指定相机进行拼接
	{
		this->isOpenAllCams=false;		
		this->camChanel = camChanel-1;
		stopFlag[this->camChanel] = false;
		_beginthread(DataJointThread, 0, this);

		CString infoStr;
		infoStr.Format(_T("开启拼接线程,线程数:1"));
		gLogFile.AddAppRecord(infoStr);
	}
	//开始数据压缩管理线程
	_beginthread(DataCompressMgrThread, 0, this);
	gLogFile.AddAppRecord(_T("开启数据压缩管理线程"));
#endif
}


/*
函数：StopDataMgr()
函数功能：通过置位参数m_bStopGrabFlag，停止相机采集；
参数含义：
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

//函数名称:记录采集信息的线程
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void RecordGrabInfoThread(void * param)
{
	CImageDataMgr *pThisObject = (CImageDataMgr*)param;
	try
	{
		while(pThisObject->m_nCompressThreadCount > 0)
		{
			Sleep(500);
		}

		gLogFile.AddAppRecord(_T("压缩线程退出"));

		GlobalParameter *pGlobalParam = GlobalParameter::GetInstance();
		if (pGlobalParam->m_tCfg.nSendImageFlag == 1)
		{
			while (!pThisObject->m_bStopSendMgrFlag /*|| pThisObject->m_nSendThreadCount > 0*/)
			{
				pThisObject->m_nSendWaitTime++;
				Sleep(1000);
			}

			gLogFile.AddAppRecord(_T("传输线程退出"));
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
				strInfo.Format(_T("过车编号：%s, 采集数：%d, 压缩数：%d, 丢失数：%d"),
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
				strInfo.Format(_T("过车编号：%s, 采集数：%d, 压缩数：%d, 丢失数：%d"),
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
			
			pThisObject->m_vImgBuf[i]->Reset();   //缓存区重置
		}

		if (GlobalParameter::GetInstance()->m_tCfg.nSaveImageFlag == 1)
		{
//			AfxBeginThread(RemoveOverdueFile, NULL);
		}

		//GlobalParameter::GetInstance()->m_tTrainInfo.workState = 3;//delete by zhuxy20190322
	}
	catch (...)
	{
		gLogFile.AddAppRecord(_T("RecordGrabInfoThread线程异常"));
	}
}


/*
函数：DataCompressThread(void* param)
函数功能：数据压缩线程函数，函数运行在多线程中，顾为全局函数；
参数含义：
param:JointImgInfo结构体，用于向多线程函数内传递信息
返回值：NULL
*/
void DataCompressThread(void* param)
{
	JointImgInfo *pJointInfo = (JointImgInfo*)param;
	int nJointIndex = pJointInfo->nJointIndex;
	int nImgIndex = pJointInfo->nImgIndex;
	int nCamIndex = pJointInfo->nCamIndex;
	int nEnhanceIndex = pJointInfo->nEnHanceIndex;
	BYTE *pJointBuf = pJointInfo->pImgData;           //获取原始数据
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
				pGlobalParam->m_vImgGrabInfo[nCamIndex].nCompressNum++;//add 注释by zhuxy 压缩数加1
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
#if OPEN_IF//凌云提供新的方式保存jpg8位深度
					//保存图像jpg
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
						strInfo.Format(_T("文件：%s 写入失败"), cDestFileName);
						gLogFile.AddCameraRecord(nCamIndex, strInfo);
					}
#endif

#if CLOSE_IF //CImage保存图片无论如何都是24位的
					//保存图像jpg
					wchar_t cDestFileName[255] = {0};
					wsprintfW(cDestFileName, _T("%s\\Camera_0%d\\jpg\\%d.jpg"), theApp.m_strCurrentImgSavePath, camCfg.nChannelNum, nImgIndex - i);//add by zhuxy 20190318
					CImage img;
					img.Create(nWidth, nHeight, 8, 0);
					//找到图像数据指针的起始地址
					BYTE *p = (BYTE *)img.GetBits() + (img.GetPitch()*(img.GetHeight() - 1));
					//复制数据
					memcpy(p, pJointInfo->pImgData, nWidth*nHeight);
					if (img.GetBPP() == 8)
					{
						//8位灰度图，设置调色板
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
					//保存图片
					long a = img.Save(cDestFileName);
					if(E_OUTOFMEMORY == a)//add by zhuxy
					{
						gLogFile.AddAppRecord(_T("存储异常,内存不够"));
					}
					else if(E_FAIL == a)
					{
						gLogFile.AddAppRecord(_T("存储异常,存储失败"));
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
					//找到图像数据指针的起始地址
					BYTE *p = (BYTE *)img.GetBits() + (img.GetPitch()*(img.GetHeight() - 1));
					//复制数据
					memcpy(p, pJointInfo->pImgData, nWidth*nHeight);
					if (img.GetBPP() == 8)
					{
						//8位灰度图，设置调色板
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
					//保存图片
					long a = img.Save(cDestFileName);
					if(E_OUTOFMEMORY == a)//add by zhuxy
					{
						gLogFile.AddAppRecord(_T("存储异常,内存不够"));
					}
					else if(E_FAIL == a)
					{
						gLogFile.AddAppRecord(_T("存储异常,存储失败"));
					}
				}
				else if (pGlobalParam->m_tCfg.nSaveImageFlag == 3)
				{
					//保存图像jpg bmp
					wchar_t cDestFileName[255] = {0};
					wsprintfW(cDestFileName, _T("%s\\Camera_0%d\\jpg\\%d.jpg"), theApp.m_strCurrentImgSavePath, camCfg.nChannelNum, nImgIndex - i);//add by zhuxy 20190318
					CImage img;
					img.Create(nWidth, nHeight, 8, 0);
					//找到图像数据指针的起始地址
					BYTE *p = (BYTE *)img.GetBits() + (img.GetPitch()*(img.GetHeight() - 1));
					//复制数据
					memcpy(p, pJointInfo->pImgData, nWidth*nHeight);
					if (img.GetBPP() == 8)
					{
						//8位灰度图，设置调色板
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
					//保存图片jpg
					long a = img.Save(cDestFileName);
					if(E_OUTOFMEMORY == a)//add by zhuxy
					{
						gLogFile.AddAppRecord(_T("存储异常,内存不够"));
					}
					else if(E_FAIL == a)
					{
						gLogFile.AddAppRecord(_T("存储异常,存储失败"));
					}
					
					wsprintfW(cDestFileName, _T("%s\\Camera_0%d\\bmp\\%d.bmp"), theApp.m_strCurrentImgSavePath, camCfg.nChannelNum, nImgIndex - i);//add by zhuxy 20190318
				
					//保存图片bmp
					a = img.Save(cDestFileName);
					if(E_OUTOFMEMORY == a)//add by zhuxy
					{
						gLogFile.AddAppRecord(_T("存储异常,内存不够"));
					}
					else if(E_FAIL == a)
					{
						gLogFile.AddAppRecord(_T("存储异常,存储失败"));
					}
				}
				free(pOutBuffer);
				pOutBuffer = NULL;
			}
			else
			{
				CString strInfo;
				strInfo.Format(_T("压缩第%张图片失败"), nImgIndex - i);
				gLogFile.AddCameraRecord(nCamIndex, strInfo);
			}
		}
	}
	catch (...)
	{
		gLogFile.AddAppRecord( _T("DataCompressThread线程异常"));
	}

	/*这里回收内存*/
	delete pJointInfo;//删除整个图片的堆内存，是个结构体
	delete [] pJointBuf;//删除图片数据的内存，是个字节数组

	WaitForSingleObject(pThisObject->m_hCompressThreadCountSemaphore, INFINITE);
	pThisObject->m_nCompressThreadCount--;
	ReleaseSemaphore(pThisObject->m_hCompressThreadCountSemaphore, 1, &lCount);
}


/*
函数：DataCompressMgrThread(void * param)
函数功能：管理压缩线程函数，用于启动设定数量的压缩线程；
参数含义：
param:CImageDataMgr结构体，用于将主线程的数据结构传递进多管理线程
返回值：NULL
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
			TRACE(_T("管理压缩线程函数就绪中:%d"),GetCurrentThreadId());
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

				//压缩线程
				_beginthread( DataCompressThread, 0, pThisObject->m_listJointInfo.GetHead());

				pThisObject->m_listJointInfo.RemoveHead();
			}
			else
			{
				Sleep(100);
			}
			//else if (pThisObject->m_bStopJointFlag)
			//if (0 == pThisObject->m_nJointThreadCount)//fix by zhuxy20190426
			if ((0==pThisObject->m_nJointThreadCount)&&(pThisObject->m_listJointInfo.IsEmpty()))//这样判断否则有的图片包不会被落盘
			{
				break;
			}
		}
	}
	catch (...)
	{
		gLogFile.AddAppRecord( _T("DataCompressMgrThread线程异常"));
	}

	_beginthread( RecordGrabInfoThread, 0, param);//add注释 开启线程写入本次过车情况
	pThisObject->m_bStopCompressMgrFlag = true;
	gLogFile.AddAppRecord(_T("数据压缩管理线程退出"));	

	//add by zhuxy 20190321 在这里结束采集过程
	if(workWay==WAY_AUTO)//自动采集下才在服务器生成对应的过车信息
	{
		CString A_B_Str;
		if (trainNumStr==_T(""))//自动模式下却没有收到过车信息则识别为未知车号
		{
			theApp.m_strTrainId = _T("UnknownTrainId");
			A_B_Str=_T("UnknownDirection");//ab端
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
		CString oldPath = theApp.m_strCurrentImgSavePath;//重命名文件夹,把车号代替时间
		CString newPath = theApp.m_strCurrentImgSavePath.Mid(0,theApp.m_strCurrentImgSavePath.Find(_T("_"))+1)+theApp.m_strTrainId;
		if(1 != GlobalParameter::GetInstance()->m_tCfg.nSpare)//如果是备控则重新设置开始过车时间
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
		CString strLogPath = strPath + _T("过车信息.txt");
		CString tmpStr;

		for (int i =0;i<gCamMgr.m_vCamCtrl.size();i++)
		{
			if(0==GlobalParameter::GetInstance()->m_vImgGrabInfo[i].nGrabNum)
			{
				continue;
			}
			CString tmpPathStr = theApp.m_strCurrentImgSavePath;
			if(0==tmpPathStr.Find(_T("\\")))//如果是服务器的地址则去掉双引号
			{
				tmpPathStr.Replace(_T("\\\\"),_T("\\"));tmpPathStr = _T("\\")+tmpPathStr;
			}
			tmpStr.Format(_T("%s&%s&%s&0%d&%s\\Camera_0%d"),strTime, theApp.m_strTrainId,A_B_Str,i,tmpPathStr,i);//D:\\GrabImg\\201903\\20190318180034_和谐号			
			gLogFile.AddAppRecord(strLogPath, tmpStr);
			CString tempString;

			tempString.Format(_T("相机%d停止采集,图像大小:%dx%d"),i+1,GlobalParameter::GetInstance()->m_vCamCfg[i].nWidth,GlobalParameter::GetInstance()->m_vCamCfg[i].nHeight);
			gLogFile.AddAppRecord(tempString);
		}
	}
	GlobalParameter::GetInstance()->m_tTrainInfo.strTrainId="";//还原初始状态
	GlobalParameter::GetInstance()->m_tTrainInfo.strTrainTime="";
	GlobalParameter::GetInstance()->m_tTrainInfo.workState=STATE_READY;
	trainNumStr = _T("");
	trainTimeStr = _T("");
	workWay = WAY_STOP;
}

void LogEnhance(BYTE* imageData,int nHeight,int nWidth,float fGamma,int nThreshold)
{
	// 由于oldPixel:[1,256],则可以先保存一个查找表
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
	//gLogFile.AddAppRecord(_T("Log增强图像！"));
}


/*
函数：DataJointThread(LPVOID param)
函数功能: 图像拼接线程函数，用于将多个图像拼接成一个图像，为多线程函数；
参数含义：
LPVOID param：用于用于将主线程参数全部传递进拼接线程
返回值：NULL
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
	int nJointIndex = 0; //拼接索引

	GlobalParameter *pGlobalParam = GlobalParameter::GetInstance();
	int nStopWaitTime = pGlobalParam->m_tCfg.nStopGrabWaitTime;
	int nWidth = pGlobalParam->m_vCamCfg[nCamIndex].nWidth;
	int nHeight = pGlobalParam->m_vCamCfg[nCamIndex].nHeight;
	int nJointNum = pGlobalParam->m_vCamCfg[nCamIndex].nJointNum;
	int nJointWidth = nWidth;
	int nJointHeight = nHeight * nJointNum;

	//申请拼接缓存
	BYTE *pJointBuf = new BYTE[nJointWidth * nJointHeight];
	memset(pJointBuf, 0, nJointWidth * nJointHeight);

	try
	{
		while (true)
		{
			bRtn = pThisObject->m_vImgBuf[nCamIndex]->BufferOut((void **)&pOriData);
			if (bRtn)
			{
				//将图像数据存入拼接缓存
				memcpy(pJointBuf + (nJointIndex++ * nWidth * nHeight), pOriData, nWidth * nHeight);

				//图像数量等于拼接数 做增强
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

					//2017 6/16 获取第一列拼接图像lut表
					//if ((nThreadIndex == 0) && ((pGlobalParam->m_vCamCfg[nCamIndex].nEnhanceType == 2) || (pGlobalParam->m_vCamCfg[nCamIndex].nEnhanceType == 3)))
					//{
					//	createlut(pJointBuf, pGlobalParam->m_vCamCfg[nCamIndex].tEnhanceParam.nSaturated, nJointWidth, nJointHeight, true, nCamIndex);
					//}
					////2017 11/16 图像增强（加亮图片）
					if (pGlobalParam->m_vCamCfg[nCamIndex].tEnhanceParam.fLogEnhance > 0)//实际工作中并没有起作用
					{
						LogEnhance(pImgData,
							nJointHeight,
							nJointWidth, pGlobalParam->m_vCamCfg[nCamIndex].tEnhanceParam.fLogEnhance,
							pGlobalParam->m_vCamCfg[nCamIndex].tEnhanceParam.nLogEnhanceThreshold);
					}
#if CLOSE_IF //这里凌云原来的处理方式，会导致压缩数小于采集数
					//解决色差问题
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

#if OPEN_IF//add by zhuxy压缩数等于采集数
					nThreadIndex++;
					nJointIndex = 0;
					pThisObject->m_listJointInfo.AddTail(pJointInfo);
#endif
					memset(pJointBuf, 0, nJointWidth * nJointHeight);
				}
			}
			else
			{
				//if ((!pThisObject->m_bStopGrabFlag) && (nTimeOut < nStopWaitTime))//add注释 如果没有停止和每达到超时等待次数则等待
				if((!pThisObject->stopFlag[nCamIndex]) && (nTimeOut < nStopWaitTime))
				{
					++nTimeOut;
#if CLOSE_IF//是否需要等待一下
					Sleep(100);
#endif
				}
				else
				{
					int nGrabNum = pGlobalParam->m_vImgGrabInfo[nCamIndex].nGrabNum;
					int nJonitNum = nThreadIndex * nJointNum + nJointIndex;

					if (pThisObject->stopFlag[nCamIndex])  //收到停止采集信号
					{
						CString strInfo;
						strInfo.Format(_T("数据拼接线程正常退出: 采集数%d, 拼接数%d, 超时等待次数%d"), nGrabNum, nJonitNum, nTimeOut);
						gLogFile.AddCameraRecord(nCamIndex, strInfo);
					}
					else  //没有收到停止采集信号
					{
						GlobalParameter::GetInstance()->m_tTrainInfo.workState = -1;
						gCamMgr.StopGrab(nCamIndex);

						CString strInfo;
						strInfo.Format(_T("数据拼接线程超时退出: 采集数%d, 拼接数%d, 超时等待次数%d"), nGrabNum, nJonitNum, nTimeOut);
						gLogFile.AddCameraRecord(nCamIndex, strInfo);
					}

					break;
				}
			}
		}

		//增强剩余图片
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
		gLogFile.AddAppRecord(_T("DataJointThread线程异常"));
	}

	delete[] pJointBuf;
	pJointBuf = NULL;
	//pThisObject->m_bStopJointFlag = true;

	WaitForSingleObject(pThisObject->m_hJointThreadCountSemaphore, INFINITE);
	pThisObject->m_nJointThreadCount--;
	ReleaseSemaphore(pThisObject->m_hJointThreadCountSemaphore, 1, &lCount);
}


/*
函数：SendImageThread(LPVOID param)
函数功能: 图像传输线程函数，用于将图像数据分包发送至指定的服务器；
参数含义：
LPVOID param：用于用于将主线程参数全部传递发送接线程
返回值：NULL
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
		//char buf[1000000];    //图像不能超过1兆
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
			wsprintfW(strError, _T("无法建立通讯,请与开发商联系，错误代码：%d\n"), WSAGetLastError());
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
			wsprintfW(strError, _T("无法连接到服务器，请检查网络，错误代码：%d\n"), WSAGetLastError());
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

			wsprintfW(strError, _T("无法建立事件对象,请与开发商联系，错误代码: %x\n"), WSAGetLastError());
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

			// 开始传输
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

						//pThisObject->m_pJPGBuffer[pThisObject->m_nFirstJPG] = pData; // FreeJPGData()函数里delete
						//pThisObject->m_nSizeJPGBuffer[pThisObject->m_nFirstJPG] = nSizeTemp;
						//pThisObject->m_nIndexJPG[pThisObject->m_nFirstJPG] = nIndex;
						//pThisObject->m_nCameraIndexJPG[pThisObject->m_nFirstJPG] = nCameraIndex;
						//pThisObject->m_nCameraIndexInConf[pThisObject->m_nFirstJPG] = nCameraIndexInConf;
						//pThisObject->m_nFirstJPG = (pThisObject->m_nFirstJPG+1)%BUFFERSIZEFORJPG;

						pUploadParam uploadParam = new UploadParam();
						uploadParam->pJPGBuffer = pData; // FreeJPGData()函数里delete
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
				    strInfo.Format(_T("图像索引:%d, 连接超时,请检查网络 -1"), nIndex);
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

						//pThisObject->m_pJPGBuffer[pThisObject->m_nFirstJPG] = pData; // FreeJPGData()函数里delete
						//pThisObject->m_nSizeJPGBuffer[pThisObject->m_nFirstJPG] = nSizeTemp;
						//pThisObject->m_nIndexJPG[pThisObject->m_nFirstJPG] = nIndex;
						//pThisObject->m_nCameraIndexJPG[pThisObject->m_nFirstJPG] = nCameraIndex;
						//pThisObject->m_nCameraIndexInConf[pThisObject->m_nFirstJPG] = nCameraIndexInConf;
						//pThisObject->m_nFirstJPG = (pThisObject->m_nFirstJPG+1)%BUFFERSIZEFORJPG;

						pUploadParam uploadParam = new UploadParam();
						uploadParam->pJPGBuffer = pData; // FreeJPGData()函数里delete
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
					strInfo.Format(_T("图像索引:%d, 连接超时,请检查网络 -2"), nIndex);
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

		// 释放网络资源
		WSACloseEvent(EventArray[0]);
		closesocket(RecvSocket);	

		free(buf);
	}
	catch (...)
	{		
		gLogFile.AddAppRecord(_T("SendImageThread线程异常"));
	}

	WaitForSingleObject(pThisObject->m_hSendThreadCountSemaphore,INFINITE);
	pThisObject->m_nSendThreadCount--;
	ReleaseSemaphore(pThisObject->m_hSendThreadCountSemaphore,1,&count);

	gLogFile.AddAppRecord(_T("发送线程退出"));
}


/*
函数：SendImageMgrThread(LPVOID param)
函数功能: 管理图像发送线程函数，用于启动指定数量的图像传输线程；
参数含义：
LPVOID param：用于用于将主线程参数全部传递发送接线程
返回值：NULL
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
				gLogFile.AddAppRecord(_T("传输管理线程超时退出"));
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
				gLogFile.AddAppRecord(_T("传输管理线程完毕退出"));
				break;
			}

			Sleep(200);
		}
	}
	catch(...)
	{
		gLogFile.AddAppRecord(_T("SendImageMgrThread线程异常"));
	}

	pThisObject->m_bStopSendMgrFlag = true;
}


/*
函数：GetJPGData(BYTE ** pData,int *nIndex,int *nCameraIndex,int *nCameraIndexInConf,int nTimeOut)
函数功能: 从压缩缓存队列中获取压缩的数据；
参数含义：
BYTE ** pData：压缩的图像数据
int *nIndex：图像在队列中编号
int *nCameraIndex：相机编号
int *nCameraIndexInConf:相机在配置文件中的编号
int nTimeOut ：设置获取压缩数据等待超时时间
返回值：NULL
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
		gLogFile.AddAppRecord(_T("CImageCompresser::GetJPGData 函数异常"));
	}

	return nSize;
}


/*
函数：RemoveOverdueFile(LPVOID pParam)
函数功能: 遍历保存路径内的村塾数据，找到保存路径内超过保存时长的数据并删除；
参数含义：
LPVOID param：用于用于将主线程参数全部传递发送接线程
返回值：NULL
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
		{ //找到目录			
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

	gLogFile.AddAppRecord(_T("删除图像"));
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
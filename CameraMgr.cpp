#include "StdAfx.h"
#include "ImageGrab.h"
#include "CameraMgr.h"


CCameraMgr::CCameraMgr(void)
{

}

CCameraMgr::~CCameraMgr(void)
{

}

//函数名称:初始化,为所有摄像机分配内存
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void CCameraMgr::Init()
{
	int nCamNum = GlobalParameter::GetInstance()->m_tCfg.nCamNum;
	theApp.m_strCamName = "";
	for (int i = 0; i < nCamNum; i++)
	{
		CCameraCtrl *pCamCtrl = NULL;
		if (GlobalParameter::GetInstance()->m_vCamCfg[i].nGrabDevice == 0)
		{
//			pCamCtrl = new CRacerAreaCtrl();
			pCamCtrl = new CRacerCtrl();
			gLogFile.AddAppRecord(_T("为Racer相机分配内存"));
		}
		else if(GlobalParameter::GetInstance()->m_vCamCfg[i].nGrabDevice == 1)
		{
			pCamCtrl = new CLineaCtrl();
			gLogFile.AddAppRecord(_T("为Linea相机分配内存"));
		}
		else if(GlobalParameter::GetInstance()->m_vCamCfg[i].nGrabDevice == 2)
		{
			//pCamCtrl = new CSpringCtrl();
		}
		else if(GlobalParameter::GetInstance()->m_vCamCfg[i].nGrabDevice == 3) 
		{
			//pCamCtrl = new CP4Ctrl();
		}
		if (pCamCtrl==NULL)//add by zhuxy,如果配置文件没有上述几个选项,这里会产生野指针
		{
			wchar_t strError[255];
			wsprintfW(strError, _T("相机内存分配失败"));
			gLogFile.AddAppRecord(strError);
			continue;
		}
		pCamCtrl->Init(i);
		m_vCamCtrl.push_back(pCamCtrl);
		CString str(wstring2string(GlobalParameter::GetInstance()->m_vCamCfg[i].strCamName).c_str());
		theApp.m_strCamName += str;
		theApp.m_strCamName += "_";
	}
}

//函数名称:回收每个摄像机的内存
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void CCameraMgr::Destory()
{
	vector<CCameraCtrl*>::iterator iter = m_vCamCtrl.begin();
	while (iter != m_vCamCtrl.end())
	{
		CCameraCtrl *pCamCtrl = *iter;
		pCamCtrl->Destory();
		delete pCamCtrl;
		pCamCtrl = NULL;

		iter = m_vCamCtrl.erase(iter);
	}
}

//函数名称:准备采集，将每个摄像机配置结构体置为零
//函数作用:NULL
//函数参数:index参数来表明初始化某一路的摄像头 从1开始 0表示全初始化
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void CCameraMgr::PrepareGrab(int index/* =0 */)//
{
#if OPEN_IF //add
	if(0!=index)
	{
		index -=1;
		GlobalParameter *pGlobalParam = GlobalParameter::GetInstance();
		pGlobalParam->m_vImgGrabInfo[index].nGrabNum = 0;
		pGlobalParam->m_vImgGrabInfo[index].nSaveNum = 0;
		pGlobalParam->m_vImgGrabInfo[index].nCompressNum = 0;
		pGlobalParam->m_vImgGrabInfo[index].nLostNum = 0;
		pGlobalParam->m_vImgGrabInfo[index].nUploadNum = 0;
		return;
	}
#endif
	int nCamNum = m_vCamCtrl.size();
	for (int i=0; i<nCamNum; i++)
	{
		GlobalParameter *pGlobalParam = GlobalParameter::GetInstance();
		pGlobalParam->m_vImgGrabInfo[i].nGrabNum = 0;
		pGlobalParam->m_vImgGrabInfo[i].nSaveNum = 0;
		pGlobalParam->m_vImgGrabInfo[i].nCompressNum = 0;
		pGlobalParam->m_vImgGrabInfo[i].nLostNum = 0;
		pGlobalParam->m_vImgGrabInfo[i].nUploadNum = 0;
	}
}

//函数名称:开始采集
//函数作用:NULL
//函数参数:index参数来表明开始采集某一路的摄像头 从1开始 0表示全采集
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void CCameraMgr::StartGrab(int index/* =0 */)
{
	if(0!=index)
	{
		m_vCamCtrl[index-1]->StartGrab();
	}
	else//add 
	{
		int nCamNum = m_vCamCtrl.size();
		for (int i=0; i<nCamNum; i++)
		{
			m_vCamCtrl[i]->StartGrab();
		}
	}
	//add by zhuxy 状态设置
	for (int i=0; i<m_vCamCtrl.size(); i++)
	{
		if(CAM_STATUS_RUNNING == gCamMgr.m_vCamCtrl[i]->m_nCamStatus)
		{
			GlobalParameter::GetInstance()->m_tTrainInfo.workState = STATE_BUSY;
			return;
		}
	}
}

//函数名称:停止采集
//函数作用:NULL
//函数参数:index参数来表明开始采集某一路的摄像头 从1开始 0表示全采集
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void CCameraMgr::StopGrab()
{
#if CLOSE_IF //add
	if(0!=index)
	{
		m_vCamCtrl[index-1]->StopGrab();
		return;
	}
#endif
	for (int i=0; i<m_vCamCtrl.size(); i++)
	{
		m_vCamCtrl[i]->StopGrab();
	}
	GlobalParameter::GetInstance()->m_tTrainInfo.workState = STATE_READY;//add by zhuxy 状态设置
}

#if OPEN_IF 
//函数名称:停止采集指定通道的相机
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void CCameraMgr::StopGrab(int nCamCtrl)
{
	if ((nCamCtrl>-1) && (nCamCtrl<m_vCamCtrl.size()))
	{
		m_vCamCtrl[nCamCtrl]->StopGrab();
	}
	int nCamNum = m_vCamCtrl.size();//add by zhuxy 状态设置
	for (int i=0; i<nCamNum; i++)
	{
		if(CAM_STATUS_RUNNING == gCamMgr.m_vCamCtrl[i]->m_nCamStatus)
		{
			GlobalParameter::GetInstance()->m_tTrainInfo.workState = STATE_READY;
			return;
		}
	}
	
}
#endif

//函数名称:检查相机状态
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void CCameraMgr::CheckCamStatus()
{
	int nCamNum = m_vCamCtrl.size();
	for (int i=0; i<nCamNum; i++)
	{
		m_vCamCtrl[i]->CheckCamStatus();
	}
}

//函数名称:检查指定相机状态
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void CCameraMgr::CheckCamStatus(int nCamCtrl)
{
	if ((nCamCtrl>-1) && (nCamCtrl<m_vCamCtrl.size()))
	{
		m_vCamCtrl[nCamCtrl]->CheckCamStatus();
	}
}

//函数名称:设置所有摄像机的参数
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void CCameraMgr::SetCameraParam()
{
	int nCamNum = m_vCamCtrl.size();
	for (int i = 0; i < nCamNum; i++)
	{
		CString strFilePath;
		strFilePath.Format(_T("%s\\config\\camera%d.txt"), theApp.m_strCurrentPath, i);

		CStdioFile stdFile;
		CFileException fileExp;

		if(stdFile.Open(strFilePath, CFile::modeRead, &fileExp))
		{
	        CString strLineData;
			CTime curTime = CTime::GetCurrentTime();
			int nCurHour = curTime.GetHour();
			stdFile.SeekToBegin();

			while(stdFile.ReadString(strLineData))//nFirstHour nLastHour nExposureTime nGain
			{
				int nIndex = strLineData.Find(_T(","));
				int nFirstHour = _wtoi(strLineData.Left(nIndex));

				strLineData = strLineData.Mid(nIndex + 1, strLineData.GetLength());
				nIndex = strLineData.Find(_T(","));
				int nLastHour = _wtoi(strLineData.Left(nIndex));

				strLineData = strLineData.Mid(nIndex + 1, strLineData.GetLength());
				nIndex = strLineData.Find(_T(","));
				int nExposureTime = _wtoi(strLineData.Left(nIndex));

				int nGain = _wtoi(strLineData.Mid(nIndex + 1, strLineData.GetLength()));

				if ((nFirstHour <= nCurHour) && (nLastHour > nCurHour))//根据当前小时设置曝光时间和增益
				{
					if ((GlobalParameter::GetInstance()->m_vCamCfg[i].nExposureTime == nExposureTime))
					{
						break;
					}

					m_vCamCtrl[i]->SetCamExposure(nExposureTime);
					m_vCamCtrl[i]->SetCamGain(nGain);
					GlobalParameter::GetInstance()->m_vCamCfg[i].nExposureTime = nExposureTime;
					GlobalParameter::GetInstance()->m_vCamCfg[i].nGain = nGain;

					CString strLog;
					strLog.Format(_T("分时设置相机参数,当前曝光：%d,增益：%d"), nExposureTime, nGain);
					gLogFile.AddCameraRecord(i, strLog);

					break;
				}
			}
		}
		else
		{
			CString strLog;
			strLog.Format(_T("读取%s文件异常,错误码:%d"), strFilePath, fileExp.m_cause);
			gLogFile.AddCameraRecord(i, strLog);
			continue;
		}

		stdFile.Close();
	}
}
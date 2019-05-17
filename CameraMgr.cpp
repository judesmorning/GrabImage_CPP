#include "StdAfx.h"
#include "ImageGrab.h"
#include "CameraMgr.h"


CCameraMgr::CCameraMgr(void)
{

}

CCameraMgr::~CCameraMgr(void)
{

}

//��������:��ʼ��,Ϊ��������������ڴ�
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
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
			gLogFile.AddAppRecord(_T("ΪRacer��������ڴ�"));
		}
		else if(GlobalParameter::GetInstance()->m_vCamCfg[i].nGrabDevice == 1)
		{
			pCamCtrl = new CLineaCtrl();
			gLogFile.AddAppRecord(_T("ΪLinea��������ڴ�"));
		}
		else if(GlobalParameter::GetInstance()->m_vCamCfg[i].nGrabDevice == 2)
		{
			//pCamCtrl = new CSpringCtrl();
		}
		else if(GlobalParameter::GetInstance()->m_vCamCfg[i].nGrabDevice == 3) 
		{
			//pCamCtrl = new CP4Ctrl();
		}
		if (pCamCtrl==NULL)//add by zhuxy,��������ļ�û����������ѡ��,��������Ұָ��
		{
			wchar_t strError[255];
			wsprintfW(strError, _T("����ڴ����ʧ��"));
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

//��������:����ÿ����������ڴ�
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
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

//��������:׼���ɼ�����ÿ����������ýṹ����Ϊ��
//��������:NULL
//��������:index������������ʼ��ĳһ·������ͷ ��1��ʼ 0��ʾȫ��ʼ��
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
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

//��������:��ʼ�ɼ�
//��������:NULL
//��������:index������������ʼ�ɼ�ĳһ·������ͷ ��1��ʼ 0��ʾȫ�ɼ�
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
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
	//add by zhuxy ״̬����
	for (int i=0; i<m_vCamCtrl.size(); i++)
	{
		if(CAM_STATUS_RUNNING == gCamMgr.m_vCamCtrl[i]->m_nCamStatus)
		{
			GlobalParameter::GetInstance()->m_tTrainInfo.workState = STATE_BUSY;
			return;
		}
	}
}

//��������:ֹͣ�ɼ�
//��������:NULL
//��������:index������������ʼ�ɼ�ĳһ·������ͷ ��1��ʼ 0��ʾȫ�ɼ�
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
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
	GlobalParameter::GetInstance()->m_tTrainInfo.workState = STATE_READY;//add by zhuxy ״̬����
}

#if OPEN_IF 
//��������:ֹͣ�ɼ�ָ��ͨ�������
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void CCameraMgr::StopGrab(int nCamCtrl)
{
	if ((nCamCtrl>-1) && (nCamCtrl<m_vCamCtrl.size()))
	{
		m_vCamCtrl[nCamCtrl]->StopGrab();
	}
	int nCamNum = m_vCamCtrl.size();//add by zhuxy ״̬����
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

//��������:������״̬
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void CCameraMgr::CheckCamStatus()
{
	int nCamNum = m_vCamCtrl.size();
	for (int i=0; i<nCamNum; i++)
	{
		m_vCamCtrl[i]->CheckCamStatus();
	}
}

//��������:���ָ�����״̬
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void CCameraMgr::CheckCamStatus(int nCamCtrl)
{
	if ((nCamCtrl>-1) && (nCamCtrl<m_vCamCtrl.size()))
	{
		m_vCamCtrl[nCamCtrl]->CheckCamStatus();
	}
}

//��������:��������������Ĳ���
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
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

				if ((nFirstHour <= nCurHour) && (nLastHour > nCurHour))//���ݵ�ǰСʱ�����ع�ʱ�������
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
					strLog.Format(_T("��ʱ�����������,��ǰ�ع⣺%d,���棺%d"), nExposureTime, nGain);
					gLogFile.AddCameraRecord(i, strLog);

					break;
				}
			}
		}
		else
		{
			CString strLog;
			strLog.Format(_T("��ȡ%s�ļ��쳣,������:%d"), strFilePath, fileExp.m_cause);
			gLogFile.AddCameraRecord(i, strLog);
			continue;
		}

		stdFile.Close();
	}
}
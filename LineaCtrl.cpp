#include "StdAfx.h"
#include "ImageGrab.h"
#include "LineaCtrl.h"

CLineaCtrl::CLineaCtrl(void)
{
	m_pAcqDevice = NULL;
	m_pBuffers = NULL;
	m_pXfer = NULL;
}

CLineaCtrl::~CLineaCtrl(void)
{
}

bool CLineaCtrl::Init()
{
	ConnectCamera();
	if (m_nCamStatus == CAM_STATUS_OPEN_SUCCEED)  //�����������
	{
		SetCamTrigger(GlobalParameter::GetInstance()->m_vCamCfg[m_nCamIndex].nTriggerMode);
		SetCamExposure(GlobalParameter::GetInstance()->m_vCamCfg[m_nCamIndex].nExposureTime);
		SetCamGain(GlobalParameter::GetInstance()->m_vCamCfg[m_nCamIndex].nGain);
	    //SetCamGrabSize(m_nWidth, m_nHeight);
	}

	return true;
}

bool CLineaCtrl::Init(int nIndex)
{
	m_nCamIndex = nIndex;

	m_nWidth = GlobalParameter::GetInstance()->m_vCamCfg[m_nCamIndex].nWidth;
	m_nHeight = GlobalParameter::GetInstance()->m_vCamCfg[m_nCamIndex].nHeight;

	ConnectCamera();

	if (m_nCamStatus == CAM_STATUS_OPEN_SUCCEED)  //�����������
	{
		SetCamTrigger(GlobalParameter::GetInstance()->m_vCamCfg[m_nCamIndex].nTriggerMode);
		SetCamExposure(GlobalParameter::GetInstance()->m_vCamCfg[m_nCamIndex].nExposureTime);
		SetCamGain(GlobalParameter::GetInstance()->m_vCamCfg[m_nCamIndex].nGain);
	    //SetCamGrabSize(m_nWidth, m_nHeight);
	}

	return 1;
}

void CLineaCtrl::Destory()
{
	gLogFile.AddCameraRecord(m_nCamIndex, _T("���������Դ"));
	DestroyObjects();
}

bool CLineaCtrl::PrepareGrab()
{
	return 1;
}

bool CLineaCtrl::StartGrab()//�ڶ�������ʼ�ɼ�
{
	gLogFile.AddCameraRecord(m_nCamIndex, _T("��ʼ�ɼ�"));
    
	try
	{
		if (m_pXfer && m_pXfer->IsConnected())
		{
			m_pXfer->Grab();
			m_nCamStatus = CAM_STATUS_RUNNING;
		}
		else
		{
			m_nCamStatus = CAM_STATUS_OPEN_FAILED;
			gLogFile.AddCameraRecord(m_nCamIndex, _T("�����ʧ��"));
		}
	}
	catch (...)
	{
		m_nCamStatus = CAM_STATUS_UNKNOWN;
		gLogFile.AddCameraRecord(m_nCamIndex, _T("CLineaCtrl::StartGrab�����쳣"));
	}

	return 1;
}

bool CLineaCtrl::StopGrab()//��������ֹͣ�ɼ�
{
	gLogFile.AddCameraRecord(m_nCamIndex, _T("ֹͣ�ɼ�"));

	try
	{
		if (m_pXfer)
		{
			if(m_pXfer->IsGrabbing())
			{
				m_pXfer->Freeze();
				if (m_pXfer->Wait(5000))
				{
					m_nCamStatus = CAM_STATUS_OPEN_SUCCEED;
				}
				else
				{
					m_pXfer->Abort();
					m_nCamStatus = CAM_STATUS_UNKNOWN;
					gLogFile.AddCameraRecord(m_nCamIndex, _T("���ֹͣץȡʧ��"));
				}
			}
			else 
			{
				//gLogFile.AddCameraRecord(m_nCamIndex, _T("���ֹͣץȡʧ��"));
				if (m_pXfer->IsConnected())
				{
					m_nCamStatus = CAM_STATUS_OPEN_SUCCEED;
				}
				else
				{
					m_nCamStatus = CAM_STATUS_OPEN_FAILED;
				}
			}			
		}
		else
		{
			m_nCamStatus = CAM_STATUS_OPEN_FAILED;
		}
	}
	catch (...)
	{
		m_nCamStatus = CAM_STATUS_UNKNOWN;
		gLogFile.AddCameraRecord(m_nCamIndex, _T("CLineaCtrl::StopGrab�����쳣"));
	}

	return 1;
}

void CLineaCtrl::SetCamTrigger(int nParam)//���ô�����ʽ
{
	if (m_pAcqDevice && *m_pAcqDevice)
	{
		bool bRtn = m_pAcqDevice->SetFeatureValue("TriggerMode", nParam);
		if (bRtn)
		{
			gLogFile.AddCameraRecord(m_nCamIndex, _T("����ģʽ���óɹ�"));
		}
		else
		{
			gLogFile.AddCameraRecord(m_nCamIndex, _T("����ģʽ����ʧ��"));
		}
	}
}

void CLineaCtrl::SetCamExposure(int nParam)//�����ع�ʱ��
{
	if (m_pAcqDevice && *m_pAcqDevice)
	{
		bool bRtn = m_pAcqDevice->SetFeatureValue("ExposureTime", (double)nParam);
		if (bRtn)
		{
			gLogFile.AddCameraRecord(m_nCamIndex, _T("�ع����óɹ�"));
		}
		else
		{
			gLogFile.AddCameraRecord(m_nCamIndex, _T("�ع�����ʧ��"));
		}
	}
}

void CLineaCtrl::SetCamGain(int nParam)//��������
{
	if (m_pAcqDevice && *m_pAcqDevice)
	{
		bool bRtn = m_pAcqDevice->SetFeatureValue("Gain", (double)nParam);
		if (bRtn)
		{
			gLogFile.AddCameraRecord(m_nCamIndex, _T("�������óɹ�"));
		}
		else
		{
			gLogFile.AddCameraRecord(m_nCamIndex, _T("��������ʧ��"));
		}
	}
}

void CLineaCtrl::SetCamGrabSize(int nWidth, int nHeight)//���ô�С
{
	if (m_pAcqDevice && *m_pAcqDevice)
	{
		m_pAcqDevice->SetFeatureValue("Width", (INT64)nWidth);
		m_pAcqDevice->SetFeatureValue("Height", (INT64)nHeight);
	}
}

void CLineaCtrl::SetCamHorzFlip(const int& param)//add by zhuxy20190326���þ���
{
	if (m_pAcqDevice && *m_pAcqDevice)
	{
		m_pAcqDevice->SetFeatureValue("ReverseX", param);
	}
}

void CLineaCtrl::SetCamLineSpeed(int nParam)//add by zhuxy20190404������Ƶ
{
	if (m_pAcqDevice && *m_pAcqDevice)
	{
		m_pAcqDevice->SetFeatureValue("AcquisitionLineRate", nParam);
	}
}

void LineaXferCallback(SapXferCallbackInfo *pInfo)
{
	CLineaCtrl *pCamCtrl = (CLineaCtrl *)pInfo->GetContext();

	// If grabbing in trash buffer, do not display the image, update the
	// appropriate number of frames on the status bar instead
	if (!pInfo->IsTrash())
	{
		BYTE *pData;
		pCamCtrl->m_pBuffers->GetAddress((void **)&pData);

		if (gDataMgr.m_vImgBuf[pCamCtrl->m_nCamIndex]->BufferIn(pData, pCamCtrl->m_nCamIndex, pCamCtrl->m_nWidth, pCamCtrl->m_nHeight))//��ͼƬд����CImageDataMgr
		{
            GlobalParameter::GetInstance()->m_vImgGrabInfo[pCamCtrl->m_nCamIndex].nGrabNum++;
		}
		else
		{
			GlobalParameter::GetInstance()->m_vImgGrabInfo[pCamCtrl->m_nCamIndex].nLostNum++;
		}

		pCamCtrl->m_pBuffers->ReleaseAddress((void *)pData);
		pCamCtrl->m_pBuffers->Clear(pCamCtrl->m_pBuffers->GetIndex());
	}
	else
	{
		CString strInfo;
		strInfo.Format(_T("Frames acquired in trash buffer: %d"), pInfo->GetEventCount());
		gLogFile.AddCameraRecord(pCamCtrl->m_nCamIndex, strInfo);
	}
}

bool CLineaCtrl::ConnectCamera()//��һ��������������������ڴ�
{
	//gLogFile.AddCameraRecord(m_nCamIndex, _T("�����������"));

	char deviceUserID[50];
	int nServerCount = SapManager::GetServerCount();//��ȡ�ܵķ�����������һ��Ϊ1
	for (int serverIndex = 0; serverIndex < nServerCount; serverIndex++)
	{
        int nResourceCount = SapManager::GetResourceCount(serverIndex, SapManager::ResourceAcqDevice);//��ȡÿ�����������ж��ٸ����
		for (int resourecIndex = 0; resourecIndex < nResourceCount; resourecIndex++)
		{
			if (SapManager::IsResourceAvailable(serverIndex, SapManager::ResourceAcqDevice, resourecIndex) == FALSE)
			{
				CString str;str.Format(_T("����쳣,��ǰ������û���ҵ����,�����:%d"),resourecIndex+1);
#if CLOSE_IF
				gLogFile.AddAppRecord(str);
#endif
				continue;
			}

			SapManager::SetDisplayStatusMode(SapManager::StatusCustom);
	
			SapLocation location(serverIndex, resourecIndex);
			m_pAcqDevice = new SapAcqDevice(location);
			m_pAcqDevice->Create();
			m_pAcqDevice->GetFeatureValue("DeviceUserID", deviceUserID, sizeof(deviceUserID));

			if (strcmp(wstring2string(GlobalParameter::GetInstance()->m_vCamCfg[m_nCamIndex].strCamName).c_str(), deviceUserID) == 0)//����������ļ����ҵ��˶�Ӧ���豸name������ڴ棬�������
			{
				goto FindDevice;
			}
			else
			{
				m_pAcqDevice->Destroy();
				delete m_pAcqDevice;
				m_pAcqDevice = NULL;
			}
		}
	}

	m_nCamStatus = CAM_STATUS_NOT_FIND_DEVICE;
   
FindDevice:

	if (m_pAcqDevice)
	{
		m_pBuffers	= new SapBufferWithTrash(10, m_pAcqDevice);
		m_pXfer		= new SapAcqDeviceToBuf(m_pAcqDevice, m_pBuffers, LineaXferCallback, this);

		CreateObjects();
	}

	return 1;
}

void CLineaCtrl::CheckCamStatus()
{
	char deviceUserID[50] = {0};
	bool bRtn = false;

	if (m_pAcqDevice == NULL)
	{
		ConnectCamera();
		return ;
	}
	bRtn = m_pAcqDevice->GetFeatureValue("DeviceUserID", deviceUserID, sizeof(deviceUserID));
	if (!bRtn)
	{
		m_nCamStatus = CAM_STATUS_OPEN_FAILED;
	}
	if((m_nCamStatus != CAM_STATUS_OPEN_SUCCEED) && (m_nCamStatus != CAM_STATUS_RUNNING))
	{
		Destory();
		Init();
	}
}

BOOL CLineaCtrl::CreateObjects()
{
	//// Create acquisition object
	//if (m_pAcqDevice && !*m_pAcqDevice && !m_pAcqDevice->Create())
	//{
	//	DestroyObjects();
	//	return FALSE;
	//}

	// Create buffer object
	if (m_pBuffers && !*m_pBuffers)
	{
		if( !m_pBuffers->Create())
		{
			m_nCamStatus = CAM_STATUS_UNKNOWN;
			DestroyObjects();
			return FALSE;
		}
		// Clear all buffers
		m_pBuffers->Clear();
	}

	// Set next empty with trash cycle mode for transfer
	if (m_pXfer && m_pXfer->GetPair(0))
	{
		if (!m_pXfer->GetPair(0)->SetCycleMode(SapXferPair::CycleNextWithTrash))
		{
			DestroyObjects();
			return FALSE;
		}
	}

	// Create transfer object
	if (m_pXfer && !*m_pXfer && !m_pXfer->Create())
	{
		m_nCamStatus = CAM_STATUS_OPEN_FAILED;
		DestroyObjects();
		return FALSE;
	}

	m_nCamStatus = CAM_STATUS_OPEN_SUCCEED;

	return TRUE;
}

BOOL CLineaCtrl::DestroyObjects()
{
	// Destroy transfer object
	if (m_pXfer && *m_pXfer) 
	{
		m_pXfer->Destroy();
		delete m_pXfer;
	    m_pXfer = NULL;
	}

	// Destroy buffer object
	if (m_pBuffers && *m_pBuffers)
	{
		m_pBuffers->Destroy();
		delete m_pBuffers;
		m_pBuffers = NULL;
	}

	// Destroy acquisition object
	if (m_pAcqDevice && *m_pAcqDevice)
	{
		m_pAcqDevice->Destroy();
		delete m_pAcqDevice;
		m_pAcqDevice = NULL;
	}

	return TRUE;
}

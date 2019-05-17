#include "StdAfx.h"
#include "ImageGrab.h"
#include "P4Ctrl.h"

CP4Ctrl::CP4Ctrl(void)
{
	m_pAcq = NULL;
	m_pBuffers = NULL;
	m_pXfer = NULL;
}

CP4Ctrl::~CP4Ctrl(void)
{
}

bool CP4Ctrl::Init()
{
	ConnectCamera();
	if (m_nCamStatus == CAM_STATUS_OPEN_SUCCEED)  //设置相机参数
	{
		SetCamTrigger(GlobalParameter::GetInstance()->m_vCamCfg[m_nCamIndex].nTriggerMode);
		SetCamExposure(GlobalParameter::GetInstance()->m_vCamCfg[m_nCamIndex].nExposureTime);
		SetCamGain(GlobalParameter::GetInstance()->m_vCamCfg[m_nCamIndex].nGain);
	}

	return true;
}

bool CP4Ctrl::Init(int nIndex)
{
	m_nCamIndex = nIndex;

	GlobalParameter *pGloabParam = GlobalParameter::GetInstance();
	m_nWidth = pGloabParam->m_vCamCfg[m_nCamIndex].nWidth;
	m_nHeight = pGloabParam->m_vCamCfg[m_nCamIndex].nHeight;

	ConnectCamera();

	if (m_nCamStatus == CAM_STATUS_OPEN_SUCCEED)  //设置相机参数
	{
		//SetCamTrigger(pGloabParam->m_vCamCfg[m_nCamIndex].nTriggerMode);
		//SetCamExposure(pGloabParam->m_vCamCfg[m_nCamIndex].nExposureTime);
		//SetCamGain(pGloabParam->m_vCamCfg[m_nCamIndex].nGain);
		//SetCamGrabSize(m_nWidth, m_nHeight);
	}

	return 1;
}

void CP4Ctrl::Destory()
{
	gLogFile.AddCameraRecord(m_nCamIndex, _T("销毁相机资源"));
	m_nCamStatus = CAM_STATUS_OPEN_DECONNECT;
	DestroyObjects();
}

bool CP4Ctrl::PrepareGrab()
{
	return 1;
}

bool CP4Ctrl::StartGrab()
{
	gLogFile.AddCameraRecord(m_nCamIndex, _T("开始采集"));

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
			gLogFile.AddCameraRecord(m_nCamIndex, _T("相机打开失败"));
		}
	}
	catch (...)
	{
		m_nCamStatus = CAM_STATUS_UNKNOWN;
		gLogFile.AddCameraRecord(m_nCamIndex, _T("CP4Ctrl::StartGrab函数异常"));
	}

	return 1;
}

bool CP4Ctrl::StopGrab()
{
	gLogFile.AddCameraRecord(m_nCamIndex, _T("停止采集"));

	try
	{
		if (m_pXfer)
		{
			if (m_pXfer->Freeze())
			{
				m_pXfer->Wait(0);
				m_pXfer->Abort();
				m_nCamStatus = CAM_STATUS_OPEN_SUCCEED;
			}
			else
			{
				gLogFile.AddCameraRecord(m_nCamIndex, _T("相机停止抓取失败"));
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
		gLogFile.AddCameraRecord(m_nCamIndex, _T("CP4Ctrl::StopGrab函数异常"));
	}

	return 1;
}

void CP4Ctrl::SetCamTrigger(int nParam)
{
}

void CP4Ctrl::SetCamExposure(int nParam)
{
}

void CP4Ctrl::SetCamGain(int nParam)
{
}

void CP4Ctrl::SetCamGrabSize(int nWidth, int nHeight)
{
}

void P4XferCallback(SapXferCallbackInfo *pInfo)
{
	CP4Ctrl *pCamCtrl = (CP4Ctrl *)pInfo->GetContext();

	// If grabbing in trash buffer, do not display the image, update the
	// appropriate number of frames on the status bar instead
	if (!pInfo->IsTrash())
	{
		BYTE *pData;
		pCamCtrl->m_pBuffers->GetAddress((void **)&pData);

		if (gDataMgr.m_vImgBuf[pCamCtrl->m_nCamIndex]->BufferIn(pData, pCamCtrl->m_nCamIndex, pCamCtrl->m_nWidth, pCamCtrl->m_nHeight))
		{
			GlobalParameter::GetInstance()->m_vImgGrabInfo[pCamCtrl->m_nCamIndex].nGrabNum++;
		}
		else
		{
			GlobalParameter::GetInstance()->m_vImgGrabInfo[pCamCtrl->m_nCamIndex].nLostNum++;
		}

		pCamCtrl->m_pBuffers->ReleaseAddress((void *)pData);
		//pCamCtrl->m_pBuffers->Clear(pCamCtrl->m_pBuffers->GetIndex());
	}
	else
	{
		CString strInfo;
		strInfo.Format(_T("Frames acquired in trash buffer: %d"), pInfo->GetEventCount());
		gLogFile.AddCameraRecord(pCamCtrl->m_nCamIndex, strInfo);
	}
}

bool CP4Ctrl::ConnectCamera()
{
	gLogFile.AddCameraRecord(m_nCamIndex, _T("启动相机连接"));

	char  acqServerName[256];
	GlobalParameter *pGloabParam = GlobalParameter::GetInstance();

	SapManager::SetDisplayStatusMode(SapManager::StatusCustom);
	BOOL bRtn = SapManager::GetServerName(pGloabParam->m_vCamCfg[m_nCamIndex].nDeviceIndex, acqServerName, 256);
	if (!bRtn)
	{
		m_nCamStatus = CAM_STATUS_NOT_FIND_DEVICE;
		return false;
	}

	SapLocation location(acqServerName, 0);

	m_pAcq = new SapAcquisition(location, wstring2string(pGloabParam->m_vCamCfg[m_nCamIndex].strCameraCfgPath).c_str());
	m_pBuffers	= new SapBufferWithTrash(10, m_pAcq);
	m_pXfer		= new SapAcqToBuf(m_pAcq, m_pBuffers, P4XferCallback, this);

	CreateObjects();

	return true;
}

void CP4Ctrl::CheckCamStatus()
{
	bool bRtn = false;

	if(NULL == m_pAcq)
	{
		Init();
		return ;
	}
	bRtn = m_pAcqDevice->GetFeatureValue("DeviceUserID", deviceUserID, sizeof(deviceUserID));
	if (bRtn)
	{
		m_nCamStatus = CAM_STATUS_OPEN_SUCCEED;
	}
	else
	{
		m_nCamStatus = CAM_STATUS_OPEN_FAILED;
	}

	if (CAM_STATUS_OPEN_SUCCEED != m_nCamStatus)
	{
		Destory();
		Init();
	}
}

BOOL CP4Ctrl::CreateObjects()
{
	// Create acquisition object
	if (m_pAcq && !*m_pAcq && !m_pAcq->Create())
	{
		m_nCamStatus = CAM_STATUS_OPEN_FAILED;
		gLogFile.AddCameraRecord(m_nCamIndex, _T("m_pAcq创建失败"));
		DestroyObjects();
		return FALSE;
	}

	// Create buffer object
	if (m_pBuffers && !*m_pBuffers)
	{
		if( !m_pBuffers->Create())
		{
			m_nCamStatus = CAM_STATUS_OPEN_FAILED;
			gLogFile.AddCameraRecord(m_nCamIndex, _T("m_pBuffers创建失败"));
			DestroyObjects();
			return FALSE;
		}
		// Clear all buffers
		m_pBuffers->Clear();
	}

	//// Set next empty with trash cycle mode for transfer
	//if (m_pXfer && m_pXfer->GetPair(0))
	//{
	//	if (!m_pXfer->GetPair(0)->SetCycleMode(SapXferPair::CycleNextWithTrash))
	//	{
	//		DestroyObjects();
	//		return FALSE;
	//	}
	//}

	// Create transfer object
	if (m_pXfer && !*m_pXfer && !m_pXfer->Create())
	{
		m_nCamStatus = CAM_STATUS_OPEN_FAILED;
		gLogFile.AddCameraRecord(m_nCamIndex, _T("m_pXfer创建失败"));
		DestroyObjects();
		return FALSE;
	}

	m_nCamStatus = CAM_STATUS_OPEN_SUCCEED;

	return TRUE;
}

BOOL CP4Ctrl::DestroyObjects()
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
	if (m_pAcq && *m_pAcq)
	{
		m_pAcq->Destroy();
		delete m_pAcq;
		m_pAcq = NULL;
	}

	return TRUE;
}

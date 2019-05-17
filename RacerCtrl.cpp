#include "StdAfx.h"
#include "ImageGrab.h"
#include "RacerCtrl.h"

CRacerCtrl::CRacerCtrl(void)
{

}

CRacerCtrl::~CRacerCtrl(void)
{

}

bool CRacerCtrl::Init()
{
	m_pCamera = new CInstantCamera();
	m_pCamera->RegisterConfiguration( this, RegistrationMode_Append, Ownership_ExternalOwnership);
	m_pCamera->RegisterImageEventHandler( this, RegistrationMode_Append, Ownership_ExternalOwnership);

	ConnectCamera();
	if (m_nCamStatus == CAM_STATUS_OPEN_SUCCEED)  //设置相机参数
	{
		SetCamTrigger(GlobalParameter::GetInstance()->m_vCamCfg[m_nCamIndex].nTriggerMode);
		SetCamExposure(GlobalParameter::GetInstance()->m_vCamCfg[m_nCamIndex].nExposureTime);
		SetCamGain(GlobalParameter::GetInstance()->m_vCamCfg[m_nCamIndex].nGain);
		SetCamGrabSize(m_nWidth, m_nHeight);
	}
	return true;
}

bool CRacerCtrl::Init(int nIndex)
{
	m_nCamIndex = nIndex;

	GlobalParameter *pGloabParam = GlobalParameter::GetInstance();
	m_nWidth = pGloabParam->m_vCamCfg[m_nCamIndex].nWidth;
	m_nHeight = pGloabParam->m_vCamCfg[m_nCamIndex].nHeight;

	m_pCamera = new CInstantCamera();
	////g_camera.RegisterConfiguration(new CAcquireContinuousConfiguration, RegistrationMode_ReplaceAll, Ownership_TakeOwnership);
	////g_camera.RegisterConfiguration( new CameraConfiguration, RegistrationMode_Append, Ownership_TakeOwnership);
	m_pCamera->RegisterConfiguration( this, RegistrationMode_Append, Ownership_ExternalOwnership);
	m_pCamera->RegisterImageEventHandler( this, RegistrationMode_Append, Ownership_ExternalOwnership);

	ConnectCamera();
	if (m_nCamStatus == CAM_STATUS_OPEN_SUCCEED)  //设置相机参数
	{
		SetCamTrigger(pGloabParam->m_vCamCfg[m_nCamIndex].nTriggerMode);
		SetCamExposure(pGloabParam->m_vCamCfg[m_nCamIndex].nExposureTime);
		SetCamGain(pGloabParam->m_vCamCfg[m_nCamIndex].nGain);
		SetCamGrabSize(m_nWidth, m_nHeight);
	}

	return 1;
}

void CRacerCtrl::Destory()
{
	//gLogFile.AddCameraRecord(m_nCamIndex, _T("销毁相机资源"));

	m_pCamera->DeregisterConfiguration(this);
	m_pCamera->DeregisterImageEventHandler(this);

	m_pCamera->DestroyDevice();
	if(NULL != m_pCamera)
	{
		delete m_pCamera; 
		m_pCamera = NULL;
	}
}

bool CRacerCtrl::ConnectCamera()
{ 
	//gLogFile.AddCameraRecord(m_nCamIndex, _T("启动相机连接"));

	try
	{
		DeviceInfoList_t deviceList;
		CTlFactory *pTlFactory = &CTlFactory::GetInstance ();
		pTlFactory->EnumerateDevices(deviceList);
		if (deviceList.empty ())
		{
            gLogFile.AddCameraRecord(m_nCamIndex, _T("未找到相机设备"));
			m_nCamStatus = CAM_STATUS_NOT_FIND_DEVICE;
			return FALSE;
		}

		bool bFlag = true;
		int nDeviceIndex = -1;
		for (DeviceInfoList_t::iterator iter = deviceList.begin (); iter != deviceList.end (); iter++)
		{  
			CString strDeviceName((*iter).GetFriendlyName().c_str ());
			int nPos = strDeviceName.Find(_T("("));
			strDeviceName = strDeviceName.Left(nPos - 1);
			nDeviceIndex++;

			if(strDeviceName == GlobalParameter::GetInstance()->m_vCamCfg[m_nCamIndex].strCamName.c_str())
			{  
				bFlag = false;
				m_pCamera->DestroyDevice();
				m_pCamera->Attach( pTlFactory->CreateDevice(deviceList[nDeviceIndex]));	
				m_pCamera->Open();
				if (m_pCamera->IsOpen())
				{
					gLogFile.AddCameraRecord(m_nCamIndex, _T("相机连接成功"));
					m_nCamStatus = CAM_STATUS_OPEN_SUCCEED;
				}
				else
				{
					gLogFile.AddCameraRecord(m_nCamIndex, _T("相机打开失败"));
					m_nCamStatus = CAM_STATUS_OPEN_FAILED;
				}
		
				break;
			}
		}

		if (bFlag)
		{
			gLogFile.AddCameraRecord(m_nCamIndex, _T("未找到相机设备"));
			m_nCamStatus = CAM_STATUS_NOT_FIND_DEVICE;
		}

	}
	catch(...)
	{
		m_nCamStatus = CAM_STATUS_OPEN_FAILED;
		gLogFile.AddCameraRecord(m_nCamIndex, _T("ConnectCamera函数异常退出"));
		return FALSE;
	}

	return TRUE;
}

void CRacerCtrl::OnImageGrabbed( Pylon::CInstantCamera& camera, const Pylon::CGrabResultPtr& ptrGrabResult)
{
	if (ptrGrabResult->GrabSucceeded())
	{ 
		if (gDataMgr.m_vImgBuf[m_nCamIndex]->BufferIn((BYTE *) ptrGrabResult->GetBuffer(), m_nCamIndex, m_nWidth, m_nHeight))
		{
			GlobalParameter::GetInstance()->m_vImgGrabInfo[m_nCamIndex].nGrabNum++;
		}
		else
		{
			GlobalParameter::GetInstance()->m_vImgGrabInfo[m_nCamIndex].nLostNum++;
		}
	}
	else
	{
		CString strDesc;
		strDesc.Format(_T("图像%d抓取错误, 描述:%s"), GlobalParameter::GetInstance()->m_vImgGrabInfo[m_nCamIndex].nGrabNum, string2wstring(ptrGrabResult->GetErrorDescription().c_str()).c_str());
		gLogFile.AddCameraRecord(m_nCamIndex, strDesc);

		BYTE *pTempData = new BYTE[m_nWidth * m_nHeight];
        memset(pTempData, 0, m_nWidth * m_nHeight);
		gDataMgr.m_vImgBuf[m_nCamIndex]->BufferIn(pTempData, m_nCamIndex, m_nWidth, m_nHeight);
		GlobalParameter::GetInstance()->m_vImgGrabInfo[m_nCamIndex].nGrabNum++;
		delete [] pTempData;
	}
}

void CRacerCtrl::OnGrabError( Pylon::CInstantCamera& camera, const wchar_t* errorMessage)
{

}

void CRacerCtrl::OnCameraDeviceRemoved( Pylon::CInstantCamera& camera)
{
	m_nCamStatus = CAM_STATUS_NOT_FIND_DEVICE;
}

bool CRacerCtrl::PrepareGrab()
{
	try
	{
		gLogFile.AddCameraRecord(m_nCamIndex, _T("准备采集"));	
	}
	catch(...)
	{
		m_nCamStatus = CAM_STATUS_UNKNOWN;
		gLogFile.AddCameraRecord(m_nCamIndex, _T("CRacerCtrl::PrepareGrab函数异常"));
	}
	
	return 1;
}

bool CRacerCtrl::StartGrab()
{
	// Start image grabbing if a camera device was created already

	try
	{
		gLogFile.AddCameraRecord(m_nCamIndex, _T("开始采集"));

		if(NULL == m_pCamera)
		{
			return 1;
		}
		if (m_pCamera->IsOpen()) 
		{
			m_pCamera->StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera); 
			m_nCamStatus = CAM_STATUS_RUNNING;	
		}
		else
		{
			m_nCamStatus = CAM_STATUS_OPEN_FAILED;
			gLogFile.AddCameraRecord(m_nCamIndex, _T("相机打开失败"));
		}
	}
	//catch (...)
	//{
	//	
	//	gLogFile.AddCameraRecord(m_nCamIndex, _T("CRacerCtrl::StartGrab函数异常"));
	//}
	catch (GenICam::GenericException& e)
	{
		m_nCamStatus = CAM_STATUS_UNKNOWN;

		CString strLog;
		strLog.Format(_T("CRacerCtrl::StartGrab异常, 错误原因:%s"), string2wstring(e.what()).c_str());
		gLogFile.AddCameraRecord(m_nCamIndex, strLog);
	}

	return 1;
}

bool CRacerCtrl::StopGrab()
{
	gLogFile.AddCameraRecord(m_nCamIndex, _T("停止采集"));

	try
	{
#if OPEN_IF//add by zhuxy有时候软件在这崩溃了 加个空指针判断
		if (!m_pCamera)
		{
			return 1;
		}
#endif
		if (m_pCamera->IsGrabbing())
		{
			m_pCamera->StopGrabbing();
			m_nCamStatus = CAM_STATUS_OPEN_SUCCEED;
		}
		else 
		{
			//gLogFile.AddCameraRecord(m_nCamIndex, _T("相机停止抓取失败"));
			if (m_pCamera->IsOpen())
			{
				m_nCamStatus = CAM_STATUS_OPEN_SUCCEED;
			}
			else
			{
				m_nCamStatus = CAM_STATUS_OPEN_FAILED;						
			}
		}
	}
	//catch (...)
	//{
	//	m_nCamStatus = CAM_STATUS_UNKNOWN;
	//	gLogFile.AddCameraRecord(m_nCamIndex, _T("CRacerCtrl::StopGrab函数异常"));
	//}
	catch (GenICam::GenericException& e)
	{
		m_nCamStatus = CAM_STATUS_UNKNOWN;

		CString strLog;
		strLog.Format(_T("CRacerCtrl::StopGrab异常, 错误原因:%s"), string2wstring(e.what()).c_str());
		gLogFile.AddCameraRecord(m_nCamIndex, strLog);
	}

	return 1;
}

void CRacerCtrl::SetCamTrigger(int nParam)
{
	try
	{
		INodeMap &cameraNodeMap = m_pCamera->GetNodeMap();
	
		CEnumerationPtr  ptrTriggerSel1 = cameraNodeMap.GetNode ("TriggerSelector");
		ptrTriggerSel1->FromString("AcquisitionStart");
		CEnumerationPtr  ptrTrigger1  = cameraNodeMap.GetNode ("TriggerMode");
		ptrTrigger1->SetIntValue(0);
		//
		CEnumerationPtr  ptrTriggerSel2 = cameraNodeMap.GetNode ("TriggerSelector");
		ptrTriggerSel2->FromString("FrameStart");
		CEnumerationPtr  ptrTrigger2  = cameraNodeMap.GetNode ("TriggerMode");
		ptrTrigger2->SetIntValue(0);
		//
		CEnumerationPtr  ptrTriggerSel3 = cameraNodeMap.GetNode ("TriggerSelector");
		ptrTriggerSel3->FromString("LineStart");
		CEnumerationPtr  ptrTrigger3  = cameraNodeMap.GetNode ("TriggerMode");
		ptrTrigger3->SetIntValue(nParam);

		CEnumerationPtr  ptrTriggerSource = cameraNodeMap.GetNode ("TriggerSource");
		ptrTriggerSource->FromString("Line2");
	}
	catch (GenICam::GenericException& e)
	{
		CString strLog;
		strLog.Format(_T("CRacerCtrl::SetCamTrigger异常, 错误原因:%s"), string2wstring(e.what()).c_str());
		gLogFile.AddCameraRecord(m_nCamIndex, strLog);
	}
}

void CRacerCtrl::SetCamExposure(int nParam)
{
	try
	{
		INodeMap &cameraNodeMap = m_pCamera->GetNodeMap();

		const CFloatPtr exposureTime = cameraNodeMap.GetNode("ExposureTimeAbs");
		exposureTime->SetValue(nParam);
	}
	catch (GenICam::GenericException& e)
	{
		CString strLog;
		strLog.Format(_T("CRacerCtrl::SetCamExposure异常, 错误原因:%s"), string2wstring(e.what()).c_str());
		gLogFile.AddCameraRecord(m_nCamIndex, strLog);
	}
}

void CRacerCtrl::SetCamGain(int nParam)
{
	try
	{
		INodeMap &cameraNodeMap = m_pCamera->GetNodeMap();

		CEnumerationPtr GainSelector(  cameraNodeMap.GetNode("GainSelector"));
		GainSelector->FromString( "DigitalAll");
		const CIntegerPtr cameraGen = cameraNodeMap.GetNode("GainRaw");
		cameraGen->SetValue(nParam);	

	}
	catch (GenICam::GenericException& e)
	{
		CString strLog;
		strLog.Format(_T("CRacerCtrl::SetCamGain异常, 错误原因:%s"), string2wstring(e.what()).c_str());
		gLogFile.AddCameraRecord(m_nCamIndex, strLog);
	}
}

void CRacerCtrl::SetCamGrabSize(int nWidth, int nHeight)
{
	try
	{
		INodeMap &cameraNodeMap = m_pCamera->GetNodeMap();

		const CIntegerPtr widthPic = cameraNodeMap.GetNode("Width");
		widthPic->SetValue(nWidth);

		const CIntegerPtr heightPic = cameraNodeMap.GetNode("Height");
		heightPic->SetValue(nHeight);
	}
	catch (GenICam::GenericException& e)
	{
	     CString strLog;
		 strLog.Format(_T("CRacerCtrl::SetCamGrabSize异常, 错误原因:%s"), string2wstring(e.what()).c_str());
         gLogFile.AddCameraRecord(m_nCamIndex, strLog);
	}
}

void CRacerCtrl::SetCamHorzFlip(const int& param)//add by zhuxy20190326设置镜像
{

}

void CRacerCtrl::SetCamLineSpeed(int nParam)
{

}

void CRacerCtrl::CheckCamStatus()
{
	//if (g_camera.IsCameraDeviceRemoved() == true || g_camera.IsOperfn() == false)

	if(NULL == m_pCamera)
	{
		Init();
		return ;
	}
	if(!(m_pCamera->IsOpen()))
	{
		m_nCamStatus = CAM_STATUS_OPEN_FAILED;
	}
	if ((CAM_STATUS_OPEN_SUCCEED != m_nCamStatus) && (CAM_STATUS_RUNNING != m_nCamStatus))
	{
		Destory();
		Init();
	}
}

UINT GrabImageThread(LPVOID p)
{
	//// This smart pointer will receive the grab result data.
	//CGrabResultPtr ptrGrabResult;

	//// Camera.StopGrabbing() is called automatically by the RetrieveResult() method
	//// when c_countOfImagesToGrab images have been retrieved.
	//while ( camera.IsGrabbing())
	//{
	//	// Wait for an image and then retrieve it. A timeout of 5000 ms is used.
	//	camera.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException);

	//	// Image grabbed successfully?
	//	if (ptrGrabResult->GrabSucceeded())
	//	{
	//		// Access the image data.
	//		cout << "SizeX: " << ptrGrabResult->GetWidth() << endl;
	//		cout << "SizeY: " << ptrGrabResult->GetHeight() << endl;
	//		const uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer();
	//		cout << "Gray value of first pixel: " << (uint32_t) pImageBuffer[0] << endl << endl;

	//		// Display the grabbed image.
	//		Pylon::DisplayImage(1, ptrGrabResult);
	//	}
	//	else
	//	{
	//		cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription();
	//	}
	//}

	return 1;
}
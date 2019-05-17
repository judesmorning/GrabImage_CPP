#include "StdAfx.h"
#include "ImageGrab.h"
#include "SpringCtrl.h"
#include "clBBProtocolLib.h"

CSpringCtrl::CSpringCtrl(void)
{
	pDataBuffer = NULL;
}


CSpringCtrl::~CSpringCtrl(void)
{
}

bool CSpringCtrl::Init(int nIndex)
{
	m_nCamIndex = nIndex;
	bool bRtn = ConnectCamera();

	if(bRtn)
	{
		 GlobalParameter *pGloabParam = GlobalParameter::GetInstance();
	     m_nWidth = pGloabParam->m_vCamCfg[m_nCamIndex].nWidth;
	     m_nHeight = pGloabParam->m_vCamCfg[m_nCamIndex].nHeight;

	     pDataBuffer = new unsigned char[m_nWidth * m_nHeight];
	}

	return true;
}

void CSpringCtrl::Destory()
{
	gLogFile.AddCameraRecord(m_nCamIndex, _T("销毁相机资源"));

	while(MilGrabBufferListSize > 0)
	{
		MbufFree(MilGrabBufferList[--MilGrabBufferListSize]);
	}
		
	MdigFree(MilDigitizer);
	MsysFree(MilSystem);
	MappFree(MilApplication);

	if (pDataBuffer)
	{
		delete [] pDataBuffer;
	}
}

MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void MPTYPE *HookDataPtr)
{
	HookDataStruct *UserHookDataPtr = (HookDataStruct *)HookDataPtr;
	CSpringCtrl *pCamCtrl = (CSpringCtrl *)UserHookDataPtr->pThisObject;

	MIL_ID ModifiedBufferId;
	/* Retrieve the MIL_ID of the grabbed buffer. */
	MdigGetHookInfo(HookId, M_MODIFIED_BUFFER+M_BUFFER_ID, &ModifiedBufferId);

	MbufGet(ModifiedBufferId, pCamCtrl->pDataBuffer);

	if (gDataMgr.m_vImgBuf[pCamCtrl->m_nCamIndex]->BufferIn(pCamCtrl->pDataBuffer, pCamCtrl->m_nCamIndex, pCamCtrl->m_nWidth, pCamCtrl->m_nHeight))
	{
		GlobalParameter::GetInstance()->m_vImgGrabInfo[pCamCtrl->m_nCamIndex].nGrabNum++;
	}
	else
	{
		GlobalParameter::GetInstance()->m_vImgGrabInfo[pCamCtrl->m_nCamIndex].nLostNum++;
	}

	return 0;
}

bool CSpringCtrl::PrepareGrab()
{
	gLogFile.AddCameraRecord(m_nCamIndex, _T("准备采集"));	
	return true;
}

bool CSpringCtrl::StartGrab()
{
	if(m_nCamStatus == CAM_STATUS_OPEN_SUCCEED)
	{
		gLogFile.AddCameraRecord(m_nCamIndex, _T("开始采集"));
		UserHookData.ProcessedImageCount = 0;
		UserHookData.pThisObject = this;

	    MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, M_START, M_DEFAULT, ProcessingFunction, &UserHookData);
	}

	return true;
}

bool CSpringCtrl::StopGrab()
{
	if(m_nCamStatus == CAM_STATUS_OPEN_SUCCEED)
	{
		gLogFile.AddCameraRecord(m_nCamIndex, _T("停止采集"));
		UserHookData.ProcessedImageCount = 0;
		UserHookData.pThisObject = this;

	    MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, M_STOP, M_DEFAULT, ProcessingFunction, &UserHookData);
	}

	return true;
}

void CSpringCtrl::SetCamTrigger(int nParam)
{

}

void CSpringCtrl::SetCamExposure(int nParam)
{
	nParam = nParam * 10;
	CLUINT32 res = clbbp_AllSerialWriteRegister(hRef, ADDRESS_EXPOSURE_RAW, (const CLINT8*)&nParam, sizeof(nParam), 350);
	if (res != CL_ERR_NO_ERR)
	{
		AfxMessageBox(_T("曝光时间设置错误！"));
		return;
	}
	else
	{
		CString s;
		s.Format(_T("曝光时间：%d(0.1us)设置成功"), nParam);
		gLogFile.AddCameraRecord(m_nCamIndex, s);

	}
}

void CSpringCtrl::SetCamGain(int nParam)
{
	////增益


	/*CLINT32 gain = CLINT32(nParam);

	CLUINT32 res = clbbp_AllSerialWriteRegister(hRef, ADDRESS_GAIN_RAW, (const CLINT8*)&gain, sizeof(gain), 350);
	if (res != CL_ERR_NO_ERR)
	{
	AfxMessageBox(_T("增益设置错误！"));
	}
	else
	{
	CString s;
	s.Format(_T("增益：%d 设置成功"), nParam);
	gLogFile.AddCameraRecord(m_nCamIndex, s);
	}*/
				
}

void CSpringCtrl::SetCamGrabSize(int nWidth, int nHeight)
{

}

void CSpringCtrl::CheckCamStatus()
{

}

bool CSpringCtrl::ConnectCamera()
{
	GlobalParameter *pGloabParam = GlobalParameter::GetInstance();

	/* 分配 App */
	MappAlloc(M_DEFAULT, &MilApplication);
	/* 分配 system. */
	MsysAlloc(M_SYSTEM_SOLIOS, M_DEV0 + pGloabParam->m_vCamCfg[m_nCamIndex].nDeviceIndex, M_SETUP, &MilSystem);
	/*分配 digitizer. */
	MdigAlloc(MilSystem, M_DEV0, pGloabParam->m_vCamCfg[m_nCamIndex].strCameraCfgPath.c_str(), M_DEFAULT, &MilDigitizer);

	//获取相机分辨率和图像通道数
	MIL_INT g_BufSizeX    = (unsigned int) MdigInquire(MilDigitizer, M_SIZE_X, M_NULL);
	long g_BufSizeY    = (unsigned int) MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL);
	long g_BufSizeBand = (unsigned int) MdigInquire(MilDigitizer, M_SIZE_BAND, M_NULL);

	/* Allocate the grab buffers and clear them. */
	MappControl(M_ERROR, M_PRINT_DISABLE);

	for(MilGrabBufferListSize = 0; 
		MilGrabBufferListSize<BUFFERING_SIZE_MAX; MilGrabBufferListSize++)
	{
		MbufAllocColor( MilSystem,g_BufSizeBand,
			MdigInquire(MilDigitizer, M_SIZE_X, M_NULL),
			MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL),
			M_DEF_IMAGE_TYPE,
			M_IMAGE+M_GRAB+M_PROC,
			&MilGrabBufferList[MilGrabBufferListSize]);

		if (MilGrabBufferList[MilGrabBufferListSize])
		{
			MbufClear(MilGrabBufferList[MilGrabBufferListSize], 0xFF);
		}
		else
			break;
	}

	MappControl(M_ERROR, M_PRINT_DISABLE);

	bool bRtn = MdigInquire(MilDigitizer,M_CAMERA_PRESENT,M_NULL);
	if(bRtn)
	{
		gLogFile.AddCameraRecord(m_nCamIndex, _T("相机连接成功"));
		m_nCamStatus = CAM_STATUS_OPEN_SUCCEED;

		MIL_TEXT_PTR  str=_T("UserSet01");
	    MdigControlFeature( MilDigitizer, M_DEFAULT, _T("UserSetSelector"), M_TYPE_STRING_ENUMERATION, str);
	    MdigControlFeature( MilDigitizer, M_DEFAULT, _T("UserSetLoad"), M_TYPE_COMMAND, "Execute()");

		MdigControl(MilDigitizer, M_GRAB_TIMEOUT, M_INFINITE);
	}
	else
	{
		gLogFile.AddCameraRecord(m_nCamIndex, _T("相机打开失败"));
		m_nCamStatus = CAM_STATUS_OPEN_FAILED;
	}



	CLINT32 res = CL_ERR_NO_ERR;
	//hSerRef hRef = NULL;UANG


	/////////////////////////////////////////////////////////////////
	// print out all ports available
	CLUINT32 numPorts = 16;
	//res = clGetNumPorts(&numPorts);
	//if (res != CL_ERR_NO_ERR)
	//{
	//    return true;
	//}

	//if (numPorts == 0)
	//{
	//    printf("Error: Could not find any clallserial ports on your system!\n");
	//    return 1;
	//}


	CLUINT32 portIndex((CLUINT32)-1); // initialize with invalid value

	char szManuName[260] = { 0 };
	char szPortID[260] = { 0 };

	CLUINT32 manuNameLen = sizeof(szManuName);
	CLUINT32 portIDLen = sizeof(szPortID);
	CLUINT32 versionNumber = 0;

	if (numPorts > 1)
	{
		for (CLUINT32 i = 0; i < numPorts; ++i)
		{
			manuNameLen = sizeof(szManuName);
			portIDLen = sizeof(szPortID);
			versionNumber = 0;

			res = clGetPortInfo(i, szManuName, &manuNameLen, szPortID, &portIDLen, &versionNumber);
			if (res != CL_ERR_NO_ERR)
			{
				continue;
			}

			printf("%u: %s#%s\n", i, szManuName, szPortID);
		}


		/*portIndex = theApp.m_ComNumber - 1;*/

		portIndex = pGloabParam->m_vCamCfg[m_nCamIndex].nComID-1;
		printf("Please select a clallserial port (0..%u): ", numPorts - 1);
		scanf("%u", &portIndex);
		if (portIndex >= numPorts)
		{
			printf("Error: Invalid port selected!\n", portIndex);
			// invalid portindex entered
			return 1;
		}
	}
	else
	{
		// there is only one port available, so use it
		portIndex = 0;
	}


	//portIndex=5;
	/////////////////////////////////////////////////////////////////
	// remember the portinfo used
	manuNameLen = sizeof(szManuName);
	portIDLen = sizeof(szPortID);
	versionNumber = 0;
	res = clGetPortInfo(portIndex, szManuName, &manuNameLen, szPortID, &portIDLen, &versionNumber);
	if (res != CL_ERR_NO_ERR)
	{
		return true;
	}


	/////////////////////////////////////////////////////////////////
	// open the port
	printf("Opening port %u: %s#%s\n", portIndex, szManuName, szPortID);

	res = clSerialInit(portIndex, &hRef);
	if (res != CL_ERR_NO_ERR)
	{
		hRef = NULL;
		return true;
	}

	/////////////////////////////////////////////////////////////////
	// set baud rate to default 9600
	printf("Setting baud rate to 9600\n\n");

	res = clSetBaudRate(hRef, CL_BAUDRATE_9600);
	if (res != CL_ERR_NO_ERR)
	{
		return true;
	}

	return bRtn;
	////CLINT8* pBuffer;
	//CLINT8 pModelBuffer[20]= {'0'};
	//res = clbbp_AllSerialReadRegister(hRef, ADDRESS_MODEL, pModelBuffer, 20, 350);
	//if (res != CL_ERR_NO_ERR)
	//{
	//	gLogFile.AddRecord("读取设备Model错误！");
	//}
	//else
	//{
	//	theApp.m_cModelName.Format("%s",pModelBuffer);
	//	CString s;
	//	s.Format("设备Model：%s",pModelBuffer);
	//	gLogFile.AddRecord(s);
	//}


	//CLINT8 pVendorBuffer[20]= {'0'};
	//res = clbbp_AllSerialReadRegister(hRef, ADDRESS_VENDOR, pVendorBuffer, 20, 350);
	//if (res != CL_ERR_NO_ERR)
	//{
	//	gLogFile.AddRecord("读取设备Vendor错误！");
	//}
	//else
	//{
	//	theApp.m_cVendorName.Format("%s",pVendorBuffer);
	//	CString s;
	//	s.Format("设备Vendor：%s",pVendorBuffer);
	//	gLogFile.AddRecord(s);
	//}

	//CLINT8 pSerialBuffer[20]= {'0'};
	//res = clbbp_AllSerialReadRegister(hRef, ADDRESS_SERIALNUM, pSerialBuffer, 20, 350);
	//if (res != CL_ERR_NO_ERR)
	//{
	//	gLogFile.AddRecord("读取设备Serial错误！");
	//}
	//else
	//{
	//	theApp.m_cSN.Format("%s",pSerialBuffer);
	//	CString s;
	//	s.Format("设备Serial：%s",pSerialBuffer);
	//	gLogFile.AddRecord(s);
	//}
	////delete pBuffer;
	////pBuffer = NULL;

	//CTime mTime;
	//CString theTime;
	//mTime=GetCurrentTime(); 
	//int h=mTime.GetHour(); 

	//CLUINT32 raw=0;

	//res = clbbp_AllSerialReadRegister(hRef,ADDRESS_EXPOSURE_RAW, ( CLINT8*)&raw, sizeof(raw), 350);
	//if (res != CL_ERR_NO_ERR)
	//{
	//	gLogFile.AddRecord("曝光时间读取错误！");
	//}
	//else
	//{
	//	raw = (CLUINT32)raw*0.1;
	//	CString s;
	//	s.Format("当前读取曝光时间：%d",raw);
	//	gLogFile.AddRecord(s);
	//}

	//theApp.m_ExposeTime = raw;

	////CLUINT32	rawGain;
	////float rawGain = 0;

	////res = clbbp_AllSerialReadRegister(hRef, ADDRESS_GAIN_RAW, ( CLINT8*)&rawGain,sizeof(rawGain), 350);
	////if (res != CL_ERR_NO_ERR)
	////{
	////	gLogFile.AddRecord("增益读取错误！");
	////}
	////else
	////{
	////	/*	rawGain = 20*log10(rawGain/4096);*/
	////	theApp.m_Gain = (int)rawGain;
	////	CString s;
	////	s.Format("当前读取增益：%d",rawGain);
	////	gLogFile.AddRecord(s);
	////}

	//return true;
}
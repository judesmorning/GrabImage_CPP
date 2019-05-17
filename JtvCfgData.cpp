#include "stdafx.h"
#include "JtvCfgData.h"
#include <iostream> 
#include <sstream> 
#include "assert.h"

#define  MAX_BUFFER_LEN (1024)

wchar_t* buf_MainKey[2] = {
	_T("config"),
	_T("cam")
};

wchar_t *buf_config[CFG_KEY_COUNT] = {//fix by zhuxy20190319 14->CFG_KEY_COUNT
	_T("Camnum"),
	_T("ServerIp"),
	_T("ServerPort"),
	_T("ControlPort"),
	_T("ImgSavePath"),
	_T("SaveImageFlag"),
	_T("SendImageFlag"),
	_T("Title"),
	_T("SendThreadCount"),
	_T("SendWaitTime"),
	_T("CompressThreadCount"),
	_T("StopGrabWaitTime"),
	_T("SaveTrainCount"),
	_T("CtlMsgNumber"),
	_T("FileExistDays"),//add by zhuxy文件存在天数
	_T("LogName"),//add by zhuxy权限
	_T("LogPaswrd"),//add by zhuxy权限
	_T("LocalImgSavePath"),//add by zhuxy本地路径
	_T("Spare")//add by zhuxy主备控
};

wchar_t *buf_camera_cfg[CAM_KEY_COUNT] = {
	_T("CamName"),
	_T("CamDesc"),
	_T("ChannelNum"),
	_T("TriggerMode"),
	_T("ExposureTime"),
	_T("Gain"),
	_T("Width"),
	_T("Height"),
	_T("BufferNum"),
	_T("HorzFlipFlag"),
	_T("EnhanceType"),
	_T("JointNum"),
	_T("GrabDevice"),
	_T("ClahWBlockNum"),
	_T("ClahHBlockNum"),
	_T("ClipLimit"),
	_T("SharpenTemplateNum"),
	_T("Saturated"),
	_T("SmoothTemplate"),
	_T("ImgQuality"),
	_T("HeadInvaildImgCount"),
	_T("TailInvaildImgCount"),
	_T("DeviceIndex"),
	_T("CameraCfgPath"),
	_T("LogEnhance"),
	_T("LogEnchanceThreshold"),
	_T("LineSpeed")
};

CJtvCfgData::CJtvCfgData()
{
	SetFilePath();
}

CJtvCfgData::~CJtvCfgData()
{
}

void CJtvCfgData::SetFilePath(void)//设置配置文件ImageGrab.ini的路径
{
	TCHAR strExePath[MAX_PATH] = {0};
	GetModuleFileName(NULL, strExePath, MAX_PATH);//获取当前路径

	wchar_t *pszPos = wcsrchr(strExePath, _T('\\'));
	if(pszPos == NULL)
		return;

	*pszPos = 0x0;
	pszPos = strExePath;
	wchar_t *postfix =  _T("/config/ImageGrab.ini");

	memset(m_FilePath,0,MAX_PATH);
	wsprintfW(m_FilePath, _T("%s%s"), pszPos, postfix);
}
void CJtvCfgData::SetMainKey(int i, int nIndex)//设置ini文件的主键,cam[cam0]、[cam1]或者[config]
{
	if (nIndex != 0)
	{
		wsprintfW(m_szMainKey, _T("cam%d"), nIndex);
	}
	else
	{
		wcscpy_s(m_szMainKey, MAX_PATH, buf_MainKey[i]);	
	}
}
//设置TEXT
bool CJtvCfgData::SetText(const wchar_t *lpszValueName, const wchar_t *lpszValue)//将某主键写入ini文件
{
	assert(lpszValueName!=NULL);

	if (WritePrivateProfileString(m_szMainKey, lpszValueName, lpszValue,m_FilePath))
	{
		return true;
	}
	return false;
}
bool CJtvCfgData::GetText(const wchar_t *lpszValueName, wchar_t *lpszValue, unsigned long& dwValueLen, const wchar_t *lpszDefValue)//读取ini文件中的某主键
{
	assert(lpszValueName!=lpszValue);
	unsigned long dwDataLen = GetPrivateProfileString(m_szMainKey, lpszValueName, lpszDefValue, lpszValue, MAX_BUFFER_LEN, m_FilePath);
	if(dwDataLen <= 0)
	{
		return false;
	}

	if(dwValueLen < dwDataLen)
	{
		dwValueLen = dwDataLen;
		return false;
	}
	dwValueLen = dwDataLen;
	return true;
}

//设置unsigned long
bool CJtvCfgData::SetDWORD(const wchar_t *lpszValueName, unsigned long dwValue)//设置ini文件中某项的右值
{
	assert(lpszValueName!=NULL);

	wchar_t tszString[255] = {0};
	wsprintfW(tszString, _T("%d"), dwValue);

	if (WritePrivateProfileString(m_szMainKey, lpszValueName, tszString, m_FilePath))
	{
		return true;
	}
	return false;
}
unsigned long CJtvCfgData::GetDWORD( const wchar_t *lpszValueName, unsigned long& dwValue,unsigned long dwDefValue)//获取ini文件中某项的右值
{
	dwValue = GetPrivateProfileInt(m_szMainKey,lpszValueName,dwDefValue,m_FilePath);
	return dwValue;
}

bool CJtvCfgData::GetConfig(config &cfg)//读取ini文件的[config],返回config结构体
{
	SetMainKey(0);//设置主键为[config]
	wchar_t szBuffer[MAX_BUFFER_LEN];
	unsigned long dwLen = GetPrivateProfileString(m_szMainKey, NULL,  _T(""), szBuffer, MAX_BUFFER_LEN, m_FilePath);
	unsigned long dwCount= 0;
	for (unsigned long pos = 0; pos<dwLen ; pos++)
	{
		if(_T('\0') == szBuffer[pos])
		{
			wchar_t szKey[30] = {0};
			wcscpy_s(szKey, 30, szBuffer+dwCount);
			dwCount = pos+1; 

			wchar_t  keyValue[MAX_PATH];
			unsigned long len = MAX_PATH;
			if (!GetText(szKey, keyValue, len))
			{
				continue;
			}
			int i=0;
			for(; i<CFG_KEY_COUNT; i++)//fix by zhuxy20190319 13->CFG_KEY_COUNT
			{
				if(!wcscmp(buf_config[i], szKey))
					break;
			}
			switch(i)
			{
			case 0:
				cfg.nCamNum = _wtoi(keyValue);
				break;
			case 1:
				cfg.strSerIp = keyValue;
				break;
			case 2:
				cfg.nSerPort = _wtoi(keyValue);
				break;
			case 3:
				cfg.nCtrlPort = _wtoi(keyValue);
				break;
			case 4:
				cfg.strImgSavePath = keyValue;
				break;
			case 5:
				cfg.nSaveImageFlag = _wtoi(keyValue);
				break;
			case 6:
				cfg.nSendImageFlag = _wtoi(keyValue);
				break;
			case 7:
				cfg.strTitle = keyValue;
				break;
			case 8:
				cfg.nSendThreadCount = _wtoi(keyValue);
				break;
			case 9:
				cfg.nSendWaitTime = _wtoi(keyValue);
				break;
			case 10:
				cfg.nCompressThreadCount = _wtoi(keyValue);
				break;
			case 11:
				cfg.nStopGrabWaitTime = _wtoi(keyValue);
				break;
			case 12:
				cfg.nSaveTrainCount = _wtoi(keyValue);
				break;
			case 13:
				cfg.nCtlMsgNumber = _wtoi(keyValue);
				break;
			case 14://add by zhuxy文件的最大值,超过则删除
				cfg.nFileExistDays = _wtoi(keyValue);
				break;
			case 15://add by zhuxy用户名
				cfg.nRootName = keyValue;
				break;
			case 16://add by zhuxy密码
				cfg.nRootpswrd = keyValue;
				break;
			case 17://add by zhuxy本地存储路径
				cfg.strLocalImgSavePath = keyValue;
				break;
			case 18://add by zhuxy主备控
				cfg.nSpare = _wtoi(keyValue);
				break;
			default:
				break;
			}
		}
	}

	return true;
}

bool CJtvCfgData::GetCameraCfg(int nCamNum, vector<CameraCfg> &vCamCfg)//把ini配置文件里的所有相机配置读取出来存在vector里
{
	vCamCfg.clear();//add by zhuxy
	SetMainKey(1);//设置主键为cam,这句话无意义,下面重新设置了主键
	for (int k=1; k<=nCamNum; k++)
	{
		CameraCfg camCfg;
		wchar_t szBuffer[MAX_BUFFER_LEN] = {0};

		wsprintfW(m_szMainKey, _T("cam%d"), k);
		unsigned long dwLen = GetPrivateProfileString(m_szMainKey, NULL, _T(""), szBuffer, MAX_BUFFER_LEN, m_FilePath);
		if (dwLen == 0)
		{
			continue;
		}

		unsigned long dwCount= 0;
		for (unsigned long pos = 0; pos<dwLen ; pos++)
		{
			if(_T('\0') == szBuffer[pos])
			{
				wchar_t szKey[30] = {0};
				wcscpy_s(szKey, 30, szBuffer+dwCount);
				dwCount = pos+1; 

				wchar_t  keyValue[MAX_PATH];
				unsigned long len = MAX_PATH;
				if (!GetText(szKey, keyValue, len))
				{
					continue;
				}

				int i=0;
				for(; i<CAM_KEY_COUNT; i++)
				{
					if(!wcscmp(buf_camera_cfg[i],szKey))
						break;
				}

				switch(i)
				{
				case 0:
					camCfg.strCamName = keyValue;
					break;
				case 1:
					camCfg.strCamDesc = keyValue;
					break;
				case 2:
					camCfg.nChannelNum = _wtoi(keyValue);
					break;
				case 3:
					camCfg.nTriggerMode = _wtoi(keyValue);
					break;
				case 4:
					camCfg.nExposureTime = _wtoi(keyValue);
					break;
				case 5:
					camCfg.nGain = _wtoi(keyValue);
					break;
				case 6:
					camCfg.nWidth = _wtoi(keyValue);
					break;
				case 7:
					camCfg.nHeight = _wtoi(keyValue);
					break;
				case 8:
					camCfg.nBufferNum = _wtoi(keyValue);
					break;
				case 9:
					camCfg.nHorzFlipFlag = _wtoi(keyValue);
					break;
				case 10:
					camCfg.nEnhanceType = _wtoi(keyValue);
					break;
				case 11:
					camCfg.nJointNum = _wtoi(keyValue);
					break;
				case 12:
					camCfg.nGrabDevice = _wtoi(keyValue);
					break;
				case 13:
					camCfg.tEnhanceParam.tClaneParam.clah_wblock_num = _wtoi(keyValue);
					break;
				case 14:
					camCfg.tEnhanceParam.tClaneParam.clah_hblock_num = _wtoi(keyValue);
					break;
				case 15:
					camCfg.tEnhanceParam.tClaneParam.clip_limit = _wtoi(keyValue);
					break;
				case 16:
					camCfg.tEnhanceParam.dSharpenTemplateNum = _wtof(keyValue);
					break;
				case 17:
					camCfg.tEnhanceParam.nSaturated = _wtof(keyValue);
					break;
				case 18:
					camCfg.tEnhanceParam.nSmoothTemplate = _wtoi(keyValue);
					break;
				case 19:
					camCfg.nImgQuality = _wtoi(keyValue);
					break;
				case 20:
					camCfg.nHeadInvaildImgCount = _wtoi(keyValue);
					break;
				case 21:
					camCfg.nTailInvaildImgCount = _wtoi(keyValue);
					break;
				case 22:
					camCfg.nDeviceIndex = _wtoi(keyValue);
					break;
				case 23:
					camCfg.strCameraCfgPath = keyValue;
					break;
				case 24:
					camCfg.tEnhanceParam.fLogEnhance = _wtof(keyValue);
					break;
				case 25:
					camCfg.tEnhanceParam.nLogEnhanceThreshold = _wtoi(keyValue);
					break;
				case 26://add by zhuxy 行频
					camCfg.nLineSpeed = _wtoi(keyValue);
					break;
				default:
					break;
				}
			}
		}
		vCamCfg.push_back(camCfg);
	}

	return true;
}

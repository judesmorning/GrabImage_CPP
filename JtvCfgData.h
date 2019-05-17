#ifndef CJTVCFGDATA
#define CJTVCFGDATA

//常用配置
typedef struct tagConfig
{
    int nCamNum;
	wstring strSerIp;
	int nSerPort;
	int nCtrlPort;
	wstring strImgSavePath;
	int nSaveImageFlag;
	int nSendImageFlag;
	wstring strTitle;
	int  nSendThreadCount;
	int  nSendWaitTime;
	int  nCompressThreadCount;
	int  nStopGrabWaitTime;
	int  nSaveTrainCount;
	int	 nCtlMsgNumber;
	int  nFileExistDays;//add by zhuxy 20190319	文件存在多少天
	wstring nRootName;//add by zhuxy 20190402	登录名
	wstring nRootpswrd;//add by zhuxy 20190402	登录密码
	wstring strLocalImgSavePath;
	int nSpare;//主备控
}config,*pConfig;

//增强参数
typedef struct tagEnhanceParam
{
	Enhance_Distribute_Params tClaneParam;   //限制直方图增强系数
	double dSharpenTemplateNum;  //锐化系数
	double nSaturated;  //饱和度	
	int nSmoothTemplate;  //平滑模板
	float fLogEnhance;	//对数增强
	int nLogEnhanceThreshold;//对数增强阈值
}EnhanceParam, *pEnhanceParam;

//相机配置
typedef struct tagCameraConfig
{   
	wstring	strCamName;
	wstring	strCamDesc;
	int  nChannelNum;
	int  nTriggerMode;
	int  nExposureTime;
	int  nGain;
	int  nWidth;
	int  nHeight;
	int  nBufferNum;
	int  nHorzFlipFlag;
	int  nEnhanceType;
	int  nJointNum;
	int  nGrabDevice;
	int  nImgQuality;
	int  nHeadInvaildImgCount;
	int  nTailInvaildImgCount;
	int  nDeviceIndex;
	wstring strCameraCfgPath;
	EnhanceParam tEnhanceParam;
	int nLineSpeed;
}CameraCfg, *pCameraCfg;


class CJtvCfgData//这个类对应cam或者config，主要做配置文件的读写
{
public:
	CJtvCfgData();
	~CJtvCfgData();

public:
	void SetFilePath();
	void SetMainKey(int i, int nIndex=0);
	//设置TEXT
	bool SetText(const wchar_t *lpszValueName, const wchar_t *lpszValue);
	bool GetText(const wchar_t *lpszValueName, wchar_t *lpszValue, unsigned long& dwValueLen, const wchar_t *lpszDefValue = NULL);

	//设置unsigned long
	bool SetDWORD(const wchar_t *lpszValueName, unsigned long dwValue);
	unsigned long GetDWORD( const wchar_t *lpszValueName, unsigned long& dwValue,unsigned long dwDefValue=0);
   
	//获取主键下面所有值
	bool GetConfig(config &cfg);
	bool GetCameraCfg(int nCamNum, vector<CameraCfg> &vCamCfg);

private: 
	TCHAR m_szMainKey[MAX_PATH];//[cam0]、[cam1]或者[config]
	TCHAR m_FilePath[MAX_PATH];//保存ImageGrab.ini的路径,../config/ImageGrab.ini
	int m_type;
};

#endif
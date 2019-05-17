#ifndef CJTVCFGDATA
#define CJTVCFGDATA

//��������
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
	int  nFileExistDays;//add by zhuxy 20190319	�ļ����ڶ�����
	wstring nRootName;//add by zhuxy 20190402	��¼��
	wstring nRootpswrd;//add by zhuxy 20190402	��¼����
	wstring strLocalImgSavePath;
	int nSpare;//������
}config,*pConfig;

//��ǿ����
typedef struct tagEnhanceParam
{
	Enhance_Distribute_Params tClaneParam;   //����ֱ��ͼ��ǿϵ��
	double dSharpenTemplateNum;  //��ϵ��
	double nSaturated;  //���Ͷ�	
	int nSmoothTemplate;  //ƽ��ģ��
	float fLogEnhance;	//������ǿ
	int nLogEnhanceThreshold;//������ǿ��ֵ
}EnhanceParam, *pEnhanceParam;

//�������
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


class CJtvCfgData//������Ӧcam����config����Ҫ�������ļ��Ķ�д
{
public:
	CJtvCfgData();
	~CJtvCfgData();

public:
	void SetFilePath();
	void SetMainKey(int i, int nIndex=0);
	//����TEXT
	bool SetText(const wchar_t *lpszValueName, const wchar_t *lpszValue);
	bool GetText(const wchar_t *lpszValueName, wchar_t *lpszValue, unsigned long& dwValueLen, const wchar_t *lpszDefValue = NULL);

	//����unsigned long
	bool SetDWORD(const wchar_t *lpszValueName, unsigned long dwValue);
	unsigned long GetDWORD( const wchar_t *lpszValueName, unsigned long& dwValue,unsigned long dwDefValue=0);
   
	//��ȡ������������ֵ
	bool GetConfig(config &cfg);
	bool GetCameraCfg(int nCamNum, vector<CameraCfg> &vCamCfg);

private: 
	TCHAR m_szMainKey[MAX_PATH];//[cam0]��[cam1]����[config]
	TCHAR m_FilePath[MAX_PATH];//����ImageGrab.ini��·��,../config/ImageGrab.ini
	int m_type;
};

#endif

#include "SafeList.h"


typedef struct PictureInfo
{
	char m_strTrainID[12];
	int m_nCamPos;
	int m_nImageIndex;
	int m_nTotalSize;
} PICTUREINFO;

//add by zhuxy20190321	过车信息 36bytes
typedef struct SOCKETMsg
{
	int m_nDelay;
	char m_strTrainID[12];
	char m_time[18];
}HRBMSG;
//add by zhuxy20190321	过车信息 16bytes
struct ZxgCGMsg
{
	int m_nDelay;
	char m_strTrainID[12];
};

typedef struct tagImgGrabInfo
{
	tagImgGrabInfo()
	{
		nGrabNum = 0;
		nSaveNum = 0;
		nCompressNum = 0;
		nLostNum = 0;
		nUploadNum = 0;
	}

	int nGrabNum;
	int nSaveNum;
	int nCompressNum;
	int nLostNum;
	int nUploadNum;
}ImgGrabInfo, *pImgGrabInfo;

typedef struct tagTrainInfo
{
	tagTrainInfo()
	{
		strTrainId = "";
        strTrainTime = "";
		workState = -1;
	}

	string strTrainId;
	string strTrainTime;
	//int nMsgOrder;//收到过车指令为2  再次收到为1  过车后为3  默认-1 delete by zhuxy 20190322改为workState
	int workState;//-1:其他 0:繁忙 1:就绪
	
}TrainInfo, *pTrainInfo;

typedef struct tagJointImgInfo
{
	tagJointImgInfo()
	{
		nJointIndex = 0;
		nImgIndex = 0;
		nCamIndex = 0;
		nEnHanceIndex = 0;
		pImgData = NULL;
		pObject = NULL;
	}

    int nJointIndex;
	int nImgIndex;
	int nCamIndex;
	int nEnHanceIndex;
	BYTE *pImgData;
	void *pObject;
}JointImgInfo, *pJointImgInfo;

typedef struct tagUploadParam
{
	tagUploadParam()
	{
		pJPGBuffer = NULL;
		nSizeJPGBuffer = -1;
		nIndexJPG = -1;
		nCameraIndexJPG = -1;
		nCameraIndexInConf = -1;
	}

	//tagUploadParam(const tagUploadParam &uploadParam)
	//{
	//	if (uploadParam.pJPGBuffer != NULL)
	//	{
	//		pJPGBuffer = new BYTE[uploadParam.nSizeJPGBuffer]; 
	//		memcpy(pJPGBuffer, uploadParam.pJPGBuffer, uploadParam.nSizeJPGBuffer);
	//	}
	//	
	//	nSizeJPGBuffer = uploadParam.nSizeJPGBuffer;
	//	nIndexJPG = uploadParam.nIndexJPG;
	//	nCameraIndexJPG = uploadParam.nCameraIndexJPG;
	//	nCameraIndexInConf = uploadParam.nCameraIndexInConf;
	//}

	BYTE *pJPGBuffer;
	int nSizeJPGBuffer;
	int nIndexJPG;
	int nCameraIndexJPG;
	int nCameraIndexInConf;
}UploadParam, *pUploadParam;


/* Camera status codes */
#define CAM_STATUS_NOT_FIND_DEVICE    0
#define CAM_STATUS_OPEN_FAILED        1
#define CAM_STATUS_OPEN_SUCCEED       2
#define CAM_STATUS_OPEN_DECONNECT     3
#define CAM_STATUS_RUNNING            4
#define CAM_STATUS_STOPPED            5
#define CAM_STATUS_ERROR              6
#define CAM_STATUS_UNKNOWN            7

#define CAMNUM  5

#define ADDRESS_GAIN_RAW (0x0E0D)//(0x20024)
#define ADDRESS_EXPOSURE_RAW (0x150D)//(0x20024)
#define ADDRESS_EXPOSURE_RAW_MIN (0x1511)//(0x20024)
#define ADDRESS_EXPOSURE_RAW_MAX (0x1515)//(0x20024)
#define ADDRESS_CAMERAVERSION_RAW (0x0501)
#define ADDRESS_MODEL 0x0201
#define ADDRESS_LINEPERIOD 0x160D
#define ADDRESS_VENDOR  0x0101
#define ADDRESS_SERIALNUM  0x0401
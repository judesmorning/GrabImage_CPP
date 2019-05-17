#pragma once

#include "CameraCtrl.h"
#include "Mil.h"
#include "clBBProtocolLib.h"
#include "clallserial.h"

#define BUFFERING_SIZE_MAX 4
#define STRING_LENGTH_MAX  20
#define STRING_POS_X       20
#define STRING_POS_Y       20

#define ADDRESS_GAIN_RAW (0x0E0D)//(0x20024)
#define ADDRESS_EXPOSURE_RAW (0x150D)//(0x20024)

typedef struct
{
	MIL_INT ProcessedImageCount;
	void *pThisObject;
} HookDataStruct;

class CSpringCtrl: public CCameraCtrl
{
public:
	CSpringCtrl(void);
	~CSpringCtrl(void);

public:
	bool Init(int nIndex);
	void Destory();
	bool PrepareGrab();
	bool StartGrab();
	bool StopGrab();

	void SetCamTrigger(int nParam);
	void SetCamExposure(int nParam);
	void SetCamGain(int nParam);
	void SetCamGrabSize(int nWidth, int nHeight);

	void CheckCamStatus();

private:
	bool ConnectCamera();

public:
	unsigned char *pDataBuffer;

private:
	MIL_ID MilApplication;
	MIL_ID MilSystem;
	MIL_ID MilDigitizer;
	MIL_ID MilGrabBufferList[BUFFERING_SIZE_MAX];
	MIL_INT MilGrabBufferListSize;	

	HookDataStruct UserHookData;

	

};


#pragma once

#include "SapClassBasic.h"
#include "SapClassGui.h"
#include "CameraCtrl.h"

class CP4Ctrl: public CCameraCtrl
{
public:
	CP4Ctrl(void);
	~CP4Ctrl(void);

public:
	bool Init();
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
	BOOL CreateObjects();
	BOOL DestroyObjects();

public:
	SapAcquisition   *m_pAcq;
	SapBuffer	   *m_pBuffers;
	SapTransfer	   *m_pXfer;
};

#pragma once

#include "SapClassBasic.h"
#include "SapClassGui.h"
#include "CameraCtrl.h"




class CLineaCtrl: public CCameraCtrl
{
public:
	CLineaCtrl(void);
	~CLineaCtrl(void);
protected:
	virtual bool Init();
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
	void SetCamHorzFlip(const int&);//add by zhuxy20190326…Ë÷√æµœÒ
	void SetCamLineSpeed(int nParam);//add by zhuxy20190404…Ë÷√––∆µ
	void CheckCamStatus();
private:
	bool ConnectCamera();
	BOOL CreateObjects();
	BOOL DestroyObjects();

public:
	SapAcqDevice   *m_pAcqDevice;
	SapBuffer	   *m_pBuffers;
	SapTransfer	   *m_pXfer;
};

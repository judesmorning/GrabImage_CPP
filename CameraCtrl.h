#pragma once

#include "ImageDataMgr.h"

class CCameraCtrl 
{
public:
	CCameraCtrl(void);
	virtual ~CCameraCtrl(void);
protected:
	virtual bool Init() = 0;
public:
	virtual bool Init(int nIndex) = 0;
	virtual void Destory() = 0;

	//采集操作
	virtual bool PrepareGrab() = 0;
	virtual bool StartGrab() = 0;
	virtual bool StopGrab() = 0;

	//相机状态监测
	virtual void CheckCamStatus() = 0;

	//相机参数设置
	virtual void SetCamTrigger(int nParam) = 0;
	virtual void SetCamExposure(int nParam) = 0;
	virtual void SetCamGain(int nParam) = 0;
	virtual void SetCamGrabSize(int nWidth, int nHeight) = 0;
	virtual void SetCamHorzFlip(const int& ) = 0;//add by zhuxy20190326设置镜像
	virtual void SetCamLineSpeed(int nParam) = 0;
public:
	int m_nCamIndex;
	int m_nCamStatus;    //相机状态
	int m_nWidth;
	int m_nHeight;
};

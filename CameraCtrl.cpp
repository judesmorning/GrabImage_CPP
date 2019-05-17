#include "StdAfx.h"
#include "ImageGrab.h"
#include "CameraCtrl.h"

CCameraCtrl::CCameraCtrl(void)
{
	m_nCamStatus = CAM_STATUS_NOT_FIND_DEVICE;
	m_nCamIndex = 0;

}

CCameraCtrl::~CCameraCtrl(void)
{
}

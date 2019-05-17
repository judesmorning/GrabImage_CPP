#pragma once

#include "CameraCtrl.h"
#include <pylon/PylonIncludes.h>

using namespace Pylon;
using namespace GenApi;

class CRacerAreaCtrl : public CImageEventHandler             // For receiving grabbed images.
	    , public CConfigurationEventHandler     // For getting notified about device removal.
        , public CCameraCtrl
{
public:
	CRacerAreaCtrl(void);
	~CRacerAreaCtrl(void);

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
	// Instant Camera Image Event.
	// This is where we are going the receive the grabbed images.
	// This method is called from the Instant Camera grab loop thread.
	virtual void OnImageGrabbed( Pylon::CInstantCamera& camera, const Pylon::CGrabResultPtr& ptrGrabResult);

	// Instant Camera Configuration Event.
	// This method is called from the Instant Camera grab loop thread.
	virtual void OnGrabError( Pylon::CInstantCamera& camera, const wchar_t* errorMessage);

	// Instant Camera Configuration Event.
	// This method is called from additional Instant Camera thread.
	virtual void OnCameraDeviceRemoved( Pylon::CInstantCamera& camera);

private:
	bool ConnectCamera();

private:
	CInstantCamera *m_pCamera;
};

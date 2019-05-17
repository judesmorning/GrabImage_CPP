#pragma once

//#include "RacerCtrl.h"
//#include "RacerAreaCtrl.h"
#include "LineaCtrl.h"
#include "RacerCtrl.h"
//#include "SpringCtrl.h"
//#include "P4Ctrl.h"

class CCameraMgr
{
public:
	CCameraMgr(void);
	~CCameraMgr(void);

	void Init();
	void Destory();
	void PrepareGrab(int index=0);
	void StartGrab(int index=0);
	void StopGrab();
	void StopGrab(int nCamCtrl);
	void CheckCamStatus();
	void CheckCamStatus(int nCamCtrl);
	void SetCameraParam();

public:
	vector<CCameraCtrl*> m_vCamCtrl; 
};


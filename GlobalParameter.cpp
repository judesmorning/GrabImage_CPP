#include "stdafx.h"
#include "GlobalParameter.h"

GlobalParameter::GlobalParameter()
{
}

GlobalParameter::~GlobalParameter()
{
}

GlobalParameter *GlobalParameter::GetInstance()
{
	static GlobalParameter gp;
	return &gp;
}

void GlobalParameter::Read()
{
	CJtvCfgData cfgData;
	cfgData.GetConfig(m_tCfg);
	cfgData.GetCameraCfg(m_tCfg.nCamNum, m_vCamCfg);

	if(m_tCfg.nCamNum != m_vCamCfg.size())
	{
		MessageBox(NULL, _T("�������������ò��������ϣ����޸�!"), _T("��ʾ"), MB_OK|MB_ICONWARNING);
		exit(1);
	}

	for (int i=0; i<m_tCfg.nCamNum; i++)
	{
		ImgGrabInfo imgGrabInfo;
		m_vImgGrabInfo.push_back(imgGrabInfo);//һ���������Ӧһ��GrabInfo
	}
}

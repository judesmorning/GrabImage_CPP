#pragma once
#include "JtvCfgData.h"
//������
class GlobalParameter
{
private:
	GlobalParameter();
	~GlobalParameter();

public:
	static GlobalParameter *GetInstance();

public:
	void Read();

public:
	config m_tCfg;//�豸��������Ϣ���������ip��
	vector<CameraCfg> m_vCamCfg;//ÿ������Ĳ������ع�ʱ�䡢������ʽ��
	vector<ImgGrabInfo> m_vImgGrabInfo;//ÿ������ɼ�����Ƭ����Ϣ���Ѳɼ����Ѵ洢��
	TrainInfo m_tTrainInfo;//������Ϣ��ʱ�䡢����
	CString imgPaths[8];//add by zhuxy ����ÿ��ͨ����ͼƬ·��
};

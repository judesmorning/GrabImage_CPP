#pragma once
#include "JtvCfgData.h"
//单例类
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
	config m_tCfg;//设备的配置信息，相机数、ip等
	vector<CameraCfg> m_vCamCfg;//每个相机的参数，曝光时间、触发方式等
	vector<ImgGrabInfo> m_vImgGrabInfo;//每个相机采集的照片的信息，已采集、已存储等
	TrainInfo m_tTrainInfo;//来车信息，时间、车号
	CString imgPaths[8];//add by zhuxy 保存每个通道的图片路径
};

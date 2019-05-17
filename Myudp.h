#pragma once

// Myudp 命令目标
#include "GlobalParameter.h"
#include "Utils.h"
class Myudp : public CSocket
{
public:
	bool stopSend;//停止发送标志位
	int spareCount;
	std::vector<CString> spareGatherIPS;
public:
	Myudp();
	virtual ~Myudp();
	virtual void OnReceive(int nErrorCode);
	void work();
};

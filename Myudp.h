#pragma once

// Myudp ����Ŀ��
#include "GlobalParameter.h"
#include "Utils.h"
class Myudp : public CSocket
{
public:
	bool stopSend;//ֹͣ���ͱ�־λ
	int spareCount;
	std::vector<CString> spareGatherIPS;
public:
	Myudp();
	virtual ~Myudp();
	virtual void OnReceive(int nErrorCode);
	void work();
};


#pragma once

#include "CameraMgr.h"

extern BOOL bExit;

/////////////////////////////////////////////////////////////////////////////
// CManagerThread thread

class CManagerThread : public CWinThread
{
	DECLARE_DYNCREATE(CManagerThread)

protected:
	CManagerThread();           // protected constructor used by dynamic creation

protected:
	virtual ~CManagerThread();

public:
	afx_msg void OnPrepareGrab(WPARAM wParam, LPARAM lParam);
	afx_msg void OnStartGrab(WPARAM wParam, LPARAM lParam);
	afx_msg void OnStopGrab(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	bool CtlMsgHandleTwo(SOCKET socket);
protected:
	CWinThread *m_pListenThread;

private:
	BOOL makeDirForAllImageChanel(int camCount);//add by zhuxy 20190315根据存储路径新建目录
};

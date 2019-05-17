
// ImageGrab.h : ImageGrab Ӧ�ó������ͷ�ļ�
//
#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"       // ������
#include "ManagerThread.h"
#include "LogFile.h"
#include "Myserial.h"
#include "Mydialog.h"
#include "Mylog.h"
#include "Myudp.h"
// CImageGrabApp:
// �йش����ʵ�֣������ ImageGrab.cpp
//

class CImageGrabApp : public CWinApp
{
public:
	CImageGrabApp();


// ��д
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// ʵ��

public:
	afx_msg void OnAppAbout();
	afx_msg void OnStartGrab();
	afx_msg void OnStopGrab(); 
	afx_msg void OnCameraSet();
	afx_msg void OnOpenLog();
	DECLARE_MESSAGE_MAP()

public:
	CManagerThread *m_pMgrThread;
	CString m_strCurrentPath;  //��ǰĿ¼
	CString m_strCurrentImgSavePath;  //��ǰͼ��洢·��
	CString m_strCamName;  //��ǰ�������
	CString m_strTrainId;  //��ǰ����

private:
	BOOL myDeleteDirectory(CString directory_path);//add by zhuxy 20190319 �ݹ�ɾ���ļ���
	BOOL removeOldDir();//add by zhuxy 20190319	ɾ�����ڵ��ļ���
	ComAsy com;//add by zhuxy 20190325���ڽӿ�
	Myudp* udp;
};

extern CImageGrabApp theApp;
extern CCameraMgr gCamMgr;
extern CImageDataMgr gDataMgr;
extern CLogFile gLogFile;
extern int workWay;//add by zhuxy 20190321	��־��ǰ�ɼ�����
extern CString trainNumStr;//add by zhuxy 20190322 ��ǰ����
extern HANDLE uiHandle;//���һֱ�������ÿ��ܻ����,���������ڶ��̻߳���,��ʵ�����ź���
extern CString trainTimeStr;//����ʱ��
extern CString lightStateStr;//����״̬

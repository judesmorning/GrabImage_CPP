
// ImageGrab.h : ImageGrab 应用程序的主头文件
//
#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"       // 主符号
#include "ManagerThread.h"
#include "LogFile.h"
#include "Myserial.h"
#include "Mydialog.h"
#include "Mylog.h"
#include "Myudp.h"
// CImageGrabApp:
// 有关此类的实现，请参阅 ImageGrab.cpp
//

class CImageGrabApp : public CWinApp
{
public:
	CImageGrabApp();


// 重写
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// 实现

public:
	afx_msg void OnAppAbout();
	afx_msg void OnStartGrab();
	afx_msg void OnStopGrab(); 
	afx_msg void OnCameraSet();
	afx_msg void OnOpenLog();
	DECLARE_MESSAGE_MAP()

public:
	CManagerThread *m_pMgrThread;
	CString m_strCurrentPath;  //当前目录
	CString m_strCurrentImgSavePath;  //当前图像存储路径
	CString m_strCamName;  //当前相机名称
	CString m_strTrainId;  //当前车号

private:
	BOOL myDeleteDirectory(CString directory_path);//add by zhuxy 20190319 递归删除文件夹
	BOOL removeOldDir();//add by zhuxy 20190319	删除过期的文件夹
	ComAsy com;//add by zhuxy 20190325串口接口
	Myudp* udp;
};

extern CImageGrabApp theApp;
extern CCameraMgr gCamMgr;
extern CImageDataMgr gDataMgr;
extern CLogFile gLogFile;
extern int workWay;//add by zhuxy 20190321	标志当前采集动作
extern CString trainNumStr;//add by zhuxy 20190322 当前车号
extern HANDLE uiHandle;//如果一直保存设置可能会出错,这个句柄用于多线程互斥,其实就是信号量
extern CString trainTimeStr;//过车时间
extern CString lightStateStr;//激光状态

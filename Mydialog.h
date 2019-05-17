#pragma once

#include "Mycamset.h"
#include "GlobalParameter.h"
#include "Mylog.h"

// Mydialog 对话框
//add by zhuxy20190327  
//用于显示图片
#define UI_TIMER_ID 100
class Mydialog : public CDialogEx
{
	DECLARE_DYNAMIC(Mydialog)

public:
	Mydialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~Mydialog();

// 对话框数据
	enum { IDD = IDD_MY_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

#if OPEN_IF //add by zhuxy 图像显示
private://方法
	void init();//变量初始化
	void ctrlsInit();//所有控件的初始化
	void makeStateUI(CDC* dc,int left,int top,int camNum);//生成状态UI，通过CDC画图
	void makeCtrlUI(int left,int top,int camNum);//生成控制相关的界面
	void makeCamSetUI(int left,int top,int camNum);//设置界面
	void makeAllCamCtrlUI(int left,int top);//一键按钮界面
	static UINT newShowImageThread(LPVOID param);//双缓冲的显示线程
private://属性
	std::vector<CButton*> pStartBtnVc;//开始采集按钮控件
	std::vector<CButton*> pStopBtnVc;//停止采集按钮控件
	std::vector<CButton*> pUploadBtnVc;//上传按钮控件
	std::vector<Mycamset*> pCamSetDlgVc;//设置的界面
	std::vector<CImage*> pImgVc;//CImage数组
	std::vector<CButton*> pBtnForAllCamVc;//所有的设备的按钮控件，打开、关闭、上传所有
	
	volatile bool RUN_FLAG;//停止线程标志位
	CString imgDefaultPath;//默认图片路径
	CFont font;//状态的字体
private://回调
	afx_msg void OnBtnForAllCamClik(UINT uID);//一键按钮群
	afx_msg void OnBtnStartClik(UINT uID);//开始采集按钮
	afx_msg void OnBtnStopClik(UINT uID);//停止采集按钮
	afx_msg void OnBtnUploadClik(UINT uID);//上传按钮
private://双缓冲绘图
	CDC memDc;
#endif

public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

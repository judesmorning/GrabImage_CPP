// Mycamset.cpp : 实现文件
//

#include "stdafx.h"
#include "ImageGrab.h"
#include "Mycamset.h"
#include "afxdialogex.h"


// Mycamset 对话框

IMPLEMENT_DYNAMIC(Mycamset, CDialogEx)

Mycamset::Mycamset(int camIndex,CWnd* pParent /*=NULL*/)
	: CDialogEx(Mycamset::IDD, pParent)
{
	this->camIndex = camIndex;
}

Mycamset::~Mycamset()
{

}

void Mycamset::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Mycamset, CDialogEx)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_BUTTON3, &Mycamset::OnBnClickedButton3)
	ON_WM_PAINT()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// Mycamset 消息处理程序


void Mycamset::PostNcDestroy()
{
	// TODO: 在此添加专用代码和/或调用基类
	delete this;
	CDialogEx::PostNcDestroy();
}

//函数名称:控件初始化
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:add by zhuxy20190402
//备注:NULL
void Mycamset::init(const int& camIndex)
{
	RET_IF_NOT_EAQU((camIndex>=0),true);
	RET_IF_NOT_EAQU((camIndex<GlobalParameter::GetInstance()->m_tCfg.nCamNum),true);
	CameraCfg cfg = GlobalParameter::GetInstance()->m_vCamCfg[camIndex];
	CString tmpStr;

	SetDlgItemTextW(IDC_EDIT1,cfg.strCamName.c_str());//相机名称
	SetDlgItemTextW(IDC_EDIT2,cfg.strCamDesc.c_str());//相机描述
	if(0==cfg.nTriggerMode)//触发方式
	{
		((CButton*)GetDlgItem(IDC_RADIO1))->SetCheck(BST_CHECKED);
	}
	else if (1==cfg.nTriggerMode)
	{
		((CButton*)GetDlgItem(IDC_RADIO2))->SetCheck(BST_CHECKED);
	}
	tmpStr.Format(_T("%d"),cfg.nExposureTime);//曝光时间
	SetDlgItemTextW(IDC_EDIT3,tmpStr);
	tmpStr.Format(_T("%d"),cfg.nGain);//增益
	SetDlgItemTextW(IDC_EDIT4,tmpStr);
	//镜像
	if (0==cfg.nHorzFlipFlag)
	{
		((CButton*)GetDlgItem(IDC_CHECK2))->SetCheck(BST_UNCHECKED);
	}
	else if(1==cfg.nHorzFlipFlag)
	{
		((CButton*)GetDlgItem(IDC_CHECK2))->SetCheck(BST_CHECKED);
	}
	//行频
	tmpStr.Format(_T("%d"),cfg.nLineSpeed);
	SetDlgItemTextW(IDC_EDIT7,tmpStr);
	//图片大小
	tmpStr.Format(_T("%d"),cfg.nWidth);
	SetDlgItemTextW(IDC_EDIT5,tmpStr);
	tmpStr.Format(_T("%d"),cfg.nHeight);
	SetDlgItemTextW(IDC_EDIT6,tmpStr);
	//压缩比
	tmpStr.Format(_T("%d"),cfg.nImgQuality);
	SetDlgItemTextW(IDC_EDIT8,tmpStr);
	//保存状态
	((CStatic*)GetDlgItem(IDC_STATIC_SAVE))->ShowWindow(SW_HIDE);
}

//函数名称:保存配置
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:add by zhuxy20190402
//备注:NULL
void Mycamset::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	if(STATE_READY != GlobalParameter::GetInstance()->m_tTrainInfo.workState)
	{
		AfxMessageBox(_T("相机未就绪,请就绪所有相机再修改参数"));
		return;
	}
	//第一步:把相关配置存到配置文件
	CJtvCfgData cfgData;
	CString tmpStr;
	cfgData.SetMainKey(1, camIndex+1);
	GetDlgItemTextW(IDC_EDIT1,tmpStr);//相机名称
	cfgData.SetText(_T("CamName"), tmpStr);
	GetDlgItemTextW(IDC_EDIT2,tmpStr);//相机描述
	cfgData.SetText(_T("CamDesc"), tmpStr);
	if (BST_CHECKED == ((CButton*)GetDlgItem(IDC_RADIO1))->GetCheck())//触发方式
	{
		cfgData.SetDWORD(_T("TriggerMode"), 0);
		((CEdit*)GetDlgItem(IDC_EDIT7))->EnableWindow(true);
	}
	else
	{
		cfgData.SetDWORD(_T("TriggerMode"), 1);
		((CEdit*)GetDlgItem(IDC_EDIT7))->EnableWindow(false);
	}
	GetDlgItemTextW(IDC_EDIT3,tmpStr);//曝光
	cfgData.SetDWORD(_T("ExposureTime"), _wtol( tmpStr));
	GetDlgItemTextW(IDC_EDIT4,tmpStr);//增益
	cfgData.SetDWORD(_T("Gain"), _wtol( tmpStr));
	if (BST_CHECKED == ((CButton*)GetDlgItem(IDC_CHECK2))->GetCheck())//镜像
	{
		cfgData.SetDWORD(_T("HorzFlipFlag"), 1);
	}
	else
	{
		cfgData.SetDWORD(_T("HorzFlipFlag"), 0);
	}
	GetDlgItemTextW(IDC_EDIT7,tmpStr);//行频
	cfgData.SetDWORD(_T("LineSpeed"), _wtol( tmpStr));

	GetDlgItemTextW(IDC_EDIT5,tmpStr);//长
	cfgData.SetDWORD(_T("Width"), _wtol( tmpStr));
	GetDlgItemTextW(IDC_EDIT6,tmpStr);//宽
	cfgData.SetDWORD(_T("Height"), _wtol( tmpStr));
	GetDlgItemTextW(IDC_EDIT8,tmpStr);//压缩比
	cfgData.SetDWORD(_T("ImgQuality"), _wtol( tmpStr));

	//第二步:把配置文件读到内存
	WaitForSingleObject(uiHandle,INFINITE);
	GlobalParameter::GetInstance()->Read();
	ReleaseSemaphore(uiHandle,1,NULL);
	//第三步:把设置存入相机
	GlobalParameter *pGlobalParam = GlobalParameter::GetInstance();
	gCamMgr.m_vCamCtrl[camIndex]->SetCamExposure(pGlobalParam->m_vCamCfg[camIndex].nExposureTime);
	gCamMgr.m_vCamCtrl[camIndex]->SetCamGain(pGlobalParam->m_vCamCfg[camIndex].nGain);
	gCamMgr.m_vCamCtrl[camIndex]->SetCamTrigger(pGlobalParam->m_vCamCfg[camIndex].nTriggerMode);
	gCamMgr.m_vCamCtrl[camIndex]->SetCamHorzFlip(pGlobalParam->m_vCamCfg[camIndex].nHorzFlipFlag);//add by zhuxy20190326设置镜像
	gCamMgr.m_vCamCtrl[camIndex]->SetCamLineSpeed(pGlobalParam->m_vCamCfg[camIndex].nLineSpeed);//add by zhuxy20190404设置行频

	((CStatic*)GetDlgItem(IDC_STATIC_SAVE))->ShowWindow(SW_SHOW);
	SetTimer(100,3000,NULL);
}

//函数名称:窗口初始化
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:add by zhuxy20190402
//备注:NULL
BOOL Mycamset::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	this->init(camIndex);//初始化
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

//函数名称:重绘
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:add by zhuxy20190404
//备注:NULL
void Mycamset::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialogEx::OnPaint()
#if OPEN_IF
	CRect rect;
	GetClientRect(rect);
	dc.FillSolidRect(rect,RGB(255,255,255));
#endif
}


void Mycamset::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	((CStatic*)GetDlgItem(IDC_STATIC_SAVE))->ShowWindow(SW_HIDE);
	KillTimer(100);
	CDialogEx::OnTimer(nIDEvent);
}

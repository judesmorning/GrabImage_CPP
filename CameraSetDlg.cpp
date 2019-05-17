// CameraSetDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ImageGrab.h"
#include "CameraSetDlg.h"
#include "afxdialogex.h"


// CameraSetDlg 对话框

IMPLEMENT_DYNAMIC(CameraSetDlg, CDialogEx)

CameraSetDlg::CameraSetDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CameraSetDlg::IDD, pParent)
	, m_strDesc(_T(""))
	, m_nGain(0)
	, m_nExposure(0)
{

}

CameraSetDlg::~CameraSetDlg()
{
}

void CameraSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_DESC, m_strDesc);
	DDX_Control(pDX, IDC_CAM_NAME_COMBO, m_CamNameComBo);
	DDX_Text(pDX, IDC_EXPOSURE, m_nExposure);
	DDX_Text(pDX, IDC_GAIN, m_nGain);
	DDX_Control(pDX, IDC_INTERNAL_TRIGGER, m_InterTrigger);
	DDX_Control(pDX, IDC_EXTERNAL_TRIGGER, m_ExterTrigger);
}

BOOL CameraSetDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	GlobalParameter *pGlobalParam = GlobalParameter::GetInstance();

	int nCamNum = gCamMgr.m_vCamCtrl.size();
	for (int i=0; i<nCamNum; i++)
	{
		m_CamNameComBo.InsertString(i, pGlobalParam->m_vCamCfg[i].strCamName.c_str());
	}

	m_CamNameComBo.SetCurSel(0);
	int nCount = m_CamNameComBo.GetCount();
	if (nCount > 0)
	{
		CameraCfg camCfg = pGlobalParam->m_vCamCfg[0];
		m_strDesc = camCfg.strCamDesc.c_str();
		m_nExposure = camCfg.nExposureTime;
		m_nGain = camCfg.nGain;

		if (camCfg.nTriggerMode == 1)
		{
			m_InterTrigger.SetCheck(0);
			m_ExterTrigger.SetCheck(1);
		}
		else
		{
			m_InterTrigger.SetCheck(1);
			m_ExterTrigger.SetCheck(0);
		}
		if (1==pGlobalParam->m_vCamCfg[0].nHorzFlipFlag)//add by zhuxy 20190326根据新的配置界面添加代码 是否镜像
		{
			CButton* btn = (CButton*)GetDlgItem(IDC_CHECK1);
			btn->SetCheck(1);
		}
		UpdateData(FALSE);
	}
	//add by zhuxy 20190326根据新的配置界面添加代码
	CEdit* edit = (CEdit*)GetDlgItem(IDC_SERVERIP);//服务器ip
	CString str(pGlobalParam->m_tCfg.strSerIp.c_str());
	edit->SetWindowTextW(str);
	edit = (CEdit*)GetDlgItem(IDC_SERVERPORT);//服务器端口
	str.Format(_T("%d"),pGlobalParam->m_tCfg.nSerPort);
	edit->SetWindowTextW(str);
	edit = (CEdit*)GetDlgItem(IDC_CTRLPORT);//控制端口
	str.Format(_T("%d"),pGlobalParam->m_tCfg.nCtrlPort);
	edit->SetWindowTextW(str);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CameraSetDlg, CDialogEx)
	ON_BN_CLICKED(IDC_SAVE, &CameraSetDlg::OnBnClickedSave)
	ON_CBN_SELCHANGE(IDC_CAM_NAME_COMBO, &CameraSetDlg::OnCbnSelchangeCamNameCombo)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CameraSetDlg 消息处理程序


void CameraSetDlg::OnBnClickedSave()
{
	// TODO: 在此添加控件通知处理程序代码
	int nCurSel = m_CamNameComBo.GetCurSel();	
	if (nCurSel > -1 && nCurSel < gCamMgr.m_vCamCtrl.size())//add by zhuxy 增加注释 这里设置相机参数
	{
		if (gCamMgr.m_vCamCtrl[nCurSel]->m_nCamStatus == CAM_STATUS_OPEN_SUCCEED)
		{
			UpdateData(TRUE);

			GlobalParameter *pGlobalParam = GlobalParameter::GetInstance();
			pGlobalParam->m_vCamCfg[nCurSel].strCamDesc = m_strDesc;
			pGlobalParam->m_vCamCfg[nCurSel].nExposureTime = m_nExposure;
			pGlobalParam->m_vCamCfg[nCurSel].nGain = m_nGain;

			if (m_InterTrigger.GetCheck() == 1)
			{
				pGlobalParam->m_vCamCfg[nCurSel].nTriggerMode = 0;
			}
			else
			{
				pGlobalParam->m_vCamCfg[nCurSel].nTriggerMode = 1;
			}
			if(1==((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck())//add by zhuxy20190326设置镜像
			{
				pGlobalParam->m_vCamCfg[nCurSel].nHorzFlipFlag = 1;
			}
			CJtvCfgData cfgData;
			cfgData.SetMainKey(0, nCurSel+1);
			cfgData.SetDWORD(_T("ExposureTime"), pGlobalParam->m_vCamCfg[nCurSel].nExposureTime);
			cfgData.SetDWORD(_T("Gain"), pGlobalParam->m_vCamCfg[nCurSel].nGain);
			cfgData.SetDWORD(_T("TriggerMode"), pGlobalParam->m_vCamCfg[nCurSel].nTriggerMode);
			cfgData.SetText(_T("CamDesc"), pGlobalParam->m_vCamCfg[nCurSel].strCamDesc.c_str());
			cfgData.SetDWORD(_T("HorzFlipFlag"), pGlobalParam->m_vCamCfg[nCurSel].nHorzFlipFlag);//add by zhuxy20190326设置镜像 写入ini
			gCamMgr.m_vCamCtrl[nCurSel]->SetCamExposure(pGlobalParam->m_vCamCfg[nCurSel].nExposureTime);
			gCamMgr.m_vCamCtrl[nCurSel]->SetCamGain(pGlobalParam->m_vCamCfg[nCurSel].nGain);
			gCamMgr.m_vCamCtrl[nCurSel]->SetCamTrigger(pGlobalParam->m_vCamCfg[nCurSel].nTriggerMode);
			gCamMgr.m_vCamCtrl[nCurSel]->SetCamHorzFlip(pGlobalParam->m_vCamCfg[nCurSel].nHorzFlipFlag);//add by zhuxy20190326设置镜像
			::MessageBox(NULL, _T("保存成功,部分功能需要重启后生效!"), _T("提示"), MB_OK);//fix by zhuxy 部分功能需要重启,主要是针对ip和端口
			return;
		}
	}

	::MessageBox(NULL, _T("保存失败!"), _T("提示"), MB_OK);
}



void CameraSetDlg::OnCbnSelchangeCamNameCombo()
{
	// TODO: 在此添加控件通知处理程序代码

	int nCurSel = m_CamNameComBo.GetCurSel();
	if (nCurSel > -1 && nCurSel < gCamMgr.m_vCamCtrl.size())
	{
		GlobalParameter *pGlobalParam = GlobalParameter::GetInstance();
		CameraCfg camCfg = pGlobalParam->m_vCamCfg[nCurSel];
		m_strDesc = camCfg.strCamDesc.c_str();
		m_nExposure = camCfg.nExposureTime;
		m_nGain = camCfg.nGain;

		if (camCfg.nTriggerMode == 1)
		{
			m_InterTrigger.SetCheck(0);
			m_ExterTrigger.SetCheck(1);
		}
		else
		{
			m_InterTrigger.SetCheck(1);
			m_ExterTrigger.SetCheck(0);
		}
		//add by zhuxy 20190326根据新的配置界面添加代码
		if (1==pGlobalParam->m_vCamCfg[nCurSel].nHorzFlipFlag)//是否镜像
		{
			CButton* btn = (CButton*)GetDlgItem(IDC_CHECK1);
			btn->SetCheck(1);
		}
	}

	UpdateData(FALSE);
}


void CameraSetDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnTimer(nIDEvent);
}

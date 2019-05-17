// CameraSetDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ImageGrab.h"
#include "CameraSetDlg.h"
#include "afxdialogex.h"


// CameraSetDlg �Ի���

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
		if (1==pGlobalParam->m_vCamCfg[0].nHorzFlipFlag)//add by zhuxy 20190326�����µ����ý�����Ӵ��� �Ƿ���
		{
			CButton* btn = (CButton*)GetDlgItem(IDC_CHECK1);
			btn->SetCheck(1);
		}
		UpdateData(FALSE);
	}
	//add by zhuxy 20190326�����µ����ý�����Ӵ���
	CEdit* edit = (CEdit*)GetDlgItem(IDC_SERVERIP);//������ip
	CString str(pGlobalParam->m_tCfg.strSerIp.c_str());
	edit->SetWindowTextW(str);
	edit = (CEdit*)GetDlgItem(IDC_SERVERPORT);//�������˿�
	str.Format(_T("%d"),pGlobalParam->m_tCfg.nSerPort);
	edit->SetWindowTextW(str);
	edit = (CEdit*)GetDlgItem(IDC_CTRLPORT);//���ƶ˿�
	str.Format(_T("%d"),pGlobalParam->m_tCfg.nCtrlPort);
	edit->SetWindowTextW(str);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CameraSetDlg, CDialogEx)
	ON_BN_CLICKED(IDC_SAVE, &CameraSetDlg::OnBnClickedSave)
	ON_CBN_SELCHANGE(IDC_CAM_NAME_COMBO, &CameraSetDlg::OnCbnSelchangeCamNameCombo)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CameraSetDlg ��Ϣ�������


void CameraSetDlg::OnBnClickedSave()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int nCurSel = m_CamNameComBo.GetCurSel();	
	if (nCurSel > -1 && nCurSel < gCamMgr.m_vCamCtrl.size())//add by zhuxy ����ע�� ���������������
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
			if(1==((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck())//add by zhuxy20190326���þ���
			{
				pGlobalParam->m_vCamCfg[nCurSel].nHorzFlipFlag = 1;
			}
			CJtvCfgData cfgData;
			cfgData.SetMainKey(0, nCurSel+1);
			cfgData.SetDWORD(_T("ExposureTime"), pGlobalParam->m_vCamCfg[nCurSel].nExposureTime);
			cfgData.SetDWORD(_T("Gain"), pGlobalParam->m_vCamCfg[nCurSel].nGain);
			cfgData.SetDWORD(_T("TriggerMode"), pGlobalParam->m_vCamCfg[nCurSel].nTriggerMode);
			cfgData.SetText(_T("CamDesc"), pGlobalParam->m_vCamCfg[nCurSel].strCamDesc.c_str());
			cfgData.SetDWORD(_T("HorzFlipFlag"), pGlobalParam->m_vCamCfg[nCurSel].nHorzFlipFlag);//add by zhuxy20190326���þ��� д��ini
			gCamMgr.m_vCamCtrl[nCurSel]->SetCamExposure(pGlobalParam->m_vCamCfg[nCurSel].nExposureTime);
			gCamMgr.m_vCamCtrl[nCurSel]->SetCamGain(pGlobalParam->m_vCamCfg[nCurSel].nGain);
			gCamMgr.m_vCamCtrl[nCurSel]->SetCamTrigger(pGlobalParam->m_vCamCfg[nCurSel].nTriggerMode);
			gCamMgr.m_vCamCtrl[nCurSel]->SetCamHorzFlip(pGlobalParam->m_vCamCfg[nCurSel].nHorzFlipFlag);//add by zhuxy20190326���þ���
			::MessageBox(NULL, _T("����ɹ�,���ֹ�����Ҫ��������Ч!"), _T("��ʾ"), MB_OK);//fix by zhuxy ���ֹ�����Ҫ����,��Ҫ�����ip�Ͷ˿�
			return;
		}
	}

	::MessageBox(NULL, _T("����ʧ��!"), _T("��ʾ"), MB_OK);
}



void CameraSetDlg::OnCbnSelchangeCamNameCombo()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

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
		//add by zhuxy 20190326�����µ����ý�����Ӵ���
		if (1==pGlobalParam->m_vCamCfg[nCurSel].nHorzFlipFlag)//�Ƿ���
		{
			CButton* btn = (CButton*)GetDlgItem(IDC_CHECK1);
			btn->SetCheck(1);
		}
	}

	UpdateData(FALSE);
}


void CameraSetDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CDialogEx::OnTimer(nIDEvent);
}

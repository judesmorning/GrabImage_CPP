// Mycamset.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ImageGrab.h"
#include "Mycamset.h"
#include "afxdialogex.h"


// Mycamset �Ի���

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


// Mycamset ��Ϣ�������


void Mycamset::PostNcDestroy()
{
	// TODO: �ڴ����ר�ô����/����û���
	delete this;
	CDialogEx::PostNcDestroy();
}

//��������:�ؼ���ʼ��
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:add by zhuxy20190402
//��ע:NULL
void Mycamset::init(const int& camIndex)
{
	RET_IF_NOT_EAQU((camIndex>=0),true);
	RET_IF_NOT_EAQU((camIndex<GlobalParameter::GetInstance()->m_tCfg.nCamNum),true);
	CameraCfg cfg = GlobalParameter::GetInstance()->m_vCamCfg[camIndex];
	CString tmpStr;

	SetDlgItemTextW(IDC_EDIT1,cfg.strCamName.c_str());//�������
	SetDlgItemTextW(IDC_EDIT2,cfg.strCamDesc.c_str());//�������
	if(0==cfg.nTriggerMode)//������ʽ
	{
		((CButton*)GetDlgItem(IDC_RADIO1))->SetCheck(BST_CHECKED);
	}
	else if (1==cfg.nTriggerMode)
	{
		((CButton*)GetDlgItem(IDC_RADIO2))->SetCheck(BST_CHECKED);
	}
	tmpStr.Format(_T("%d"),cfg.nExposureTime);//�ع�ʱ��
	SetDlgItemTextW(IDC_EDIT3,tmpStr);
	tmpStr.Format(_T("%d"),cfg.nGain);//����
	SetDlgItemTextW(IDC_EDIT4,tmpStr);
	//����
	if (0==cfg.nHorzFlipFlag)
	{
		((CButton*)GetDlgItem(IDC_CHECK2))->SetCheck(BST_UNCHECKED);
	}
	else if(1==cfg.nHorzFlipFlag)
	{
		((CButton*)GetDlgItem(IDC_CHECK2))->SetCheck(BST_CHECKED);
	}
	//��Ƶ
	tmpStr.Format(_T("%d"),cfg.nLineSpeed);
	SetDlgItemTextW(IDC_EDIT7,tmpStr);
	//ͼƬ��С
	tmpStr.Format(_T("%d"),cfg.nWidth);
	SetDlgItemTextW(IDC_EDIT5,tmpStr);
	tmpStr.Format(_T("%d"),cfg.nHeight);
	SetDlgItemTextW(IDC_EDIT6,tmpStr);
	//ѹ����
	tmpStr.Format(_T("%d"),cfg.nImgQuality);
	SetDlgItemTextW(IDC_EDIT8,tmpStr);
	//����״̬
	((CStatic*)GetDlgItem(IDC_STATIC_SAVE))->ShowWindow(SW_HIDE);
}

//��������:��������
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:add by zhuxy20190402
//��ע:NULL
void Mycamset::OnBnClickedButton3()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(STATE_READY != GlobalParameter::GetInstance()->m_tTrainInfo.workState)
	{
		AfxMessageBox(_T("���δ����,���������������޸Ĳ���"));
		return;
	}
	//��һ��:��������ô浽�����ļ�
	CJtvCfgData cfgData;
	CString tmpStr;
	cfgData.SetMainKey(1, camIndex+1);
	GetDlgItemTextW(IDC_EDIT1,tmpStr);//�������
	cfgData.SetText(_T("CamName"), tmpStr);
	GetDlgItemTextW(IDC_EDIT2,tmpStr);//�������
	cfgData.SetText(_T("CamDesc"), tmpStr);
	if (BST_CHECKED == ((CButton*)GetDlgItem(IDC_RADIO1))->GetCheck())//������ʽ
	{
		cfgData.SetDWORD(_T("TriggerMode"), 0);
		((CEdit*)GetDlgItem(IDC_EDIT7))->EnableWindow(true);
	}
	else
	{
		cfgData.SetDWORD(_T("TriggerMode"), 1);
		((CEdit*)GetDlgItem(IDC_EDIT7))->EnableWindow(false);
	}
	GetDlgItemTextW(IDC_EDIT3,tmpStr);//�ع�
	cfgData.SetDWORD(_T("ExposureTime"), _wtol( tmpStr));
	GetDlgItemTextW(IDC_EDIT4,tmpStr);//����
	cfgData.SetDWORD(_T("Gain"), _wtol( tmpStr));
	if (BST_CHECKED == ((CButton*)GetDlgItem(IDC_CHECK2))->GetCheck())//����
	{
		cfgData.SetDWORD(_T("HorzFlipFlag"), 1);
	}
	else
	{
		cfgData.SetDWORD(_T("HorzFlipFlag"), 0);
	}
	GetDlgItemTextW(IDC_EDIT7,tmpStr);//��Ƶ
	cfgData.SetDWORD(_T("LineSpeed"), _wtol( tmpStr));

	GetDlgItemTextW(IDC_EDIT5,tmpStr);//��
	cfgData.SetDWORD(_T("Width"), _wtol( tmpStr));
	GetDlgItemTextW(IDC_EDIT6,tmpStr);//��
	cfgData.SetDWORD(_T("Height"), _wtol( tmpStr));
	GetDlgItemTextW(IDC_EDIT8,tmpStr);//ѹ����
	cfgData.SetDWORD(_T("ImgQuality"), _wtol( tmpStr));

	//�ڶ���:�������ļ������ڴ�
	WaitForSingleObject(uiHandle,INFINITE);
	GlobalParameter::GetInstance()->Read();
	ReleaseSemaphore(uiHandle,1,NULL);
	//������:�����ô������
	GlobalParameter *pGlobalParam = GlobalParameter::GetInstance();
	gCamMgr.m_vCamCtrl[camIndex]->SetCamExposure(pGlobalParam->m_vCamCfg[camIndex].nExposureTime);
	gCamMgr.m_vCamCtrl[camIndex]->SetCamGain(pGlobalParam->m_vCamCfg[camIndex].nGain);
	gCamMgr.m_vCamCtrl[camIndex]->SetCamTrigger(pGlobalParam->m_vCamCfg[camIndex].nTriggerMode);
	gCamMgr.m_vCamCtrl[camIndex]->SetCamHorzFlip(pGlobalParam->m_vCamCfg[camIndex].nHorzFlipFlag);//add by zhuxy20190326���þ���
	gCamMgr.m_vCamCtrl[camIndex]->SetCamLineSpeed(pGlobalParam->m_vCamCfg[camIndex].nLineSpeed);//add by zhuxy20190404������Ƶ

	((CStatic*)GetDlgItem(IDC_STATIC_SAVE))->ShowWindow(SW_SHOW);
	SetTimer(100,3000,NULL);
}

//��������:���ڳ�ʼ��
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:add by zhuxy20190402
//��ע:NULL
BOOL Mycamset::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	this->init(camIndex);//��ʼ��
	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

//��������:�ػ�
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:add by zhuxy20190404
//��ע:NULL
void Mycamset::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: �ڴ˴������Ϣ����������
	// ��Ϊ��ͼ��Ϣ���� CDialogEx::OnPaint()
#if OPEN_IF
	CRect rect;
	GetClientRect(rect);
	dc.FillSolidRect(rect,RGB(255,255,255));
#endif
}


void Mycamset::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	((CStatic*)GetDlgItem(IDC_STATIC_SAVE))->ShowWindow(SW_HIDE);
	KillTimer(100);
	CDialogEx::OnTimer(nIDEvent);
}

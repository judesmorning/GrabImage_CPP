// Mylog.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ImageGrab.h"
#include "Mylog.h"
#include "afxdialogex.h"


// Mylog �Ի���

IMPLEMENT_DYNAMIC(Mylog, CDialogEx)

Mylog::Mylog(CWnd* pParent /*=NULL*/)
	: CDialogEx(Mylog::IDD, pParent)
{
	
}

Mylog::~Mylog()
{
}

CString Mylog::USERNAME = _T("yunda");
CString Mylog::PASSWORD = _T("admin");
LogStruct Mylog::logInfo;
void Mylog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_LOGRET, m_log_result);
}


BEGIN_MESSAGE_MAP(Mylog, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &Mylog::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &Mylog::OnBnClickedOk)
	ON_WM_CREATE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// Mylog ��Ϣ�������

//��������:ȡ����¼��ť
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void Mylog::OnBnClickedCancel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CDialogEx::OnCancel();
	logInfo.isVaild = false;
}


//��������:��¼��ť
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void Mylog::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	GetDlgItemTextW(IDC_EDIT1,logInfo.userName);
	GetDlgItemTextW(IDC_EDIT3,logInfo.passWord);
	if(logInfo.userName==USERNAME && logInfo.passWord==PASSWORD)//admin��¼�ɹ�
	{
		logInfo.root = ROOT_ADMIN;
		logInfo.isVaild = true;
		m_log_result.ShowWindow(SW_HIDE);
		CDialogEx::OnOK();
		
	}
	else if(logInfo.userName==GlobalParameter::GetInstance()->m_tCfg.nRootName.c_str() && logInfo.passWord==GlobalParameter::GetInstance()->m_tCfg.nRootpswrd.c_str())//�ͻ���¼�ɹ�
	{
		logInfo.root = ROOT_GEUST;
		logInfo.isVaild = true;
		m_log_result.ShowWindow(SW_HIDE);
		CDialogEx::OnOK();
	}
	else//��¼ʧ��
	{
		logInfo.isVaild = false;		
		m_log_result.ShowWindow(SW_SHOW);
	}
}


int Mylog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  �ڴ������ר�õĴ�������

	
	return 0;
}

//��������:�޸ĵ�¼ʧ�ܵĿؼ���ɫ
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
HBRUSH Mylog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����
	if(pWnd->GetDlgCtrlID()==IDC_STATIC_LOGRET)
	{
		pDC->SetTextColor(RGB(255,0,0));
	}
	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}

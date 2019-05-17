// Mylog.cpp : 实现文件
//

#include "stdafx.h"
#include "ImageGrab.h"
#include "Mylog.h"
#include "afxdialogex.h"


// Mylog 对话框

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


// Mylog 消息处理程序

//函数名称:取消登录按钮
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void Mylog::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
	logInfo.isVaild = false;
}


//函数名称:登录按钮
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void Mylog::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItemTextW(IDC_EDIT1,logInfo.userName);
	GetDlgItemTextW(IDC_EDIT3,logInfo.passWord);
	if(logInfo.userName==USERNAME && logInfo.passWord==PASSWORD)//admin登录成功
	{
		logInfo.root = ROOT_ADMIN;
		logInfo.isVaild = true;
		m_log_result.ShowWindow(SW_HIDE);
		CDialogEx::OnOK();
		
	}
	else if(logInfo.userName==GlobalParameter::GetInstance()->m_tCfg.nRootName.c_str() && logInfo.passWord==GlobalParameter::GetInstance()->m_tCfg.nRootpswrd.c_str())//客户登录成功
	{
		logInfo.root = ROOT_GEUST;
		logInfo.isVaild = true;
		m_log_result.ShowWindow(SW_HIDE);
		CDialogEx::OnOK();
	}
	else//登录失败
	{
		logInfo.isVaild = false;		
		m_log_result.ShowWindow(SW_SHOW);
	}
}


int Mylog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码

	
	return 0;
}

//函数名称:修改登录失败的控件颜色
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
HBRUSH Mylog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if(pWnd->GetDlgCtrlID()==IDC_STATIC_LOGRET)
	{
		pDC->SetTextColor(RGB(255,0,0));
	}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

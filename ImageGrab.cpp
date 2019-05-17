
// ImageGrab.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "ImageGrab.h"
#include "MainFrm.h"
#include "CameraSetDlg.h"
#include "Mydialog.h"
#include "Mywnd.h"
#include "Myframe.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CImageGrabApp

BEGIN_MESSAGE_MAP(CImageGrabApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CImageGrabApp::OnAppAbout)
	ON_COMMAND(ID_STARTGRAB, OnStartGrab)
	ON_COMMAND(ID_STOPGRAB, OnStopGrab)
	ON_COMMAND(ID_CAMERA_SET, OnCameraSet)
	ON_COMMAND(ID_OPEN_LOG, OnOpenLog)
END_MESSAGE_MAP()


// CImageGrabApp ����

CImageGrabApp::CImageGrabApp()
{
	// TODO: ������Ӧ�ó��� ID �ַ����滻ΪΨһ�� ID �ַ�����������ַ�����ʽ
	//Ϊ CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("ImageGrab.AppID.NoVersion"));
	m_pMgrThread = NULL;

	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}

// Ψһ��һ�� CImageGrabApp ����

CImageGrabApp theApp;
CCameraMgr gCamMgr;
CImageDataMgr gDataMgr;
CLogFile gLogFile;
int workWay;//add by zhuxy 20190321	��־��ǰ�ɼ�����
CString trainNumStr;//add by zhuxy 20190322 ��ǰ����
HANDLE uiHandle;//�ź���,����������ÿ��ܳ�������
CString trainTimeStr;//��ǰ����ʱ��
CString lightStateStr;//����״̬
// CImageGrabApp ��ʼ��


//��������:��ʼ��
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
BOOL CImageGrabApp::InitInstance()
{
	CWinApp::InitInstance();

	EnableTaskbarInteraction(FALSE);

	// ʹ�� RichEdit �ؼ���Ҫ  AfxInitRichEdit2()	
	// AfxInitRichEdit2();

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	HANDLE hMutex = CreateMutex(NULL, TRUE, _T("ImageGrabInstance"));
	if (hMutex)
	{
		if (ERROR_ALREADY_EXISTS == GetLastError())
		{
			MessageBox(NULL, _T("�ó����Ѿ�����"), _T("��ʾ"), MB_OK);
			return FALSE;
		}
	}

	//�����쳣
	GetAppPath(m_strCurrentPath);
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)CrashHandler); 
	gLogFile.AddAppRecord(_T("������۲ɼ����Ӧ�ó�������,V1.0"));
	//��ʼ���豸����
	GlobalParameter::GetInstance()->Read();//��ini�ļ���ȡ���ڴ���
	gLogFile.AddAppRecord(_T("��ȡ�����ļ�,��ʼ�������ṹ��"));
	CreateMultiDirs(GlobalParameter::GetInstance()->m_tCfg.strImgSavePath.c_str());//�������img��·�����������½��ļ���
	gLogFile.AddAppRecord(_T("�½�������ͼƬ�洢�ļ���"));
	CreateMultiDirs(GlobalParameter::GetInstance()->m_tCfg.strLocalImgSavePath.c_str());//add by zhuxy ������ر���img��·�����������½��ļ���
	gLogFile.AddAppRecord(_T("�½�����ͼƬ�洢�ļ���"));

	m_pMgrThread = (CManagerThread *)AfxBeginThread(RUNTIME_CLASS(CManagerThread), THREAD_PRIORITY_NORMAL, 10240, NULL);//���������̣߳�ͨ��������Ϣ�����غ���
	gLogFile.AddAppRecord(_T("��ʼ�������߳�"));
#if CLOSE_IF//ɾ������,ɾ�������ļ�
	if (!removeOldDir())//add by zhuxyɾ�������ļ�
	{
		gLogFile.AddAppRecord(_T("ɾ�������ļ�ʧ��"));
		TRACE("delete old file faild");
	}
#endif

#if OPEN_IF//ȫ�ֱ�����ʼ��
	uiHandle = CreateSemaphore(NULL,1,1,NULL);//ˢ�½�����ź���
	trainNumStr = _T("");//add by zhuxy 20190322
	trainTimeStr = _T("");
	workWay = WAY_STOP;
	lightStateStr = _T("δ֪");
#endif

#if CLOSE_IF//���ڳ�ʼ��
	com.InitCOM(_T("COM1"));//add by zhuxy 20190322 
	gLogFile.AddAppRecord(_T("��ʼ������"));
#endif


#if CLOSE_IF//�����¼ʧ���򷵻�
	Mylog log;
	log.DoModal();
	if(false == log.logInfo.isVaild)
	{
		return false;
	}
#endif

#if OPEN_IF//udp��ʼ��
	udp = new Myudp();
	if(NULL!=udp)
	{
		udp->work();
	}

#endif
	
#if OPEN_IF//չʾ������
	gLogFile.AddAppRecord(_T("��ʼ������"));
	Mydialog* dg=new Mydialog;
	m_pMainWnd = dg;
	dg->DoModal();
#endif

	return TRUE;
}

//��������:app�ر�
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
int CImageGrabApp::ExitInstance()
{
	// TODO: Add your specialized code here and/or call the base class

	gLogFile.AddAppRecord(_T("������۲ɼ����Ӧ�ó���ر�"));
#if OPEN_IF//udp����
	udp->stopSend=true;
	delete udp;
	udp = NULL;
#endif
	return CWinApp::ExitInstance();
}

//��������:��ʼ�ɼ�
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void CImageGrabApp::OnStartGrab() 
{
	try
	{
		::PostThreadMessage(m_pMgrThread->m_nThreadID, ID_STARTGRAB, 0, 1);
		gLogFile.AddAppRecord(_T("�յ�ģ��ɼ�ָ��,��ʼ�ɼ�"));
	}
	catch (...)
	{
	}
}

//��������:ֹͣ�ɼ�
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void CImageGrabApp::OnStopGrab() 
{
	try
	{
		::PostThreadMessage(m_pMgrThread->m_nThreadID, ID_STOPGRAB, 0, 0);
		gLogFile.AddAppRecord(_T("�յ�ģ��ɼ�ָ��,ֹͣ�ɼ�"));
	}
	catch(...)
	{
	}
}

//��������:����
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:����ʹ��
void CImageGrabApp::OnCameraSet()
{
	CameraSetDlg camSetDlg;
	camSetDlg.DoModal();
}

//��������:����־·��
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void CImageGrabApp::OnOpenLog()
{
	ShellExecute(NULL, _T("open"), gLogFile.GetLogPath(), NULL, NULL, SW_SHOW);
}

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
public:
	CString m_strTitle;
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
	, m_strTitle(_T(""))
{
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	wchar_t cTitle[100];
	::GetWindowText(theApp.m_pMainWnd->m_hWnd, cTitle, sizeof(cTitle));
	m_strTitle = cTitle;

	UpdateData(FALSE);

	return TRUE;
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TITLE, m_strTitle);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// �������жԻ����Ӧ�ó�������
void CImageGrabApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

//��������:ɾ�����ڵ��ļ���
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
BOOL CImageGrabApp::removeOldDir()
{
	BOOL ret = TRUE;
	CString checkPathStr = GlobalParameter::GetInstance()->m_tCfg.strImgSavePath.c_str();//D:\\GrabImg
	CTime curTime = CTime::GetCurrentTime();
	CString strTime = curTime.Format(_T("%Y%m"));
	checkPathStr += strTime;//D://GrabImg//201903	
	CFileFind finder;
	bool bFind = finder.FindFile(checkPathStr+"\\*.*");
	CString dirStr;
	while (bFind)
	{
		bFind = finder.FindNextFileW();
		if(finder.IsDots())//Nonzero if the found file has the name "." or "..", which indicates that the found file is actually a directory. Otherwise 0.
		{
			continue;
		}
		if(finder.IsDirectory())
		{
			dirStr = finder.GetFileName();//20190319112142_��г��
			if(dirStr.GetLength()<TIME_LENGTH)
			{
				continue;
			}
			COleDateTime time(_wtoi(dirStr.Left(TIME_LENGTH).Mid(0,4)),_wtoi(dirStr.Left(TIME_LENGTH).Mid(4,2)),_wtoi(dirStr.Left(TIME_LENGTH).Mid(6,2)),_wtoi(dirStr.Left(TIME_LENGTH).Mid(8,2)),_wtoi(dirStr.Left(TIME_LENGTH).Mid(10,2)),_wtoi(dirStr.Left(TIME_LENGTH).Mid(12,2)));

			SYSTEMTIME sysTime;   // Win32 time information
			GetSystemTime(&sysTime);
			COleDateTime curTime(sysTime); 
			TRACE("%d\n",GlobalParameter::GetInstance()->m_tCfg.nFileExistDays);
			TRACE("%d\n",curTime.GetDayOfYear());
			TRACE("%d\n",time.GetDayOfYear());
			if(GlobalParameter::GetInstance()->m_tCfg.nFileExistDays < (curTime.GetDayOfYear()-time.GetDayOfYear()))//������������ļ���������ɾ���ļ�
			{
				CString tmpStr = checkPathStr+"\\\\"+dirStr;
				ret = myDeleteDirectory(tmpStr);
				RET_VALUE_IF_NOT_EAQU(ret,TRUE,FALSE);
			}

		}
	}
	return ret;
}

//��������:�ݹ�ɾ���ļ���
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
BOOL CImageGrabApp::myDeleteDirectory(CString directory_path)   //ɾ��һ���ļ����µ���������
{   
	BOOL ret=TRUE;
	CFileFind finder;
	CString path;
	path.Format(_T("%s/*.*"),directory_path);
	BOOL bWorking = finder.FindFile(path);
	while(bWorking)
	{
		bWorking = finder.FindNextFile();
		if(finder.IsDirectory() && !finder.IsDots())
		{//�����ļ���
			myDeleteDirectory(finder.GetFilePath()); //�ݹ�ɾ���ļ���
			RemoveDirectory(finder.GetFilePath());
		}
		else
		{//�����ļ�
			DeleteFile(finder.GetFilePath());
		}
	}
	ret = RemoveDirectoryW(directory_path);
	return ret;
}

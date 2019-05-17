
// ImageGrab.cpp : 定义应用程序的类行为。
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


// CImageGrabApp 构造

CImageGrabApp::CImageGrabApp()
{
	// TODO: 将以下应用程序 ID 字符串替换为唯一的 ID 字符串；建议的字符串格式
	//为 CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("ImageGrab.AppID.NoVersion"));
	m_pMgrThread = NULL;

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}

// 唯一的一个 CImageGrabApp 对象

CImageGrabApp theApp;
CCameraMgr gCamMgr;
CImageDataMgr gDataMgr;
CLogFile gLogFile;
int workWay;//add by zhuxy 20190321	标志当前采集动作
CString trainNumStr;//add by zhuxy 20190322 当前车号
HANDLE uiHandle;//信号量,解决保存设置可能出错问题
CString trainTimeStr;//当前过车时间
CString lightStateStr;//激光状态
// CImageGrabApp 初始化


//函数名称:初始化
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
BOOL CImageGrabApp::InitInstance()
{
	CWinApp::InitInstance();

	EnableTaskbarInteraction(FALSE);

	// 使用 RichEdit 控件需要  AfxInitRichEdit2()	
	// AfxInitRichEdit2();

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	HANDLE hMutex = CreateMutex(NULL, TRUE, _T("ImageGrabInstance"));
	if (hMutex)
	{
		if (ERROR_ALREADY_EXISTS == GetLastError())
		{
			MessageBox(NULL, _T("该程序已经运行"), _T("提示"), MB_OK);
			return FALSE;
		}
	}

	//捕获异常
	GetAppPath(m_strCurrentPath);
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)CrashHandler); 
	gLogFile.AddAppRecord(_T("车体外观采集软件应用程序启动,V1.0"));
	//初始化设备参数
	GlobalParameter::GetInstance()->Read();//把ini文件读取到内存中
	gLogFile.AddAppRecord(_T("读取配置文件,初始化计数结构体"));
	CreateMultiDirs(GlobalParameter::GetInstance()->m_tCfg.strImgSavePath.c_str());//如果保存img的路径不存在则新建文件夹
	gLogFile.AddAppRecord(_T("新建服务器图片存储文件夹"));
	CreateMultiDirs(GlobalParameter::GetInstance()->m_tCfg.strLocalImgSavePath.c_str());//add by zhuxy 如果本地保存img的路径不存在则新建文件夹
	gLogFile.AddAppRecord(_T("新建本地图片存储文件夹"));

	m_pMgrThread = (CManagerThread *)AfxBeginThread(RUNTIME_CLASS(CManagerThread), THREAD_PRIORITY_NORMAL, 10240, NULL);//开启界面线程，通过发送消息完成相关函数
	gLogFile.AddAppRecord(_T("初始化界面线程"));
#if CLOSE_IF//删除需求,删除过期文件
	if (!removeOldDir())//add by zhuxy删除过期文件
	{
		gLogFile.AddAppRecord(_T("删除过期文件失败"));
		TRACE("delete old file faild");
	}
#endif

#if OPEN_IF//全局变量初始化
	uiHandle = CreateSemaphore(NULL,1,1,NULL);//刷新界面的信号量
	trainNumStr = _T("");//add by zhuxy 20190322
	trainTimeStr = _T("");
	workWay = WAY_STOP;
	lightStateStr = _T("未知");
#endif

#if CLOSE_IF//串口初始化
	com.InitCOM(_T("COM1"));//add by zhuxy 20190322 
	gLogFile.AddAppRecord(_T("初始化串口"));
#endif


#if CLOSE_IF//如果登录失败则返回
	Mylog log;
	log.DoModal();
	if(false == log.logInfo.isVaild)
	{
		return false;
	}
#endif

#if OPEN_IF//udp初始化
	udp = new Myudp();
	if(NULL!=udp)
	{
		udp->work();
	}

#endif
	
#if OPEN_IF//展示基本框
	gLogFile.AddAppRecord(_T("初始化界面"));
	Mydialog* dg=new Mydialog;
	m_pMainWnd = dg;
	dg->DoModal();
#endif

	return TRUE;
}

//函数名称:app关闭
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
int CImageGrabApp::ExitInstance()
{
	// TODO: Add your specialized code here and/or call the base class

	gLogFile.AddAppRecord(_T("车体外观采集软件应用程序关闭"));
#if OPEN_IF//udp回收
	udp->stopSend=true;
	delete udp;
	udp = NULL;
#endif
	return CWinApp::ExitInstance();
}

//函数名称:开始采集
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void CImageGrabApp::OnStartGrab() 
{
	try
	{
		::PostThreadMessage(m_pMgrThread->m_nThreadID, ID_STARTGRAB, 0, 1);
		gLogFile.AddAppRecord(_T("收到模拟采集指令,开始采集"));
	}
	catch (...)
	{
	}
}

//函数名称:停止采集
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void CImageGrabApp::OnStopGrab() 
{
	try
	{
		::PostThreadMessage(m_pMgrThread->m_nThreadID, ID_STOPGRAB, 0, 0);
		gLogFile.AddAppRecord(_T("收到模拟采集指令,停止采集"));
	}
	catch(...)
	{
	}
}

//函数名称:设置
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:不再使用
void CImageGrabApp::OnCameraSet()
{
	CameraSetDlg camSetDlg;
	camSetDlg.DoModal();
}

//函数名称:打开日志路径
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void CImageGrabApp::OnOpenLog()
{
	ShellExecute(NULL, _T("open"), gLogFile.GetLogPath(), NULL, NULL, SW_SHOW);
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

// 实现
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

// 用于运行对话框的应用程序命令
void CImageGrabApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

//函数名称:删除过期的文件夹
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
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
			dirStr = finder.GetFileName();//20190319112142_和谐号
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
			if(GlobalParameter::GetInstance()->m_tCfg.nFileExistDays < (curTime.GetDayOfYear()-time.GetDayOfYear()))//如果超过配置文件的天数则删除文件
			{
				CString tmpStr = checkPathStr+"\\\\"+dirStr;
				ret = myDeleteDirectory(tmpStr);
				RET_VALUE_IF_NOT_EAQU(ret,TRUE,FALSE);
			}

		}
	}
	return ret;
}

//函数名称:递归删除文件夹
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
BOOL CImageGrabApp::myDeleteDirectory(CString directory_path)   //删除一个文件夹下的所有内容
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
		{//处理文件夹
			myDeleteDirectory(finder.GetFilePath()); //递归删除文件夹
			RemoveDirectory(finder.GetFilePath());
		}
		else
		{//处理文件
			DeleteFile(finder.GetFilePath());
		}
	}
	ret = RemoveDirectoryW(directory_path);
	return ret;
}

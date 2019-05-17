// Mydialog.cpp : 实现文件
//

#include "stdafx.h"
#include "ImageGrab.h"
#include "Mydialog.h"
#include "afxdialogex.h"
#include <fstream>
#include <ShellAPI.h>
// Mydialog 对话框

IMPLEMENT_DYNAMIC(Mydialog, CDialogEx)

	Mydialog::Mydialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(Mydialog::IDD, pParent)
{
	this->init();//初始化
}

Mydialog::~Mydialog()
{

}

void Mydialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Mydialog, CDialogEx)
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_COMMAND_RANGE(BTN_ID_FOR_ALL_CAM+0,BTN_ID_FOR_ALL_CAM+2,OnBtnForAllCamClik)
	ON_COMMAND_RANGE(START_BTN_ID+0,START_BTN_ID+GlobalParameter::GetInstance()->m_tCfg.nCamNum-1,OnBtnStartClik)
	ON_COMMAND_RANGE(STOP_BTN_ID+0,STOP_BTN_ID+GlobalParameter::GetInstance()->m_tCfg.nCamNum-1,OnBtnStopClik)
	ON_COMMAND_RANGE(UPLOAD_BTN_ID+0,UPLOAD_BTN_ID+GlobalParameter::GetInstance()->m_tCfg.nCamNum-1,OnBtnUploadClik)
END_MESSAGE_MAP()


// Mydialog 消息处理程序




//定时器 修改按钮状态
void Mydialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (UI_TIMER_ID==nIDEvent)//根据当前采集状态更新控件状态
	{
		if(WAY_AUTO==workWay)
		{
			for (int i=0;i<pStartBtnVc.size();i++)
			{
				((CButton*)pStartBtnVc[i])->EnableWindow(false);
				((CButton*)pStopBtnVc[i])->EnableWindow(false);
				((CButton*)pUploadBtnVc[i])->EnableWindow(false);
			}
			for (int i=0;i<pBtnForAllCamVc.size();i++)
			{
				((CButton*)pBtnForAllCamVc[i])->EnableWindow(false);
			}
		}
		else if(WAY_STOP==workWay)
		{
			for (int i=0;i<pStartBtnVc.size();i++)
			{
				((CButton*)pStartBtnVc[i])->EnableWindow(true);
				((CButton*)pStopBtnVc[i])->EnableWindow(true);
				((CButton*)pUploadBtnVc[i])->EnableWindow(true);
			}
			for (int i=0;i<pBtnForAllCamVc.size();i++)
			{
				((CButton*)pBtnForAllCamVc[i])->EnableWindow(true);
			}
		}

	}
	int a = GlobalParameter::GetInstance()->m_vCamCfg.size();
	CString str;str.Format(_T("camCfg个数:%d\n"),a);
	OutputDebugString(str);
	CDialogEx::OnTimer(nIDEvent);
}

// 界面初始化
int Mydialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	ShowWindow(SW_SHOWMAXIMIZED);//窗口最大化
	gLogFile.AddAppRecord(_T("初始化界面,最大化显示"));
	HICON m_hIcon;//设置图标
	m_hIcon = AfxGetApp()->LoadIconW(IDR_MAINFRAME_ICON);
	this->SetIcon(m_hIcon,TRUE);
	this->SetWindowTextW(GlobalParameter::GetInstance()->m_tCfg.strTitle.c_str());
	ctrlsInit();//控件初始化	
	gLogFile.AddAppRecord(_T("界面动态控件初始化完毕"));
	AfxBeginThread(newShowImageThread,this);//开启双缓冲刷新线程
	gLogFile.AddAppRecord(_T("开始双缓冲线程刷新界面"));
	SetTimer(UI_TIMER_ID,1000,0);
	gLogFile.AddAppRecord(_T("开启定时器监控当前过车情况,修改对应控件状态"));
	return 0;
}

//函数名称:双缓冲刷新线程函数
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:add by zhuxy20190404
//备注:NULL
UINT Mydialog::newShowImageThread(LPVOID param)
{
	HRESULT ret=0;
	CRect rect;
	theApp.m_pMainWnd->GetWindowRect(rect);
	int w=rect.Width();int h=rect.Height();
	Mydialog* mainThis = (Mydialog*)param;
	mainThis->memDc.CreateCompatibleDC(NULL);
	HDC hdc = mainThis->memDc.GetSafeHdc();
	wchar_t szText[MAX_PATH] = {0};
	CBitmap memBp;
	while (mainThis->RUN_FLAG)
	{
		CClientDC testdc(mainThis);
		CDC* tmpDc = &testdc;
		if(tmpDc==NULL)
		{
			continue;
		}
		memBp.CreateCompatibleBitmap(tmpDc,w,h);

		mainThis->memDc.SelectObject(&memBp);
		mainThis->memDc.FillSolidRect(0,0,w,h,RGB(255,255,255));

		gCamMgr.CheckCamStatus();
		for(int i=0;i<mainThis->pImgVc.size();i++)//依次对n个摄像头进行判断
		{
			if(gCamMgr.m_vCamCtrl.size()!=mainThis->pImgVc.size())//等待camMgr初始化完成
			{
				goto Clear;
			}
			//图像区域重绘
			if(CAM_STATUS_RUNNING!=gCamMgr.m_vCamCtrl[i]->m_nCamStatus)//如果没有进行采集则显示对应图片
			{
				try
				{
					mainThis->pImgVc[i]->Load(mainThis->imgDefaultPath);
					mainThis->pImgVc[i]->Draw(hdc,0,(h/mainThis->pImgVc.size())*i,IMAGE_SHOW_SIZE,h/mainThis->pImgVc.size()-5);
					mainThis->pImgVc[i]->Destroy();

				}
				catch (CException* e)
				{
					TCHAR   szCause[255];
					CString strFormatted;
					e->GetErrorMessage(szCause, 255);
					strFormatted.Format(_T("%s\n"),szCause);
					TRACE(strFormatted);
				}
			}
			else//正在进行采集动作
			{
				CString pathStr;
				pathStr.Format(_T("%s\\Camera_0%d\\jpg\\%d.jpg"),theApp.m_strCurrentImgSavePath,i,GlobalParameter::GetInstance()->m_vImgGrabInfo[i].nCompressNum-2);
				if(0==pathStr.Find(_T("\\")))//如果是服务器的地址则去掉双引号
				{
					pathStr.Replace(_T("\\\\"),_T("\\"));pathStr = _T("\\")+pathStr;
				}
				

				try
				{
					if(TRUE==PathFileExistsW(pathStr))//如果文件存在
					{
						mainThis->pImgVc[i]->Load(pathStr);
						if (!mainThis->pImgVc[i]->IsNull())
						{
							mainThis->pImgVc[i]->Draw(hdc,0,(h/mainThis->pImgVc.size())*i,IMAGE_SHOW_SIZE,h/mainThis->pImgVc.size()-5);
							mainThis->pImgVc[i]->Destroy();
						}
					}
					else
					{
						mainThis->pImgVc[i]->Load(mainThis->imgDefaultPath);
						mainThis->pImgVc[i]->Draw(hdc,0,(h/mainThis->pImgVc.size())*i,IMAGE_SHOW_SIZE,h/mainThis->pImgVc.size()-5);
						mainThis->pImgVc[i]->Destroy();
					}
				}
				catch (CException* e)
				{
					TCHAR   szCause[255];
					CString strFormatted;
					e->GetErrorMessage(szCause, 255);
					strFormatted.Format(_T("%s\n"),szCause);
					TRACE(strFormatted);
				}
			}
			WaitForSingleObject(uiHandle,INFINITE);
			//状态区域重绘
			mainThis->makeStateUI(&(mainThis->memDc),IMAGE_SHOW_SIZE,(h/mainThis->pImgVc.size())*i,i);//绘制状态界面
			ReleaseSemaphore(uiHandle,1,NULL);
			//控制区域重绘
			mainThis->makeCtrlUI(IMAGE_SHOW_SIZE+350,(h/mainThis->pImgVc.size())*i,i);
			//设置区域重绘
			mainThis->makeCamSetUI(IMAGE_SHOW_SIZE+500,(h/mainThis->pImgVc.size())*i,i);			
		}

		mainThis->makeAllCamCtrlUI(IMAGE_SHOW_SIZE+770,0);//绘图一键区域
		tmpDc->BitBlt(0,0,IMAGE_SHOW_SIZE+350+40,h,&(mainThis->memDc),0,0,SRCCOPY);
Clear:
		memBp.DeleteObject();
		Sleep(500);
	}

	DeleteObject(hdc);
	mainThis->memDc.DeleteDC();
	return 0;
}

BOOL Mydialog::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	return CDialogEx::OnEraseBkgnd(pDC);/*TRUE*/;
}


void Mydialog::OnPaint()
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


void Mydialog::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	
	Mylog log;
	log.DoModal();
	if(false == log.logInfo.isVaild)
	{
		gLogFile.AddAppRecord(_T("用户名或密码输入错误,退出程序失败"));
		return;
	}
	gLogFile.AddAppRecord(_T("回收定时器及界面相关内存"));
	RUN_FLAG = false;
	Sleep(500);
	for(int i=0;i<pImgVc.size();i++)//控件及相关内存回收
	{
		delete pImgVc[i];
		delete pStartBtnVc[i];
		delete pStopBtnVc[i];
		pCamSetDlgVc[i]->DestroyWindow();pCamSetDlgVc[i]=NULL;
	}
	for (int i=0;i<pBtnForAllCamVc.size();i++)
	{
		delete pBtnForAllCamVc[i];
	}
	KillTimer(UI_TIMER_ID);
	CloseHandle(uiHandle);
	CDialogEx::OnClose();
}

//函数名称:初始化
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:add by zhuxy20190330
//备注:NULL
void Mydialog::init()
{
	//变量初始化
	pImgVc.resize(GlobalParameter::GetInstance()->m_tCfg.nCamNum);
	for(int i=0;i<pImgVc.size();i++)
	{
		pImgVc[i] = new CImage;
	}

	RUN_FLAG = true;
	GetAppPath(imgDefaultPath);
	imgDefaultPath+=_T("\\protect\\defalut.jpg");
	//字体初始化
	font.CreateFont(
		15,                        // nHeight
		0,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		500,                 // nWeight
		FALSE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		GB2312_CHARSET,              // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		_T("幼圆"));                 // lpszFacename
}

//函数名称:控件初始化
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:add by zhuxy20190330
//备注:需要放在oncreate里
void Mydialog::ctrlsInit()
{
	//单个设备的开始采集、停止采集、上传
	pStartBtnVc.resize(GlobalParameter::GetInstance()->m_tCfg.nCamNum);
	pStopBtnVc.resize(GlobalParameter::GetInstance()->m_tCfg.nCamNum);
	pUploadBtnVc.resize(GlobalParameter::GetInstance()->m_tCfg.nCamNum);
	for(int i=0;i<GlobalParameter::GetInstance()->m_tCfg.nCamNum;i++)
	{
		pStartBtnVc[i] = new CButton;
		pStopBtnVc[i] = new CButton;
		pUploadBtnVc[i] = new CButton;

		pStartBtnVc[i]->Create(_T("开始采集"),WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|BS_FLAT,CRect(0,0,0,0),this,START_BTN_ID+i);
		pStopBtnVc[i]->Create(_T("停止采集"),WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|BS_FLAT,CRect(0,0,0,0),this,STOP_BTN_ID+i);
		pUploadBtnVc[i]->Create(_T("上传数据"),WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|BS_FLAT,CRect(0,0,0,0),this,UPLOAD_BTN_ID+i);
	}
	//设置
	pCamSetDlgVc.resize(GlobalParameter::GetInstance()->m_tCfg.nCamNum);
	for(int i=0;i<pStartBtnVc.size();i++)
	{
		pCamSetDlgVc[i] = new Mycamset(i);
		pCamSetDlgVc[i]->Create(IDD_DIALOG_MYSET,this);		
	}
	//一键操作的按钮
	pBtnForAllCamVc.resize(3);//开始、停止、上传
	CString str[3]={_T("一键开始"),_T("一键停止"),_T("一键上传")};
	for (int i=0;i<3;i++)
	{
		pBtnForAllCamVc[i] = new CButton;
		pBtnForAllCamVc[i]->Create(str[i],WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|BS_FLAT,CRect(0,0,0,0),this,BTN_ID_FOR_ALL_CAM+i);
	}
	//双缓冲初始化相关
}

//函数名称:生成状态界面
//函数作用:NULL
//函数参数:dc:画图的dc left,top:画图的位置  camNum:当前相机号
//函数返回值:NULL
//函数信息:add by zhuxy20190330
//备注:NULL
void Mydialog::makeStateUI(CDC* dc,int left,int top,int camNum)
{
	RET_IF_EAQU(dc,NULL);
	left += 40;top+=20;

	CPen pen(PS_SOLID, 2, RGB(0, 0, 128));
	dc->SelectObject(&pen);
	dc->SelectObject(&font);
	wchar_t szText[MAX_PATH] = {0};
	wsprintfW(szText, _T("相机号: %ld"), camNum+1);
	dc->TextOut(left, top, szText);
	left +=LINE_WIGHT;
	
	wsprintfW(szText, _T("相机名称：%s"), GlobalParameter::GetInstance()->m_vCamCfg[camNum].strCamName.c_str());
	dc->TextOut(left , top, szText);
	left -=LINE_WIGHT;top+=LINE_HIGHT;


	wsprintfW(szText, _T("相机位置：%s"), GlobalParameter::GetInstance()->m_vCamCfg[camNum].strCamDesc.c_str());
	dc->TextOut(left, top, szText);
	left +=LINE_WIGHT;

	switch (gCamMgr.m_vCamCtrl[camNum]->m_nCamStatus)
	{
	case CAM_STATUS_NOT_FIND_DEVICE:
		dc->SetTextColor(RGB(255,0,0));
		/*wcscpy_s(szText, _T("相机状态：未找到设备"));*/
		wcscpy_s(szText, _T("相机状态：连接失败"));
		break;
	case CAM_STATUS_OPEN_FAILED:
		dc->SetTextColor(RGB(255,0,0));
		wcscpy_s(szText, _T("相机状态：连接失败"));
		break;
	case CAM_STATUS_OPEN_SUCCEED:
		wcscpy_s(szText, _T("相机状态：连接成功"));
		break;
	case CAM_STATUS_RUNNING:
		wcscpy_s(szText, _T("相机状态：正在采集"));
		break;
	case CAM_STATUS_OPEN_DECONNECT:
		dc->SetTextColor(RGB(255,0,0));
		wcscpy_s(szText, _T("相机状态：连接断开"));
		break;
	default:
		dc->SetTextColor(RGB(255,0,0));
		wcscpy_s(szText, _T("相机状态：未知"));
		break;
	}
	dc->TextOut(left, top, szText);
	dc->SetTextColor(RGB(0,0,0));
	left -=LINE_WIGHT;top+=LINE_HIGHT;

	if (_T("") == trainNumStr)
	{
		wsprintfW(szText, _T("过车编号：%s"), _T("Unknown"));
	}
	else
	{
		CString carStr = trainNumStr.Left(trainNumStr.GetLength()-2);
		wsprintfW(szText, _T("过车编号：%s"), carStr);
	}
	dc->TextOut(left, top, szText);
	left += LINE_WIGHT;

	wsprintfW(szText, _T("采集图像数：%d"), GlobalParameter::GetInstance()->m_vImgGrabInfo[camNum].nGrabNum);
	dc->TextOut(left, top, szText);
	left -=LINE_WIGHT;top+=LINE_HIGHT;

	wsprintfW(szText, _T("压缩图像数：%d"), GlobalParameter::GetInstance()->m_vImgGrabInfo[camNum].nCompressNum);
	dc->TextOut(left, top, szText);
	left += LINE_WIGHT;

	wsprintfW(szText, _T("丢失图像数：%d"), GlobalParameter::GetInstance()->m_vImgGrabInfo[camNum].nLostNum);
	dc->TextOut(left, top, szText);
	left -=LINE_WIGHT;top+=LINE_HIGHT;

	wsprintfW(szText, _T("激光状态：")+lightStateStr);
	dc->TextOut(left, top, szText);
	left += LINE_WIGHT;

	wsprintfW(szText, _T("图像大小：%dx%d"), GlobalParameter::GetInstance()->m_vCamCfg[camNum].nWidth, GlobalParameter::GetInstance()->m_vCamCfg[camNum].nHeight);
	dc->TextOut(left, top,   szText);
	left -=LINE_WIGHT;top+=LINE_HIGHT;

	wsprintfW(szText, _T("曝光时间：%d"), GlobalParameter::GetInstance()->m_vCamCfg[camNum].nExposureTime);
	dc->TextOut(left, top,   szText);
	left += LINE_WIGHT;

	wsprintfW(szText, _T("增益：%d"), GlobalParameter::GetInstance()->m_vCamCfg[camNum].nGain);
	dc->TextOut(left, top,   szText);
	left -=LINE_WIGHT;top+=LINE_HIGHT;

	//内存回收
	pen.DeleteObject();
}

//函数名称:生成控制界面
//函数作用:NULL
//函数参数:left,top:画图的位置  camNum:当前相机号
//函数返回值:NULL
//函数信息:add by zhuxy20190401
//备注:NULL
void Mydialog::makeCtrlUI(int left,int top,int camNum)
{
	left += 40;top+=20;
	RET_IF_EAQU(pStartBtnVc[camNum],NULL);
	pStartBtnVc[camNum]->SetWindowPos(NULL,left,top,80,25,SWP_SHOWWINDOW);
	top+=LINE_HIGHT;

	RET_IF_EAQU(pStopBtnVc[camNum],NULL);
	pStopBtnVc[camNum]->SetWindowPos(NULL,left,top,80,25,SWP_SHOWWINDOW);
	top+=LINE_HIGHT;

	RET_IF_EAQU(pUploadBtnVc[camNum],NULL);
	pUploadBtnVc[camNum]->SetWindowPos(NULL,left,top,80,25,SWP_SHOWWINDOW);
}

//函数名称:生成设置界面
//函数作用:NULL
//函数参数:left,top:画图的位置  camNum:当前相机号
//函数返回值:NULL
//函数信息:add by zhuxy20190401
//备注:NULL
void Mydialog::makeCamSetUI(int left,int top,int camNum)
{
	left += 40;top+=20;
	RET_IF_EAQU(pCamSetDlgVc[camNum],NULL);
	CRect rect;pCamSetDlgVc[camNum]->GetClientRect(rect);
	pCamSetDlgVc[camNum]->MoveWindow(left,top,rect.Width(),rect.Height());
	pCamSetDlgVc[camNum]->ShowWindow(SW_SHOW);
}

//函数名称:生成一键控制界面
//函数作用:NULL
//函数参数:left,top:画图的位置  camNum:当前相机号
//函数返回值:NULL
//函数信息:add by zhuxy20190401
//备注:NULL
void Mydialog::makeAllCamCtrlUI(int left,int top)
{
	left += 40;top+=20;
	for (int i=0;i<pBtnForAllCamVc.size();i++)
	{
		RET_IF_EAQU(pBtnForAllCamVc[i],NULL);
		pBtnForAllCamVc[i]->SetWindowPos(NULL,left,top,80,25,SWP_SHOWWINDOW);
		top+=LINE_HIGHT;
	}
}

//函数名称:窗体大小变化
//函数作用:重绘窗口
//函数参数:NULL
//函数返回值:NULL
//函数信息:add by zhuxy20190401
//备注:NULL
void Mydialog::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	Invalidate(FALSE);
}



//函数名称:一键按钮回调函数
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:add by zhuxy20190401
//备注:NULL
void Mydialog::OnBtnForAllCamClik(UINT uID)
{
	int btnIndex = uID-BTN_ID_FOR_ALL_CAM;
	switch(btnIndex)
	{
	case 0://一键开始
		{
			gLogFile.AddAppRecord(_T("一键开始按钮已被点击"));
			if(workWay==WAY_AUTO)
			{
				gLogFile.AddAppRecord(_T("当前正在进行自动采集动作,无法手动采集"));
				AfxMessageBox(_T("当前正在进行自动采集动作,请稍后再试"));
				return;
			}
#if OPEN_IF //如果没有就绪的相机就返回
			bool ok=false;
			for (int i=0;i<gCamMgr.m_vCamCtrl.size();i++)
			{
				if(CAM_STATUS_OPEN_SUCCEED == gCamMgr.m_vCamCtrl[i]->m_nCamStatus)
				{
					ok=true;
					break;
				}
			}
			if(!ok)
			{
				gLogFile.AddAppRecord(_T("当前无可用相机,无法手动采集"));
				AfxMessageBox(_T("无可用相机"));
				break;
			}
#endif
			theApp.OnStartGrab();		
			((CButton*)GetDlgItem(uID))->EnableWindow(false);
			//TRACE(_T("start\n"));
			break;
		}
	case 1://一键停止
		{
			gLogFile.AddAppRecord(_T("一键停止按钮已被点击"));
			if(workWay==WAY_AUTO)
			{
				gLogFile.AddAppRecord(_T("当前正在进行自动采集动作,无法停止手动采集"));
				AfxMessageBox(_T("当前正在进行自动采集动作,请稍后再试"));
				return;
			}
			theApp.OnStopGrab();

			CString tmpStr;
			CString strPath = GlobalParameter::GetInstance()->m_tCfg.strLocalImgSavePath.c_str();//获取txt路径
			strPath += _T("trainInfo.txt");

			SYSTEMTIME stLocal;//获取时间
			GetLocalTime(&stLocal);
			CString strTime;
			strTime.Format(_T("%04u-%02u-%02u %02u:%02u:%02u:%03u"),
				stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour,
				stLocal.wMinute, stLocal.wSecond, stLocal.wMilliseconds);

			CTime curTime = CTime::GetCurrentTime();//获取车号
			CString strCarNum = curTime.Format(_T("%Y%m%d%H%M%S"));

			for (int i=0;i<gCamMgr.m_vCamCtrl.size();i++)
			{
				if( GlobalParameter::GetInstance()->m_vImgGrabInfo[i].nGrabNum>0)
				{
					tmpStr.Format(_T("%s&%s&0%d&%s\\\\Camera_0%d"),strTime, strCarNum,i,theApp.m_strCurrentImgSavePath,i);//D:\\GrabImg\\201903\\20190318180034_和谐号
					gLogFile.AddAppRecord(strPath, tmpStr);
				}
			}
			gLogFile.AddAppRecord(_T("已写入手动采集过车信息"));
			((CButton*)GetDlgItem(uID))->EnableWindow(true);
			//TRACE(_T("stop\n"));
			break;
		}
	case 2://一键上传
		{
			gLogFile.AddAppRecord(_T("一键上传按钮已被点击"));
			CString strPath = GlobalParameter::GetInstance()->m_tCfg.strLocalImgSavePath.c_str();//获取txt路径
			strPath += _T("trainInfo.txt");
			std::ifstream f(strPath.GetBuffer());
			if (!f)
			{
				break;
			}
			std::vector<std::string> recordsVc,txtVc;
			std::string line;
			while (getline(f,line))
			{
				recordsVc.push_back(line);
			}
			for (auto line=recordsVc.begin();line!=recordsVc.end();line++)
			{
				std::string lineStr = *line;
				if(std::string::npos == lineStr.find("&"))//如果是错误的路径则返回
				{
					txtVc.push_back(lineStr);
					continue;
				}
				std::string oldPath = lineStr.substr(lineStr.find_last_of("&")+1);//E:\\GrabImg\\201904\\20190403095543_20190403095543\\Camera_00
				std::string newPath = oldPath;//D:\\GrabImg\\201904\\20190403095543_20190403095543\\Camera_00\ 
				newPath.erase(0,GlobalParameter::GetInstance()->m_tCfg.strLocalImgSavePath.size());
				newPath = wstring2string(GlobalParameter::GetInstance()->m_tCfg.strImgSavePath) + newPath;
				int tmpIndex=0;
				if (!oldPath.empty())//去空格
				{
					while((tmpIndex=oldPath.find(' ',tmpIndex)) != string::npos)
					{
						oldPath.erase(tmpIndex,1);
					}
				}
				tmpIndex=0;
				if (!newPath.empty())//去空格
				{
					while((tmpIndex=newPath.find(' ',tmpIndex)) != string::npos)
					{
						newPath.erase(tmpIndex,1);
					}
				}
				string_replase(newPath,"\\\\","\\");//去掉双斜杠变为单斜杠
				if(newPath.at(0)=='\\')//如果是远程路径则在首位置加上斜杠
				{
					newPath = '\\'+ newPath;
				}
				CreateMultiDirs(CString(newPath.c_str()));//在服务器上把文件夹建立
				wchar_t *oldBuf = new wchar_t[oldPath.size()+2];//移动文件夹
				wchar_t *newBuf = new wchar_t[newPath.size()+2];
				MultiByteToWideChar(CP_ACP,0,oldPath.c_str(),oldPath.size(),oldBuf,oldPath.size()*sizeof(wchar_t));
				MultiByteToWideChar(CP_ACP,0,newPath.c_str(),newPath.size(),newBuf,newPath.size()*sizeof(wchar_t));

				oldBuf[oldPath.size()]='\0';
				oldBuf[oldPath.size()+1]='\0';
				newBuf[newPath.size()]='\0';
				newBuf[newPath.size()+1]='\0';

				SHFILEOPSTRUCT FileOp;
				ZeroMemory((void*)&FileOp,sizeof(SHFILEOPSTRUCT));
				FileOp.fFlags = FOF_NOCONFIRMATION ;
				FileOp.hNameMappings = NULL;
				FileOp.hwnd = NULL;
				FileOp.lpszProgressTitle = NULL;
				FileOp.pFrom = oldBuf;
				FileOp.pTo = newBuf;
				FileOp.wFunc = FO_MOVE;
				int ret = SHFileOperation(&FileOp);
				if(0==ret)
				{
#if OPEN_IF
					AfxMessageBox(_T("上传成功"));
					std::string writeStr = lineStr.substr(0,lineStr.find_last_of("&")+1);
					writeStr += newPath;
					CString txtPath = GlobalParameter::GetInstance()->m_tCfg.strImgSavePath.c_str();
					txtPath = txtPath + _T("过车信息.txt");
					CString carInfo(writeStr.c_str());
					gLogFile.AddAppRecord(txtPath, carInfo);
#endif
					TRACE("upload success\n");
				}
				else
				{
					txtVc.push_back(lineStr);
				}
				delete oldBuf;oldBuf=nullptr;
				delete newBuf;newBuf=nullptr; 
			}
			f.close();
			gLogFile.clearRecord(strPath);
			CString tmpStr;
			for (auto tmpLine=txtVc.begin();tmpLine!=txtVc.end();tmpLine++)
			{
				std::string tmpStdStr = *tmpLine;
				tmpStr += tmpStdStr.c_str();
				tmpStr += _T("\r\n");
			}
			gLogFile.AddAppRecord(strPath,tmpStr);
			break;
		}
	default:
		break;
	}		
}

//函数名称:开始采集按钮回调函数
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:add by zhuxy20190402
//备注:NULL
void Mydialog::OnBtnStartClik(UINT uID)
{
	gLogFile.AddAppRecord(_T("单次手动采集按钮已被点击"));
	if(workWay==WAY_AUTO)
	{
		gLogFile.AddAppRecord(_T("当前正在进行自动采集动作,请稍后再试"));
		AfxMessageBox(_T("当前正在进行自动采集动作,请稍后再试"));
		return;
	}
	int btnIndex = uID-START_BTN_ID;
#if OPEN_IF //如果没有就绪的相机就返回
		if(CAM_STATUS_OPEN_SUCCEED != gCamMgr.m_vCamCtrl[btnIndex]->m_nCamStatus)
		{
			gLogFile.AddAppRecord(_T("当前相机未就绪"));
			AfxMessageBox(_T("当前相机未就绪"));
			return;
		}
#endif
#if OPEN_IF//清空计数
		gCamMgr.PrepareGrab(btnIndex+1);
#endif

	::PostThreadMessageW(theApp.m_pMgrThread->m_nThreadID, ID_STARTGRAB, btnIndex+1, WAY_MANUAL);
	((CButton*)GetDlgItem(uID))->EnableWindow(false);
}

//函数名称:停止采集按钮回调函数
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:add by zhuxy20190401
//备注:NULL
void Mydialog::OnBtnStopClik(UINT uID)
{
	gLogFile.AddAppRecord(_T("单次停止手动采集按钮已被点击"));
	if(workWay==WAY_AUTO)
	{
		gLogFile.AddAppRecord(_T("当前正在进行自动采集动作,请稍后再试"));
		AfxMessageBox(_T("当前正在进行自动采集动作,请稍后再试"));
		return;
	}
	int btnIndex = uID-STOP_BTN_ID;
	::PostThreadMessageW(theApp.m_pMgrThread->m_nThreadID, ID_STOPGRAB, btnIndex+1, 1);

	CString tmpStr;
	CString strPath = GlobalParameter::GetInstance()->m_tCfg.strLocalImgSavePath.c_str();//获取txt路径
	strPath += _T("trainInfo.txt");

	SYSTEMTIME stLocal;//获取时间
	GetLocalTime(&stLocal);
	CString strTime;
	strTime.Format(_T("%04u-%02u-%02u %02u:%02u:%02u"),
		stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour,
		stLocal.wMinute, stLocal.wSecond);

	CTime curTime = CTime::GetCurrentTime();//获取车号
	CString strCarNum = curTime.Format(_T("%Y%m%d%H%M%S"));

	if( GlobalParameter::GetInstance()->m_vImgGrabInfo[btnIndex].nGrabNum>0)
	{
		tmpStr.Format(_T("%s&%s&0%d&%s\\\\Camera_0%d"),strTime, strCarNum,btnIndex,GlobalParameter::GetInstance()->imgPaths[btnIndex],btnIndex);//D:\\GrabImg\\201903\\20190318180034_和谐号
		gLogFile.AddAppRecord(strPath, tmpStr);
	}
	((CButton*)GetDlgItem(btnIndex+START_BTN_ID))->EnableWindow(true);
}

//函数名称:上传按钮回调函数
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:add by zhuxy20190401
//备注:NULL
void Mydialog::OnBtnUploadClik(UINT uID)
{
	gLogFile.AddAppRecord(_T("单次上传按钮已被点击"));
	int btnIndex = uID-UPLOAD_BTN_ID;
	CString strPath = GlobalParameter::GetInstance()->m_tCfg.strLocalImgSavePath.c_str();//获取txt路径
	strPath += _T("trainInfo.txt");
	std::ifstream f(strPath.GetBuffer());
	if (!f)
	{
		AfxMessageBox(_T("过车信息文件打开失败!"));
		return;
	}
	std::vector<std::string> recordsVc,txtVc;//txtVc用来保存没有处理的过车信息
	std::string line;
	while (getline(f,line))
	{
		recordsVc.push_back(line);
	}
	for (auto line=recordsVc.begin();line!=recordsVc.end();line++)//对每个记录都做一次处理
	{
		std::string lineStr = *line;
		if(std::string::npos == lineStr.find("&"))//如果是错误的路径则返回
		{
			txtVc.push_back(lineStr);
			continue;
		}
		std::string oldPath = lineStr.substr(lineStr.find_last_of("&")+1);//E:\\GrabImg\\201904\\20190403095543_20190403095543\\Camera_00
		{
			std::string tmp = oldPath.substr(oldPath.size()-2,1);
			if(atoi(tmp.c_str())!=btnIndex)
			{
				txtVc.push_back(lineStr);
				continue;
			}
		}

		std::string newPath = oldPath;//D:\\GrabImg\\201904\\20190403095543_20190403095543\\Camera_00\ 
		newPath.erase(0,GlobalParameter::GetInstance()->m_tCfg.strLocalImgSavePath.size());
		newPath = wstring2string(GlobalParameter::GetInstance()->m_tCfg.strImgSavePath) + newPath;
		int tmpIndex=0;
		if (!oldPath.empty())//去掉空格
		{
			while((tmpIndex=oldPath.find(' ',tmpIndex)) != string::npos)
			{
				oldPath.erase(tmpIndex,1);
			}
		}
		tmpIndex=0;
		if (!newPath.empty())//去掉空格
		{
			while((tmpIndex=newPath.find(' ',tmpIndex)) != string::npos)
			{
				newPath.erase(tmpIndex,1);
			}
		}
		string_replase(newPath,"\\\\","\\");//去掉双斜杠变为单斜杠
		if(newPath.at(0)=='\\')//如果是远程路径则在首位置加上斜杠
		{
			newPath = '\\'+ newPath;
		}
		CreateMultiDirs(CString(newPath.c_str()));//在服务器上把文件夹建立
		wchar_t *oldBuf = new wchar_t[oldPath.size()+2];//移动文件夹
		wchar_t *newBuf = new wchar_t[newPath.size()+2];
		MultiByteToWideChar(CP_ACP,0,oldPath.c_str(),oldPath.size(),oldBuf,oldPath.size()*sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP,0,newPath.c_str(),newPath.size(),newBuf,newPath.size()*sizeof(wchar_t));

		oldBuf[oldPath.size()]='\0';
		oldBuf[oldPath.size()+1]='\0';
		newBuf[newPath.size()]='\0';
		newBuf[newPath.size()+1]='\0';

		SHFILEOPSTRUCT FileOp;
		ZeroMemory((void*)&FileOp,sizeof(SHFILEOPSTRUCT));
		FileOp.fFlags = FOF_NOCONFIRMATION ;
		FileOp.hNameMappings = NULL;
		FileOp.hwnd = NULL;
		FileOp.lpszProgressTitle = NULL;
		FileOp.pFrom = oldBuf;
		FileOp.pTo = newBuf;
		FileOp.wFunc = FO_MOVE;
		int ret = SHFileOperation(&FileOp);
		if(0==ret)
		{
#if OPEN_IF
			AfxMessageBox(_T("上传成功"));
			std::string writeStr = lineStr.substr(0,lineStr.find_last_of("&")+1);
			writeStr += newPath;
			CString txtPath = GlobalParameter::GetInstance()->m_tCfg.strImgSavePath.c_str();
			txtPath = txtPath + _T("过车信息.txt");
			CString carInfo(writeStr.c_str());
			gLogFile.AddAppRecord(txtPath, carInfo);
#endif
			TRACE("upload success\n");
		}
		else
		{
			txtVc.push_back(lineStr);
		}
		delete oldBuf;oldBuf=nullptr;
		delete newBuf;newBuf=nullptr; 
	}
	f.close();
	gLogFile.clearRecord(strPath);
	CString tmpStr;
	for (auto tmpLine=txtVc.begin();tmpLine!=txtVc.end();tmpLine++)
	{
		std::string tmpStdStr = *tmpLine;
		tmpStr += tmpStdStr.c_str();
		tmpStr += _T("\r\n");
	}
	gLogFile.AddAppRecord(strPath,tmpStr);
}


// MainFrm.cpp : CMainFrame 类的实现
//

#include "stdafx.h"
#include "ImageGrab.h"

#include "MainFrm.h"
#include "Mydialog.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // 状态行指示器
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame 构造/析构

CMainFrame::CMainFrame()
{
	// TODO: 在此添加成员初始化代码
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// 创建一个视图以占用框架的工作区
	//if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	//{
	//	TRACE0("未能创建视图窗口\n");
	//	return -1;
	//}

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("未能创建状态栏\n");
		return -1;      // 未能创建
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	SetWindowText(GlobalParameter::GetInstance()->m_tCfg.strTitle.c_str());

#if OPEN_IF //add by zhuxy20190326显示图片的窗口
	this->ShowWindow(SW_SHOWMAXIMIZED);
	dg = new Mydialog;
	dg->Create(IDD_MY_DIALOG);
	CRect rect;
	//theApp.m_pMainWnd->GetClientRect(rect);
	GetWindowRect(rect);
	//dg->SetWindowPos(this,40,520,GlobalParameter::GetInstance()->m_tCfg.nCamNum*IMAGE_SHOW_SIZE,IMAGE_SHOW_SIZE,SWP_SHOWWINDOW);
	dg->SetWindowPos(this,rect.left,rect.top,rect.Width(),rect.Height(),SWP_SHOWWINDOW);
	//dg->ShowWindow(SW_SHOW);
#endif
	
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	cs.cx = 1200;
	cs.cy = 500;

	return TRUE;
}

// CMainFrame 诊断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame 消息处理程序

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// 将焦点前移到视图窗口
	//m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// 让视图第一次尝试该命令
	//if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
	//	return TRUE;

	// 否则，执行默认处理
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnClose()
{
	// TODO: 在此处添加消息处理程序代码

	//m_wndView.KillTimer(1);
	//m_wndView.KillTimer(2);

	if (theApp.m_pMgrThread)
	{
		::PostThreadMessage(theApp.m_pMgrThread->m_nThreadID, WM_QUIT, 0, 0);
	}

	WaitForSingleObject(theApp.m_pMgrThread->m_hThread, 10000);

	CFrameWnd::OnClose();
}
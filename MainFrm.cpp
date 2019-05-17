
// MainFrm.cpp : CMainFrame ���ʵ��
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
	ID_SEPARATOR,           // ״̬��ָʾ��
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame ����/����

CMainFrame::CMainFrame()
{
	// TODO: �ڴ���ӳ�Ա��ʼ������
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// ����һ����ͼ��ռ�ÿ�ܵĹ�����
	//if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	//{
	//	TRACE0("δ�ܴ�����ͼ����\n");
	//	return -1;
	//}

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("δ�ܴ���״̬��\n");
		return -1;      // δ�ܴ���
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	SetWindowText(GlobalParameter::GetInstance()->m_tCfg.strTitle.c_str());

#if OPEN_IF //add by zhuxy20190326��ʾͼƬ�Ĵ���
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
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	cs.cx = 1200;
	cs.cy = 500;

	return TRUE;
}

// CMainFrame ���

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


// CMainFrame ��Ϣ�������

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// ������ǰ�Ƶ���ͼ����
	//m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// ����ͼ��һ�γ��Ը�����
	//if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
	//	return TRUE;

	// ����ִ��Ĭ�ϴ���
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnClose()
{
	// TODO: �ڴ˴������Ϣ����������

	//m_wndView.KillTimer(1);
	//m_wndView.KillTimer(2);

	if (theApp.m_pMgrThread)
	{
		::PostThreadMessage(theApp.m_pMgrThread->m_nThreadID, WM_QUIT, 0, 0);
	}

	WaitForSingleObject(theApp.m_pMgrThread->m_hThread, 10000);

	CFrameWnd::OnClose();
}
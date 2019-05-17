// Mydialog.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ImageGrab.h"
#include "Mydialog.h"
#include "afxdialogex.h"
#include <fstream>
#include <ShellAPI.h>
// Mydialog �Ի���

IMPLEMENT_DYNAMIC(Mydialog, CDialogEx)

	Mydialog::Mydialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(Mydialog::IDD, pParent)
{
	this->init();//��ʼ��
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


// Mydialog ��Ϣ�������




//��ʱ�� �޸İ�ť״̬
void Mydialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (UI_TIMER_ID==nIDEvent)//���ݵ�ǰ�ɼ�״̬���¿ؼ�״̬
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
	CString str;str.Format(_T("camCfg����:%d\n"),a);
	OutputDebugString(str);
	CDialogEx::OnTimer(nIDEvent);
}

// �����ʼ��
int Mydialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  �ڴ������ר�õĴ�������
	ShowWindow(SW_SHOWMAXIMIZED);//�������
	gLogFile.AddAppRecord(_T("��ʼ������,�����ʾ"));
	HICON m_hIcon;//����ͼ��
	m_hIcon = AfxGetApp()->LoadIconW(IDR_MAINFRAME_ICON);
	this->SetIcon(m_hIcon,TRUE);
	this->SetWindowTextW(GlobalParameter::GetInstance()->m_tCfg.strTitle.c_str());
	ctrlsInit();//�ؼ���ʼ��	
	gLogFile.AddAppRecord(_T("���涯̬�ؼ���ʼ�����"));
	AfxBeginThread(newShowImageThread,this);//����˫����ˢ���߳�
	gLogFile.AddAppRecord(_T("��ʼ˫�����߳�ˢ�½���"));
	SetTimer(UI_TIMER_ID,1000,0);
	gLogFile.AddAppRecord(_T("������ʱ����ص�ǰ�������,�޸Ķ�Ӧ�ؼ�״̬"));
	return 0;
}

//��������:˫����ˢ���̺߳���
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:add by zhuxy20190404
//��ע:NULL
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
		for(int i=0;i<mainThis->pImgVc.size();i++)//���ζ�n������ͷ�����ж�
		{
			if(gCamMgr.m_vCamCtrl.size()!=mainThis->pImgVc.size())//�ȴ�camMgr��ʼ�����
			{
				goto Clear;
			}
			//ͼ�������ػ�
			if(CAM_STATUS_RUNNING!=gCamMgr.m_vCamCtrl[i]->m_nCamStatus)//���û�н��вɼ�����ʾ��ӦͼƬ
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
			else//���ڽ��вɼ�����
			{
				CString pathStr;
				pathStr.Format(_T("%s\\Camera_0%d\\jpg\\%d.jpg"),theApp.m_strCurrentImgSavePath,i,GlobalParameter::GetInstance()->m_vImgGrabInfo[i].nCompressNum-2);
				if(0==pathStr.Find(_T("\\")))//����Ƿ������ĵ�ַ��ȥ��˫����
				{
					pathStr.Replace(_T("\\\\"),_T("\\"));pathStr = _T("\\")+pathStr;
				}
				

				try
				{
					if(TRUE==PathFileExistsW(pathStr))//����ļ�����
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
			//״̬�����ػ�
			mainThis->makeStateUI(&(mainThis->memDc),IMAGE_SHOW_SIZE,(h/mainThis->pImgVc.size())*i,i);//����״̬����
			ReleaseSemaphore(uiHandle,1,NULL);
			//���������ػ�
			mainThis->makeCtrlUI(IMAGE_SHOW_SIZE+350,(h/mainThis->pImgVc.size())*i,i);
			//���������ػ�
			mainThis->makeCamSetUI(IMAGE_SHOW_SIZE+500,(h/mainThis->pImgVc.size())*i,i);			
		}

		mainThis->makeAllCamCtrlUI(IMAGE_SHOW_SIZE+770,0);//��ͼһ������
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	return CDialogEx::OnEraseBkgnd(pDC);/*TRUE*/;
}


void Mydialog::OnPaint()
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


void Mydialog::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	
	Mylog log;
	log.DoModal();
	if(false == log.logInfo.isVaild)
	{
		gLogFile.AddAppRecord(_T("�û����������������,�˳�����ʧ��"));
		return;
	}
	gLogFile.AddAppRecord(_T("���ն�ʱ������������ڴ�"));
	RUN_FLAG = false;
	Sleep(500);
	for(int i=0;i<pImgVc.size();i++)//�ؼ�������ڴ����
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

//��������:��ʼ��
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:add by zhuxy20190330
//��ע:NULL
void Mydialog::init()
{
	//������ʼ��
	pImgVc.resize(GlobalParameter::GetInstance()->m_tCfg.nCamNum);
	for(int i=0;i<pImgVc.size();i++)
	{
		pImgVc[i] = new CImage;
	}

	RUN_FLAG = true;
	GetAppPath(imgDefaultPath);
	imgDefaultPath+=_T("\\protect\\defalut.jpg");
	//�����ʼ��
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
		_T("��Բ"));                 // lpszFacename
}

//��������:�ؼ���ʼ��
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:add by zhuxy20190330
//��ע:��Ҫ����oncreate��
void Mydialog::ctrlsInit()
{
	//�����豸�Ŀ�ʼ�ɼ���ֹͣ�ɼ����ϴ�
	pStartBtnVc.resize(GlobalParameter::GetInstance()->m_tCfg.nCamNum);
	pStopBtnVc.resize(GlobalParameter::GetInstance()->m_tCfg.nCamNum);
	pUploadBtnVc.resize(GlobalParameter::GetInstance()->m_tCfg.nCamNum);
	for(int i=0;i<GlobalParameter::GetInstance()->m_tCfg.nCamNum;i++)
	{
		pStartBtnVc[i] = new CButton;
		pStopBtnVc[i] = new CButton;
		pUploadBtnVc[i] = new CButton;

		pStartBtnVc[i]->Create(_T("��ʼ�ɼ�"),WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|BS_FLAT,CRect(0,0,0,0),this,START_BTN_ID+i);
		pStopBtnVc[i]->Create(_T("ֹͣ�ɼ�"),WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|BS_FLAT,CRect(0,0,0,0),this,STOP_BTN_ID+i);
		pUploadBtnVc[i]->Create(_T("�ϴ�����"),WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|BS_FLAT,CRect(0,0,0,0),this,UPLOAD_BTN_ID+i);
	}
	//����
	pCamSetDlgVc.resize(GlobalParameter::GetInstance()->m_tCfg.nCamNum);
	for(int i=0;i<pStartBtnVc.size();i++)
	{
		pCamSetDlgVc[i] = new Mycamset(i);
		pCamSetDlgVc[i]->Create(IDD_DIALOG_MYSET,this);		
	}
	//һ�������İ�ť
	pBtnForAllCamVc.resize(3);//��ʼ��ֹͣ���ϴ�
	CString str[3]={_T("һ����ʼ"),_T("һ��ֹͣ"),_T("һ���ϴ�")};
	for (int i=0;i<3;i++)
	{
		pBtnForAllCamVc[i] = new CButton;
		pBtnForAllCamVc[i]->Create(str[i],WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|BS_FLAT,CRect(0,0,0,0),this,BTN_ID_FOR_ALL_CAM+i);
	}
	//˫�����ʼ�����
}

//��������:����״̬����
//��������:NULL
//��������:dc:��ͼ��dc left,top:��ͼ��λ��  camNum:��ǰ�����
//��������ֵ:NULL
//������Ϣ:add by zhuxy20190330
//��ע:NULL
void Mydialog::makeStateUI(CDC* dc,int left,int top,int camNum)
{
	RET_IF_EAQU(dc,NULL);
	left += 40;top+=20;

	CPen pen(PS_SOLID, 2, RGB(0, 0, 128));
	dc->SelectObject(&pen);
	dc->SelectObject(&font);
	wchar_t szText[MAX_PATH] = {0};
	wsprintfW(szText, _T("�����: %ld"), camNum+1);
	dc->TextOut(left, top, szText);
	left +=LINE_WIGHT;
	
	wsprintfW(szText, _T("������ƣ�%s"), GlobalParameter::GetInstance()->m_vCamCfg[camNum].strCamName.c_str());
	dc->TextOut(left , top, szText);
	left -=LINE_WIGHT;top+=LINE_HIGHT;


	wsprintfW(szText, _T("���λ�ã�%s"), GlobalParameter::GetInstance()->m_vCamCfg[camNum].strCamDesc.c_str());
	dc->TextOut(left, top, szText);
	left +=LINE_WIGHT;

	switch (gCamMgr.m_vCamCtrl[camNum]->m_nCamStatus)
	{
	case CAM_STATUS_NOT_FIND_DEVICE:
		dc->SetTextColor(RGB(255,0,0));
		/*wcscpy_s(szText, _T("���״̬��δ�ҵ��豸"));*/
		wcscpy_s(szText, _T("���״̬������ʧ��"));
		break;
	case CAM_STATUS_OPEN_FAILED:
		dc->SetTextColor(RGB(255,0,0));
		wcscpy_s(szText, _T("���״̬������ʧ��"));
		break;
	case CAM_STATUS_OPEN_SUCCEED:
		wcscpy_s(szText, _T("���״̬�����ӳɹ�"));
		break;
	case CAM_STATUS_RUNNING:
		wcscpy_s(szText, _T("���״̬�����ڲɼ�"));
		break;
	case CAM_STATUS_OPEN_DECONNECT:
		dc->SetTextColor(RGB(255,0,0));
		wcscpy_s(szText, _T("���״̬�����ӶϿ�"));
		break;
	default:
		dc->SetTextColor(RGB(255,0,0));
		wcscpy_s(szText, _T("���״̬��δ֪"));
		break;
	}
	dc->TextOut(left, top, szText);
	dc->SetTextColor(RGB(0,0,0));
	left -=LINE_WIGHT;top+=LINE_HIGHT;

	if (_T("") == trainNumStr)
	{
		wsprintfW(szText, _T("������ţ�%s"), _T("Unknown"));
	}
	else
	{
		CString carStr = trainNumStr.Left(trainNumStr.GetLength()-2);
		wsprintfW(szText, _T("������ţ�%s"), carStr);
	}
	dc->TextOut(left, top, szText);
	left += LINE_WIGHT;

	wsprintfW(szText, _T("�ɼ�ͼ������%d"), GlobalParameter::GetInstance()->m_vImgGrabInfo[camNum].nGrabNum);
	dc->TextOut(left, top, szText);
	left -=LINE_WIGHT;top+=LINE_HIGHT;

	wsprintfW(szText, _T("ѹ��ͼ������%d"), GlobalParameter::GetInstance()->m_vImgGrabInfo[camNum].nCompressNum);
	dc->TextOut(left, top, szText);
	left += LINE_WIGHT;

	wsprintfW(szText, _T("��ʧͼ������%d"), GlobalParameter::GetInstance()->m_vImgGrabInfo[camNum].nLostNum);
	dc->TextOut(left, top, szText);
	left -=LINE_WIGHT;top+=LINE_HIGHT;

	wsprintfW(szText, _T("����״̬��")+lightStateStr);
	dc->TextOut(left, top, szText);
	left += LINE_WIGHT;

	wsprintfW(szText, _T("ͼ���С��%dx%d"), GlobalParameter::GetInstance()->m_vCamCfg[camNum].nWidth, GlobalParameter::GetInstance()->m_vCamCfg[camNum].nHeight);
	dc->TextOut(left, top,   szText);
	left -=LINE_WIGHT;top+=LINE_HIGHT;

	wsprintfW(szText, _T("�ع�ʱ�䣺%d"), GlobalParameter::GetInstance()->m_vCamCfg[camNum].nExposureTime);
	dc->TextOut(left, top,   szText);
	left += LINE_WIGHT;

	wsprintfW(szText, _T("���棺%d"), GlobalParameter::GetInstance()->m_vCamCfg[camNum].nGain);
	dc->TextOut(left, top,   szText);
	left -=LINE_WIGHT;top+=LINE_HIGHT;

	//�ڴ����
	pen.DeleteObject();
}

//��������:���ɿ��ƽ���
//��������:NULL
//��������:left,top:��ͼ��λ��  camNum:��ǰ�����
//��������ֵ:NULL
//������Ϣ:add by zhuxy20190401
//��ע:NULL
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

//��������:�������ý���
//��������:NULL
//��������:left,top:��ͼ��λ��  camNum:��ǰ�����
//��������ֵ:NULL
//������Ϣ:add by zhuxy20190401
//��ע:NULL
void Mydialog::makeCamSetUI(int left,int top,int camNum)
{
	left += 40;top+=20;
	RET_IF_EAQU(pCamSetDlgVc[camNum],NULL);
	CRect rect;pCamSetDlgVc[camNum]->GetClientRect(rect);
	pCamSetDlgVc[camNum]->MoveWindow(left,top,rect.Width(),rect.Height());
	pCamSetDlgVc[camNum]->ShowWindow(SW_SHOW);
}

//��������:����һ�����ƽ���
//��������:NULL
//��������:left,top:��ͼ��λ��  camNum:��ǰ�����
//��������ֵ:NULL
//������Ϣ:add by zhuxy20190401
//��ע:NULL
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

//��������:�����С�仯
//��������:�ػ洰��
//��������:NULL
//��������ֵ:NULL
//������Ϣ:add by zhuxy20190401
//��ע:NULL
void Mydialog::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
	Invalidate(FALSE);
}



//��������:һ����ť�ص�����
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:add by zhuxy20190401
//��ע:NULL
void Mydialog::OnBtnForAllCamClik(UINT uID)
{
	int btnIndex = uID-BTN_ID_FOR_ALL_CAM;
	switch(btnIndex)
	{
	case 0://һ����ʼ
		{
			gLogFile.AddAppRecord(_T("һ����ʼ��ť�ѱ����"));
			if(workWay==WAY_AUTO)
			{
				gLogFile.AddAppRecord(_T("��ǰ���ڽ����Զ��ɼ�����,�޷��ֶ��ɼ�"));
				AfxMessageBox(_T("��ǰ���ڽ����Զ��ɼ�����,���Ժ�����"));
				return;
			}
#if OPEN_IF //���û�о���������ͷ���
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
				gLogFile.AddAppRecord(_T("��ǰ�޿������,�޷��ֶ��ɼ�"));
				AfxMessageBox(_T("�޿������"));
				break;
			}
#endif
			theApp.OnStartGrab();		
			((CButton*)GetDlgItem(uID))->EnableWindow(false);
			//TRACE(_T("start\n"));
			break;
		}
	case 1://һ��ֹͣ
		{
			gLogFile.AddAppRecord(_T("һ��ֹͣ��ť�ѱ����"));
			if(workWay==WAY_AUTO)
			{
				gLogFile.AddAppRecord(_T("��ǰ���ڽ����Զ��ɼ�����,�޷�ֹͣ�ֶ��ɼ�"));
				AfxMessageBox(_T("��ǰ���ڽ����Զ��ɼ�����,���Ժ�����"));
				return;
			}
			theApp.OnStopGrab();

			CString tmpStr;
			CString strPath = GlobalParameter::GetInstance()->m_tCfg.strLocalImgSavePath.c_str();//��ȡtxt·��
			strPath += _T("trainInfo.txt");

			SYSTEMTIME stLocal;//��ȡʱ��
			GetLocalTime(&stLocal);
			CString strTime;
			strTime.Format(_T("%04u-%02u-%02u %02u:%02u:%02u:%03u"),
				stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour,
				stLocal.wMinute, stLocal.wSecond, stLocal.wMilliseconds);

			CTime curTime = CTime::GetCurrentTime();//��ȡ����
			CString strCarNum = curTime.Format(_T("%Y%m%d%H%M%S"));

			for (int i=0;i<gCamMgr.m_vCamCtrl.size();i++)
			{
				if( GlobalParameter::GetInstance()->m_vImgGrabInfo[i].nGrabNum>0)
				{
					tmpStr.Format(_T("%s&%s&0%d&%s\\\\Camera_0%d"),strTime, strCarNum,i,theApp.m_strCurrentImgSavePath,i);//D:\\GrabImg\\201903\\20190318180034_��г��
					gLogFile.AddAppRecord(strPath, tmpStr);
				}
			}
			gLogFile.AddAppRecord(_T("��д���ֶ��ɼ�������Ϣ"));
			((CButton*)GetDlgItem(uID))->EnableWindow(true);
			//TRACE(_T("stop\n"));
			break;
		}
	case 2://һ���ϴ�
		{
			gLogFile.AddAppRecord(_T("һ���ϴ���ť�ѱ����"));
			CString strPath = GlobalParameter::GetInstance()->m_tCfg.strLocalImgSavePath.c_str();//��ȡtxt·��
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
				if(std::string::npos == lineStr.find("&"))//����Ǵ����·���򷵻�
				{
					txtVc.push_back(lineStr);
					continue;
				}
				std::string oldPath = lineStr.substr(lineStr.find_last_of("&")+1);//E:\\GrabImg\\201904\\20190403095543_20190403095543\\Camera_00
				std::string newPath = oldPath;//D:\\GrabImg\\201904\\20190403095543_20190403095543\\Camera_00\ 
				newPath.erase(0,GlobalParameter::GetInstance()->m_tCfg.strLocalImgSavePath.size());
				newPath = wstring2string(GlobalParameter::GetInstance()->m_tCfg.strImgSavePath) + newPath;
				int tmpIndex=0;
				if (!oldPath.empty())//ȥ�ո�
				{
					while((tmpIndex=oldPath.find(' ',tmpIndex)) != string::npos)
					{
						oldPath.erase(tmpIndex,1);
					}
				}
				tmpIndex=0;
				if (!newPath.empty())//ȥ�ո�
				{
					while((tmpIndex=newPath.find(' ',tmpIndex)) != string::npos)
					{
						newPath.erase(tmpIndex,1);
					}
				}
				string_replase(newPath,"\\\\","\\");//ȥ��˫б�ܱ�Ϊ��б��
				if(newPath.at(0)=='\\')//�����Զ��·��������λ�ü���б��
				{
					newPath = '\\'+ newPath;
				}
				CreateMultiDirs(CString(newPath.c_str()));//�ڷ������ϰ��ļ��н���
				wchar_t *oldBuf = new wchar_t[oldPath.size()+2];//�ƶ��ļ���
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
					AfxMessageBox(_T("�ϴ��ɹ�"));
					std::string writeStr = lineStr.substr(0,lineStr.find_last_of("&")+1);
					writeStr += newPath;
					CString txtPath = GlobalParameter::GetInstance()->m_tCfg.strImgSavePath.c_str();
					txtPath = txtPath + _T("������Ϣ.txt");
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

//��������:��ʼ�ɼ���ť�ص�����
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:add by zhuxy20190402
//��ע:NULL
void Mydialog::OnBtnStartClik(UINT uID)
{
	gLogFile.AddAppRecord(_T("�����ֶ��ɼ���ť�ѱ����"));
	if(workWay==WAY_AUTO)
	{
		gLogFile.AddAppRecord(_T("��ǰ���ڽ����Զ��ɼ�����,���Ժ�����"));
		AfxMessageBox(_T("��ǰ���ڽ����Զ��ɼ�����,���Ժ�����"));
		return;
	}
	int btnIndex = uID-START_BTN_ID;
#if OPEN_IF //���û�о���������ͷ���
		if(CAM_STATUS_OPEN_SUCCEED != gCamMgr.m_vCamCtrl[btnIndex]->m_nCamStatus)
		{
			gLogFile.AddAppRecord(_T("��ǰ���δ����"));
			AfxMessageBox(_T("��ǰ���δ����"));
			return;
		}
#endif
#if OPEN_IF//��ռ���
		gCamMgr.PrepareGrab(btnIndex+1);
#endif

	::PostThreadMessageW(theApp.m_pMgrThread->m_nThreadID, ID_STARTGRAB, btnIndex+1, WAY_MANUAL);
	((CButton*)GetDlgItem(uID))->EnableWindow(false);
}

//��������:ֹͣ�ɼ���ť�ص�����
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:add by zhuxy20190401
//��ע:NULL
void Mydialog::OnBtnStopClik(UINT uID)
{
	gLogFile.AddAppRecord(_T("����ֹͣ�ֶ��ɼ���ť�ѱ����"));
	if(workWay==WAY_AUTO)
	{
		gLogFile.AddAppRecord(_T("��ǰ���ڽ����Զ��ɼ�����,���Ժ�����"));
		AfxMessageBox(_T("��ǰ���ڽ����Զ��ɼ�����,���Ժ�����"));
		return;
	}
	int btnIndex = uID-STOP_BTN_ID;
	::PostThreadMessageW(theApp.m_pMgrThread->m_nThreadID, ID_STOPGRAB, btnIndex+1, 1);

	CString tmpStr;
	CString strPath = GlobalParameter::GetInstance()->m_tCfg.strLocalImgSavePath.c_str();//��ȡtxt·��
	strPath += _T("trainInfo.txt");

	SYSTEMTIME stLocal;//��ȡʱ��
	GetLocalTime(&stLocal);
	CString strTime;
	strTime.Format(_T("%04u-%02u-%02u %02u:%02u:%02u"),
		stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour,
		stLocal.wMinute, stLocal.wSecond);

	CTime curTime = CTime::GetCurrentTime();//��ȡ����
	CString strCarNum = curTime.Format(_T("%Y%m%d%H%M%S"));

	if( GlobalParameter::GetInstance()->m_vImgGrabInfo[btnIndex].nGrabNum>0)
	{
		tmpStr.Format(_T("%s&%s&0%d&%s\\\\Camera_0%d"),strTime, strCarNum,btnIndex,GlobalParameter::GetInstance()->imgPaths[btnIndex],btnIndex);//D:\\GrabImg\\201903\\20190318180034_��г��
		gLogFile.AddAppRecord(strPath, tmpStr);
	}
	((CButton*)GetDlgItem(btnIndex+START_BTN_ID))->EnableWindow(true);
}

//��������:�ϴ���ť�ص�����
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:add by zhuxy20190401
//��ע:NULL
void Mydialog::OnBtnUploadClik(UINT uID)
{
	gLogFile.AddAppRecord(_T("�����ϴ���ť�ѱ����"));
	int btnIndex = uID-UPLOAD_BTN_ID;
	CString strPath = GlobalParameter::GetInstance()->m_tCfg.strLocalImgSavePath.c_str();//��ȡtxt·��
	strPath += _T("trainInfo.txt");
	std::ifstream f(strPath.GetBuffer());
	if (!f)
	{
		AfxMessageBox(_T("������Ϣ�ļ���ʧ��!"));
		return;
	}
	std::vector<std::string> recordsVc,txtVc;//txtVc��������û�д���Ĺ�����Ϣ
	std::string line;
	while (getline(f,line))
	{
		recordsVc.push_back(line);
	}
	for (auto line=recordsVc.begin();line!=recordsVc.end();line++)//��ÿ����¼����һ�δ���
	{
		std::string lineStr = *line;
		if(std::string::npos == lineStr.find("&"))//����Ǵ����·���򷵻�
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
		if (!oldPath.empty())//ȥ���ո�
		{
			while((tmpIndex=oldPath.find(' ',tmpIndex)) != string::npos)
			{
				oldPath.erase(tmpIndex,1);
			}
		}
		tmpIndex=0;
		if (!newPath.empty())//ȥ���ո�
		{
			while((tmpIndex=newPath.find(' ',tmpIndex)) != string::npos)
			{
				newPath.erase(tmpIndex,1);
			}
		}
		string_replase(newPath,"\\\\","\\");//ȥ��˫б�ܱ�Ϊ��б��
		if(newPath.at(0)=='\\')//�����Զ��·��������λ�ü���б��
		{
			newPath = '\\'+ newPath;
		}
		CreateMultiDirs(CString(newPath.c_str()));//�ڷ������ϰ��ļ��н���
		wchar_t *oldBuf = new wchar_t[oldPath.size()+2];//�ƶ��ļ���
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
			AfxMessageBox(_T("�ϴ��ɹ�"));
			std::string writeStr = lineStr.substr(0,lineStr.find_last_of("&")+1);
			writeStr += newPath;
			CString txtPath = GlobalParameter::GetInstance()->m_tCfg.strImgSavePath.c_str();
			txtPath = txtPath + _T("������Ϣ.txt");
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

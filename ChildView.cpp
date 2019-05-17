
// ChildView.cpp : CChildView ���ʵ��
//

#include "stdafx.h"
#include "ImageGrab.h"
#include "ChildView.h"
#include "SapClassBasic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
#if OPEN_IF  //add by zhuxy20190325 ����ͼ����ʾ

#endif
}

CChildView::~CChildView()
{
#if OPEN_IF  //add by zhuxy20190325 ����ͼ����ʾ

#endif
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()



// CChildView ��Ϣ�������

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);
	return TRUE;
}

void CChildView::OnPaint() 
{
#if CLOSE_IF
	CPaintDC dc(this); // ���ڻ��Ƶ��豸������
	// TODO: �ڴ˴������Ϣ����������
	// ��ҪΪ������Ϣ������ CWnd::OnPaint()
	CRect clientRect;
	GetClientRect(&clientRect);
	int nWidth = clientRect.Width();
	int nHeight = clientRect.Height();

	CDC MemDC;
	CBitmap MemBitmap;

	MemDC.CreateCompatibleDC(NULL);	
	MemBitmap.CreateCompatibleBitmap(&dc, nWidth, nHeight);
	MemDC.SelectObject(&MemBitmap);

	MemDC.FillSolidRect(0,0,nWidth,nHeight,RGB(255,255,255));

	CPen pen(PS_SOLID, 2, RGB(0, 0, 128));
	MemDC.SelectObject(&pen);

	CFont font;
	font.CreateFont(
		18,                        // nHeight
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

	MemDC.SelectObject(&font);

	int cx = 1100;
	int cy = 10;

	int nCamNum = gCamMgr.m_vCamCtrl.size();
	if (nCamNum > 0)
	{
		long i(0);
		wchar_t szText[MAX_PATH] = {0};
		for (i = 0; i < nCamNum; i++)
		{
			GlobalParameter *pGlobalParam = GlobalParameter::GetInstance();
			int nCamIndex = gCamMgr.m_vCamCtrl[i]->m_nCamIndex;

			wsprintfW(szText, _T("�����: %ld"), i+1);
			MemDC.TextOut(cx, cy, szText);
			cy += LINE_HIGHT;

			wsprintfW(szText, _T("������ƣ�%s"), pGlobalParam->m_vCamCfg[nCamIndex].strCamName.c_str());
			MemDC.TextOut(cx + LINE_HIGHT, cy, szText);
			cy += LINE_HIGHT;

			wsprintfW(szText, _T("���λ�ã�%s"), pGlobalParam->m_vCamCfg[nCamIndex].strCamDesc.c_str());
			MemDC.TextOut(cx + LINE_HIGHT, cy, szText);
			cy += LINE_HIGHT;

			switch (gCamMgr.m_vCamCtrl[nCamIndex]->m_nCamStatus)
			{
			case CAM_STATUS_NOT_FIND_DEVICE:
				MemDC.SetTextColor(RGB(255,0,0));
				wcscpy_s(szText, _T("���״̬��δ�ҵ��豸"));
				break;
			case CAM_STATUS_OPEN_FAILED:
				MemDC.SetTextColor(RGB(255,0,0));
				wcscpy_s(szText, _T("���״̬������ʧ��"));
				break;
			case CAM_STATUS_OPEN_SUCCEED:
				wcscpy_s(szText, _T("���״̬�����ӳɹ�"));
				break;
			case CAM_STATUS_RUNNING:
				wcscpy_s(szText, _T("���״̬�����ڲɼ�"));
				break;
			case CAM_STATUS_OPEN_DECONNECT:
				MemDC.SetTextColor(RGB(255,0,0));
				wcscpy_s(szText, _T("���״̬�����ӶϿ�"));
				break;
			default:
				wcscpy_s(szText, _T("���״̬��δ֪"));
				break;
			}

			MemDC.TextOut(cx + LINE_HIGHT, cy, szText);
			MemDC.SetTextColor(RGB(0,0,0));
			cy += LINE_HIGHT;

			wsprintfW(szText, _T("���¹�����ţ�%s"), string2wstring(pGlobalParam->m_tTrainInfo.strTrainId).c_str());
			MemDC.TextOut(cx+LINE_HIGHT, cy, szText);
			cy += LINE_HIGHT;

			wsprintfW(szText, _T("�ɼ�ͼ������%d"), pGlobalParam->m_vImgGrabInfo[nCamIndex].nGrabNum);
			MemDC.TextOut(cx + LINE_HIGHT, cy, szText);
			cy += LINE_HIGHT;

			wsprintfW(szText, _T("ѹ��ͼ������%d"), pGlobalParam->m_vImgGrabInfo[nCamIndex].nCompressNum);
			MemDC.TextOut(cx + LINE_HIGHT, cy, szText);
			cy += LINE_HIGHT;

			wsprintfW(szText, _T("�ϴ�ͼ������%d"), pGlobalParam->m_vImgGrabInfo[nCamIndex].nUploadNum);
			MemDC.TextOut(cx + LINE_HIGHT, cy, szText);
			cy += LINE_HIGHT;

			//add by zhuxy20190326�������������Ҫ��ʾ������
			wsprintfW(szText, _T("������ͼ���С��%dx%d"), pGlobalParam->m_vCamCfg[nCamIndex].nWidth, pGlobalParam->m_vCamCfg[nCamIndex].nHeight);
			MemDC.TextOut(cx + LINE_HIGHT, cy, szText);
			cy += LINE_HIGHT;

			wsprintfW(szText, _T("�ع�ʱ�䣺%d"), pGlobalParam->m_vCamCfg[nCamIndex].nExposureTime);
			MemDC.TextOut(cx + LINE_HIGHT, cy, szText);
			cy += LINE_HIGHT;

			wsprintfW(szText, _T("���棺%d"), pGlobalParam->m_vCamCfg[nCamIndex].nGain);
			MemDC.TextOut(cx + LINE_HIGHT, cy, szText);
			cy += LINE_HIGHT;

	/*		cx += 300;
			cy = 60;		*/
			cy += LINE_HIGHT;
			
		}
	}
#endif


#if CLOSE_IF	//add by zhuxy 20190325������ʾͼƬ
	CRect rect;
	GetWindowRect(rect);
	//int w=rect.Width();int h=rect.Height();
	int yAdd=rect.Height()/GlobalParameter::GetInstance()->m_tCfg.nCamNum;
	int x=0,y=0;
	CImage* tmpImg=new CImage;
	tmpImg->Load(_T("C:\\Users\\jude\\Desktop\\new\\x64\\Debug\\protect\\defalut.jpg"));
	tmpImg->Draw(MemDC.m_hDC,x,y,1000,yAdd);
	tmpImg->Destroy();
	delete tmpImg;tmpImg=NULL;
	y+=yAdd;

	tmpImg=new CImage;
	tmpImg->Load(_T("C:\\Users\\jude\\Desktop\\new\\x64\\Debug\\protect\\defalut.jpg"));
	tmpImg->Draw(MemDC.m_hDC,x,y,1000,yAdd);
	tmpImg->Destroy();
	delete tmpImg;tmpImg=NULL;
	y+=yAdd;

	tmpImg=new CImage;
	tmpImg->Load(_T("C:\\Users\\jude\\Desktop\\new\\x64\\Debug\\protect\\defalut.jpg"));
	tmpImg->Draw(MemDC.m_hDC,x,y,1000,yAdd);
	tmpImg->Destroy();
	delete tmpImg;tmpImg=NULL;
	y+=yAdd;

	tmpImg=new CImage;
	tmpImg->Load(_T("C:\\Users\\jude\\Desktop\\new\\x64\\Debug\\protect\\defalut.jpg"));
	tmpImg->Draw(MemDC.m_hDC,x,y,1000,yAdd);
	tmpImg->Destroy();
	delete tmpImg;tmpImg=NULL;




	dc.BitBlt(0, 0, nWidth, nHeight, &MemDC, 0, 0, SRCCOPY);
	MemBitmap.DeleteObject();
	MemDC.DeleteDC();
#endif
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  �ڴ������ר�õĴ�������
	SetTimer(1, 500, NULL);
	SetTimer(2, 5000, NULL);
	//AfxBeginThread(showImageThread,this);

	return 0;
}


void CChildView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	switch (nIDEvent)
	{
	case 1:
			Invalidate();
		break;
	case 2:
			gCamMgr.CheckCamStatus();		    
		break;
	default:
		break;
	}

	CWnd::OnTimer(nIDEvent);
}

BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}


void CChildView::OnDestroy()
{
	CWnd::OnDestroy();

	// TODO: �ڴ˴������Ϣ����������
	KillTimer(1);
	KillTimer(2);
	//delete tmpImg;tmpImg=NULL;
}

//add by zhuxy20190325 ����ͼ����ʾ
void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
}




//��ʾimge�̺߳���
UINT CChildView::showImageThread(LPVOID param)
{
	CChildView* mainThis = (CChildView*)param;	
	HDC hdc = mainThis->GetDC()->GetSafeHdc();
	HRESULT ret=0;
	while (1)
	{
		for(int i=0;i<4;i++)//���ζ�n������ͷ�����ж�
		{
			if(gCamMgr.m_vCamCtrl.size()!=4)//�ȴ�camMgr��ʼ�����
			{
				continue;
			}
			if(CAM_STATUS_RUNNING!=gCamMgr.m_vCamCtrl[i]->m_nCamStatus)//���û�н��вɼ�����ʾ��ӦͼƬ
			{
				CImage* tmpImg=new CImage;
				tmpImg->Load(_T("C:\\Users\\jude\\Desktop\\1.jpg"));
				tmpImg->Draw(hdc,0,IMAGE_SHOW_SIZE*i,IMAGE_SHOW_SIZE,IMAGE_SHOW_SIZE);
				tmpImg->Destroy();
				delete tmpImg;tmpImg=NULL;
			}
			else//���ڽ��вɼ�����
			{
				TRACE(_T("in action\n"));
				CString pathStr;
				pathStr.Format(_T("%s\\Camera_0%d\\jpg\\%d.jpg"),theApp.m_strCurrentImgSavePath,i,GlobalParameter::GetInstance()->m_vImgGrabInfo[i].nCompressNum-2);
				if(FALSE==PathFileExistsW(pathStr))//����ļ�������
				{
					continue;
				}
				CImage* tmpImg=new CImage;
				tmpImg->Load(pathStr);
				tmpImg->Draw(hdc,IMAGE_SHOW_SIZE*i,0,IMAGE_SHOW_SIZE,IMAGE_SHOW_SIZE);
				tmpImg->Destroy();
				delete tmpImg;tmpImg=NULL;
			}
		}
		Sleep(1000);
	}
	return 0;
}
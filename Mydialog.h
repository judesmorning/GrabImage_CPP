#pragma once

#include "Mycamset.h"
#include "GlobalParameter.h"
#include "Mylog.h"

// Mydialog �Ի���
//add by zhuxy20190327  
//������ʾͼƬ
#define UI_TIMER_ID 100
class Mydialog : public CDialogEx
{
	DECLARE_DYNAMIC(Mydialog)

public:
	Mydialog(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~Mydialog();

// �Ի�������
	enum { IDD = IDD_MY_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

#if OPEN_IF //add by zhuxy ͼ����ʾ
private://����
	void init();//������ʼ��
	void ctrlsInit();//���пؼ��ĳ�ʼ��
	void makeStateUI(CDC* dc,int left,int top,int camNum);//����״̬UI��ͨ��CDC��ͼ
	void makeCtrlUI(int left,int top,int camNum);//���ɿ�����صĽ���
	void makeCamSetUI(int left,int top,int camNum);//���ý���
	void makeAllCamCtrlUI(int left,int top);//һ����ť����
	static UINT newShowImageThread(LPVOID param);//˫�������ʾ�߳�
private://����
	std::vector<CButton*> pStartBtnVc;//��ʼ�ɼ���ť�ؼ�
	std::vector<CButton*> pStopBtnVc;//ֹͣ�ɼ���ť�ؼ�
	std::vector<CButton*> pUploadBtnVc;//�ϴ���ť�ؼ�
	std::vector<Mycamset*> pCamSetDlgVc;//���õĽ���
	std::vector<CImage*> pImgVc;//CImage����
	std::vector<CButton*> pBtnForAllCamVc;//���е��豸�İ�ť�ؼ����򿪡��رա��ϴ�����
	
	volatile bool RUN_FLAG;//ֹͣ�̱߳�־λ
	CString imgDefaultPath;//Ĭ��ͼƬ·��
	CFont font;//״̬������
private://�ص�
	afx_msg void OnBtnForAllCamClik(UINT uID);//һ����ťȺ
	afx_msg void OnBtnStartClik(UINT uID);//��ʼ�ɼ���ť
	afx_msg void OnBtnStopClik(UINT uID);//ֹͣ�ɼ���ť
	afx_msg void OnBtnUploadClik(UINT uID);//�ϴ���ť
private://˫�����ͼ
	CDC memDc;
#endif

public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

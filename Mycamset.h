#pragma once
#include "GlobalParameter.h"

// Mycamset �Ի���

class Mycamset : public CDialogEx
{
	DECLARE_DYNAMIC(Mycamset)

public:
	Mycamset(int camIndex,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~Mycamset();

// �Ի�������
	enum { IDD = IDD_DIALOG_MYSET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
	virtual void PostNcDestroy();
private:
	int camIndex;
	void init(const int& camIndex);

public:
	afx_msg void OnBnClickedButton3();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

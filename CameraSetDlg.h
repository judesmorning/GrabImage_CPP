#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CameraSetDlg �Ի���

class CameraSetDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CameraSetDlg)

public:
	CameraSetDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CameraSetDlg();

// �Ի�������
	enum { IDD = IDD_CAMSET_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedSave();
	DECLARE_MESSAGE_MAP()

public:
	CString m_strDesc;	
	CComboBox m_CamNameComBo;
	int m_nExposure;
	int m_nGain;
	CButton m_InterTrigger;
	CButton m_ExterTrigger;
	afx_msg void OnCbnSelchangeCamNameCombo();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

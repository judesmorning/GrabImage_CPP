#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CameraSetDlg 对话框

class CameraSetDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CameraSetDlg)

public:
	CameraSetDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CameraSetDlg();

// 对话框数据
	enum { IDD = IDD_CAMSET_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
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

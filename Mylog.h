#pragma once



// Mylog �Ի���
struct LogStruct{
	LogStruct()
	{
		userName = _T("");
		passWord = _T("");
		root = 0;
		isVaild = false;
	}
	CString userName;
	CString passWord;
	int root;
	bool isVaild;
};
class Mylog : public CDialogEx
{
	DECLARE_DYNAMIC(Mylog)

public:
	Mylog(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~Mylog();

// �Ի�������
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
private:
	static CString USERNAME;
	static CString PASSWORD;

public:
	static LogStruct logInfo;
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	CStatic m_log_result;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

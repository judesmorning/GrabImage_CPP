#pragma once

class CImageBuffer  
{
public:
	CImageBuffer();
	virtual ~CImageBuffer();

public: 
	void InitBuffer(int nImgBufNum, int nImgSize, int nCamIndex);
	void ClearBuffer();
    void Reset();
	BOOL BufferIn(void *buf, int nCamIndex, int nWidth, int nHeight);
	BOOL BufferOut(void **ppImgData/*, int &nCamIndex*/);

private:
	HANDLE m_hDataReady;
	HANDLE m_hProtectBuf;

	BYTE **m_ppBuffer;

	int *m_pCamIndex;
    int *m_pWidth;
	int *m_pHeight;

	int m_nImgBufNum;  //ͼ��������
    int m_nImgBufSize;  //ͼ�������С
	int m_nCamIndex;   //�������

	int m_nTop;
	int m_nBottom;
};

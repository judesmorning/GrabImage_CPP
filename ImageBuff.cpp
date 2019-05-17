// ImageBuffer.cpp: implementation of the CImageBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImageGrab.h"
#include "ImageBuff.h"
#include "LogFile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CImageBuffer::CImageBuffer()
{
	m_ppBuffer = NULL;
	m_pCamIndex = NULL;
	m_pWidth = NULL;
	m_pHeight = NULL;
	m_hDataReady = NULL;
	m_hProtectBuf = NULL;
}

CImageBuffer::~CImageBuffer()
{

}

//��������:��ʼ��
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void CImageBuffer::InitBuffer(int nImgBufNum, int nImgBufSize, int nCamIndex)
{	
	int i = 0;
	try
	{
		CString strInfo;	
		strInfo.Format(_T("���뻺�������������%d, ÿ������Ĵ�С��%d"), nImgBufNum, nImgBufSize);
		gLogFile.AddCameraRecord(nCamIndex, strInfo);
		
		m_nImgBufNum = nImgBufNum;
		m_nImgBufSize = nImgBufSize;
		m_nCamIndex = nCamIndex;
		m_ppBuffer = (BYTE **)new BYTE[m_nImgBufNum * sizeof(BYTE *)];

		BYTE *pBuf = NULL;
		for (i=0; i<m_nImgBufNum; i++)
		{
			m_ppBuffer[i] = new BYTE[m_nImgBufSize];
			memset(m_ppBuffer[i], 0, m_nImgBufSize);
		}

		m_pCamIndex = new int[m_nImgBufNum];
		m_pWidth = new int[m_nImgBufNum];
		m_pHeight = new int[m_nImgBufNum];

		m_nBottom = 0;
		m_nTop = 0;

		m_hDataReady = ::CreateSemaphore(NULL, 0, 3000, NULL);
        m_hProtectBuf = ::CreateSemaphore(NULL, 1, 1, NULL);

		gLogFile.AddCameraRecord(m_nCamIndex, _T("���뻺������Դ�ɹ�"));
	}
	catch(...)
	{
		gLogFile.AddCameraRecord(m_nCamIndex, _T("���뻺������Դ�쳣"));
	}
}

//��������:����
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void CImageBuffer::ClearBuffer()
{
	gLogFile.AddCameraRecord(m_nCamIndex, _T("���뻺���ͷ�"));
	
	try
	{
		if (m_ppBuffer)
		{
			for (int i=0; i<m_nImgBufNum; i++)
			{
				if(m_ppBuffer[i])
				{
					delete [] m_ppBuffer[i];
					m_ppBuffer[i] = NULL;
				}
			}

			delete [] m_ppBuffer;
			m_ppBuffer = NULL;
		}

		if (m_pCamIndex)
		{
			delete [] m_pCamIndex;
			m_pCamIndex = NULL;
		}

		if (m_pWidth)
		{
			delete [] m_pWidth;
			m_pWidth = NULL;
		}

		if (m_pHeight)
		{
			delete [] m_pHeight;
			m_pHeight = NULL;
		}

		if (m_hDataReady)
		{
			CloseHandle(m_hDataReady);
		}
		
		if (m_hProtectBuf)
		{
			CloseHandle(m_hProtectBuf);
		}
	}
	catch (...)
	{
		gLogFile.AddCameraRecord(m_nCamIndex, _T("�����ͷ��쳣"));
	}

	gLogFile.AddCameraRecord(m_nCamIndex, _T("�����ͷŽ���"));
}

//��������:��λ
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void CImageBuffer::Reset()
{
	m_nTop = 0;
	m_nBottom = 0;

	if (m_hDataReady)
	{
		CloseHandle(m_hDataReady);
	}

	if (m_hProtectBuf)
	{
		CloseHandle(m_hProtectBuf);
	}

	m_hDataReady = ::CreateSemaphore(NULL, 0, 3000, NULL);
	m_hProtectBuf = ::CreateSemaphore(NULL, 1, 1, NULL);

	for(int i = 0; i < m_nImgBufNum; i++)
	{
		memset(m_ppBuffer[i], 0, m_nImgBufSize);
		m_pCamIndex[i] = 0;
		m_pWidth[i] = 0;
		m_pHeight[i] = 0;
	}
}

//��������:�洢�������
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
BOOL CImageBuffer::BufferIn(void *pImgData, int nCamIndex, int nWidth, int nHeight)
{
	long lPreCount;

	__try
	{		
		if (WaitForSingleObject(m_hProtectBuf, 1000)==WAIT_OBJECT_0)
		{
			memcpy(m_ppBuffer[m_nTop], pImgData, nWidth * nHeight);
			m_pCamIndex[m_nTop] = nCamIndex;
			m_pWidth[m_nTop] = nWidth;
			m_pHeight[m_nTop] = nHeight;
			m_nTop = (++m_nTop) % m_nImgBufNum;

			ReleaseSemaphore(m_hDataReady, 1, &lPreCount);
			ReleaseSemaphore(m_hProtectBuf, 1, &lPreCount);
		}
		else
		{		
			ReleaseSemaphore(m_hProtectBuf, 1, &lPreCount);
			gLogFile.AddCameraRecord(nCamIndex, _T("ץȡ����д�����鳬ʱ"));
			return FALSE;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		ReleaseSemaphore(m_hProtectBuf, 1, &lPreCount);
		gLogFile.AddCameraRecord(m_nCamIndex, _T("BufferIn�����쳣"));

		return FALSE;
	}

	return TRUE;
}

//��������:��ȡ�������
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
BOOL CImageBuffer::BufferOut(void **ppImgData/*, int &nCamIndex*/)
{
	__try
	{
		if (WaitForSingleObject(m_hDataReady,1000) == WAIT_OBJECT_0)
		{
			*ppImgData = m_ppBuffer[m_nBottom];
			//nCamIndex = m_pCamIndex[m_nBottom];		
			m_nBottom = (++m_nBottom) % m_nImgBufNum;
		}
		else
		{
			*ppImgData = NULL;
			return FALSE;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		gLogFile.AddCameraRecord(m_nCamIndex, _T("BufferOut�����쳣"));
		return FALSE;
	}

	return TRUE;
}




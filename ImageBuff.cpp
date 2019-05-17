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

//函数名称:初始化
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void CImageBuffer::InitBuffer(int nImgBufNum, int nImgBufSize, int nCamIndex)
{	
	int i = 0;
	try
	{
		CString strInfo;	
		strInfo.Format(_T("申请缓存区数组个数：%d, 每个数组的大小：%d"), nImgBufNum, nImgBufSize);
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

		gLogFile.AddCameraRecord(m_nCamIndex, _T("申请缓冲区资源成功"));
	}
	catch(...)
	{
		gLogFile.AddCameraRecord(m_nCamIndex, _T("申请缓冲区资源异常"));
	}
}

//函数名称:回收
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void CImageBuffer::ClearBuffer()
{
	gLogFile.AddCameraRecord(m_nCamIndex, _T("进入缓存释放"));
	
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
		gLogFile.AddCameraRecord(m_nCamIndex, _T("缓存释放异常"));
	}

	gLogFile.AddCameraRecord(m_nCamIndex, _T("缓存释放结束"));
}

//函数名称:复位
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
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

//函数名称:存储相机数据
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
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
			gLogFile.AddCameraRecord(nCamIndex, _T("抓取数据写入数组超时"));
			return FALSE;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		ReleaseSemaphore(m_hProtectBuf, 1, &lPreCount);
		gLogFile.AddCameraRecord(m_nCamIndex, _T("BufferIn函数异常"));

		return FALSE;
	}

	return TRUE;
}

//函数名称:获取相机数据
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
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
		gLogFile.AddCameraRecord(m_nCamIndex, _T("BufferOut函数异常"));
		return FALSE;
	}

	return TRUE;
}




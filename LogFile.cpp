// LogFile.cpp: implementation of the CLogFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImageGrab.h"
#include "LogFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CRITICAL_SECTION g_WriteLog;		// �ٽ�α���

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CLogFile::CLogFile()
{
	InitializeCriticalSection(&g_WriteLog);
	CreateFloder();
}

CLogFile::~CLogFile()
{

}
void CLogFile::CreateFloder()
{
	CTime ptime = CTime::GetCurrentTime();
    GetAppPath(strLogBaseFloder);
	strLogBaseFloder += _T("\\Log");
	CreateDirectory(strLogBaseFloder,NULL);
}

CString GetBackupFileName(LPCTSTR filename)
{
	BOOL bRet = FALSE;
	CString strPreFileName,strTemp,strOldestFileName;
	BOOL bFound = FALSE;
	strPreFileName = filename;
	strOldestFileName = filename;
	try
	{	CTime tOldestTime;										// ��¼�ļ��Ĵ���ʱ��
	tOldestTime = CTime::GetCurrentTime();                    // ��¼�����ʱ��
	int nBackFileCnt = 70;									// ��־�ļ�����
	int nLength = strPreFileName.GetLength();				// �ļ��ַ�������
	strPreFileName = strPreFileName.Left(nLength - 4 );		// �ļ�·���ĳ���
	
	// �ļ���Ϣ
	CFileStatus status;
	
	// �����ļ������ļ��������ļ�����ΪTFDSClie00�� TFDSClie01����������
	for(int i=0;i<nBackFileCnt;i++)
	{
		strTemp.Format(_T("%s%02d.log"), strPreFileName,i);
		// ����ļ��Ƿ���ڣ������ڣ���ֱ�ӷ��ص�ǰ���ļ���	
		if(CFile::GetStatus(strTemp,status) ==TRUE)
		{
			// �ļ����ڣ���ȡ�ļ���ʱ��,���Ҽ�¼�����ļ���ʱ��
			if(tOldestTime >= status.m_mtime)
			{
				bFound = TRUE;
				tOldestTime = status.m_mtime;
				strOldestFileName = strTemp;					
			}
		}
		else
		{
			strOldestFileName = strTemp;
			break;
		}
	}
	// ����ļ��Ѿ����ڣ�ɾ�����ϵ��ļ���Ȼ��������ļ�������Ϊ�µ��ļ�
	if(bFound)
	{
		DeleteFile(strOldestFileName);
	}
				
	}
	catch (...)
	{
		PostQuitMessage(0);
	}
	return 	strOldestFileName;
	
}

BOOL CLogFile::OpenLogFile(LPCTSTR filename)
{
	BOOL bFileOpened = FALSE;
	// we don't want to open the file if there is no file name
	if (filename[0] !=  '\0')
	{
		if (Open(filename, CFile::modeCreate | CFile::modeNoTruncate | 
			CFile::modeReadWrite|CFile::shareDenyNone))
		{
			// return value
			bFileOpened = TRUE;
			DWORD length = (DWORD)GetLength();
			
			if (length != 0)
			{
				// check if we have reached the maximum size of the log
				if (length >= 3096*1024)
				{
					CString strFileName = GetBackupFileName(filename);
					
					SeekToBegin();
					
					char* cBuffer = new char[length];
					memset(cBuffer, 0, length);
					Read(cBuffer, length);
					
					// truncate the file to 0 length
					SetLength(0);
					
					// ��ǰ����д�뵽���ļ�
					CStdioFile file;
					file.Open(strFileName, CFile::modeCreate | CFile::modeNoTruncate | 
						CFile::modeReadWrite|CFile::shareDenyNone );
					file.Write(cBuffer,length);
					file.Close();
					
					delete [] cBuffer;
					
				}
				
				SeekToEnd();
			}
		}
	}
	return bFileOpened;
}


void CLogFile::AddRecord(CString strFileName, LPCTSTR description)
{
	EnterCriticalSection(&g_WriteLog);
	LogRecord logrec;
	memset(&logrec, 0, sizeof(LogRecord));
	//add by zhuxy 20190319	���Ӻ���
	SYSTEMTIME stLocal; 
	GetLocalTime(&stLocal);
	CString strInfo;
	strInfo.Format(_T("%04u-%02u-%02u %02u:%02u:%02u:%03u"),
		stLocal.wYear,stLocal.wMonth,stLocal.wDay,stLocal.wHour,
		stLocal.wMinute, stLocal.wSecond, stLocal.wMilliseconds);
#if CLOSE_IF
	strncpy_s(logrec.szDate, (CStringA)strInfo, DATETIMESIZE-2);
	strncpy_s(logrec.szDescription, wstring2string(description).c_str(), DESCRIPTIONSIZE-2);
#endif
	strInfo = strInfo + _T("    ") + description;
	USES_CONVERSION;
	char* info = W2A(strInfo);

	if (OpenLogFile(strFileName))
	{
#if CLOSE_IF
		Write(&logrec, sizeof(LogRecord));
#endif
		Write(info, CStringA(strInfo).GetLength());
		Write("\r\n", 2);
		Close();
	}
	
	LeaveCriticalSection(&g_WriteLog);
}

void CLogFile::AddRecord2(CString strFileName, LPCTSTR description)
{
	EnterCriticalSection(&g_WriteLog);

	LogRecord logrec;
	memset(&logrec, 0, sizeof(LogRecord));

	// fill the record
	CTime tTime = CTime::GetCurrentTime();	// current time
	strncpy_s(logrec.szDate, (CStringA)tTime.Format(""), DATETIMESIZE - 1);
	strncpy_s(logrec.szDescription, wstring2string(description).c_str(), DESCRIPTIONSIZE - 1);
	char str[255];
	memset(str, 0, 255);
	strncpy_s(str, wstring2string(description).c_str(), DESCRIPTIONSIZE - 1);


	if (OpenLogFile(strFileName))
	{
		//Write(&logrec, sizeof(LogRecord)-1);
		int nLength = strlen(str);
		Write(str, nLength);
		Write(" \r\n", 3);
		Close();
	}

	LeaveCriticalSection(&g_WriteLog);
}

void CLogFile::AddAppRecord(LPCTSTR description)
{
	if (wcslen(description) <= 0)
	{
		return;
	}

	CString strLogFileName;
	CString strLogPath;
	CString strTemp;

	CTime curTime = CTime::GetCurrentTime();	
	strTemp.Format(_T("\\%d��%d��%d��"), curTime.GetYear(), curTime.GetMonth(), curTime.GetDay());

	strLogPath = strLogBaseFloder + strTemp;
	CreateDirectory(strLogPath,NULL);

	strLogFileName = strLogPath + _T("//ImageGrab.log");

	AddRecord(strLogFileName, description);
}

void CLogFile::AddAppRecord(CString strFileName,LPCTSTR description)
{
	if (wcslen(description) <= 0)
	{
		return;
	}
	AddRecord2(strFileName, description);
}

void CLogFile::AddCameraRecord(int nEvent, LPCTSTR description)
{
	if (wcslen(description) <= 0)
	{
		return;
	}

	CString strLogFileName;
	CString strLogPath;
	CString strTemp;

	CTime curTime = CTime::GetCurrentTime();	
	strTemp.Format(_T("\\%d��%d��%d��\\Camera%d\\"), curTime.GetYear(), curTime.GetMonth(), curTime.GetDay(), nEvent);

	strLogPath = strLogBaseFloder + strTemp;
	CreateMultiDirs(strLogPath);

	strLogFileName = strLogPath + _T("CameraInfo.log");

	AddRecord(strLogFileName, description);
}

void CLogFile::AddTrainInfoRecord(int nEvent, LPCTSTR description)
{
	if (wcslen(description) <= 0)
	{
		return;
	}

	CString strLogFileName;
	CString strLogPath;
	CString strTemp;

	CTime curTime = CTime::GetCurrentTime();	
	strTemp.Format(_T("\\%d��%d��%d��\\Camera%d\\"), curTime.GetYear(), curTime.GetMonth(), curTime.GetDay(), nEvent);

	strLogPath = strLogBaseFloder + strTemp;
	CreateMultiDirs(strLogPath);

	strLogFileName = strLogPath + _T("\\TrainInfo.log");

	AddRecord(strLogFileName, description);
}

//add by zhuxy ���ָ��txt
void CLogFile::clearRecord(CString strFileName)
{
	CStdioFile file(strFileName,CFile::modeWrite);
	file.SetLength(0);
}

//// write the record
//const int UNICODE_TXT_FLG = 0xFEFF; 
//Write(&UNICODE_TXT_FLG,2);//д��ͷ��
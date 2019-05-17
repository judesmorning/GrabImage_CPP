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

CRITICAL_SECTION g_WriteLog;		// 临界段变量

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
	{	CTime tOldestTime;										// 记录文件的创建时间
	tOldestTime = CTime::GetCurrentTime();                    // 记录最早的时间
	int nBackFileCnt = 70;									// 日志文件数量
	int nLength = strPreFileName.GetLength();				// 文件字符串长度
	strPreFileName = strPreFileName.Left(nLength - 4 );		// 文件路径的长度
	
	// 文件信息
	CFileStatus status;
	
	// 查找文件备份文件：备份文件名称为TFDSClie00、 TFDSClie01、。。。。
	for(int i=0;i<nBackFileCnt;i++)
	{
		strTemp.Format(_T("%s%02d.log"), strPreFileName,i);
		// 检查文件是否存在，不存在，则直接返回当前的文件名	
		if(CFile::GetStatus(strTemp,status) ==TRUE)
		{
			// 文件存在，获取文件的时间,并且记录最早文件的时间
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
	// 如果文件已经存在，删除最老的文件，然后以这个文件名字作为新的文件
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
					
					// 当前数据写入到新文件
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
	//add by zhuxy 20190319	增加毫秒
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
	strTemp.Format(_T("\\%d年%d月%d日"), curTime.GetYear(), curTime.GetMonth(), curTime.GetDay());

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
	strTemp.Format(_T("\\%d年%d月%d日\\Camera%d\\"), curTime.GetYear(), curTime.GetMonth(), curTime.GetDay(), nEvent);

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
	strTemp.Format(_T("\\%d年%d月%d日\\Camera%d\\"), curTime.GetYear(), curTime.GetMonth(), curTime.GetDay(), nEvent);

	strLogPath = strLogBaseFloder + strTemp;
	CreateMultiDirs(strLogPath);

	strLogFileName = strLogPath + _T("\\TrainInfo.log");

	AddRecord(strLogFileName, description);
}

//add by zhuxy 清空指定txt
void CLogFile::clearRecord(CString strFileName)
{
	CStdioFile file(strFileName,CFile::modeWrite);
	file.SetLength(0);
}

//// write the record
//const int UNICODE_TXT_FLG = 0xFEFF; 
//Write(&UNICODE_TXT_FLG,2);//写入头部
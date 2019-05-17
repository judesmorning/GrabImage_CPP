#include "StdAfx.h"
#include "ImageGrab.h"
#include "Utils.h"
#include "Dbghelp.h"

//��������:��stringת����wstring  
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
wstring string2wstring(string str)  
{  
	wstring result;  
	//��ȡ��������С��������ռ䣬��������С���ַ�����  
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);  
	TCHAR* buffer = new TCHAR[len + 1];  
	//���ֽڱ���ת���ɿ��ֽڱ���  
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);  
	buffer[len] = '\0';             //����ַ�����β  
	//ɾ��������������ֵ  
	result.append(buffer);  
	delete[] buffer;  
	return result;  
}  

//��������:��wstringת����string
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
string wstring2string(wstring wstr)  
{  
	string result;  
	//��ȡ��������С��������ռ䣬��������С�°��ֽڼ����  
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);  
	char* buffer = new char[len + 1];  
	//���ֽڱ���ת���ɶ��ֽڱ���  
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);  
	buffer[len] = '\0';  
	//ɾ��������������ֵ  
	result.append(buffer);  
	delete[] buffer;  
	return result;  
} 

//��������:��ȡ��ǰapp��·��
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void GetAppPath(CString &strPath)
{
	TCHAR strExePath[MAX_PATH] = {0};
	GetModuleFileName(NULL, strExePath, MAX_PATH);

	wchar_t *pszPos = wcsrchr(strExePath, _T('\\'));
	if(pszPos == NULL)
		return;
	*pszPos = 0x0;
	pszPos = strExePath;
	strPath = pszPos;
}

//��������:�ݹ鴴���ļ���
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
BOOL CreateMultiDirs(CString xFilePath)
{
	int xFlag=xFilePath.Find(_T("\\"));
	CString s;
	for(int i=0;i<256;i++)
	{
		xFlag=xFilePath.Find(_T("\\"),xFlag+1);
		if(xFlag>3)
		{
			s=xFilePath.Left(xFlag);
			if (!PathFileExists(xFilePath))
			{
				CreateDirectory(s,0);
			}	
		}
		if(xFlag<0)
			return TRUE;
	}
	return FALSE;
}

//��������:�ݹ�ɾ���ļ���
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
BOOL DeleteDirectory(CString &pFilePath)
{ 
	if (pFilePath.IsEmpty())
	{
		return FALSE;
	}

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
  
	CString DirSpec;
	DirSpec += pFilePath;
	DirSpec += _T("/*");

    hFind = FindFirstFile(DirSpec, &FindFileData);
	while (FindNextFile(hFind, &FindFileData) != 0)
   {
	   if(_tcscmp(FindFileData.cFileName, _T(".")) != 0
		   && _tcscmp(FindFileData.cFileName,_T("..")) != 0)
	   {

			if (FindFileData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
			{   //�ҵ��ļ�
				CString Dir;
				Dir.Format(_T("%s\\%s"),pFilePath,FindFileData.cFileName);
				DeleteFile(Dir);
			}
			else if(FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY
				&& _tcscmp(FindFileData.cFileName, _T(".")) != 0
				&& _tcscmp(FindFileData.cFileName, _T("..")) != 0)
			{ //�ҵ�Ŀ¼
				 
				CString Dir;
				Dir.Format(_T("%s\\%s"),pFilePath,FindFileData.cFileName);
				DeleteDirectory(Dir);
			}
		}
	}
	
	FindClose(hFind);
	RemoveDirectory(pFilePath); 
	
	return  true;
} 
//��������:����dump�ļ�
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void CreateDumpFile(LPCTSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException)  
{  
	// ����Dump�ļ�  
	//  
	HANDLE hDumpFile = CreateFile(lpstrDumpFilePathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);  

	// Dump��Ϣ  
	//  
	MINIDUMP_EXCEPTION_INFORMATION dumpInfo;  
	dumpInfo.ExceptionPointers = pException;  
	dumpInfo.ThreadId = GetCurrentThreadId();  
	dumpInfo.ClientPointers = TRUE;  

	// д��Dump�ļ�����  
	//  
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);  

	CloseHandle(hDumpFile);  
}  

//��������:��ȡ��ǰʱ��
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
wstring GetPresentTime()  
{  
	SYSTEMTIME time;  
	GetLocalTime(&time);   

	wchar_t wszTime[128];  
	wsprintfW(wszTime, _T("%04d-%02d-%02d %02d-%02d-%02d-%03d"), time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);  

	return wstring(wszTime);  
}  

//��������:NULL
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
LONG CrashHandler(EXCEPTION_POINTERS *pException)  
{     
	wstring strDumpFileName = theApp.m_strCurrentPath + _T("\\Dump\\");
	CreateDirectory(strDumpFileName.c_str(), NULL);

	strDumpFileName += GetPresentTime();
	strDumpFileName += _T(".dmp");  

	// ����Dump�ļ�  
	//  
	CreateDumpFile(strDumpFileName.c_str(), pException);  

	return EXCEPTION_EXECUTE_HANDLER;  
}  

//��������:NULL
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
void Delay(HANDLE hEvent)
{
	if(hEvent == NULL)
	{
		return;
	}

	while(WaitForSingleObject(hEvent, 50) == WAIT_TIMEOUT)
	{
		MSG msg;
		while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
}

//��������:NULL
//��������:NULL
//��������:NULL
//��������ֵ:NULL
//������Ϣ:NULL
//��ע:NULL
int string_replase(string &s1, const string &s2, const string &s3)
{
	string::size_type pos = 0;
	string::size_type a = s2.size();
	string::size_type b = s3.size();
	while ((pos = s1.find(s2, pos)) != string::npos)
	{
		s1.replace(pos, a, s3);
		pos += b;
	}
	return 0;
}

#include "StdAfx.h"
#include "ImageGrab.h"
#include "Utils.h"
#include "Dbghelp.h"

//函数名称:将string转换成wstring  
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
wstring string2wstring(string str)  
{  
	wstring result;  
	//获取缓冲区大小，并申请空间，缓冲区大小按字符计算  
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);  
	TCHAR* buffer = new TCHAR[len + 1];  
	//多字节编码转换成宽字节编码  
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);  
	buffer[len] = '\0';             //添加字符串结尾  
	//删除缓冲区并返回值  
	result.append(buffer);  
	delete[] buffer;  
	return result;  
}  

//函数名称:将wstring转换成string
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
string wstring2string(wstring wstr)  
{  
	string result;  
	//获取缓冲区大小，并申请空间，缓冲区大小事按字节计算的  
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);  
	char* buffer = new char[len + 1];  
	//宽字节编码转换成多字节编码  
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);  
	buffer[len] = '\0';  
	//删除缓冲区并返回值  
	result.append(buffer);  
	delete[] buffer;  
	return result;  
} 

//函数名称:获取当前app的路径
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
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

//函数名称:递归创建文件夹
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
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

//函数名称:递归删除文件夹
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
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
			{   //找到文件
				CString Dir;
				Dir.Format(_T("%s\\%s"),pFilePath,FindFileData.cFileName);
				DeleteFile(Dir);
			}
			else if(FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY
				&& _tcscmp(FindFileData.cFileName, _T(".")) != 0
				&& _tcscmp(FindFileData.cFileName, _T("..")) != 0)
			{ //找到目录
				 
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
//函数名称:创建dump文件
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
void CreateDumpFile(LPCTSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException)  
{  
	// 创建Dump文件  
	//  
	HANDLE hDumpFile = CreateFile(lpstrDumpFilePathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);  

	// Dump信息  
	//  
	MINIDUMP_EXCEPTION_INFORMATION dumpInfo;  
	dumpInfo.ExceptionPointers = pException;  
	dumpInfo.ThreadId = GetCurrentThreadId();  
	dumpInfo.ClientPointers = TRUE;  

	// 写入Dump文件内容  
	//  
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);  

	CloseHandle(hDumpFile);  
}  

//函数名称:获取当前时间
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
wstring GetPresentTime()  
{  
	SYSTEMTIME time;  
	GetLocalTime(&time);   

	wchar_t wszTime[128];  
	wsprintfW(wszTime, _T("%04d-%02d-%02d %02d-%02d-%02d-%03d"), time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);  

	return wstring(wszTime);  
}  

//函数名称:NULL
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
LONG CrashHandler(EXCEPTION_POINTERS *pException)  
{     
	wstring strDumpFileName = theApp.m_strCurrentPath + _T("\\Dump\\");
	CreateDirectory(strDumpFileName.c_str(), NULL);

	strDumpFileName += GetPresentTime();
	strDumpFileName += _T(".dmp");  

	// 创建Dump文件  
	//  
	CreateDumpFile(strDumpFileName.c_str(), pException);  

	return EXCEPTION_EXECUTE_HANDLER;  
}  

//函数名称:NULL
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
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

//函数名称:NULL
//函数作用:NULL
//函数参数:NULL
//函数返回值:NULL
//函数信息:NULL
//备注:NULL
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

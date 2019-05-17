#ifndef _com_h_
#define _com_h_
#include <tchar.h>
#include <math.h>
#include<iostream>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
 
extern BYTE aaaa[4096];
class ComAsy//定义类
{
public: 
	ComAsy();
	~ComAsy();
	bool InitCOM(LPCTSTR Port);//打开串口
	void UninitCOM(); //关闭串口并清理
 
	//写入数据
	bool ComWrite(LPBYTE buf, int &len);
 
	//读取线程
	static unsigned int __stdcall OnRecv(void*);
 
private:
	HANDLE m_hCom;
	OVERLAPPED m_ovWrite;//用于写入数据
	OVERLAPPED m_ovRead;//用于读取数据
	OVERLAPPED m_ovWait;//用于等待数据
	volatile bool m_IsOpen;//串口是否打开
	HANDLE m_Thread;//读取线程句柄
};
#endif
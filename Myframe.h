#pragma once


// Myframe 框架

class Myframe : public CFrameWnd
{
	DECLARE_DYNCREATE(Myframe)
public:
	Myframe();           // 动态创建所使用的受保护的构造函数
	virtual ~Myframe();

protected:
	DECLARE_MESSAGE_MAP()
};



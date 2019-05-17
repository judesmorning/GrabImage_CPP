#ifndef __SAFELIST_H_
#define  __SAFELIST_H_

#include "stdafx.h"

namespace train
{
///////////////////////////////////////////////////////////////////////////////
	class XMutex
	{
	public:
		void Lock() {EnterCriticalSection(&m_sec);}
		void Unlock() {LeaveCriticalSection(&m_sec);}
		XMutex() {InitializeCriticalSection(&m_sec);}
		~XMutex() {DeleteCriticalSection(&m_sec);}
	protected:
		CRITICAL_SECTION m_sec;
	};

	#define SYN_OBJ()      XSynchronization _objXSynchronization(this)
	#define SYN_OBJ_EX(x)  XSynchronization _objXSynchronization(x)
///////////////////////////////////////////////////////////////////////////////
	class XSynchronization
	{
	public:
		XSynchronization(XMutex* lpMutex):m_lpMutex(lpMutex)
		{
			_ASSERT(NULL != m_lpMutex);
			m_lpMutex->Lock();
		}
		~XSynchronization()
		{
			m_lpMutex->Unlock();
		}
	private:
		XMutex* m_lpMutex;
	};

//////////////////////////////////////////////////////////////////////////////
//简单list，不用继承屏蔽其他list特性
//////////////////////////////////////////////////////////////////////////////
	template <class T > 
	class CSafeList:public XMutex  
	{
	public:
		typedef CList<T> SAFELIST;

		CSafeList(){}

		~CSafeList(){}


		inline BOOL IsEmpty()
		{
			SYN_OBJ();
            return m_List.IsEmpty();
		}

		void AddTail(const T& myT)
		{
			SYN_OBJ();
			m_List.AddTail(myT);
		}

		inline void RemoveHead()
		{
			SYN_OBJ();
			m_List.RemoveHead();
		}

		T GetHead()
		{
			SYN_OBJ();		
			return m_List.GetHead();
		}
	
		void RemoveAll()
		{
			SYN_OBJ();		
			m_List.RemoveAll();
		}

	private:
		SAFELIST m_List;
	};

//////////////////////////////////////////////////////////////////////

	//安全的计数类
	class CSafeCounter: public XMutex  
	{
	public:
		CSafeCounter(){ m_nNum=0;}

		~CSafeCounter(){}

	public:

		int GetNum()
		{
			SYN_OBJ();
			return m_nNum;
		}

		CSafeCounter& operator=(int num)
		{
			SYN_OBJ();
			m_nNum = num;

			return *this;
		}

		CSafeCounter& operator++()
		{
			SYN_OBJ();
			m_nNum++;

			return *this;
		}

		//CSafeCounter operator++(int)
		//{
		//	SYN_OBJ();
		//	CSafeCounter t;
		//	t.m_nNum = m_nNum++;

		//	return t;
		//}

		int operator++(int)
		{
			SYN_OBJ();
			int nNum;
			nNum = m_nNum++;

			return nNum;
		}

		CSafeCounter& operator--()
		{
			SYN_OBJ();
			m_nNum--;

			return *this;
		}

		CSafeCounter operator--(int)
		{
			SYN_OBJ();
			CSafeCounter t;
			t.m_nNum = m_nNum--;

			return t;
		}

		bool operator==(const int num) const
		{
			if (m_nNum == num)
			{
				return true;
			}

			return false;
		}

	private:
		int m_nNum;
	};
}



#endif
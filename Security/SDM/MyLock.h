// MyLock.h: interface for the CMyLock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYLOCK_H__839434D1_4762_4EE8_AE85_6FB2A8A68AF7__INCLUDED_)
#define AFX_MYLOCK_H__839434D1_4762_4EE8_AE85_6FB2A8A68AF7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMyLock  
{
public:
	CMyLock(){
		InitializeCriticalSection(&m_critSect);
	}

	virtual ~CMyLock(){
		DeleteCriticalSection(&m_critSect);
	}

public:
	void Lock(){
		EnterCriticalSection(&m_critSect);
	}

	void Unlock(){
		LeaveCriticalSection(&m_critSect);
	}
	
private:
	CRITICAL_SECTION m_critSect;
};

class CScopedLocker
{
public:
	CScopedLocker(CMyLock *lock)
		:m_pLock(lock)
	{
		m_pLock->Lock();
	}

	~CScopedLocker(){
		m_pLock->Unlock();
	}
private:
	CMyLock *m_pLock;
};

#endif // !defined(AFX_MYLOCK_H__839434D1_4762_4EE8_AE85_6FB2A8A68AF7__INCLUDED_)

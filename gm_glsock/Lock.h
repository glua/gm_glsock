#if !defined(LOCK_H)
#define LOCK_H

#if defined(WIN32)
#pragma once
#endif

#include <boost/thread.hpp>

template<class T>
class CScopedMutex
{
private:
	T* m_pInterface;
public:
	CScopedMutex(T* Interface)
	{
		m_pInterface = Interface;
		m_pInterface->Lock();
	}
	~CScopedMutex(void)
	{
		m_pInterface->UnLock();
	}
};

#define SCOPED_LOCK(l) boost::recursive_mutex::scoped_lock __Lock(l); CScopedMutex<ILuaInterface> __LuaLock(Lua())

#endif // LOCK_H

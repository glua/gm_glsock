#if !defined(SOCK_H)
#define SOCK_H

#if defined(WIN32)
#pragma once
#endif

#include <vector>
#include <string>
#include <stack>
#include "GLSock.h"

class CSockMgr
{
	typedef boost::recursive_mutex Mutex_t;

private:
	boost::asio::io_service m_IOService;
	boost::asio::io_service::work m_Worker;
	boost::thread m_Thread;
	Mutex_t m_Mutex;
	std::vector<GLSock::ISock*> m_vecSocks;
	std::stack<boost::function<void(lua_State*)> > m_Callbacks;

public:
	CSockMgr(void);
	~CSockMgr(void);

	/*
	bool StartThread();
	bool StopThread();
	*/

	GLSock::ISock* CreateAcceptorSock(lua_State* L);
	GLSock::ISock* CreateTCPSock(lua_State* L);
	GLSock::ISock* CreateUDPSock(lua_State* L);

	bool RemoveSock(GLSock::ISock* pSock);
	bool ValidHandle(GLSock::ISock* pSock);

	template<typename T>
	void StoreCallback(T cb)
	{
		Mutex_t::scoped_lock lock(m_Mutex);

		m_Callbacks.push(cb);
	}

	void Poll(lua_State* L)
	{
		Mutex_t::scoped_lock lock(m_Mutex);

		m_IOService.poll_one();

		while( !m_Callbacks.empty() )
		{
			boost::function<void(lua_State*)>& cb = m_Callbacks.top();
			cb(L);
			m_Callbacks.pop();
		}
	}

	boost::recursive_mutex& Mutex()
	{
		return m_Mutex;
	}
};

extern CSockMgr* g_pSockMgr;

#endif // SOCK_H

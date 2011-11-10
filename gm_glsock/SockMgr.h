#if !defined(SOCK_H)
#define SOCK_H

#if defined(WIN32)
#pragma once
#endif

#include <vector>
#include <string>
#include <stack>

#include "GLSockAcceptor.h"
#include "GLSockTCP.h"
#include "GLSockUDP.h"

#include <boost/thread.hpp>

class CSockMgr
{
	typedef boost::recursive_mutex Mutex_t;

private:
	boost::asio::io_service m_IOService;
	boost::asio::io_service::work* m_pWorker;
	boost::thread m_Thread;
	Mutex_t m_Mutex;
	std::vector<GLSock::CGLSock*> m_vecSocks;
	std::stack<boost::function<void(lua_State*)> > m_Callbacks;

public:
	CSockMgr(void);
	~CSockMgr(void);

	GLSock::CGLSock* CreateAcceptorSock(lua_State* L);
	GLSock::CGLSock* CreateTCPSock(lua_State* L, bool bOpen = true);
	GLSock::CGLSock* CreateUDPSock(lua_State* L);

	bool RemoveSock(GLSock::CGLSock* pSock);
	bool ValidHandle(GLSock::CGLSock* pSock);

	bool CloseSockets();

	template<typename T>
	void StoreCallback(T cb)
	{
		Mutex_t::scoped_lock lock(m_Mutex);

		m_Callbacks.push(cb);
	}

	void Poll(lua_State* L)
	{
		Mutex_t::scoped_lock lock(m_Mutex);
		try
		{
			m_IOService.poll();
			if( !m_Callbacks.empty() )
			{
				boost::function<void(lua_State*)> cb = m_Callbacks.top();
				m_Callbacks.pop();
				cb(L);
			}
		}
		catch (boost::exception& ex)
		{
			Lua()->Msg("GLSock(Polling): %s\n",  boost::diagnostic_information(ex).c_str());
		}
	}

	boost::recursive_mutex& Mutex()
	{
		return m_Mutex;
	}
};

extern CSockMgr* g_pSockMgr;

#endif // SOCK_H

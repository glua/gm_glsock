#include "SockMgr.h"

static CSockMgr s_SockMgr;
CSockMgr* g_pSockMgr = &s_SockMgr;

CSockMgr::CSockMgr( void ) :
	m_Worker(m_IOService)
{
}

CSockMgr::~CSockMgr( void )
{
	std::vector<GLSock::CGLSock*>::iterator itr;

	m_Mutex.lock();

	for( itr = m_vecSocks.begin(); itr != m_vecSocks.end(); itr++ )
	{
		//(*itr)->Destroy();
		(*itr)->Cancel();
		(*itr)->Close();
		delete *itr;
	}
	
	m_Mutex.unlock();

	for(;;)
	{
		{
			Mutex_t::scoped_lock lock(m_Mutex);
			if( m_vecSocks.empty() )
				break;
		}
		m_IOService.poll_one();
	}

	m_IOService.stop();
	m_IOService.reset();
}

GLSock::CGLSock* CSockMgr::CreateAcceptorSock(lua_State* L)
{
	GLSock::CGLSockAcceptor* pSock = new GLSock::CGLSockAcceptor(m_IOService, L);
	m_vecSocks.push_back(pSock);

	return pSock;
}

GLSock::CGLSock* CSockMgr::CreateTCPSock( lua_State* L, bool bOpen )
{
	GLSock::CGLSockTCP* pSock = new GLSock::CGLSockTCP(m_IOService, L, bOpen);
	m_vecSocks.push_back(pSock);

	return pSock;
}

GLSock::CGLSock* CSockMgr::CreateUDPSock( lua_State* L )
{
	GLSock::CGLSockUDP* pSock = new GLSock::CGLSockUDP(m_IOService, L);
	m_vecSocks.push_back(pSock);

	return pSock;
}

bool CSockMgr::RemoveSock( GLSock::CGLSock* pSock )
{
	std::vector<GLSock::CGLSock*>::iterator itr;
	for( itr = m_vecSocks.begin(); itr != m_vecSocks.end(); itr++ )
	{
		if( *itr == pSock )
		{
			m_vecSocks.erase(itr);
			return true;
		}
	}

	return false;
}

bool CSockMgr::ValidHandle( GLSock::CGLSock* pSock )
{
	std::vector<GLSock::CGLSock*>::iterator itr;
	for( itr = m_vecSocks.begin(); itr != m_vecSocks.end(); itr++ )
	{
		if( *itr == pSock )
			return true;
	}

	return false;
}

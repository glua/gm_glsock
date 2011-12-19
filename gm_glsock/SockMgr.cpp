#include "Common.h"
#include "SockMgr.h"

static CSockMgr s_SockMgr;
CSockMgr* g_pSockMgr = &s_SockMgr;

CSockMgr::CSockMgr( void )
{
	m_pWorker = new boost::asio::io_service::work(m_IOService);
}

CSockMgr::~CSockMgr( void )
{
	delete m_pWorker;
}

GLSock::CGLSock* CSockMgr::CreateAcceptorSock(lua_State* L)
{
	Mutex_t::scoped_lock lock(m_Mutex);

	GLSock::CGLSock* pSock = new GLSock::CGLSockAcceptor(m_IOService, L);
	pSock->m_nTableRef = 0;
	m_vecSocks.push_back(pSock);

	return pSock;
}

GLSock::CGLSock* CSockMgr::CreateTCPSock( lua_State* L, bool bOpen )
{
	Mutex_t::scoped_lock lock(m_Mutex);

	GLSock::CGLSockTCP* pSock = new GLSock::CGLSockTCP(m_IOService, L, bOpen);
	pSock->m_nTableRef = 0;
	m_vecSocks.push_back(pSock);

	return pSock;
}

GLSock::CGLSock* CSockMgr::CreateUDPSock( lua_State* L )
{
	Mutex_t::scoped_lock lock(m_Mutex);

	GLSock::CGLSockUDP* pSock = new GLSock::CGLSockUDP(m_IOService, L);
	pSock->m_nTableRef = 0;
	m_vecSocks.push_back(pSock);

	return pSock;
}

bool CSockMgr::RemoveSock( GLSock::CGLSock* pSock )
{
	Mutex_t::scoped_lock lock(m_Mutex);

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
	Mutex_t::scoped_lock lock(m_Mutex);

	std::vector<GLSock::CGLSock*>::iterator itr;
	for( itr = m_vecSocks.begin(); itr != m_vecSocks.end(); itr++ )
	{
		if( *itr == pSock )
			return true;
	}

	return false;
}

bool CSockMgr::CloseSockets()
{
	std::vector<GLSock::CGLSock*>::iterator itr;

	for( itr = m_vecSocks.begin(); itr != m_vecSocks.end(); itr++ )
	{
		(*itr)->Close();
		delete *itr;
	}
	m_vecSocks.clear();

	m_IOService.stop();

	return true;
}

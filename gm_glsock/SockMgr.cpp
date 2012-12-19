#include "Common.h"
#include "SockMgr.h"

static CSockMgr s_SockMgr;
CSockMgr* g_pSockMgr = &s_SockMgr;

CSockMgr::CSockMgr( void ) 
	: m_IOService(),
	m_Worker(m_IOService)
{
}

CSockMgr::~CSockMgr( void )
{
}

void CSockMgr::Startup()
{
}

void CSockMgr::Cleanup()
{
#ifdef _WIN32
	// https://svn.boost.org/trac/boost/ticket/6654
	boost::asio::detail::win_thread::set_terminate_threads(true);
#endif
}

GLSock::CGLSock* CSockMgr::CreateAcceptorSock(lua_State *state)
{
	Mutex_t::scoped_lock lock(m_Mutex);

	GLSock::CGLSock* pSock = new GLSock::CGLSockAcceptor(m_IOService, state);
	pSock->m_nTableRef = 0;
	m_vecSocks.push_back(pSock);

	return pSock;
}

GLSock::CGLSock* CSockMgr::CreateTCPSock( lua_State *state, bool bOpen )
{
	Mutex_t::scoped_lock lock(m_Mutex);

	GLSock::CGLSockTCP* pSock = new GLSock::CGLSockTCP(m_IOService, state, bOpen);
	pSock->m_nTableRef = 0;
	m_vecSocks.push_back(pSock);

	return pSock;
}

GLSock::CGLSock* CSockMgr::CreateUDPSock( lua_State *state )
{
	Mutex_t::scoped_lock lock(m_Mutex);

	GLSock::CGLSockUDP* pSock = new GLSock::CGLSockUDP(m_IOService, state);
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

	try
	{
		for( itr = m_vecSocks.begin(); itr != m_vecSocks.end(); itr++ )
		{
			(*itr)->Close();
			delete *itr;
		}
		m_vecSocks.clear();

		m_IOService.stop();
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		std::cout << "Dirty cleanup: " << boost::diagnostic_information(ex).c_str() << std::endl;
#endif
		UNREFERENCED_PARAM(ex);
	}

	return true;
}

#include "Common.h"
#include "GLSockAcceptor.h"
#include "SockMgr.h"
#include "LuaHelper.h"
#include "BindingGLSock.h"

namespace GLSock {

CGLSockAcceptor::CGLSockAcceptor( IOService_t& IOService_t, lua_State* pLua ) 
	: m_Sock(IOService_t)
{
	state = pLua;
	m_nReferences = 0;

	try
	{
		m_Sock.open(boost::asio::ip::tcp::v4());
		m_Sock.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	}
	catch (boost::exception& ex)
	{
		//LUA->Msg("GLSock(Acceptor): %s\n",  boost::diagnostic_information(ex).c_str());
		UNREFERENCED_PARAM(ex);
	}
}

bool CGLSockAcceptor::Bind( CEndpoint& Endpoint, Callback_t Callback )
{
	bool bResult = true;

	try
	{
		boost::system::error_code ec;
		boost::asio::ip::tcp::endpoint ep = Endpoint.ToTCP();

		m_Sock.bind( ep, ec );

		if( ec )
		{
#if defined(_DEBUG)
			//LUA->Msg("GLSock(Acceptor): Unable to bind to %s:%u\n", ep.address().to_string().c_str(), ep.port());
#endif
		}
		else
		{
#if defined(_DEBUG)
			//LUA->Msg("GLSock(Acceptor): Bound to %s:%u\n", ep.address().to_string().c_str(), ep.port());
#endif
		}

		if( g_pSockMgr->ValidHandle(this) )
			g_pSockMgr->StoreCallback( this, boost::bind(&CGLSock::CallbackBind, this, Callback, this, TranslateErrorMessage(ec), _1) );
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		//LUA->Msg("GLSock(Acceptor): %s\n",  boost::diagnostic_information(ex).c_str());
#endif
		UNREFERENCED_PARAM(ex);
		bResult = false;
	}

	return bResult;
}

bool CGLSockAcceptor::Listen( int iBacklog, Callback_t Callback )
{
	bool bResult = true;

	try
	{
		boost::system::error_code ec;
		m_Sock.listen(iBacklog, ec);

		if( ec )
		{
#if defined(_DEBUG)
			//LUA->Msg("GLSock(Acceptor): %s\n", ec.message().c_str());
#endif
		}
		if( g_pSockMgr->ValidHandle(this) )
			g_pSockMgr->StoreCallback( this, boost::bind(&CGLSock::CallbackListen, this, Callback, this, TranslateErrorMessage(ec), _1) );
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		//LUA->Msg("GLSock(Acceptor): %s\n",  boost::diagnostic_information(ex).c_str());
#endif
		UNREFERENCED_PARAM(ex);
		bResult = false;
	}

	return bResult;
}

bool CGLSockAcceptor::Accept( Callback_t Callback )
{
	bool bResult = true;

	try
	{
		CGLSockTCP* pSock = (CGLSockTCP*)g_pSockMgr->CreateTCPSock(state, false);

		m_Sock.async_accept(pSock->Socket(), 
			boost::bind(&CGLSockAcceptor::OnAccept, this, Callback, pSock, boost::asio::placeholders::error));
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		//LUA->Msg("GLSock(Acceptor): %s\n",  boost::diagnostic_information(ex).c_str());
#endif
		UNREFERENCED_PARAM(ex);
		bResult = false;
	}

	return bResult;
}

bool CGLSockAcceptor::Resolve( const char* cszHostname, Callback_t Callback )
{
	// TODO: Implement me
	return true;
}

bool CGLSockAcceptor::Close( void )
{
	bool bResult = true;

	boost::system::error_code ec;
	try
	{
		m_Sock.cancel(ec);
		m_Sock.close(ec);
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		//LUA->Msg("GLSock(Acceptor): %s\n",  boost::diagnostic_information(ex).c_str());
#endif
		UNREFERENCED_PARAM(ex);
		bResult = false;
	}

	return bResult;
}

int CGLSockAcceptor::Reference( void )
{
	m_nReferences++;
	return m_nReferences;
}

int CGLSockAcceptor::Unreference( void )
{
	m_nReferences--;
	if( m_nReferences <= 0 )
	{
		//g_pSockMgr->RemoveSock(this);
		Destroy();
	}
	return m_nReferences;
}

void CGLSockAcceptor::OnAccept( Callback_t Callback, CGLSockTCP* pSock, const boost::system::error_code& ec )
{
	if( ec )
	{
#if defined(_DEBUG)
		//LUA->Msg("GLSock(Acceptor): %s\n", ec.message().c_str());
#endif
		if( g_pSockMgr->ValidHandle(pSock) )
		{
			pSock->Close();
			delete pSock;
		}
		pSock = NULL;
	}

	if( g_pSockMgr->ValidHandle(this) )
		g_pSockMgr->StoreCallback( this, boost::bind(&CGLSock::CallbackAccept, this, Callback, this, pSock, TranslateErrorMessage(ec), _1) );
}

void CGLSockAcceptor::OnDestroy( void )
{
	if( g_pSockMgr->RemoveSock(this) )
		delete this;
}

}

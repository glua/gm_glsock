#include "Common.h"
#include "GLSockAcceptor.h"
#include "SockMgr.h"

namespace GLSock {

CGLSockAcceptor::CGLSockAcceptor( IOService_t& IOService_t, lua_State* pLua ) 
	: m_Sock(IOService_t)
{
	L = pLua;
	m_nReferences = 0;

	try
	{
		m_Sock.open(boost::asio::ip::tcp::v4());
		m_Sock.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	}
	catch (boost::exception& ex)
	{
		Lua()->Msg("GLSock(Acceptor): %s\n",  boost::diagnostic_information(ex).c_str());
	}
}

void CGLSockAcceptor::CallbackBind(Callback_t Callback, CGLSock* pHandle, int iErrorMsg, lua_State* L)
{
	pHandle->Reference();
        
	CAutoUnRef MetaTable = Lua()->GetMetaTable(GLSOCK_NAME, GLSOCK_TYPE);

	Lua()->PushReference(Callback);
	Lua()->PushUserData(MetaTable, static_cast<void*>(pHandle));
	Lua()->Push((float)iErrorMsg);

	Lua()->Call(2, 0);
        
	pHandle->Unreference();
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
			Lua()->Msg("GLSock(Acceptor): Unable to bind to %s:%u\n", ep.address().to_string().c_str(), ep.port());
#endif
		}
		else
		{
#if defined(_DEBUG)
			Lua()->Msg("GLSock(Acceptor): Bound to %s:%u\n", ep.address().to_string().c_str(), ep.port());
#endif
		}

		if( g_pSockMgr->ValidHandle(this) )
			g_pSockMgr->StoreCallback( boost::bind(&CGLSockAcceptor::CallbackBind, this, Callback, this, TranslateErrorMessage(ec), _1) );
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(Acceptor): %s\n",  boost::diagnostic_information(ex).c_str());
#else
		UNREFERENCED_PARAM(ex);
#endif
		bResult = false;
	}

	return bResult;
}

void CGLSockAcceptor::CallbackListen(Callback_t Callback, CGLSock* pHandle, int iErrorMsg, lua_State* L) // Callback(Handle, Error)
{
	pHandle->Reference();
        
	CAutoUnRef MetaTable = Lua()->GetMetaTable(GLSOCK_NAME, GLSOCK_TYPE);

	Lua()->PushReference(Callback);
	Lua()->PushUserData(MetaTable, static_cast<void*>(pHandle));
	Lua()->Push((float)iErrorMsg);

	Lua()->Call(2, 0);
        
	pHandle->Unreference();
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
			Lua()->Msg("GLSock(Acceptor): %s\n", ec.message().c_str());
#endif
		}
		if( g_pSockMgr->ValidHandle(this) )
			g_pSockMgr->StoreCallback( boost::bind(&CGLSockAcceptor::CallbackListen, this, Callback, this, TranslateErrorMessage(ec), _1) );
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(Acceptor): %s\n",  boost::diagnostic_information(ex).c_str());
#else
		UNREFERENCED_PARAM(ex);
#endif
		bResult = false;
	}

	return bResult;
}

bool CGLSockAcceptor::Accept( Callback_t Callback )
{
	bool bResult = true;

	try
	{
		CGLSockTCP* pSock = (CGLSockTCP*)g_pSockMgr->CreateTCPSock(L, false);

		m_Sock.async_accept(pSock->Socket(), 
			boost::bind(&CGLSockAcceptor::OnAccept, this, Callback, pSock, boost::asio::placeholders::error));
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(Acceptor): %s\n",  boost::diagnostic_information(ex).c_str());
#else
		UNREFERENCED_PARAM(ex);
#endif
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
		m_Sock.close();
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(Acceptor): %s\n",  boost::diagnostic_information(ex).c_str());
#else
		UNREFERENCED_PARAM(ex);
#endif
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

void CGLSockAcceptor::CallbackAccept(Callback_t Callback, CGLSock* pHandle, CGLSock* pSock, int iErrorMsg, lua_State* L)
{
	pHandle->Reference();
        
	CAutoUnRef MetaTable = Lua()->GetMetaTable(GLSOCK_NAME, GLSOCK_TYPE);

	Lua()->PushReference(Callback);
	Lua()->PushUserData(MetaTable, static_cast<void*>(pHandle));

	if(!pSock)
	{
		Lua()->PushNil();
	}
	else
	{
		CAutoUnRef MetaTable2 = Lua()->GetMetaTable(GLSOCK_NAME, GLSOCK_TYPE);

		pSock->Reference();
		Lua()->PushUserData(MetaTable2, static_cast<void*>(pSock));
	}

	Lua()->Push((float)iErrorMsg);
	Lua()->Call(3, 0);
        
	//pHandle->Unreference();
}

void CGLSockAcceptor::OnAccept( Callback_t Callback, CGLSockTCP* pSock, const boost::system::error_code& ec )
{
	if( ec )
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(Acceptor): %s\n", ec.message().c_str());
#endif
		if( g_pSockMgr->ValidHandle(pSock) )
		{
			pSock->Close();
			delete pSock;
		}
		pSock = NULL;
	}

	if( g_pSockMgr->ValidHandle(this) )
		g_pSockMgr->StoreCallback( boost::bind(&CGLSockAcceptor::CallbackAccept, this, Callback, this, pSock, TranslateErrorMessage(ec), _1) );
}

void CGLSockAcceptor::OnDestroy( void )
{
	if( g_pSockMgr->RemoveSock(this) )
		delete this;
}

}

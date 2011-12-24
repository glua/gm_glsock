#include "Common.h"
#include "GLSockTCP.h"
#include "SockMgr.h"
#include "BufferMgr.h"

namespace GLSock {

CGLSockTCP::CGLSockTCP( IOService_t& IOService, lua_State* pLua, bool bOpen ) 
	: m_Sock(IOService), m_Resolver(IOService)
{
	L = pLua;
	m_nReferences = 0;

	try
	{
		if( bOpen )
			m_Sock.open(boost::asio::ip::tcp::v4());
	}
	catch (boost::exception& ex)
	{
		Lua()->Msg("GLSock(TCP): %s\n",  boost::diagnostic_information(ex).c_str());
	}
}

void CGLSockTCP::CallbackBind(Callback_t Callback, CGLSock* pHandle, int iErrorMsg, lua_State* L)
{
	pHandle->Reference();
        
	CAutoUnRef MetaTable = Lua()->GetMetaTable(GLSOCK_NAME, GLSOCK_TYPE);

	Lua()->PushReference(Callback);
	Lua()->PushUserData(MetaTable, static_cast<void*>(pHandle));
	Lua()->Push((float)iErrorMsg);

	Lua()->Call(2, 0);
        
	pHandle->Unreference();
}

bool CGLSockTCP::Bind( CEndpoint& Endpoint, Callback_t Callback )
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
			Lua()->Msg("GLSock(TCP): Unable to bind to %s:%u\n", ep.address().to_string().c_str(), ep.port());
#endif
		}
		else
		{
#if defined(_DEBUG)
			Lua()->Msg("GLSock(TCP): Bound to %s:%u\n", ep.address().to_string().c_str(), ep.port());
#endif
		}
		if( g_pSockMgr->ValidHandle(this) )
			g_pSockMgr->StoreCallback( this, boost::bind(&CGLSockTCP::CallbackBind, this, Callback, this, TranslateErrorMessage(ec), _1) );
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(TCP): %s\n",  boost::diagnostic_information(ex).c_str());
#else
		UNREFERENCED_PARAM(ex);
#endif
		bResult = false;
	}

	return bResult;
}

bool CGLSockTCP::Connect( std::string strHost, std::string strPort, Callback_t Callback )
{
	bool bResult = true;

	try
	{
		TCPResolver_t::query query(strHost, strPort);
		TCPResolver_t::iterator endpoint_iterator = m_Resolver.resolve(query);

		boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

		m_Sock.async_connect( endpoint,
			boost::bind(&CGLSockTCP::OnConnect, this, Callback, boost::asio::placeholders::error, ++endpoint_iterator ));
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(TCP): %s\n",  boost::diagnostic_information(ex).c_str());
#else
		UNREFERENCED_PARAM(ex);
#endif
		bResult = false;
	}

	return bResult;
}

bool CGLSockTCP::Send( const char* cbData, unsigned int cubBuffer, Callback_t Callback )
{
	bool bResult = true;

	try
	{
		m_Sock.async_send( boost::asio::buffer(cbData, cubBuffer),
			boost::bind(&CGLSockTCP::OnSend, this, Callback, boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error) );
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(TCP): %s\n",  boost::diagnostic_information(ex).c_str());
#else
		UNREFERENCED_PARAM(ex);
#endif
		bResult = false;
	}

	return bResult;
}

bool CGLSockTCP::Read( unsigned int cubBuffer, Callback_t Callback )
{
	bool bResult = true;

	try
	{
		char* pData = new char[cubBuffer];

		m_Sock.async_receive(boost::asio::buffer(pData, cubBuffer),
			boost::bind(&CGLSockTCP::OnRead, this, Callback, pData, boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error) );
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(TCP): %s\n",  boost::diagnostic_information(ex).c_str());
#else
		UNREFERENCED_PARAM(ex);
#endif
		bResult = false;
	}

	return bResult;
}

bool CGLSockTCP::ReadUntil( std::string strDelimiter, Callback_t Callback )
{
	bool bResult = true;

	try
	{
		char* pData = new char[1];

		m_Sock.async_receive(boost::asio::buffer(pData, 1),
			boost::bind(&CGLSockTCP::OnReadUntil, this, Callback, strDelimiter, pData, boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error) );
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(TCP): %s\n",  boost::diagnostic_information(ex).c_str());
#else
		UNREFERENCED_PARAM(ex);
#endif
		bResult = false;
	}

	return bResult;
}

bool CGLSockTCP::Resolve( const char* cszHostname, Callback_t Callback )
{
	// TODO: Implement me.
	return true;
}

bool CGLSockTCP::Close( void )
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
		Lua()->Msg("GLSock(TCP): %s\n",  boost::diagnostic_information(ex).c_str());
#else
		UNREFERENCED_PARAM(ex);
#endif
		bResult = false;
	}

	return bResult;
}

int CGLSockTCP::Reference( void )
{
	m_nReferences++;
	return m_nReferences;
}

int CGLSockTCP::Unreference( void )
{
	m_nReferences--;
	if( m_nReferences <= 0 )
	{
		//g_pSockMgr->RemoveSock(this);
		Destroy();
	}
	return m_nReferences;
}

void CGLSockTCP::OnResolve( Callback_t Callback, const boost::system::error_code& err, TCPResolver_t::iterator endpoint_iterator )
{
	// TODO: Implement me.
}

void CGLSockTCP::CallbackConnect( Callback_t Callback, CGLSock* pHandle, int iErrorMsg, lua_State* L )
{
	pHandle->Reference();
        
	CAutoUnRef MetaTable = Lua()->GetMetaTable(GLSOCK_NAME, GLSOCK_TYPE);

	Lua()->PushReference(Callback);
	Lua()->PushUserData(MetaTable, static_cast<void*>(pHandle));
	Lua()->Push((float)iErrorMsg);

	Lua()->Call(2, 0);
        
	pHandle->Unreference();
}

void CGLSockTCP::OnConnect( Callback_t Callback, const boost::system::error_code& ec, TCPResolver_t::iterator endpoint_iterator )
{
	if( !ec )
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(TCP): Connected to Host!\n");
#endif

		if( g_pSockMgr->ValidHandle(this) )
			g_pSockMgr->StoreCallback( this, boost::bind(&CGLSockTCP::CallbackConnect, this, Callback, this, TranslateErrorMessage(ec), _1) );
	}
	else
	{
		if( endpoint_iterator != boost::asio::ip::tcp::resolver::iterator() )
		{
#if defined(_DEBUG)
			Lua()->Msg("GLSock(TCP): Connect attempt failed, trying next resolver.\n");
#endif

			boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

			//m_Sock.close();
			m_Sock.async_connect( endpoint,
				boost::bind(&CGLSockTCP::OnConnect, this, Callback, boost::asio::placeholders::error, endpoint_iterator++) );
		}
		else
		{
#if defined(_DEBUG)
			Lua()->Msg("GLSock(TCP) Connection failed: %s\n", ec.message().c_str());
#endif
			if( g_pSockMgr->ValidHandle(this) )
				g_pSockMgr->StoreCallback( this, boost::bind(&CGLSockTCP::CallbackConnect, this, Callback, this, TranslateErrorMessage(ec), _1) );
		}
	}
}

void CGLSockTCP::CalllbackSend( Callback_t Callback, CGLSock* pHandle, unsigned int cubBytes, int iErrorMsg, lua_State* L )
{
	pHandle->Reference();
        
	CAutoUnRef MetaTable = Lua()->GetMetaTable(GLSOCK_NAME, GLSOCK_TYPE);

	Lua()->PushReference(Callback);
	Lua()->PushUserData(MetaTable, static_cast<void*>(pHandle));
	Lua()->PushLong(cubBytes);
	Lua()->Push((float)iErrorMsg);

	Lua()->Call(3, 0);
        
	pHandle->Unreference();
}

void CGLSockTCP::OnSend( Callback_t Callback, unsigned int cubBytes, const boost::system::error_code& ec )
{
	if( ec )
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(TCP): %s\n", ec.message().c_str());
#endif
	}

	if( g_pSockMgr->ValidHandle(this) )
		g_pSockMgr->StoreCallback( this, boost::bind(&CGLSockTCP::CalllbackSend, this, Callback, this, cubBytes, TranslateErrorMessage(ec), _1) );
}

void CGLSockTCP::CallbackRead( Callback_t Callback, CGLSock* pHandle, GLSockBuffer::CGLSockBuffer* pBuffer, int iErrorMsg, lua_State* L)
{
	pHandle->Reference();
        
	CAutoUnRef MetaTable = Lua()->GetMetaTable(GLSOCK_NAME, GLSOCK_TYPE);
	Lua()->PushReference(Callback);
	Lua()->PushUserData(MetaTable, static_cast<void*>(pHandle));

	if( !pBuffer )
	{
		Lua()->PushNil();
	}
	else
	{
		// TODO: See if the reference should last longer
		CAutoUnRef MetaTableBuffer = Lua()->GetMetaTable(GLSOCKBUFFER_NAME, GLSOCKBUFFER_TYPE);

		pBuffer->Reference();
		Lua()->PushUserData(MetaTableBuffer, static_cast<void*>(pBuffer));
	}

	Lua()->Push((float)iErrorMsg);
	Lua()->Call(3, 0);
        
	pHandle->Unreference();
}

void CGLSockTCP::OnRead( Callback_t Callback, const char* pData, unsigned int cubBytes, const boost::system::error_code& ec )
{
	GLSockBuffer::CGLSockBuffer* pBuffer = NULL;

	if( ec )
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(TCP): %s\n", ec.message().c_str());
#endif
	}
	else
	{
		pBuffer = g_pBufferMgr->Create();
		pBuffer->Write(pData, cubBytes);

		pBuffer->Seek(0, SOCKBUFFER_SEEK_SET);
	}

	delete[] pData;

	if( g_pSockMgr->ValidHandle(this) )
		g_pSockMgr->StoreCallback( this, boost::bind(&CGLSockTCP::CallbackRead, this, Callback, this, pBuffer, TranslateErrorMessage(ec), _1) );
}

void CGLSockTCP::OnReadUntil( Callback_t Callback, std::string strDelimiter, const char* pData, unsigned int cubBytes, const boost::system::error_code& ec )
{
	GLSockBuffer::CGLSockBuffer* pBuffer = NULL;

	if( ec )
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(TCP): %s\n", ec.message().c_str());
#endif
		delete pData;

		if( g_pSockMgr->ValidHandle(this) )
			g_pSockMgr->StoreCallback( this, boost::bind(&CGLSockTCP::CallbackRead, this, Callback, this, pBuffer, TranslateErrorMessage(ec), _1) );
	}
	else
	{
		m_strBuffered.append(pData, cubBytes);
		if( m_strBuffered.size() >= strDelimiter.size() )
		{
			std::size_t nPos = m_strBuffered.size() - strDelimiter.size();
			if( m_strBuffered.substr(nPos, strDelimiter.size()) == strDelimiter )
			{
				pBuffer = g_pBufferMgr->Create();
				pBuffer->Write(m_strBuffered.c_str(), m_strBuffered.size());
				pBuffer->Seek(0, SOCKBUFFER_SEEK_SET);

				m_strBuffered.clear();
				delete pData;

				if( g_pSockMgr->ValidHandle(this) )
					g_pSockMgr->StoreCallback( this, boost::bind(&CGLSockTCP::CallbackRead, this, Callback, this, pBuffer, TranslateErrorMessage(ec), _1) );

				return;
			}
		}

		// Keep reading.
		m_Sock.async_receive(boost::asio::buffer((char*)pData, 1),
			boost::bind(&CGLSockTCP::OnReadUntil, this, Callback, strDelimiter, pData, boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error) );
	}

}

void CGLSockTCP::OnDestroy( void )
{
	if( g_pSockMgr->RemoveSock(this) )
		delete this;
}

}

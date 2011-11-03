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
			g_pSockMgr->StoreCallback( boost::bind(&CGLSockTCP::CallbackBind, this, Callback, this, TranslateErrorMessage(ec), _1) );
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(TCP): %s\n",  boost::diagnostic_information(ex).c_str());
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
#endif
		bResult = false;
	}

	return bResult;
}

bool CGLSockTCP::ReadUntil( const char* pszDelimiter, Callback_t Callback )
{
	bool bResult = true;

	try
	{
		boost::asio::streambuf* pStreamBuf = new boost::asio::streambuf;

		boost::asio::async_read_until(m_Sock, *pStreamBuf, pszDelimiter,
			boost::bind(&CGLSockTCP::OnReadUntil, this, Callback, pStreamBuf, boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error));
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(TCP): %s\n",  boost::diagnostic_information(ex).c_str());
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
#endif
		bResult = false;
	}

	return bResult;
}

void CGLSockTCP::Reference( void )
{
	m_nReferences++;
}

void CGLSockTCP::Unreference( void )
{
	m_nReferences--;
	if( m_nReferences <= 0 )
	{
		//g_pSockMgr->RemoveSock(this);
		Destroy();
	}
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
		Lua()->Msg("GLSock(TCP): Connected to Host!\n");

		if( g_pSockMgr->ValidHandle(this) )
			g_pSockMgr->StoreCallback( boost::bind(&CGLSockTCP::CallbackConnect, this, Callback, this, TranslateErrorMessage(ec), _1) );
	}
	else
	{
		if( endpoint_iterator != boost::asio::ip::tcp::resolver::iterator() )
		{
			Lua()->Msg("GLSock(TCP): Connect attempt failed, trying next resolver.\n");

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
				g_pSockMgr->StoreCallback( boost::bind(&CGLSockTCP::CallbackConnect, this, Callback, this, TranslateErrorMessage(ec), _1) );
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
		g_pSockMgr->StoreCallback( boost::bind(&CGLSockTCP::CalllbackSend, this, Callback, this, cubBytes, TranslateErrorMessage(ec), _1) );
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
		g_pSockMgr->StoreCallback( boost::bind(&CGLSockTCP::CallbackRead, this, Callback, this, pBuffer, TranslateErrorMessage(ec), _1) );
}

void CGLSockTCP::OnReadUntil( Callback_t Callback, boost::asio::streambuf* pStreamBuf, unsigned int cubBytes, const boost::system::error_code& ec )
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
		boost::asio::streambuf::const_buffers_type bufs = pStreamBuf->data();
		std::string strData(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs) + cubBytes);

		pBuffer = g_pBufferMgr->Create();
		pBuffer->Write(strData.c_str(), strData.size());
		pBuffer->Seek(0, SOCKBUFFER_SEEK_SET);
	}

	delete pStreamBuf;

	if( g_pSockMgr->ValidHandle(this) )
		g_pSockMgr->StoreCallback( boost::bind(&CGLSockTCP::CallbackRead, this, Callback, this, pBuffer, TranslateErrorMessage(ec), _1) );
}

void CGLSockTCP::OnDestroy( void )
{
	if( g_pSockMgr->RemoveSock(this) )
		delete this;
}

}

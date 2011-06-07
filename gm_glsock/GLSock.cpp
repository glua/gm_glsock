#include "GLSock.h"
#include "GLSockBuffer.h"

#include "SockMgr.h"
#include "BufferMgr.h"

namespace GLSock {

CEndpoint::CEndpoint( unsigned int nIP, unsigned short usPort )
{
	m_TCP = boost::asio::ip::tcp::endpoint( boost::asio::ip::address_v4(nIP), usPort );
	m_UDP = boost::asio::ip::udp::endpoint( boost::asio::ip::address_v4(nIP), usPort );
}

CEndpoint::CEndpoint( boost::asio::ip::tcp::endpoint& Endpoint )
{
	m_TCP = Endpoint;
	m_UDP = boost::asio::ip::udp::endpoint( Endpoint.address(), Endpoint.port() );
}

CEndpoint::CEndpoint( boost::asio::ip::udp::endpoint& Endpoint )
{
	m_TCP = boost::asio::ip::tcp::endpoint( Endpoint.address(), Endpoint.port() );
	m_UDP = Endpoint;
}

unsigned int CEndpoint::IP( void )
{
	return m_TCP.address().to_v4().to_ulong();
}

unsigned short CEndpoint::Port( void )
{
	return m_TCP.port();
}

boost::asio::ip::tcp::endpoint& CEndpoint::ToTCP()
{
	return m_TCP;
}

boost::asio::ip::udp::endpoint& CEndpoint::ToUDP()
{
	return m_UDP;
}

// Acceptor Sock
CSockAcceptor::CSockAcceptor( IOService_t& IOService_t, lua_State* pLua ) :
	m_Sock(IOService_t)
{
	L = pLua;
	m_nReferences = 0;

	m_Sock.open(boost::asio::ip::tcp::v4());
	m_Sock.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
}

void cbBind(Callback_t Callback, ISock* pHandle, int iErrorMsg, lua_State* L) // Callback(Handle, Error)
{
	pHandle->Reference();

	CAutoUnRef MetaTable = Lua()->GetMetaTable(GLSOCK_NAME, GLSOCK_TYPE);

	Lua()->PushReference(Callback);
	Lua()->PushUserData(MetaTable, static_cast<void*>(pHandle));
	Lua()->Push((float)iErrorMsg);

	Lua()->Call(2, 0);
}

bool CSockAcceptor::Bind( CEndpoint& Endpoint, Callback_t Callback )
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

		g_pSockMgr->StoreCallback( boost::bind(cbBind, Callback, this, TranslateErrorMessage(ec), _1) );
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(Acceptor): %s\n",  boost::diagnostic_information(ex).c_str());
#endif
		bResult = false;
	}

	return bResult;
}

void cbListen(Callback_t Callback, ISock* pHandle, int iErrorMsg, lua_State* L) // Callback(Handle, Error)
{
	pHandle->Reference();

	CAutoUnRef MetaTable = Lua()->GetMetaTable(GLSOCK_NAME, GLSOCK_TYPE);

	Lua()->PushReference(Callback);
	Lua()->PushUserData(MetaTable, static_cast<void*>(pHandle));
	Lua()->Push((float)iErrorMsg);

	Lua()->Call(2, 0);
}

bool CSockAcceptor::Listen( int iBacklog, Callback_t Callback )
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

		g_pSockMgr->StoreCallback( boost::bind(cbListen, Callback, this, TranslateErrorMessage(ec), _1) );
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(Acceptor): %s\n",  boost::diagnostic_information(ex).c_str());
#endif
		bResult = false;
	}

	return bResult;
}

bool CSockAcceptor::Accept( Callback_t Callback )
{
	CSockTCP* pSock = (CSockTCP*)g_pSockMgr->CreateTCPSock(L, false);
	
	m_Sock.async_accept(pSock->Socket(), 
		boost::bind(&CSockAcceptor::OnAccept, this, Callback, pSock, boost::asio::placeholders::error));

	return true;
}

bool CSockAcceptor::Resolve( const char* cszHostname, Callback_t Callback )
{
	// TODO: Implement me
	return true;
}

bool CSockAcceptor::Close( void )
{
	bool bResult = true;

	try
	{
		m_Sock.cancel();
		m_Sock.close();
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(Acceptor): %s\n",  boost::diagnostic_information(ex).c_str());
#endif
		bResult = false;
	}

	return bResult;
}

void CSockAcceptor::Reference( void )
{
	m_nReferences++;
}

void CSockAcceptor::Unreference( void )
{
	m_nReferences--;
	if( m_nReferences <= 0 )
	{
		g_pSockMgr->RemoveSock(this);
		Destroy();
	}
}

void cbAccept(Callback_t Callback, ISock* pHandle, ISock* pSock, int iErrorMsg, lua_State* L) // Callback(Handle, Sock, Error)
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
}

void CSockAcceptor::OnAccept( Callback_t Callback, CSockTCP* pSock, const boost::system::error_code& ec )
{
	if( ec )
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(Acceptor): %s\n", ec.message().c_str());
#endif
		g_pSockMgr->RemoveSock(pSock);
		delete pSock;
		pSock = NULL;
	}

	g_pSockMgr->StoreCallback( boost::bind(cbAccept, Callback, this, pSock, TranslateErrorMessage(ec), _1) );
}

void CSockAcceptor::OnDestroy( void )
{
	delete this;
}

// TCP Sock
CSockTCP::CSockTCP( IOService_t& IOService, lua_State* pLua, bool bOpen ) :
	m_Sock(IOService), m_Resolver(IOService)
{
	L = pLua;
	m_nReferences = 0;

	if( bOpen )
		m_Sock.open(boost::asio::ip::tcp::v4());
}

bool CSockTCP::Bind( CEndpoint& Endpoint, Callback_t Callback )
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

		g_pSockMgr->StoreCallback( boost::bind(cbBind, Callback, this, TranslateErrorMessage(ec), _1) );
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

bool CSockTCP::Connect( std::string strHost, std::string strPort, Callback_t Callback )
{
	bool bResult = true;

	try
	{
		TCPResolver_t::query query(strHost, strPort);
		TCPResolver_t::iterator endpoint_iterator = m_Resolver.resolve(query);

		boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

		m_Sock.async_connect( endpoint,
			boost::bind(&CSockTCP::OnConnect, this, Callback, boost::asio::placeholders::error, ++endpoint_iterator ));
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

bool CSockTCP::Send( const char* cbData, unsigned int cubBuffer, Callback_t Callback )
{
	m_Sock.async_send( boost::asio::buffer(cbData, cubBuffer),
		boost::bind(&CSockTCP::OnSend, this, Callback, boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error) );

	return true;
}

bool CSockTCP::Read( unsigned int cubBuffer, Callback_t Callback )
{
	char* pData = new char[cubBuffer];

	m_Sock.async_receive(boost::asio::buffer(pData, cubBuffer),
		boost::bind(&CSockTCP::OnRead, this, Callback, pData, boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error) );

	return true;
}

bool CSockTCP::Resolve( const char* cszHostname, Callback_t Callback )
{
	// TODO: Implement me.
	return true;
}

bool CSockTCP::Close( void )
{
	bool bResult = true;

	try
	{
		m_Sock.cancel();
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

void CSockTCP::Reference( void )
{
	m_nReferences++;
}

void CSockTCP::Unreference( void )
{
	m_nReferences--;
	if( m_nReferences <= 0 )
	{
		g_pSockMgr->RemoveSock(this);
		Destroy();
	}
}

void CSockTCP::OnResolve( Callback_t Callback, const boost::system::error_code& err, TCPResolver_t::iterator endpoint_iterator )
{
	// TODO: Implement me.
}

void cbConnect( Callback_t Callback, ISock* pHandle, int iErrorMsg, lua_State* L )
{
	pHandle->Reference();

	CAutoUnRef MetaTable = Lua()->GetMetaTable(GLSOCK_NAME, GLSOCK_TYPE);

	Lua()->PushReference(Callback);
	Lua()->PushUserData(MetaTable, static_cast<void*>(pHandle));
	Lua()->Push((float)iErrorMsg);

	Lua()->Call(2, 0);
}

void CSockTCP::OnConnect( Callback_t Callback, const boost::system::error_code& ec, TCPResolver_t::iterator endpoint_iterator )
{
	if( !ec )
	{
		Lua()->Msg("GLSock(TCP): Connected to Host!\n");

		g_pSockMgr->StoreCallback( boost::bind(cbConnect, Callback, this, TranslateErrorMessage(ec), _1) );
	}
	else
	{
		if( endpoint_iterator != boost::asio::ip::tcp::resolver::iterator() )
		{
			SCOPED_LOCK(g_pSockMgr->Mutex());
			Lua()->Msg("GLSock(TCP): Connect attempt failed, trying next resolver.\n");

			boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

			//m_Sock.close();
			m_Sock.async_connect( endpoint,
				boost::bind(&CSockTCP::OnConnect, this, Callback, boost::asio::placeholders::error, endpoint_iterator++) );
		}
		else
		{
#if defined(_DEBUG)
			Lua()->Msg("GLSock(TCP) Connection failed: %s\n", ec.message().c_str());
#endif

			g_pSockMgr->StoreCallback( boost::bind(cbConnect, Callback, this, TranslateErrorMessage(ec), _1) );
		}
	}
}

void cbSend( Callback_t Callback, ISock* pHandle, unsigned int cubBytes, int iErrorMsg, lua_State* L )
{
	pHandle->Reference();

	CAutoUnRef MetaTable = Lua()->GetMetaTable(GLSOCK_NAME, GLSOCK_TYPE);

	Lua()->PushReference(Callback);
	Lua()->PushUserData(MetaTable, static_cast<void*>(pHandle));
	Lua()->PushLong(cubBytes);
	Lua()->Push((float)iErrorMsg);

	Lua()->Call(3, 0);
}

void CSockTCP::OnSend( Callback_t Callback, unsigned int cubBytes, const boost::system::error_code& ec )
{
	if( ec )
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(TCP): %s\n", ec.message().c_str());
#endif
	}

	g_pSockMgr->StoreCallback( boost::bind(cbSend, Callback, this, cubBytes, TranslateErrorMessage(ec), _1) );
}

void cbTCPRead( Callback_t Callback, ISock* pHandle, GLSockBuffer::CGLSockBuffer* pBuffer, int iErrorMsg, lua_State* L)
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
}

void CSockTCP::OnRead( Callback_t Callback, const char* pData, unsigned int cubBytes, const boost::system::error_code& ec )
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

	g_pSockMgr->StoreCallback( boost::bind(cbTCPRead, Callback, this, pBuffer, TranslateErrorMessage(ec), _1) );
}

void CSockTCP::OnDestroy( void )
{
	delete this;
}


// UDP Sock
CSockUDP::CSockUDP( IOService_t& IOService, lua_State* pLua ) :
	m_Sock(IOService), m_Resolver(IOService)
{
	L = pLua;
	m_nReferences = 0;

	m_Sock.open(boost::asio::ip::udp::v4());
}

bool CSockUDP::Bind( CEndpoint& Endpoint, Callback_t Callback )
{
	bool bResult = true;

	try
	{
		boost::system::error_code ec;
		boost::asio::ip::udp::endpoint ep = Endpoint.ToUDP();

		m_Sock.bind( ep, ec );

		if( ec )
		{
#if defined(_DEBUG)
			Lua()->Msg("GLSock(UDP): Unable to bind to %s:%u\n", ep.address().to_string().c_str(), ep.port());
#endif
		}
		else
		{
#if defined(_DEBUG)
			Lua()->Msg("GLSock(UDP): Bound to %s:%u\n", ep.address().to_string().c_str(), ep.port());
#endif
		}

		g_pSockMgr->StoreCallback( boost::bind(cbBind, Callback, this, TranslateErrorMessage(ec), _1) );
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(UDP): %s\n",  boost::diagnostic_information(ex).c_str());
#endif
		bResult = false;
	}

	return bResult;
}

bool CSockUDP::SendTo( const char* cbData, unsigned int cubBuffer, std::string strHost, std::string strPort, Callback_t Callback )
{
	UDPResolver_t::query query(strHost, strPort);
	UDPResolver_t::iterator iterator = m_Resolver.resolve(query);

	char* pBuffer = new char[cubBuffer];

	boost::asio::ip::udp::endpoint ep = *iterator;

#if defined(_DEBUG)
	Lua()->Msg("GLSock(UDP): Send attempt to %s:%u\n", ep.address().to_string().c_str(), ep.port());
#endif

	m_Sock.async_send_to( boost::asio::buffer(cbData, cubBuffer),
		ep,
		boost::bind(&CSockUDP::OnSend, 
			this, 
			Callback, 
			boost::asio::placeholders::bytes_transferred, 
			boost::asio::placeholders::error, 
			++iterator,
			pBuffer,
			cubBuffer
			) 
		);

	return true;
}

bool CSockUDP::ReadFrom(unsigned int cubBuffer, Callback_t Callback )
{
	char* pData = new char[cubBuffer];

	boost::asio::ip::udp::endpoint* pEndpoint = new boost::asio::ip::udp::endpoint;

	m_Sock.async_receive_from(boost::asio::buffer(pData, cubBuffer),
		*pEndpoint,
		boost::bind(&CSockUDP::OnRead, this, Callback, pEndpoint, pData, boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error) );

	return true;
}

bool CSockUDP::Resolve( const char* cszHostname, Callback_t Callback )
{
	// TODO: Implement me.
	return true;
}

bool CSockUDP::Close( void )
{
	bool bResult = true;

	try
	{
		m_Sock.cancel();
		m_Sock.close();
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(UDP): %s\n",  boost::diagnostic_information(ex).c_str());
#endif
		bResult = false;
	}

	return bResult;
}

void CSockUDP::Reference( void )
{
	m_nReferences++;
}

void CSockUDP::Unreference( void )
{
	m_nReferences--;
	if( m_nReferences <= 0 )
	{
		g_pSockMgr->RemoveSock(this);
		Destroy();
	}
}

void CSockUDP::OnSend( Callback_t Callback, unsigned int cubBytes, const boost::system::error_code& ec, UDPResolver_t::iterator iterator, const char* pBuffer, unsigned int cubBuffer )
{
	if( ec )
	{
		if( iterator != boost::asio::ip::udp::resolver::iterator() )
		{
			boost::asio::ip::udp::endpoint ep = *iterator;

#if defined(_DEBUG)
			Lua()->Msg("GLSock(UDP): Send attempt to %s:%u\n", ep.address().to_string().c_str(), ep.port());
#endif

			m_Sock.async_send_to( boost::asio::buffer(pBuffer, cubBuffer),
				ep,
				boost::bind(&CSockUDP::OnSend, 
					this, 
					Callback, 
					boost::asio::placeholders::bytes_transferred, 
					boost::asio::placeholders::error, 
					++iterator,
					pBuffer,
					cubBuffer
					) 
				);

			return;
		}
	}

	delete[] pBuffer;

	g_pSockMgr->StoreCallback( boost::bind(cbSend, Callback, this, cubBytes, TranslateErrorMessage(ec), _1) );
}

void cbUDPRead( Callback_t Callback, 
	ISock* pHandle, 
	std::string strSender, 
	unsigned short usPort, 
	GLSockBuffer::CGLSockBuffer* pBuffer,
	int iErrorMsg,
	lua_State* L)
{
	pHandle->Reference();

	CAutoUnRef MetaTable = Lua()->GetMetaTable(GLSOCK_NAME, GLSOCK_TYPE);

	Lua()->PushReference(Callback);
	Lua()->PushUserData(MetaTable, static_cast<void*>(pHandle));

	if( !pBuffer )
	{
		Lua()->PushNil();
		Lua()->PushNil();
		Lua()->PushNil();
	}
	else
	{
		Lua()->Push(strSender.c_str());
		Lua()->Push((float)usPort);
		{
			CAutoUnRef MetaTableBuffer = Lua()->GetMetaTable(GLSOCKBUFFER_NAME, GLSOCKBUFFER_TYPE);

			pBuffer->Reference();
			Lua()->PushUserData(MetaTableBuffer, static_cast<void*>(pBuffer));
		}
	}

	Lua()->Push((float)iErrorMsg);
	Lua()->Call(5, 0);
}

void CSockUDP::OnRead( Callback_t Callback, boost::asio::ip::udp::endpoint* pSender, const char* pData, unsigned int cubBytes, const boost::system::error_code& ec )
{
	GLSockBuffer::CGLSockBuffer* pBuffer = NULL;

	if( ec )
	{
#if defined(_DEBUG)
		Lua()->Msg("GLSock(UDP): %s\n", ec.message().c_str());
#endif
	}
	else
	{
		pBuffer = g_pBufferMgr->Create();
		pBuffer->Write(pData, cubBytes);
		pBuffer->Seek(0, SOCKBUFFER_SEEK_SET);
	}

	g_pSockMgr->StoreCallback( boost::bind(cbUDPRead, Callback, this, pSender->address().to_string(), pSender->port(), pBuffer, TranslateErrorMessage(ec), _1) );

	delete pSender;
}

void CSockUDP::OnDestroy( void )
{
	delete this;
}

int TranslateErrorMessage( const boost::system::error_code& ec )
{
	switch(ec.value())
	{
	case 0:
		return eSockErrorSuccess;
	case boost::asio::error::access_denied:
		return eSockErrorAccessDenied;
	case boost::asio::error::address_family_not_supported:
		return eSockErrorAddressFamilyNotSupported;
	case boost::asio::error::address_in_use:
		return eSockErrorAddressInUse;
	case boost::asio::error::already_connected:
		return eSockErrorAlreadyConnected;
	case boost::asio::error::already_started:
		return eSockErrorAlreadyStarted;
	case boost::asio::error::broken_pipe:
		return eSockErrorBrokenPipe;
	case boost::asio::error::connection_refused:
		return eSockErrorConnectionRefused;
	case boost::asio::error::connection_reset:
		return eSockErrorConnectionReset;
	case boost::asio::error::bad_descriptor:
		return eSockErrorBadDescriptor;
	case boost::asio::error::fault:
		return eSockErrorBadAddress;
	case boost::asio::error::host_unreachable:
		return eSockErrorHostUnreachable;
	case boost::asio::error::in_progress:
		return eSockErrorInProgress;
	case boost::asio::error::interrupted:
		return eSockErrorInterrupted;
	case boost::asio::error::invalid_argument:
		return eSockErrorInvalidArgument;
	case boost::asio::error::message_size:
		return eSockErrorMessageSize;
	case boost::asio::error::name_too_long:
		return eSockErrorNameTooLong;
	case boost::asio::error::network_down:
		return eSockErrorNetworkDown;
	case boost::asio::error::network_reset:
		return eSockErrorNetworkReset;
	case boost::asio::error::network_unreachable:
		return eSockErrorNetworkUnreachable;
	case boost::asio::error::no_descriptors:
		return eSockErrorNoDescriptors;
	case boost::asio::error::no_buffer_space:
		return eSockErrorNoBufferSpace;
	case boost::asio::error::no_memory:
		return eSockErrorNoMemory;
	case boost::asio::error::no_permission:
		return eSockErrorNoPermission;
	case boost::asio::error::no_protocol_option:
		return eSockErrorNoProtocolOption;
	case boost::asio::error::not_connected:
		return eSockErrorNotConnected;
	case boost::asio::error::not_socket:
		return eSockErrorNotSocket;
	case boost::asio::error::operation_aborted:
		return eSockErrorOperationAborted;
	case boost::asio::error::operation_not_supported:
		return eSockErrorOperationNotSupported;
	case boost::asio::error::shut_down:
		return eSockErrorShutDown;
	case boost::asio::error::timed_out:
		return eSockErrorTimedOut;
	case boost::asio::error::try_again:
		return eSockErrorTryAgain;
	case boost::asio::error::would_block:
		return eSockErrorWouldBlock;
	}

	return ec.value();
}

} // GLSock
#include "GLSockUDP.h"
#include "SockMgr.h"
#include "BufferMgr.h"

namespace GLSock {

CGLSockUDP::CGLSockUDP( IOService_t& IOService, lua_State* pLua ) 
	: m_Sock(IOService), m_Resolver(IOService)
{
	L = pLua;
	m_nReferences = 0;

	m_Sock.open(boost::asio::ip::udp::v4());
}

void CGLSockUDP::CallbackBind(Callback_t Callback, CGLSock* pHandle, int iErrorMsg, lua_State* L)
{
	pHandle->Reference();

	CAutoUnRef MetaTable = Lua()->GetMetaTable(GLSOCK_NAME, GLSOCK_TYPE);

	Lua()->PushReference(Callback);
	Lua()->PushUserData(MetaTable, static_cast<void*>(pHandle));
	Lua()->Push((float)iErrorMsg);

	Lua()->Call(2, 0);
}

bool CGLSockUDP::Bind( CEndpoint& Endpoint, Callback_t Callback )
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

		g_pSockMgr->StoreCallback( boost::bind(&CGLSockUDP::CallbackBind, this, Callback, this, TranslateErrorMessage(ec), _1) );
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

bool CGLSockUDP::SendTo( const char* cbData, unsigned int cubBuffer, std::string strHost, std::string strPort, Callback_t Callback )
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
		boost::bind(&CGLSockUDP::OnSend, 
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

bool CGLSockUDP::ReadFrom(unsigned int cubBuffer, Callback_t Callback )
{
	char* pData = new char[cubBuffer];

	boost::asio::ip::udp::endpoint* pEndpoint = new boost::asio::ip::udp::endpoint;

	m_Sock.async_receive_from(boost::asio::buffer(pData, cubBuffer),
		*pEndpoint,
		boost::bind(&CGLSockUDP::OnRead, this, Callback, pEndpoint, pData, boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error) );

	return true;
}

bool CGLSockUDP::Resolve( const char* cszHostname, Callback_t Callback )
{
	// TODO: Implement me.
	return true;
}

bool CGLSockUDP::Close( void )
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

void CGLSockUDP::Reference( void )
{
	m_nReferences++;
}

void CGLSockUDP::Unreference( void )
{
	m_nReferences--;
	if( m_nReferences <= 0 )
	{
		g_pSockMgr->RemoveSock(this);
		Destroy();
	}
}

void CGLSockUDP::CallbackSend( Callback_t Callback, CGLSock* pHandle, unsigned int cubBytes, int iErrorMsg, lua_State* L )
{
	pHandle->Reference();

	CAutoUnRef MetaTable = Lua()->GetMetaTable(GLSOCK_NAME, GLSOCK_TYPE);

	Lua()->PushReference(Callback);
	Lua()->PushUserData(MetaTable, static_cast<void*>(pHandle));
	Lua()->PushLong(cubBytes);
	Lua()->Push((float)iErrorMsg);

	Lua()->Call(3, 0);
}

void CGLSockUDP::OnSend( Callback_t Callback, unsigned int cubBytes, const boost::system::error_code& ec, UDPResolver_t::iterator iterator, const char* pBuffer, unsigned int cubBuffer )
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
				boost::bind(&CGLSockUDP::OnSend, 
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

	g_pSockMgr->StoreCallback( boost::bind(&CGLSockUDP::CallbackSend, this, Callback, this, cubBytes, TranslateErrorMessage(ec), _1) );
}

void CGLSockUDP::CallbackRead( Callback_t Callback, 
	CGLSock* pHandle, 
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

void CGLSockUDP::OnRead( Callback_t Callback, boost::asio::ip::udp::endpoint* pSender, const char* pData, unsigned int cubBytes, const boost::system::error_code& ec )
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

	delete[] pData;

	g_pSockMgr->StoreCallback( boost::bind(&CGLSockUDP::CallbackRead, this, Callback, this, pSender->address().to_string(), pSender->port(), pBuffer, TranslateErrorMessage(ec), _1) );

	delete pSender;
}

void CGLSockUDP::OnDestroy( void )
{
	delete this;
}

}

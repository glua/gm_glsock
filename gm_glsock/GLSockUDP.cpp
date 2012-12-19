#include "Common.h"
#include "GLSockUDP.h"
#include "SockMgr.h"
#include "BufferMgr.h"
#include "BindingGLSock.h"

namespace GLSock {

CGLSockUDP::CGLSockUDP( IOService_t& IOService, lua_State* pLua ) 
	: m_Sock(IOService), m_Resolver(IOService)
{
	state = pLua;
	m_nReferences = 0;

	try
	{
		m_Sock.open(boost::asio::ip::udp::v4());
	}
	catch (boost::exception& ex)
	{
		//LUA->Msg("GLSock(UDP): %s\n",  boost::diagnostic_information(ex).c_str());
		UNREFERENCED_PARAM(ex);
	}
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
			//LUA->Msg("GLSock(UDP): Unable to bind to %s:%u\n", ep.address().to_string().c_str(), ep.port());
#endif
		}
		else
		{
#if defined(_DEBUG)
			//LUA->Msg("GLSock(UDP): Bound to %s:%u\n", ep.address().to_string().c_str(), ep.port());
#endif
		}
		if( g_pSockMgr->ValidHandle(this) )
			g_pSockMgr->StoreCallback( this, boost::bind(&CGLSock::CallbackBind, this, Callback, this, TranslateErrorMessage(ec), _1) );
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		//LUA->Msg("GLSock(UDP): %s\n",  boost::diagnostic_information(ex).c_str());
#endif
		UNREFERENCED_PARAM(ex);
		bResult = false;
	}

	return bResult;
}

bool CGLSockUDP::SendTo( const char* cbData, unsigned int cubBuffer, std::string strHost, std::string strPort, Callback_t Callback )
{
	bool bResult = true;

	try
	{
		UDPResolver_t::query query(strHost, strPort);
		UDPResolver_t::iterator iterator = m_Resolver.resolve(query);

		char* pBuffer = new char[cubBuffer];
		memcpy(pBuffer, cbData, cubBuffer);

		boost::asio::ip::udp::endpoint ep = *iterator;

#if defined(_DEBUG)
		//LUA->Msg("GLSock(UDP): Send attempt to %s:%u\n", ep.address().to_string().c_str(), ep.port());
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
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		//LUA->Msg("GLSock(UDP): %s\n",  boost::diagnostic_information(ex).c_str());
#endif
		UNREFERENCED_PARAM(ex);
		bResult = false;
	}

	return bResult;
}

bool CGLSockUDP::ReadFrom(unsigned int cubBuffer, Callback_t Callback )
{
	bool bResult = true;
	try
	{
		char* pData = new char[cubBuffer];

		boost::asio::ip::udp::endpoint* pEndpoint = new boost::asio::ip::udp::endpoint;

		m_Sock.async_receive_from(boost::asio::buffer(pData, cubBuffer),
			*pEndpoint,
			boost::bind(&CGLSockUDP::OnRead, this, Callback, pEndpoint, pData, boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error) );
	}
	catch (boost::exception& ex)
	{
#if defined(_DEBUG)
		//LUA->Msg("GLSock(UDP): %s\n",  boost::diagnostic_information(ex).c_str());
#endif
		UNREFERENCED_PARAM(ex);
		bResult = false;
	}

	return bResult;
}

bool CGLSockUDP::Resolve( const char* cszHostname, Callback_t Callback )
{
	// TODO: Implement me.
	return true;
}

bool CGLSockUDP::Close( void )
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
		//LUA->Msg("GLSock(UDP): %s\n",  boost::diagnostic_information(ex).c_str());
#endif
		UNREFERENCED_PARAM(ex);
		bResult = false;
	}

	return bResult;
}

int CGLSockUDP::Reference( void )
{
	m_nReferences++;
	return m_nReferences;
}

int CGLSockUDP::Unreference( void )
{
	m_nReferences--;
	if( m_nReferences <= 0 )
	{
		//g_pSockMgr->RemoveSock(this);
		Destroy();
	}
	return m_nReferences;
}

void CGLSockUDP::OnSend( Callback_t Callback, unsigned int cubBytes, const boost::system::error_code& ec, UDPResolver_t::iterator iterator, const char* pBuffer, unsigned int cubBuffer )
{
	if( ec )
	{
		if( iterator != boost::asio::ip::udp::resolver::iterator() )
		{
			boost::asio::ip::udp::endpoint ep = *iterator;

#if defined(_DEBUG)
			//LUA->Msg("GLSock(UDP): Send attempt to %s:%u\n", ep.address().to_string().c_str(), ep.port());
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

	if( g_pSockMgr->ValidHandle(this) )
		g_pSockMgr->StoreCallback( this, boost::bind(&CGLSock::CallbackSend, this, Callback, this, cubBytes, TranslateErrorMessage(ec), _1) );
}

void CGLSockUDP::OnRead( Callback_t Callback, boost::asio::ip::udp::endpoint* pSender, const char* pData, unsigned int cubBytes, const boost::system::error_code& ec )
{
	GLSockBuffer::CGLSockBuffer* pBuffer = NULL;

	if( ec )
	{
#if defined(_DEBUG)
		//LUA->Msg("GLSock(UDP): %s\n", ec.message().c_str());
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
		g_pSockMgr->StoreCallback( this, boost::bind(&CGLSock::CallbackReadFrom, this, Callback, this, pSender->address().to_string(), pSender->port(), pBuffer, TranslateErrorMessage(ec), _1) );

	delete pSender;
}

void CGLSockUDP::OnDestroy( void )
{
	if( g_pSockMgr->RemoveSock(this) )
		delete this;
}

}

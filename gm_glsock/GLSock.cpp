#include "GLSock.h"

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

bool CGLSock::Bind( CEndpoint& Endpoint, Callback_t Callback )
{
	return false;
}

bool CGLSock::Listen( int iBacklog, Callback_t Callback )
{
	return false;
}

bool CGLSock::Accept( Callback_t Callback )
{
	return false;
}

bool CGLSock::Connect( std::string strHost, std::string strPort, Callback_t Callback )
{
	return false;
}

bool CGLSock::Send( const char* cbData, unsigned int cubBuffer, Callback_t Callback )
{
	return false;
}

bool CGLSock::SendTo( const char* cbData, unsigned int cubBuffer, std::string strHost, std::string strPort, Callback_t Callback )
{
	return false;
}

bool CGLSock::Read( unsigned int cubBuffer, Callback_t Callback )
{
	return false;
}

bool CGLSock::ReadUntil( const char* pszDelimiter, Callback_t Callback )
{
	return false;
}

bool CGLSock::ReadFrom( unsigned int cubBuffer, Callback_t Callback )
{
	return false;
}

bool CGLSock::Resolve( const char* cszHostname, Callback_t Callback )
{
	return false;
}

bool CGLSock::Close( void )
{
	return false;
}

bool CGLSock::Cancel( void )
{
	return false;
}

int CGLSock::Type( void )
{
	return eSockTypeInvalid;
}

int CGLSock::TranslateErrorMessage( const boost::system::error_code& ec )
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
	case boost::asio::error::would_block:
		return eSockErrorWouldBlock;
	}

	return ec.value();
}


} // GLSock

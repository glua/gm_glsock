#if !defined(SOCKINTERFACE_H)
#define SOCKINTERFACE_H

#if defined(WIN32)
#pragma once
#endif

#define NO_SDK

#include "GMLuaModule.h"
#include "Lock.h"

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <sstream>

namespace GLSock {

#define GLSOCK_NAME							"GLSock"
#define GLSOCK_TYPE							59313

// Socket Types
enum ESockType
{
	eSockTypeInvalid = -1,
	eSockTypeAcceptor = 0,
	eSockTypeTCP,
	eSockTypeUDP,
};

// Socket Errors
enum ESockError
{
	eSockErrorSuccess,
	eSockErrorAccessDenied,
	eSockErrorAddressFamilyNotSupported,
	eSockErrorAddressInUse,
	eSockErrorAlreadyConnected,
	eSockErrorAlreadyStarted,
	eSockErrorBrokenPipe,
	eSockErrorConnectionAborted,
	eSockErrorConnectionRefused,
	eSockErrorConnectionReset,
	eSockErrorBadDescriptor,
	eSockErrorBadAddress,
	eSockErrorHostUnreachable,
	eSockErrorInProgress,
	eSockErrorInterrupted,
	eSockErrorInvalidArgument,
	eSockErrorMessageSize,
	eSockErrorNameTooLong,
	eSockErrorNetworkDown,
	eSockErrorNetworkReset,
	eSockErrorNetworkUnreachable,
	eSockErrorNoDescriptors,
	eSockErrorNoBufferSpace,
	eSockErrorNoMemory,
	eSockErrorNoPermission,
	eSockErrorNoProtocolOption,
	eSockErrorNotConnected,
	eSockErrorNotSocket,
	eSockErrorOperationAborted,
	eSockErrorOperationNotSupported,
	eSockErrorShutDown,
	eSockErrorTimedOut,
	eSockErrorTryAgain,
	eSockErrorWouldBlock,
};

// Typedefs
typedef boost::asio::ip::tcp::socket TCPSock_t;
typedef boost::asio::ip::udp::socket UDPSock_t;
typedef boost::asio::ip::tcp::acceptor AcceptorSock_t;

typedef boost::asio::ip::tcp::resolver TCPResolver_t;
typedef boost::asio::ip::udp::resolver UDPResolver_t;

typedef boost::asio::io_service IOService_t;
typedef int Callback_t;

#if defined(WIN32)
#pragma warning( disable: 4996 )
#endif

// Endpoint
class CEndpoint
{
private:
	boost::asio::ip::tcp::endpoint m_TCP;
	boost::asio::ip::udp::endpoint m_UDP;
public:
	CEndpoint(unsigned int nIP, unsigned short usPort);
	CEndpoint(boost::asio::ip::tcp::endpoint& Endpoint);
	CEndpoint(boost::asio::ip::udp::endpoint& Endpoint);

	unsigned int IP(void);
	unsigned short Port(void);

	boost::asio::ip::tcp::endpoint& ToTCP();
	boost::asio::ip::udp::endpoint& ToUDP();
};

// Interfaces
class CGLSock
{
public:
	int m_nTableRef;

	virtual bool Bind(CEndpoint& Endpoint, Callback_t Callback);
	virtual bool Listen(int iBacklog, Callback_t Callback);
	virtual bool Accept(Callback_t Callback);
	virtual bool Connect(std::string strHost, std::string strPort, Callback_t Callback);
	virtual bool Send(const char* cbData, unsigned int cubBuffer, Callback_t Callback);
	virtual bool SendTo(const char* cbData, unsigned int cubBuffer, std::string strHost, std::string strPort, Callback_t Callback);
	virtual bool Read(unsigned int cubBuffer, Callback_t Callback);
	virtual bool ReadUntil(std::string strDelimiter, Callback_t Callback);
	virtual bool ReadFrom(unsigned int cubBuffer, Callback_t Callback);
	virtual bool Resolve(const char* cszHostname, Callback_t Callback);
	virtual bool Close(void);
	virtual bool Cancel(void);
	virtual int Type(void);
	virtual std::string RemoteAddress(void);
	virtual unsigned short RemotePort(void);

	virtual int Reference(void) = 0;
	virtual int Unreference(void) = 0;
	virtual void Destroy(void) = 0;

	virtual int TranslateErrorMessage(const boost::system::error_code& ec);
};

} // GLSock

#endif // SOCKINTERFACE_H

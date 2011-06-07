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
#define GLSOCK_TYPE							'GLSK'

// Socket Types
enum ESockType
{
	eSockTypeAcceptor,
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
class ISock
{
public:
	virtual bool Bind(CEndpoint& Endpoint, Callback_t Callback) = 0;
	virtual bool Listen(int iBacklog, Callback_t Callback) = 0;
	virtual bool Accept(Callback_t Callback) = 0;
	virtual bool Connect(std::string strHost, std::string strPort, Callback_t Callback) = 0;
	virtual bool Send(const char* cbData, unsigned int cubBuffer, Callback_t Callback) = 0;
	virtual bool SendTo(const char* cbData, unsigned int cubBuffer, std::string strHost, std::string strPort, Callback_t Callback) = 0;
	virtual bool Read(unsigned int cubBuffer, Callback_t Callback) = 0;
	virtual bool ReadFrom(unsigned int cubBuffer, Callback_t Callback) = 0;
	virtual bool Resolve(const char* cszHostname, Callback_t Callback) = 0;
	virtual bool Close(void) = 0;
	virtual bool Cancel(void) = 0;
	virtual int Type(void) = 0;

	virtual void Reference(void) = 0;
	virtual void Unreference(void) = 0;
	virtual void Destroy(void) = 0;
};

// Implementation
class CSockAcceptor : public ISock
{
private:
	boost::mutex m_Mutex;
	int m_nReferences;
	AcceptorSock_t m_Sock;
	lua_State* L;

public:
	CSockAcceptor(IOService_t& IOService_t, lua_State* pLua);

	virtual bool Bind(CEndpoint& Endpoint, Callback_t Callback);
	virtual bool Listen(int iBacklog, Callback_t Callback);
	virtual bool Accept(Callback_t Callback);
	virtual bool Connect(std::string strHost, std::string strPort, Callback_t Callback)
	{
		return false; // Stub
	}
	virtual bool Send(const char* cbData, unsigned int cubBuffer, Callback_t Callback)
	{
		return false; // Stub
	}
	virtual bool SendTo(const char* cbData, unsigned int cubBuffer, std::string strHost, std::string strPort, Callback_t Callback)
	{
		return false; // Stub
	}
	virtual bool Read(unsigned int cubBuffer, Callback_t Callback)
	{
		return false; // Stub
	}
	virtual bool ReadFrom(unsigned int cubBuffer, Callback_t Callback)
	{
		return false; // Stub
	}
	virtual bool Resolve(const char* cszHostname, Callback_t Callback);
	virtual bool Close(void);
	virtual bool Cancel(void)
	{
		m_Sock.cancel();
		return true;
	}
	virtual int Type(void)
	{
		return eSockTypeAcceptor;
	}
	AcceptorSock_t& Socket(void)
	{
		return m_Sock;
	}
	virtual void Reference(void);
	virtual void Unreference(void);
	virtual void Destroy(void)
	{
		m_Sock.cancel();
		m_Sock.close();
		m_Sock.io_service().dispatch( boost::bind(&CSockAcceptor::OnDestroy, this));
	}

private:
	void OnAccept(Callback_t Callback, class CSockTCP* pSock, const boost::system::error_code& ec);
	void OnDestroy(void);
};

class CSockTCP : public ISock
{
private:
	boost::mutex m_Mutex;
	int m_nReferences;
	TCPSock_t m_Sock;
	TCPResolver_t m_Resolver;
	lua_State* L;

public:
	CSockTCP(IOService_t& IOService, lua_State* pLua, bool bOpen = true);

	virtual bool Bind(CEndpoint& Endpoint, Callback_t Callback);
	virtual bool Listen(int iBacklog, Callback_t Callback)
	{
		return false; // Stub
	}
	virtual bool Accept(Callback_t Callback)
	{
		return false; // Stub
	}
	virtual bool Connect(std::string strHost, std::string strPort, Callback_t Callback);
	virtual bool Send(const char* cbData, unsigned int cubBuffer, Callback_t Callback);
	virtual bool SendTo(const char* cbData, unsigned int cubBuffer, std::string strHost, std::string strPort, Callback_t Callback)
	{
		return false; // Stub
	}
	virtual bool Read(unsigned int cubBuffer, Callback_t Callback);
	virtual bool ReadFrom(unsigned int cubBuffer, Callback_t Callback)
	{
		return false; // Stub
	}
	virtual bool Resolve(const char* cszHostname, Callback_t Callback);
	virtual bool Close(void);
	virtual bool Cancel(void)
	{
		m_Sock.cancel();
		return true;
	}
	virtual int Type(void)
	{
		return eSockTypeTCP;
	}
	TCPSock_t& Socket(void)
	{
		return m_Sock;
	}
	virtual void Reference(void);
	virtual void Unreference(void);
	virtual void Destroy(void)
	{
		m_Sock.cancel();
		m_Sock.close();
		m_Sock.io_service().dispatch( boost::bind(&CSockTCP::OnDestroy, this));
	}

private:
	void OnResolve(Callback_t Callback, const boost::system::error_code& ec, TCPResolver_t::iterator endpoint_iterator);
	void OnConnect(Callback_t Callback, const boost::system::error_code& ec, TCPResolver_t::iterator endpoint_iterator);
	void OnSend(Callback_t Callback, unsigned int cubBytes, const boost::system::error_code& ec);
	void OnRead(Callback_t Callback, const char* pData, unsigned int cubBytes, const boost::system::error_code& ec);
	void OnDestroy(void);
};

class CSockUDP : public ISock
{
private:
	boost::mutex m_Mutex;
	int m_nReferences;
	UDPSock_t m_Sock;
	UDPResolver_t m_Resolver;
	lua_State* L;

public:
	CSockUDP(IOService_t& IOService, lua_State* pLua);

	virtual bool Bind(CEndpoint& Endpoint, Callback_t Callback);
	virtual bool Listen(int iBacklog, Callback_t Callback)
	{
		return false; // Stub
	}
	virtual bool Accept(Callback_t Callback)
	{
		return false; // Stub
	}
	virtual bool Connect(std::string strHost, std::string strPort, Callback_t Callback)
	{
		return false; // Stub
	}
	virtual bool Send(const char* cbData, unsigned int cubBuffer, Callback_t Callback)
	{
		return false; // Stub
	}
	virtual bool SendTo(const char* cbData, unsigned int cubBuffer, std::string strHost, std::string strPort, Callback_t Callback);
	virtual bool Read(unsigned int cubBuffer, Callback_t Callback)
	{
		return false; // Stub
	}
	virtual bool ReadFrom(unsigned int cubBuffer, Callback_t Callback);
	virtual bool Resolve(const char* cszHostname, Callback_t Callback);
	virtual bool Close(void);
	virtual bool Cancel(void)
	{
		m_Sock.cancel();
		return true;
	}
	virtual int Type(void)
	{
		return eSockTypeUDP;
	}
	UDPSock_t& Socket(void)
	{
		return m_Sock;
	}
	virtual void Reference(void);
	virtual void Unreference(void);
	virtual void Destroy(void)
	{
		m_Sock.cancel();
		m_Sock.close();
		m_Sock.io_service().dispatch( boost::bind(&CSockUDP::OnDestroy, this));
	}
	
private:
	void OnSend(Callback_t Callback, unsigned int cubBytes, const boost::system::error_code& ec, UDPResolver_t::iterator iterator, const char* pBuffer, unsigned int cubBuffer);
	void OnRead(Callback_t Callback, boost::asio::ip::udp::endpoint* pSender, const char* pData, unsigned int cubBytes, const boost::system::error_code& ec);
	void OnDestroy(void);
};

// Internal Functions
int TranslateErrorMessage(const boost::system::error_code& ec);

} // GLSock

#endif // SOCKINTERFACE_H
#if !defined(GLSOCKUDP_H)
#define GLSOCKUDP_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "GLSock.h"
#include "GLSockBuffer.h"

namespace GLSock {
	
class CGLSockUDP : public CGLSock
{
private:
	boost::mutex m_Mutex;
	int m_nReferences;
	UDPSock_t m_Sock;
	UDPResolver_t m_Resolver;
	lua_State *state;

public:
	CGLSockUDP(IOService_t& IOService, lua_State* pLua);

	virtual bool Bind(CEndpoint& Endpoint, Callback_t Callback);
	virtual bool SendTo(const char* cbData, unsigned int cubBuffer, std::string strHost, std::string strPort, Callback_t Callback);
	virtual bool ReadFrom(unsigned int cubBuffer, Callback_t Callback);
	virtual bool Resolve(const char* cszHostname, Callback_t Callback);
	virtual bool Close(void);
	virtual bool Cancel(void)
	{
		boost::system::error_code ec;
		m_Sock.cancel(ec);
		return ec.value() == 0;
	}
	virtual int Type(void)
	{
		return eSockTypeUDP;
	}
	UDPSock_t& Socket(void)
	{
		return m_Sock;
	}
	virtual int Reference(void);
	virtual int Unreference(void);
	virtual void Destroy(void)
	{
		boost::system::error_code ec;
		try
		{
			//m_Sock.cancel(ec);
			m_Sock.shutdown(boost::asio::ip::udp::socket::shutdown_both, ec);
			m_Sock.close(ec);
			m_Sock.get_io_service().post(boost::bind(&CGLSockUDP::OnDestroy, this));
		}
		catch (boost::exception& ex)
		{
#if defined(_DEBUG)
			//LUA->Msg("GLSock(UDP): %s\n",  boost::diagnostic_information(ex).c_str());
#endif
			UNREFERENCED_PARAM(ex);
		}
	}
	virtual std::string RemoteAddress(void)
	{
		try
		{
			return m_Sock.remote_endpoint().address().to_string();
		}
		catch (boost::exception&)
		{
		}
		return "0.0.0.0";
	}
	virtual unsigned short RemotePort(void)
	{
		try
		{
			return m_Sock.remote_endpoint().port();
		}
		catch (boost::exception&)
		{
		}
		return 0;
	}

private:
	void OnSend(Callback_t Callback, unsigned int cubBytes, const boost::system::error_code& ec, UDPResolver_t::iterator iterator, const char* pBuffer, unsigned int cubBuffer);
	void OnRead(Callback_t Callback, boost::asio::ip::udp::endpoint* pSender, const char* pData, unsigned int cubBytes, const boost::system::error_code& ec);
	void OnDestroy(void);
};

}

#endif // GLSOCKUDP_H

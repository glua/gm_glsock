#if !defined(GLSOCKTCP_H)
#define GLSOCKTCP_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "GLSock.h"
#include "GLSockBuffer.h"

namespace GLSock {

class CGLSockTCP : public CGLSock
{
private:
	boost::mutex m_Mutex;
	int m_nReferences;
	TCPSock_t m_Sock;
	TCPResolver_t m_Resolver;
	lua_State *state;
	std::string m_strBuffered;

public:
	CGLSockTCP(IOService_t& IOService, lua_State* pLua, bool bOpen = true);

	virtual bool Bind(CEndpoint& Endpoint, Callback_t Callback);
	virtual bool Connect(std::string strHost, std::string strPort, Callback_t Callback);
	virtual bool Send(const char* cbData, unsigned int cubBuffer, Callback_t Callback);
	virtual bool Read(unsigned int cubBuffer, Callback_t Callback);
	virtual bool ReadUntil(std::string strDelimiter, Callback_t Callback);
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
		return eSockTypeTCP;
	}
	TCPSock_t& Socket(void)
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
			m_Sock.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
			m_Sock.close(ec);
			m_Sock.get_io_service().post(boost::bind(&CGLSockTCP::OnDestroy, this));
		}
		catch (boost::exception& ex)
		{
#if defined(_DEBUG)
			//LUA->Msg("GLSock(TCP): %s\n",  boost::diagnostic_information(ex).c_str());
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
	void OnResolve(Callback_t Callback, const boost::system::error_code& ec, TCPResolver_t::iterator endpoint_iterator);
	void OnConnect(Callback_t Callback, const boost::system::error_code& ec, TCPResolver_t::iterator endpoint_iterator);
	void OnSend(Callback_t Callback, unsigned int cubBytes, const boost::system::error_code& ec);
	void OnRead(Callback_t Callback, const char* pData, unsigned int cubBytes, const boost::system::error_code& ec);
	void OnReadUntil(Callback_t Callback, std::string strDelimiter, const char* pData, unsigned int cubBytes, const boost::system::error_code& ec);
	void OnDestroy(void);
};

}

#endif // GLSOCKTCP_H

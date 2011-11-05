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
	lua_State* L;

public:
	CGLSockTCP(IOService_t& IOService, lua_State* pLua, bool bOpen = true);

	virtual bool Bind(CEndpoint& Endpoint, Callback_t Callback);
	virtual bool Connect(std::string strHost, std::string strPort, Callback_t Callback);
	virtual bool Send(const char* cbData, unsigned int cubBuffer, Callback_t Callback);
	virtual bool Read(unsigned int cubBuffer, Callback_t Callback);
	virtual bool ReadUntil(const char* pszDelimiter, Callback_t Callback);
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
	virtual void Reference(void);
	virtual void Unreference(void);
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
			Lua()->Msg("GLSock(TCP): %s\n",  boost::diagnostic_information(ex).c_str());
#else
			UNREFERENCED_PARAM(ex);
#endif
		}
	}

private:
	void OnResolve(Callback_t Callback, const boost::system::error_code& ec, TCPResolver_t::iterator endpoint_iterator);
	void OnConnect(Callback_t Callback, const boost::system::error_code& ec, TCPResolver_t::iterator endpoint_iterator);
	void OnSend(Callback_t Callback, unsigned int cubBytes, const boost::system::error_code& ec);
	void OnRead(Callback_t Callback, const char* pData, unsigned int cubBytes, const boost::system::error_code& ec);
	void OnReadUntil(Callback_t Callback, boost::asio::streambuf* pStreamBuf, unsigned int cubBytes, const boost::system::error_code& ec);
	void OnDestroy(void);

	void CallbackBind(Callback_t Callback, CGLSock* pHandle, int iErrorMsg, lua_State* L);
	void CallbackConnect( Callback_t Callback, CGLSock* pHandle, int iErrorMsg, lua_State* L );
	void CalllbackSend( Callback_t Callback, CGLSock* pHandle, unsigned int cubBytes, int iErrorMsg, lua_State* L );
	void CallbackRead( Callback_t Callback, CGLSock* pHandle, GLSockBuffer::CGLSockBuffer* pBuffer, int iErrorMsg, lua_State* L);
};

}

#endif // GLSOCKTCP_H

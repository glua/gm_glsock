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
	lua_State* L;

public:
	CGLSockUDP(IOService_t& IOService, lua_State* pLua);

	virtual bool Bind(CEndpoint& Endpoint, Callback_t Callback);
	virtual bool SendTo(const char* cbData, unsigned int cubBuffer, std::string strHost, std::string strPort, Callback_t Callback);
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
		m_Sock.get_io_service().dispatch( boost::bind(&CGLSockUDP::OnDestroy, this));
	}

private:
	void OnSend(Callback_t Callback, unsigned int cubBytes, const boost::system::error_code& ec, UDPResolver_t::iterator iterator, const char* pBuffer, unsigned int cubBuffer);
	void OnRead(Callback_t Callback, boost::asio::ip::udp::endpoint* pSender, const char* pData, unsigned int cubBytes, const boost::system::error_code& ec);
	void OnDestroy(void);

	void CallbackBind(Callback_t Callback, CGLSock* pHandle, int iErrorMsg, lua_State* L);
	void CallbackSend(Callback_t Callback, CGLSock* pHandle, unsigned int cubBytes, int iErrorMsg, lua_State* L);
	void CallbackRead(Callback_t Callback, CGLSock* pHandle, std::string strSender, unsigned short usPort, GLSockBuffer::CGLSockBuffer* pBuffer, int iErrorMsg, lua_State* L);
};

}

#endif // GLSOCKUDP_H

#if !defined(GLSOCKACCEPTOR_H)
#define GLSOCKACCEPTOR_H

#include "GLSock.h"
#include "GLSockTCP.h"

namespace GLSock {

class CGLSockAcceptor : public CGLSock
{
private:
	boost::mutex m_Mutex;
	int m_nReferences;
	AcceptorSock_t m_Sock;
	lua_State* L;

public:
	CGLSockAcceptor(IOService_t& IOService_t, lua_State* pLua);

	virtual bool Bind(CEndpoint& Endpoint, Callback_t Callback);
	virtual bool Listen(int iBacklog, Callback_t Callback);
	virtual bool Accept(Callback_t Callback);
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
		m_Sock.io_service().dispatch( boost::bind(&CGLSockAcceptor::OnDestroy, this));
	}

private:
	void OnAccept(Callback_t Callback, class CGLSockTCP* pSock, const boost::system::error_code& ec);
	void OnDestroy(void);

	void CallbackBind(Callback_t Callback, CGLSock* pHandle, int iErrorMsg, lua_State* L);
	void CallbackListen(Callback_t Callback, CGLSock* pHandle, int iErrorMsg, lua_State* L);
	void CallbackAccept(Callback_t Callback, CGLSock* pHandle, CGLSock* pSock, int iErrorMsg, lua_State* L);
};

}

#endif // GLSOCKACCEPTOR_H

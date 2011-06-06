#include "BindingGLSock.h"
#include "SockMgr.h"
#include "GLSockBuffer.h"

namespace GLSock {

static int __new(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	if( !L )
		return 0;

	Lua()->CheckType(1, GLua::TYPE_NUMBER);
	
	int nType = Lua()->GetInteger(1);
	if( nType != SOCK_TYPE_ACCEPTOR && nType != SOCK_TYPE_TCP && nType != SOCK_TYPE_UDP )
	{
		Lua()->LuaError("Inalid socket type, must be either SOCKET_TYPE_ACCEPTOR, SOCKET_TYPE_TCP, SOCKET_TYPE_UDP", 1);
		return 0;
	}

	ISock* pSock = NULL;

	if( nType == SOCK_TYPE_ACCEPTOR )
		pSock = g_pSockMgr->CreateAcceptorSock(L);
	else if( nType == SOCK_TYPE_TCP )
		pSock = g_pSockMgr->CreateTCPSock(L);
	else if( nType == SOCK_TYPE_UDP )
		pSock = g_pSockMgr->CreateUDPSock(L);

	pSock->Reference();

	CAutoUnRef MetaTable = Lua()->GetMetaTable(GLSOCK_NAME, GLSOCK_TYPE);
	Lua()->PushUserData(MetaTable, static_cast<void*>(pSock));

#if defined(SOCK_DEBUG)
	Lua()->Msg("Created Socket of Type (%u) - 0x%p\n", nType, pSock);
#endif

	return 1;
}

static int __delete(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	if( !L )
		return 0;

	Lua()->CheckType(1, GLSOCK_TYPE);

	ISock* pSock = reinterpret_cast<ISock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		// Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	pSock->Unreference();

	return 0;
}

static int Bind(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	if( !L )
		return 0;

	Lua()->CheckType(1, GLSOCK_TYPE);
	Lua()->CheckType(2, GLua::TYPE_STRING);
	Lua()->CheckType(3, GLua::TYPE_NUMBER);
	Lua()->CheckType(4, GLua::TYPE_FUNCTION);

	ISock* pSock = reinterpret_cast<ISock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	std::string strHost( Lua()->GetString(2) );
	unsigned short usPort = (unsigned short)Lua()->GetInteger(3);
	Callback_t nCallback = Lua()->GetReference(4);

	unsigned int nHost = INADDR_ANY;

	// Not the best solution but its the best to keep TCP/UDP in a compatible method of the call method.
	CEndpoint* pEndpoint = NULL;
	if( !strHost.empty() )
	{
		// Conversation from string to long
		boost::asio::ip::address_v4 v4;
		v4.from_string(strHost);

		pEndpoint = new CEndpoint( v4.to_ulong(), usPort );
	}
	else
	{
		// Bind to all local ips.
		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), usPort);
		pEndpoint = new CEndpoint( endpoint.address().to_v4().to_ulong(), usPort );
	}

	Lua()->Push( pSock->Bind(*pEndpoint, nCallback) );

	delete pEndpoint;
	
	return 1;
}

static int Listen(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	if( !L )
		return 0;

	Lua()->CheckType(1, GLSOCK_TYPE);
	Lua()->CheckType(2, GLua::TYPE_NUMBER);
	Lua()->CheckType(3, GLua::TYPE_FUNCTION);

	ISock* pSock = reinterpret_cast<ISock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	int iBacklog = Lua()->GetInteger(2);
	Callback_t nCallback = Lua()->GetReference(3);

	Lua()->Push( pSock->Listen(iBacklog, nCallback) );
	return 1;
}

static int Accept(lua_State* L)
{	
	// SCOPED_LOCK(g_pSockMgr->Mutex());
#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	if( !L )
		return 0;

	Lua()->CheckType(1, GLSOCK_TYPE);
	Lua()->CheckType(2, GLua::TYPE_FUNCTION);

	ISock* pSock = reinterpret_cast<ISock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	Callback_t nCallback = Lua()->GetReference(2);

	Lua()->Push( pSock->Accept(nCallback) );
	return 1;
}

static int Connect(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	if( !L )
		return 0;

	Lua()->CheckType(1, GLSOCK_TYPE);
	Lua()->CheckType(2, GLua::TYPE_STRING);
	Lua()->CheckType(3, GLua::TYPE_NUMBER);
	Lua()->CheckType(4, GLua::TYPE_FUNCTION);

	ISock* pSock = reinterpret_cast<ISock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	std::string strHost( Lua()->GetString(2) );
	unsigned short usPort = (unsigned short)Lua()->GetInteger(3);

	Callback_t nCallback = Lua()->GetReference(4);

#if defined(SOCK_DEBUG)
	Lua()->Msg("Connecting to Host '%s:%u' Callback: %p\n", strHost.c_str(), usPort, nCallback);
#endif

	Lua()->Push( pSock->Connect(strHost, boost::lexical_cast<std::string>(usPort), nCallback) );

	return 1;
}

static int Send(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	if( !L )
		return 0;

	Lua()->CheckType(1, GLSOCK_TYPE);
	Lua()->CheckType(2, GLSOCKBUFFER_TYPE);
	Lua()->CheckType(3, GLua::TYPE_FUNCTION);

	ISock* pSock = reinterpret_cast<ISock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	GLSockBuffer::CGLSockBuffer* pBuffer = reinterpret_cast<GLSockBuffer::CGLSockBuffer*>( Lua()->GetUserData(2) );

	Callback_t nCallback = Lua()->GetReference(3);

	Lua()->Push( pSock->Send(pBuffer->Buffer(), pBuffer->Size(), nCallback) );
	return 1;
}

static int SendTo(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	if( !L )
		return 0;

	Lua()->CheckType(1, GLSOCK_TYPE);
	Lua()->CheckType(2, GLSOCKBUFFER_TYPE);
	Lua()->CheckType(3, GLua::TYPE_STRING);
	Lua()->CheckType(4, GLua::TYPE_NUMBER);
	Lua()->CheckType(5, GLua::TYPE_FUNCTION);

	ISock* pSock = reinterpret_cast<ISock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	GLSockBuffer::CGLSockBuffer* pBuffer = reinterpret_cast<GLSockBuffer::CGLSockBuffer*>( Lua()->GetUserData(2) );

	std::string strHost( Lua()->GetString(3) );
	std::string strPort = boost::lexical_cast<std::string>( (unsigned int)Lua()->GetInteger(4) );

	Callback_t nCallback = Lua()->GetReference(3);

	Lua()->Push( pSock->SendTo(pBuffer->Buffer(), pBuffer->Size(), strHost, strPort, nCallback) );
	return 1;
}

static int Read(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	if( !L )
		return 0;

	Lua()->CheckType(1, GLSOCK_TYPE);
	Lua()->CheckType(2, GLua::TYPE_NUMBER);
	Lua()->CheckType(3, GLua::TYPE_FUNCTION);

	ISock* pSock = reinterpret_cast<ISock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	int iCount = Lua()->GetInteger(2);
	Callback_t nCallback = Lua()->GetReference(3);

	Lua()->Push( pSock->Read(iCount, nCallback) );
	return 1;
}

static int ReadFrom(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	if( !L )
		return 0;

	Lua()->CheckType(1, GLSOCK_TYPE);
	Lua()->CheckType(2, GLua::TYPE_NUMBER);
	Lua()->CheckType(3, GLua::TYPE_FUNCTION);

	ISock* pSock = reinterpret_cast<ISock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	int iCount = Lua()->GetInteger(2);
	Callback_t nCallback = Lua()->GetReference(3);

	Lua()->Push( pSock->ReadFrom(iCount, nCallback) );
	return 1;
}

static int Resolve(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	if( !L )
		return 0;

	Lua()->CheckType(1, GLSOCK_TYPE);

	ISock* pSock = reinterpret_cast<ISock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	return 0;
}

static int Close(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	if( !L )
		return 0;

	Lua()->CheckType(1, GLSOCK_TYPE);

	ISock* pSock = reinterpret_cast<ISock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	Lua()->Push( pSock->Close() );
	return 1;
}

static int Poll(lua_State* L)
{
	if( !L )
		return 0;

	g_pSockMgr->Poll(L);
	return 0;
}

void Startup( lua_State* L )
{
	// Looks like using thread is a bad thing, lets stick to poll_one in Dispatch

	//g_pSockMgr->StartThread();

	CAutoUnRef MetaTable = Lua()->GetMetaTable(GLSOCK_NAME, GLSOCK_TYPE);
	{
		CAutoUnRef Index = Lua()->GetNewTable();

		Index->SetMember("Bind", GLSock::Bind);
		Index->SetMember("Listen", GLSock::Listen);
		Index->SetMember("Accept", GLSock::Accept);
		Index->SetMember("Connect", GLSock::Connect);
		Index->SetMember("Send", GLSock::Send);
		Index->SetMember("SendTo", GLSock::SendTo);
		Index->SetMember("Read", GLSock::Read);
		Index->SetMember("ReadFrom", GLSock::ReadFrom);
		Index->SetMember("Resolve", GLSock::Resolve);
		Index->SetMember("Close", GLSock::Close);

		MetaTable->SetMember("__index", Index);
	}
	MetaTable->SetMember("__gc", GLSock::__delete);

	Lua()->SetGlobal("GLSock", GLSock::__new);

	ILuaObject* hook = Lua()->GetGlobal("hook");
	ILuaObject* hook_Add = hook->GetMember("Add");

	hook_Add->Push();
	Lua()->Push("Think");
	Lua()->Push("GLSockPolling");
	Lua()->Push(GLSock::Poll);
	Lua()->Call(3);

	// Socket Types
	Lua()->SetGlobal("SOCK_TYPE_ACCEPTOR", (float)SOCK_TYPE_ACCEPTOR);
	Lua()->SetGlobal("SOCK_TYPE_TCP", (float)SOCK_TYPE_TCP);
	Lua()->SetGlobal("SOCK_TYPE_UDP", (float)SOCK_TYPE_UDP);

	// Socket Errors
	Lua()->SetGlobal("SOCK_ERROR_SUCCESS", (float)SOCKET_ERROR_SUCCESS);
	Lua()->SetGlobal("SOCK_ERROR_ALREADY_CONNECTED", (float)SOCKET_ERROR_ALREADY_CONNECTED);
	Lua()->SetGlobal("SOCK_ERROR_CONNECTION_ABORTED", (float)SOCKET_ERROR_CONNECTION_ABORTED);
	Lua()->SetGlobal("SOCK_ERROR_CONNECTION_REFUSED", (float)SOCKET_ERROR_CONNECTION_REFUSED);
	Lua()->SetGlobal("SOCK_ERROR_CONNECTION_RESET", (float)SOCKET_ERROR_CONNECTION_RESET);
	Lua()->SetGlobal("SOCK_ERROR_ADDRESS_IN_USE", (float)SOCKET_ERROR_ADDRESS_IN_USE);
	Lua()->SetGlobal("SOCK_ERROR_TIMED_OUT", (float)SOCKET_ERROR_TIMED_OUT);
	Lua()->SetGlobal("SOCK_ERROR_HOST_UNREACHABLE", (float)SOCKET_ERROR_HOST_UNREACHABLE);
	Lua()->SetGlobal("SOCK_ERROR_NOT_CONNECTED", (float)SOCKET_ERROR_NOT_CONNECTED);
	Lua()->SetGlobal("SOCK_ERROR_OPERATION_ABORTED", (float)SOCKET_ERROR_OPERATION_ABORTED);
}

void Cleanup( lua_State* L )
{
	ILuaObject* hook = Lua()->GetGlobal("hook");
	ILuaObject* hook_Remove = hook->GetMember("Remove");

	hook_Remove->Push();
	Lua()->Push("Think");
	Lua()->Push("GLSockPolling");
	Lua()->Call(2);
}

} // GLSock
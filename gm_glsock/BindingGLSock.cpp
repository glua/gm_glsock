#include "Common.h"
#include "BindingGLSock.h"
#include "SockMgr.h"
#include "GLSockBuffer.h"
#include "BufferMgr.h"

namespace GLSock {

static int __new(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLua::TYPE_NUMBER);
	
	int nType = Lua()->GetInteger(1);
	CGLSock* pSock = NULL;

	switch(nType)
	{
	case eSockTypeAcceptor:
		{
			pSock = g_pSockMgr->CreateAcceptorSock(L);
		}
		break;
	case eSockTypeTCP:
		{
			pSock = g_pSockMgr->CreateTCPSock(L);
		}
		break;
	case eSockTypeUDP:
		{
			pSock = g_pSockMgr->CreateUDPSock(L);
		}
		break;
	default:
		{
			Lua()->LuaError("Inalid socket type, must be either SOCKET_TYPE_ACCEPTOR, SOCKET_TYPE_TCP, SOCKET_TYPE_UDP", 1);
			return 0;
		}
		break;
	}

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
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCK_TYPE);

	CGLSock* pSock = reinterpret_cast<CGLSock*>( Lua()->GetUserData(1) );
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
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCK_TYPE);
	Lua()->CheckType(2, GLua::TYPE_STRING);
	Lua()->CheckType(3, GLua::TYPE_NUMBER);
	Lua()->CheckType(4, GLua::TYPE_FUNCTION);

        boost::system::error_code ex;
        
	CGLSock* pSock = reinterpret_cast<CGLSock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	std::string strHost( Lua()->GetString(2) );
	unsigned short usPort = (unsigned short)Lua()->GetInteger(3);
	Callback_t nCallback = Lua()->GetReference(4);

	// Not the best solution but its the best to keep TCP/UDP in a compatible method of the call method.
	CEndpoint* pEndpoint = NULL;
	if( !strHost.empty() )
	{
		// Conversation from string to long
		boost::asio::ip::address_v4 v4;
		v4.from_string(strHost, ex);

		pEndpoint = new CEndpoint( v4.to_ulong(), usPort );
	}
	else
	{
		// Bind to all local ips.
		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), usPort);
		pEndpoint = new CEndpoint( endpoint.address().to_v4().to_ulong(), usPort );
	}

        if( ex )
        {
            if( pEndpoint )
                delete pEndpoint;
            return 0;
        }
        
        pSock->Reference();
	Lua()->Push( pSock->Bind(*pEndpoint, nCallback) );

	delete pEndpoint;
	
	return 1;
}

static int Listen(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCK_TYPE);
	Lua()->CheckType(2, GLua::TYPE_NUMBER);
	Lua()->CheckType(3, GLua::TYPE_FUNCTION);

	CGLSock* pSock = reinterpret_cast<CGLSock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	if( Lua()->GetType(2) != GLua::TYPE_NUMBER )
		return 0;
	int iBacklog = Lua()->GetInteger(2);

	if( Lua()->GetType(3) != GLua::TYPE_FUNCTION )
		return 0;
	Callback_t nCallback = Lua()->GetReference(3);

	pSock->Reference();
	Lua()->Push( pSock->Listen(iBacklog, nCallback) );
        
	return 1;
}

static int Accept(lua_State* L)
{	
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCK_TYPE);
	Lua()->CheckType(2, GLua::TYPE_FUNCTION);

	CGLSock* pSock = reinterpret_cast<CGLSock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	if( Lua()->GetType(2) != GLua::TYPE_FUNCTION )
		return 0;
	Callback_t nCallback = Lua()->GetReference(2);

	pSock->Reference();
	Lua()->Push( pSock->Accept(nCallback) );
        
	return 1;
}

static int Connect(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCK_TYPE);
	Lua()->CheckType(2, GLua::TYPE_STRING);
	Lua()->CheckType(3, GLua::TYPE_NUMBER);
	Lua()->CheckType(4, GLua::TYPE_FUNCTION);

	CGLSock* pSock = reinterpret_cast<CGLSock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	if( Lua()->GetType(2) != GLua::TYPE_STRING )
		return 0;
	std::string strHost( Lua()->GetString(2) );

	if( Lua()->GetType(3) != GLua::TYPE_NUMBER )
		return 0;
	unsigned short usPort = (unsigned short)Lua()->GetInteger(3);

	if( Lua()->GetType(4) != GLua::TYPE_FUNCTION )
		return 0;
	Callback_t nCallback = Lua()->GetReference(4);

#if defined(SOCK_DEBUG)
	Lua()->Msg("Connecting to Host '%s:%u' Callback: %p\n", strHost.c_str(), usPort, nCallback);
#endif

	pSock->Reference();
	Lua()->Push( pSock->Connect(strHost, boost::lexical_cast<std::string>(usPort), nCallback) );

	return 1;
}

static int Send(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCK_TYPE);
	Lua()->CheckType(2, GLSOCKBUFFER_TYPE);
	Lua()->CheckType(3, GLua::TYPE_FUNCTION);

	CGLSock* pSock = reinterpret_cast<CGLSock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	GLSockBuffer::CGLSockBuffer* pBuffer = reinterpret_cast<GLSockBuffer::CGLSockBuffer*>( Lua()->GetUserData(2) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	if( Lua()->GetType(3) != GLua::TYPE_FUNCTION )
		return 0;
	Callback_t nCallback = Lua()->GetReference(3);

	pSock->Reference();

	Lua()->Push( pSock->Send(pBuffer->Buffer(), pBuffer->Size(), nCallback) );
        
	return 1;
}

static int SendTo(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCK_TYPE);
	Lua()->CheckType(2, GLSOCKBUFFER_TYPE);
	Lua()->CheckType(3, GLua::TYPE_STRING);
	Lua()->CheckType(4, GLua::TYPE_NUMBER);
	Lua()->CheckType(5, GLua::TYPE_FUNCTION);

	CGLSock* pSock = reinterpret_cast<CGLSock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	GLSockBuffer::CGLSockBuffer* pBuffer = reinterpret_cast<GLSockBuffer::CGLSockBuffer*>( Lua()->GetUserData(2) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	if( Lua()->GetType(3) != GLua::TYPE_STRING )
		return 0;
	std::string strHost( Lua()->GetString(3) );

	if( Lua()->GetType(4) != GLua::TYPE_NUMBER )
		return 0;
	std::string strPort = boost::lexical_cast<std::string>( (unsigned int)Lua()->GetInteger(4) );

	if( Lua()->GetType(5) != GLua::TYPE_FUNCTION )
		return 0;
	Callback_t nCallback = Lua()->GetReference(5);

	pSock->Reference();

	Lua()->Push( pSock->SendTo(pBuffer->Buffer(), pBuffer->Size(), strHost, strPort, nCallback) );
        
	return 1;
}

static int Read(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCK_TYPE);
	Lua()->CheckType(2, GLua::TYPE_NUMBER);
	Lua()->CheckType(3, GLua::TYPE_FUNCTION);

	CGLSock* pSock = reinterpret_cast<CGLSock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	if( Lua()->GetType(2) != GLua::TYPE_NUMBER )
		return 0;
	int iCount = Lua()->GetInteger(2);

	if( Lua()->GetType(3) != GLua::TYPE_FUNCTION )
		return 0;
	Callback_t nCallback = Lua()->GetReference(3);

	pSock->Reference();
	Lua()->Push( pSock->Read(iCount, nCallback) );
        
	return 1;
}

static int ReadUntil(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCK_TYPE);
	Lua()->CheckType(2, GLua::TYPE_STRING);
	Lua()->CheckType(3, GLua::TYPE_FUNCTION);

	CGLSock* pSock = reinterpret_cast<CGLSock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	if( Lua()->GetType(2) != GLua::TYPE_STRING )
		return 0;
	const char* pszDelimiter = Lua()->GetString(2);

	if( Lua()->GetType(3) != GLua::TYPE_FUNCTION )
		return 0;
	Callback_t nCallback = Lua()->GetReference(3);

	pSock->Reference();
	Lua()->Push( pSock->ReadUntil(pszDelimiter, nCallback) );
        
	return 1;
}


static int ReadFrom(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCK_TYPE);
	Lua()->CheckType(2, GLua::TYPE_NUMBER);
	Lua()->CheckType(3, GLua::TYPE_FUNCTION);

	CGLSock* pSock = reinterpret_cast<CGLSock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	if( Lua()->GetType(2) != GLua::TYPE_NUMBER )
		return 0;
	int iCount = Lua()->GetInteger(2);

	if( Lua()->GetType(3) != GLua::TYPE_FUNCTION )
		return 0;
	Callback_t nCallback = Lua()->GetReference(3);

	pSock->Reference();
	Lua()->Push( pSock->ReadFrom(iCount, nCallback) );
        
	return 1;
}

static int Resolve(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCK_TYPE);

	CGLSock* pSock = reinterpret_cast<CGLSock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	return 0;
}

static int Cancel(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCK_TYPE);

	CGLSock* pSock = reinterpret_cast<CGLSock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	//pSock->Reference();
	Lua()->Push( pSock->Cancel() );

	return 1;
}

static int Close(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCK_TYPE);

	CGLSock* pSock = reinterpret_cast<CGLSock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	//pSock->Reference();
	Lua()->Push( pSock->Close() );
        
	return 1;
}

static int Destroy(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCK_TYPE);

	CGLSock* pSock = reinterpret_cast<CGLSock*>( Lua()->GetUserData(1) );
	if( !g_pSockMgr->ValidHandle(pSock) )
	{
		Lua()->LuaError("Invalid socket handle", 1);
		return 0;
	}

	pSock->Destroy();

	return 0;
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
#if defined(_DEBUG)
        Lua()->Msg("GLSock Debug Build");
#endif
    
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
		Index->SetMember("ReadUntil", GLSock::ReadUntil);
		Index->SetMember("ReadFrom", GLSock::ReadFrom);
		Index->SetMember("Resolve", GLSock::Resolve);
		Index->SetMember("Close", GLSock::Close);
		Index->SetMember("Cancel", GLSock::Cancel);
		Index->SetMember("Destroy", GLSock::Destroy);

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
	Lua()->SetGlobal("GLSOCK_TYPE_ACCEPTOR", (float)eSockTypeAcceptor);
	Lua()->SetGlobal("GLSOCK_TYPE_TCP", (float)eSockTypeTCP);
	Lua()->SetGlobal("GLSOCK_TYPE_UDP", (float)eSockTypeUDP);

	// Socket Errors
	Lua()->SetGlobal("GLSOCK_ERROR_SUCCESS", (float)eSockErrorSuccess);
	Lua()->SetGlobal("GLSOCK_ERROR_ACCESSDENIED", (float)eSockErrorAccessDenied);
	Lua()->SetGlobal("GLSOCK_ERROR_ADDRESSFAMILYNOTSUPPORTED", (float)eSockErrorAddressFamilyNotSupported);
	Lua()->SetGlobal("GLSOCK_ERROR_ADDRESSINUSE", (float)eSockErrorAddressInUse);
	Lua()->SetGlobal("GLSOCK_ERROR_ALREADYCONNECTED", (float)eSockErrorAlreadyConnected);
	Lua()->SetGlobal("GLSOCK_ERROR_ALREADYSTARTED", (float)eSockErrorAlreadyStarted);
	Lua()->SetGlobal("GLSOCK_ERROR_BROKENPIPE", (float)eSockErrorBrokenPipe);
	Lua()->SetGlobal("GLSOCK_ERROR_CONNECTIONABORTED", (float)eSockErrorConnectionAborted);
	Lua()->SetGlobal("GLSOCK_ERROR_CONNECTIONREFUSED", (float)eSockErrorConnectionRefused);
	Lua()->SetGlobal("GLSOCK_ERROR_CONNECTIONRESET", (float)eSockErrorConnectionReset);
	Lua()->SetGlobal("GLSOCK_ERROR_BADDESCRIPTOR", (float)eSockErrorBadDescriptor);
	Lua()->SetGlobal("GLSOCK_ERROR_BADADDRESS", (float)eSockErrorBadAddress);
	Lua()->SetGlobal("GLSOCK_ERROR_HOSTUNREACHABLE", (float)eSockErrorHostUnreachable);
	Lua()->SetGlobal("GLSOCK_ERROR_INPROGRESS", (float)eSockErrorInProgress);
	Lua()->SetGlobal("GLSOCK_ERROR_INTERRUPTED", (float)eSockErrorInterrupted);
	Lua()->SetGlobal("GLSOCK_ERROR_INVALIDARGUMENT", (float)eSockErrorInvalidArgument);
	Lua()->SetGlobal("GLSOCK_ERROR_MESSAGESIZE", (float)eSockErrorMessageSize);
	Lua()->SetGlobal("GLSOCK_ERROR_NAMETOOLONG", (float)eSockErrorNameTooLong);
	Lua()->SetGlobal("GLSOCK_ERROR_NETWORKDOWN", (float)eSockErrorNetworkDown);
	Lua()->SetGlobal("GLSOCK_ERROR_NETWORKRESET", (float)eSockErrorNetworkReset);
	Lua()->SetGlobal("GLSOCK_ERROR_NETWORKUNREACHABLE", (float)eSockErrorNetworkUnreachable);
	Lua()->SetGlobal("GLSOCK_ERROR_NODESCRIPTORS", (float)eSockErrorNoDescriptors);
	Lua()->SetGlobal("GLSOCK_ERROR_NOBUFFERSPACE", (float)eSockErrorNoBufferSpace);
	Lua()->SetGlobal("GLSOCK_ERROR_NOMEMORY", (float)eSockErrorNoMemory);
	Lua()->SetGlobal("GLSOCK_ERROR_NOPERMISSION", (float)eSockErrorNoPermission);
	Lua()->SetGlobal("GLSOCK_ERROR_NOPROTOCOLOPTION", (float)eSockErrorNoProtocolOption);
	Lua()->SetGlobal("GLSOCK_ERROR_NOTCONNECTED", (float)eSockErrorNotConnected);
	Lua()->SetGlobal("GLSOCK_ERROR_NOTSOCKET", (float)eSockErrorNotSocket);
	Lua()->SetGlobal("GLSOCK_ERROR_OPERATIONABORTED", (float)eSockErrorOperationAborted);
	Lua()->SetGlobal("GLSOCK_ERROR_OPERATIONNOTSUPPORTED", (float)eSockErrorOperationNotSupported);
	Lua()->SetGlobal("GLSOCK_ERROR_SHUTDOWN", (float)eSockErrorShutDown);
	Lua()->SetGlobal("GLSOCK_ERROR_TIMEDOUT", (float)eSockErrorTimedOut);
	Lua()->SetGlobal("GLSOCK_ERROR_TRYAGAIN", (float)eSockErrorTryAgain);
	Lua()->SetGlobal("GLSOCK_ERROR_WOULDBLOCK", (float)eSockErrorWouldBlock);
}

void Cleanup( lua_State* L )
{
	g_pSockMgr->CloseSockets();

	ILuaObject* hook = Lua()->GetGlobal("hook");
	ILuaObject* hook_Remove = hook->GetMember("Remove");

	hook_Remove->Push();
	Lua()->Push("Think");
	Lua()->Push("GLSockPolling");
	Lua()->Call(2);
}

} // GLSock

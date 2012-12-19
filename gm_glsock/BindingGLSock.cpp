#include "Common.h"
#include "SockMgr.h"
#include "GLSockBuffer.h"
#include "BufferMgr.h"

#include "BindingGLSock.h"
#include "BindingGLSockBuffer.h"

namespace GLSock {

CGLSock* CheckSocket(lua_State *state, int idx)
{
	LUA->CheckType(idx, GLSOCK_TYPE);

	GarrysMod::Lua::UserData *pUserData = (GarrysMod::Lua::UserData*)LUA->GetUserdata(idx);
	if( !pUserData )
		return NULL;

	if( pUserData->type != GLSOCK_TYPE )
		return NULL;

	CGLSock *pSock = reinterpret_cast<CGLSock*>(pUserData->data);
	if( !g_pSockMgr->ValidHandle(pSock) )
		return NULL;

	return pSock;
}

void PushSocket(lua_State *state, CGLSock *pSock)
{
	GarrysMod::Lua::UserData *pUserData = static_cast<GarrysMod::Lua::UserData*>(LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData)));
	int iRefUserData = LUA->ReferenceCreate();

	pUserData->type = GLSOCK_TYPE;
	pUserData->data = static_cast<void*>(pSock);

	LUA->CreateMetaTableType(GLSOCK_NAME, GLSOCK_TYPE);
	int iRefMetatable = LUA->ReferenceCreate();

	LUA->ReferencePush(iRefUserData);
	LUA->ReferencePush(iRefMetatable);
	LUA->SetMetaTable(-2);

	LUA->ReferenceFree(iRefMetatable);
	LUA->ReferenceFree(iRefUserData);
}

static int __new(lua_State* state)
{
	if( !state )
		return 0;

	LUA->CheckType(1, GarrysMod::Lua::Type::NUMBER);
	
	int nType = (int)LUA->GetNumber(1);
	CGLSock* pSock = NULL;

	switch(nType)
	{
	case eSockTypeAcceptor:
		{
			pSock = g_pSockMgr->CreateAcceptorSock(state);
		}
		break;
	case eSockTypeTCP:
		{
			pSock = g_pSockMgr->CreateTCPSock(state);
		}
		break;
	case eSockTypeUDP:
		{
			pSock = g_pSockMgr->CreateUDPSock(state);
		}
		break;
	default:
		{
			LUA->ThrowError("Invalid socket type, must be either SOCKET_TYPE_ACCEPTOR, SOCKET_TYPE_TCP, SOCKET_TYPE_UDP");
			return 0;
		}
		break;
	}

	pSock->Reference();
	PushSocket(state, pSock);

	return 1;
}

static int __delete(lua_State* state)
{
	if( !state )
		return 0;

	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	int nTableRef = pSock->m_nTableRef;
	if( pSock->Unreference() <= 0 )
	{
		LUA->ReferenceFree(nTableRef);
	}

	return 0;
}

static int __index(lua_State* state)
{
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	LUA->CreateMetaTableType(GLSOCK_NAME, GLSOCK_TYPE);
	CAutoUnRef Metatable(state);
	{
		Metatable.Push();
		LUA->PushString("__functions");
		LUA->RawGet(-2);

		LUA->Push(2);
		LUA->RawGet(-2);

		if( !LUA->IsType(-1, GarrysMod::Lua::Type::NIL) )
			return 1;
		else
			LUA->Pop();
	}

	if( pSock->m_nTableRef == 0 )
		return 0;

	LUA->ReferencePush(pSock->m_nTableRef);
	LUA->Push(2);
	LUA->RawGet(-2);

	return 1;
}

static int __newindex(lua_State* state)
{
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	if( pSock->m_nTableRef == 0 )
	{
		LUA->CreateTable();
		pSock->m_nTableRef = LUA->ReferenceCreate();
	}

	LUA->ReferencePush(pSock->m_nTableRef);
	LUA->Push(2);
	LUA->Push(3);
	LUA->RawSet(-3);
	
	return 0;
}

static int __tostring(lua_State* state)
{
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	std::stringstream ss;
	switch(pSock->Type())
	{
	case eSockTypeUDP:
		{
			ss << "GLSockUDP: " << std::hex << std::setfill('0') << std::setw(8) << pSock;
		}
		break;
	case eSockTypeTCP:
		{
			ss << "GLSockTCP: " << std::hex << std::setfill('0') << std::setw(8) << pSock;
		}
		break;
	case eSockTypeAcceptor:
		{
			ss << "GLSockAcceptor: " << std::hex << std::setfill('0') << std::setw(8) << pSock;
		}
		break;
	default:
		{
			ss << "GLSock: " << std::hex << std::setfill('0') << std::setw(8) << pSock;
		}
		break;
	}

	std::string strType = ss.str();
	LUA->PushString(strType.c_str());

	return 1;
}

static int __eq(lua_State* state)
{
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	CGLSock* pSockCmp = CheckSocket(state, 2);
	if( pSockCmp )
		return 0;

	LUA->PushBool( pSock == pSockCmp );
	return 1;
}

static int Bind(lua_State* state)
{
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	LUA->CheckType(2, GarrysMod::Lua::Type::STRING);
	LUA->CheckType(3, GarrysMod::Lua::Type::NUMBER);
	LUA->CheckType(4, GarrysMod::Lua::Type::FUNCTION);

    boost::system::error_code ex;

	std::string strHost( LUA->GetString(2) );
	unsigned short usPort = (unsigned short)LUA->GetNumber(3);

	LUA->Push(4);
	Callback_t nCallback = LUA->ReferenceCreate();

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
	LUA->PushBool( pSock->Bind(*pEndpoint, nCallback) );

	delete pEndpoint;
	
	return 1;
}

static int Listen(lua_State* state)
{
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER);
	LUA->CheckType(3, GarrysMod::Lua::Type::FUNCTION);

	int iBacklog = (int)LUA->GetNumber(2);

	LUA->Push(3);
	Callback_t nCallback = LUA->ReferenceCreate();

	pSock->Reference();
	LUA->PushBool( pSock->Listen(iBacklog, nCallback) );
        
	return 1;
}

static int Accept(lua_State* state)
{	
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	LUA->CheckType(2, GarrysMod::Lua::Type::FUNCTION);

	LUA->Push(2);
	Callback_t nCallback = LUA->ReferenceCreate();

	pSock->Reference();
	LUA->PushBool( pSock->Accept(nCallback) );
        
	return 1;
}

static int Connect(lua_State* state)
{
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	LUA->CheckType(2, GarrysMod::Lua::Type::STRING);
	LUA->CheckType(3, GarrysMod::Lua::Type::NUMBER);
	LUA->CheckType(4, GarrysMod::Lua::Type::FUNCTION);

	std::string strHost( LUA->GetString(2) );
	unsigned short usPort = static_cast<unsigned short>(LUA->GetNumber(3));

	LUA->Push(4);
	Callback_t nCallback = LUA->ReferenceCreate();

	pSock->Reference();
	LUA->PushBool( pSock->Connect(strHost, boost::lexical_cast<std::string>(usPort), nCallback) );

	return 1;
}

static int Send(lua_State* state)
{
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	LUA->CheckType(3, GarrysMod::Lua::Type::FUNCTION);

	GLSockBuffer::CGLSockBuffer* pBuffer = GLSockBuffer::CheckBuffer(state, 2);
	if( !pBuffer )
	{
		LUA->ThrowError("Invalid buffer handle");
		return 0;
	}

	LUA->Push(3);
	Callback_t nCallback = LUA->ReferenceCreate();

	pSock->Reference();
	LUA->PushBool( pSock->Send(pBuffer->Buffer(), pBuffer->Size(), nCallback) );
        
	return 1;
}

static int SendTo(lua_State* state)
{
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	LUA->CheckType(3, GarrysMod::Lua::Type::STRING);
	LUA->CheckType(4, GarrysMod::Lua::Type::NUMBER);
	LUA->CheckType(5, GarrysMod::Lua::Type::FUNCTION);

	GLSockBuffer::CGLSockBuffer* pBuffer = GLSockBuffer::CheckBuffer(state, 2);
	if( !pBuffer )
	{
		LUA->ThrowError("Invalid buffer handle");
		return 0;
	}

	std::string strHost( LUA->GetString(3) );
	std::string strPort = boost::lexical_cast<std::string>( (unsigned int)LUA->GetNumber(4) );

	LUA->Push(5);
	Callback_t nCallback = LUA->ReferenceCreate();

	pSock->Reference();
	LUA->PushBool( pSock->SendTo(pBuffer->Buffer(), pBuffer->Size(), strHost, strPort, nCallback) );
        
	return 1;
}

static int Read(lua_State* state)
{
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER);
	LUA->CheckType(3, GarrysMod::Lua::Type::FUNCTION);

	int iCount = static_cast<int>( LUA->GetNumber(2) );

	LUA->Push(3);
	Callback_t nCallback = LUA->ReferenceCreate();

	pSock->Reference();
	LUA->PushBool( pSock->Read(iCount, nCallback) );
        
	return 1;
}

static int ReadUntil(lua_State* state)
{
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	LUA->CheckType(2, GarrysMod::Lua::Type::STRING);
	LUA->CheckType(3, GarrysMod::Lua::Type::FUNCTION);

	unsigned int nLen = 0;
	const char *pData = LUA->GetString(2, &nLen);
	std::string strDelimiter(pData, nLen);

	LUA->Push(3);
	Callback_t nCallback = LUA->ReferenceCreate();

	pSock->Reference();
	LUA->PushBool( pSock->ReadUntil(strDelimiter, nCallback) );
        
	return 1;
}


static int ReadFrom(lua_State* state)
{
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER);
	LUA->CheckType(3, GarrysMod::Lua::Type::FUNCTION);

	int iCount = static_cast<int>( LUA->GetNumber(2) );

	LUA->Push(3);
	Callback_t nCallback = LUA->ReferenceCreate();

	pSock->Reference();
	LUA->PushBool( pSock->ReadFrom(iCount, nCallback) );
        
	return 1;
}

static int Resolve(lua_State* state)
{
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	return 0;
}

static int Cancel(lua_State* state)
{
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	//pSock->Reference();
	LUA->PushBool( pSock->Cancel() );

	return 1;
}

static int Close(lua_State* state)
{
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	//pSock->Reference();
	LUA->PushBool( pSock->Close() );
        
	return 1;
}

static int Destroy(lua_State* state)
{
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	pSock->Destroy();

	return 0;
}

static int Type(lua_State* state)
{
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	LUA->PushNumber( static_cast<double>( pSock->Type() ) );

	return 1;
}

static int RemoteAddress(lua_State* state)
{
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	std::string strAddress = pSock->RemoteAddress();
	LUA->PushString( strAddress.c_str() );

	return 1;
}

static int RemotePort(lua_State* state)
{
	CGLSock* pSock = CheckSocket(state, 1);
	if( !pSock )
		return 0;

	LUA->PushNumber( static_cast<double>(pSock->RemotePort()) );

	return 1;
}

static int Poll(lua_State* state)
{
	g_pSockMgr->Poll(state);
	return 0;
}

void Startup( lua_State* state )
{
	LUA->CreateMetaTableType(GLSOCK_NAME, GLSOCK_TYPE);
	CAutoUnRef Metatable(state);	
	{
		Metatable.Push();

		#define SetMember(k, v) LUA->PushString(k); LUA->PushCFunction(v); LUA->SetTable(-3);

		LUA->PushString("__functions");
		LUA->CreateTable();
		{
			SetMember("Bind", GLSock::Bind);
			SetMember("Listen", GLSock::Listen);
			SetMember("Accept", GLSock::Accept);
			SetMember("Connect", GLSock::Connect);
			SetMember("Send", GLSock::Send);
			SetMember("SendTo", GLSock::SendTo);
			SetMember("Read", GLSock::Read);
			SetMember("ReadUntil", GLSock::ReadUntil);
			SetMember("ReadFrom", GLSock::ReadFrom);
			SetMember("Resolve", GLSock::Resolve);
			SetMember("Close", GLSock::Close);
			SetMember("Cancel", GLSock::Cancel);
			SetMember("Destroy", GLSock::Destroy);
			SetMember("RemoteAddress", GLSock::RemoteAddress);
			SetMember("RemotePort", GLSock::RemotePort);
		}
		LUA->RawSet(-3);

		SetMember("__gc", GLSock::__delete);
		SetMember("__eq", GLSock::__eq);
		SetMember("__tostring", GLSock::__tostring);
		SetMember("__index", GLSock::__index);
		SetMember("__newindex", GLSock::__newindex);

		#undef SetMember
	}

	LuaSetGlobal(state, "GLSock", GLSock::__new);

	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );
	LUA->PushString("hook");
	LUA->RawGet(-2);
	LUA->PushString("Add");
	LUA->RawGet(-2);
	LUA->PushString("Think");
	LUA->PushString("GLSockPolling");
	LUA->PushCFunction(GLSock::Poll);
	LUA->Call(3, 0);
	LUA->Pop();

	// Socket Types
	LuaSetGlobal(state, "GLSOCK_TYPE_ACCEPTOR", eSockTypeAcceptor);
	LuaSetGlobal(state, "GLSOCK_TYPE_TCP", eSockTypeTCP);
	LuaSetGlobal(state, "GLSOCK_TYPE_UDP", eSockTypeUDP);

	// Socket Errors
	LuaSetGlobal(state, "GLSOCK_ERROR_SUCCESS", eSockErrorSuccess);
	LuaSetGlobal(state, "GLSOCK_ERROR_ACCESSDENIED", eSockErrorAccessDenied);
	LuaSetGlobal(state, "GLSOCK_ERROR_ADDRESSFAMILYNOTSUPPORTED", eSockErrorAddressFamilyNotSupported);
	LuaSetGlobal(state, "GLSOCK_ERROR_ADDRESSINUSE", eSockErrorAddressInUse);
	LuaSetGlobal(state, "GLSOCK_ERROR_ALREADYCONNECTED", eSockErrorAlreadyConnected);
	LuaSetGlobal(state, "GLSOCK_ERROR_ALREADYSTARTED", eSockErrorAlreadyStarted);
	LuaSetGlobal(state, "GLSOCK_ERROR_BROKENPIPE", eSockErrorBrokenPipe);
	LuaSetGlobal(state, "GLSOCK_ERROR_CONNECTIONABORTED", eSockErrorConnectionAborted);
	LuaSetGlobal(state, "GLSOCK_ERROR_CONNECTIONREFUSED", eSockErrorConnectionRefused);
	LuaSetGlobal(state, "GLSOCK_ERROR_CONNECTIONRESET", eSockErrorConnectionReset);
	LuaSetGlobal(state, "GLSOCK_ERROR_BADDESCRIPTOR", eSockErrorBadDescriptor);
	LuaSetGlobal(state, "GLSOCK_ERROR_BADADDRESS", eSockErrorBadAddress);
	LuaSetGlobal(state, "GLSOCK_ERROR_HOSTUNREACHABLE", eSockErrorHostUnreachable);
	LuaSetGlobal(state, "GLSOCK_ERROR_INPROGRESS", eSockErrorInProgress);
	LuaSetGlobal(state, "GLSOCK_ERROR_INTERRUPTED", eSockErrorInterrupted);
	LuaSetGlobal(state, "GLSOCK_ERROR_INVALIDARGUMENT", eSockErrorInvalidArgument);
	LuaSetGlobal(state, "GLSOCK_ERROR_MESSAGESIZE", eSockErrorMessageSize);
	LuaSetGlobal(state, "GLSOCK_ERROR_NAMETOOLONG", eSockErrorNameTooLong);
	LuaSetGlobal(state, "GLSOCK_ERROR_NETWORKDOWN", eSockErrorNetworkDown);
	LuaSetGlobal(state, "GLSOCK_ERROR_NETWORKRESET", eSockErrorNetworkReset);
	LuaSetGlobal(state, "GLSOCK_ERROR_NETWORKUNREACHABLE", eSockErrorNetworkUnreachable);
	LuaSetGlobal(state, "GLSOCK_ERROR_NODESCRIPTORS", eSockErrorNoDescriptors);
	LuaSetGlobal(state, "GLSOCK_ERROR_NOBUFFERSPACE", eSockErrorNoBufferSpace);
	LuaSetGlobal(state, "GLSOCK_ERROR_NOMEMORY", eSockErrorNoMemory);
	LuaSetGlobal(state, "GLSOCK_ERROR_NOPERMISSION", eSockErrorNoPermission);
	LuaSetGlobal(state, "GLSOCK_ERROR_NOPROTOCOLOPTION", eSockErrorNoProtocolOption);
	LuaSetGlobal(state, "GLSOCK_ERROR_NOTCONNECTED", eSockErrorNotConnected);
	LuaSetGlobal(state, "GLSOCK_ERROR_NOTSOCKET", eSockErrorNotSocket);
	LuaSetGlobal(state, "GLSOCK_ERROR_OPERATIONABORTED", eSockErrorOperationAborted);
	LuaSetGlobal(state, "GLSOCK_ERROR_OPERATIONNOTSUPPORTED", eSockErrorOperationNotSupported);
	LuaSetGlobal(state, "GLSOCK_ERROR_SHUTDOWN", eSockErrorShutDown);
	LuaSetGlobal(state, "GLSOCK_ERROR_TIMEDOUT", eSockErrorTimedOut);
	LuaSetGlobal(state, "GLSOCK_ERROR_TRYAGAIN", eSockErrorTryAgain);
	LuaSetGlobal(state, "GLSOCK_ERROR_WOULDBLOCK", eSockErrorWouldBlock);
}

void Cleanup( lua_State* state )
{
	g_pSockMgr->CloseSockets();

	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );
	LUA->PushString("hook");
	LUA->RawGet(-2);
	LUA->PushString("Remove");
	LUA->RawGet(-2);
	LUA->PushString("Think");
	LUA->PushString("GLSockPolling");
	LUA->Call(2, 0);
	LUA->Pop();
}

} // GLSock

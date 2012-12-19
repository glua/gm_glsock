#include "Common.h"
#include "BindingGLSockBuffer.h"
#include "GLSockBuffer.h"
#include "BufferMgr.h"
#include "SockMgr.h"

namespace GLSockBuffer {

CGLSockBuffer* CheckBuffer(lua_State *state, int idx)
{
	LUA->CheckType(idx, GLSOCKBUFFER_TYPE);

	GarrysMod::Lua::UserData *pUserData = (GarrysMod::Lua::UserData*)LUA->GetUserdata(idx);
	if( !pUserData )
		return NULL;

	if( pUserData->type != GLSOCKBUFFER_TYPE )
		return NULL;

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>(pUserData->data);
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
		return NULL;

	return pBuffer;
}

void PushBuffer(lua_State *state, CGLSockBuffer *pBuffer)
{
	GarrysMod::Lua::UserData *pUserData = static_cast<GarrysMod::Lua::UserData*>(LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData)));
	int iRefUserData = LUA->ReferenceCreate();

	pUserData->type = GLSOCKBUFFER_TYPE;
	pUserData->data = static_cast<void*>(pBuffer);

	LUA->CreateMetaTableType(GLSOCKBUFFER_NAME, GLSOCKBUFFER_TYPE);
	int iRefMetatable = LUA->ReferenceCreate();

	LUA->ReferencePush(iRefUserData);
	LUA->ReferencePush(iRefMetatable);
	LUA->SetMetaTable(-2);

	LUA->ReferenceFree(iRefMetatable);
	LUA->ReferenceFree(iRefUserData);
}

static int __new(lua_State *state)
{
	CGLSockBuffer* pBuffer = g_pBufferMgr->Create();
	pBuffer->Reference();

	PushBuffer(state, pBuffer);

	return 1;
}

static int __delete(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	int nTableRef = pBuffer->m_nTableRef;
	if( pBuffer->Unreference() <= 0 )
	{
		LUA->ReferenceFree(nTableRef);
	}

	return 0;
}

static int __index(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	LUA->CreateMetaTableType(GLSOCKBUFFER_NAME, GLSOCKBUFFER_TYPE);
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

	if( pBuffer->m_nTableRef == 0 )
		return 0;

	LUA->ReferencePush(pBuffer->m_nTableRef);
	LUA->Push(2);
	LUA->RawGet(-2);

	return 1;
}

static int __newindex(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	if( pBuffer->m_nTableRef == 0 )
	{
		LUA->CreateTable();
		pBuffer->m_nTableRef = LUA->ReferenceCreate();
	}

	LUA->ReferencePush(pBuffer->m_nTableRef);
	LUA->Push(2);
	LUA->Push(3);
	LUA->RawSet(-3);

	return 0;
}

static int __tostring(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	std::stringstream ss;
	ss << "GLSockBuffer: " << (void*)pBuffer;

	std::string strType = ss.str();
	LUA->PushString(strType.c_str());

	return 1;
}

static int __eq(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	CGLSockBuffer* pBufferCmp = CheckBuffer(state, 2);
	if( !pBuffer )
		return 0;

	LUA->PushBool(pBuffer == pBufferCmp);
	return 1;
}

static int Write(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	LUA->CheckType(2, GarrysMod::Lua::Type::STRING);

	unsigned int nLen = 0;
	const char *pData = LUA->GetString(2, &nLen);

	LUA->PushNumber( pBuffer->Write(pData, nLen) );
	return 1;
}

static int Read(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER);

	unsigned int nReadBytes = static_cast<unsigned int>(LUA->GetNumber(2));
	
	char* szData = new char[nReadBytes + 1];
	unsigned int nRead = pBuffer->Read(szData, nReadBytes);
	szData[nRead] = '\0';

	if( nReadBytes > 0 )
	{
		LUA->PushNumber(static_cast<double>(nReadBytes));
		LUA->PushString(szData, nReadBytes);
	}
	else
	{
		LUA->PushNumber(0);
		LUA->PushNil();
	}

	delete[] szData;

	return 2;
}

static int WriteString(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	LUA->CheckType(2, GarrysMod::Lua::Type::STRING);

	const char *pData = LUA->GetString(2);

	LUA->PushNumber( static_cast<double>(pBuffer->Write(pData, strlen(pData) + 1)) );

	return 1;
}

static int ReadString(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;
	
	unsigned int uStringLength = 0;
	const char* pData = pBuffer->Buffer();
	bool bValid = false;

	for(unsigned int i = pBuffer->Tell(); i < pBuffer->Size(); i++)
	{
		if( pData[i] == '\0' )
		{
			bValid = true;
			break;
		}
		uStringLength++;
	}

	if( bValid )
	{
		// Copy string.
		LUA->PushNumber(static_cast<double>(uStringLength + 1));
		std::string strData( pData + pBuffer->Tell(), uStringLength );
		LUA->PushString(pData + pBuffer->Tell());

		// Update position.
		pBuffer->Seek(uStringLength + 1, SOCKBUFFER_SEEK_CUR);
	}
	else
	{
		LUA->PushNumber(0);
		LUA->PushNil();
	}

	return 2;
}

static int WriteDouble(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER);
	double nValue = LUA->GetNumber(2);

	if( LUA->Top() >= 3 )
	{
		LUA->CheckType(3, GarrysMod::Lua::Type::BOOL);

		bool bSwap = LUA->GetBool(3);
		if( bSwap )
			pBuffer->SwapEndian(nValue);
	}

	LUA->PushNumber( static_cast<double>(pBuffer->Write(nValue)) );
	return 1;
}

static int ReadDouble(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	double nValue = 0;
	unsigned int nRead = pBuffer->Read(nValue);

	if( LUA->Top() >= 2 )
	{
		LUA->CheckType(2, GarrysMod::Lua::Type::BOOL);

		bool bSwap = LUA->GetBool(2);
		if( bSwap )
			pBuffer->SwapEndian(nValue);
	}

	if( nRead == 0 )
	{
		LUA->PushNumber(0);
		LUA->PushNil();
	}
	else
	{
		LUA->PushNumber(static_cast<double>(nRead));
		LUA->PushNumber(static_cast<double>(nValue));
	}

	return 2;
}

static int WriteFloat(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER);

	float nValue = static_cast<float>(LUA->GetNumber(2));

	if( LUA->Top() >= 3 )
	{
		LUA->CheckType(3, GarrysMod::Lua::Type::BOOL);

		bool bSwap = LUA->GetBool(3);
		if( bSwap )
			pBuffer->SwapEndian(nValue);
	}

	LUA->PushNumber( static_cast<double>(pBuffer->Write(nValue)) );
	return 1;
}

static int ReadFloat(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	float nValue = 0;
	unsigned int nRead = pBuffer->Read(nValue);

	if( LUA->Top() >= 2 )
	{
		LUA->CheckType(2, GarrysMod::Lua::Type::BOOL);

		bool bSwap = LUA->GetBool(2);
		if( bSwap )
			pBuffer->SwapEndian(nValue);
	}

	if( nRead == 0 )
	{
		LUA->PushNumber(0);
		LUA->PushNil();
	}
	else
	{
		LUA->PushNumber(nRead);
		LUA->PushNumber(static_cast<double>(nValue));
	}

	return 2;
}

static int WriteLong(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER);

	unsigned int nValue = static_cast<unsigned int>(LUA->GetNumber(2));

	if( LUA->Top() >= 3 )
	{
		LUA->CheckType(3, GarrysMod::Lua::Type::BOOL);

		bool bSwap = LUA->GetBool(3);
		if( bSwap )
			pBuffer->SwapEndian(nValue);
	}

	LUA->PushNumber( static_cast<double>(pBuffer->Write(nValue)) );
	return 1;
}

static int ReadLong(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	unsigned int nValue = 0;
	unsigned int nRead = pBuffer->Read(nValue);

	if( LUA->Top() >= 2 )
	{
		LUA->CheckType(2, GarrysMod::Lua::Type::BOOL);

		bool bSwap = LUA->GetBool(2);
		if( bSwap )
			pBuffer->SwapEndian(nValue);
	}

	if( nRead == 0 )
	{
		LUA->PushNumber(0);
		LUA->PushNil();
	}
	else
	{
		LUA->PushNumber(static_cast<double>(nRead));
		LUA->PushNumber(static_cast<double>(nValue));
	}

	return 2;
}

static int WriteShort(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER);

	unsigned short nValue = static_cast<unsigned short>(LUA->GetNumber(2));

	if( LUA->Top() >= 3 )
	{
		LUA->CheckType(3, GarrysMod::Lua::Type::BOOL);

		bool bSwap = LUA->GetBool(3);
		if( bSwap )
			pBuffer->SwapEndian(nValue);
	}

	LUA->PushNumber( static_cast<double>(pBuffer->Write(nValue)) );
	return 1;
}

static int ReadShort(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	unsigned short nValue = 0;
	unsigned int nRead = pBuffer->Read(nValue);

	if( LUA->Top() >= 2 )
	{
		LUA->CheckType(2, GarrysMod::Lua::Type::BOOL);

		bool bSwap = LUA->GetBool(2);
		if( bSwap )
			pBuffer->SwapEndian(nValue);
	}

	if( nRead == 0 )
	{
		LUA->PushNumber(0);
		LUA->PushNil();
	}
	else
	{
		LUA->PushNumber(static_cast<double>(nRead));
		LUA->PushNumber(static_cast<double>(nValue));
	}

	return 2;
}

static int WriteByte(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER);
	
	unsigned char nValue = static_cast<unsigned char>(LUA->GetNumber(2));

	LUA->PushNumber( static_cast<double>(pBuffer->Write(nValue)) );
	return 1;
}

static int ReadByte(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	unsigned char nValue = 0;
	unsigned int nRead = pBuffer->Read(nValue);

	if( nRead == 0 )
	{
		LUA->PushNumber(0);
		LUA->PushNil();
	}
	else
	{
		LUA->PushNumber(static_cast<double>(nRead));
		LUA->PushNumber(static_cast<double>(nValue));
	}

	return 2;
}

static int Size(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	LUA->PushNumber(static_cast<double>(pBuffer->Size()));
	return 1;
}

static int Tell(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	LUA->PushNumber(static_cast<double>(pBuffer->Tell()));
	return 1;
}

static int Seek(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER);
	LUA->CheckType(3, GarrysMod::Lua::Type::NUMBER);

	unsigned int nPos = static_cast<unsigned int>(LUA->GetNumber(2));
	unsigned int nMethod = static_cast<unsigned int>(LUA->GetNumber(3));

	LUA->PushNumber( static_cast<double>(pBuffer->Seek(nPos, nMethod)) );
	return 1;
}

static int EOB(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	LUA->PushBool( pBuffer->EOB() );
	return 1;
}

static int Empty(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	LUA->PushBool( pBuffer->Empty() );
	return 1;
}

static int Clear(lua_State *state)
{
	CGLSockBuffer* pBuffer = CheckBuffer(state, 1);
	if( !pBuffer )
		return 0;

	LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER);

	unsigned int nPos = 0;
	unsigned int cSize = static_cast<unsigned int>(LUA->GetNumber(2));

	if( LUA->Top() >= 3 )
	{
		LUA->CheckType(3, GarrysMod::Lua::Type::NUMBER);
		nPos = static_cast<unsigned int>(LUA->GetNumber(3));
	}

	LUA->PushBool( pBuffer->Clear(nPos, cSize) );

	return 1;
}

void Startup( lua_State *state )
{
	LUA->CreateMetaTableType(GLSOCKBUFFER_NAME, GLSOCKBUFFER_TYPE);
	CAutoUnRef Metatable(state);
	{
		#define SetMember(k, v) LUA->PushString(k); LUA->PushCFunction(v); LUA->SetTable(-3);

		Metatable.Push();
		LUA->PushString("__functions");
		LUA->CreateTable();
		{
			SetMember("Write", GLSockBuffer::Write);
			SetMember("Read", GLSockBuffer::Read);

			SetMember("WriteString", GLSockBuffer::WriteString);
			SetMember("ReadString", GLSockBuffer::ReadString);
			SetMember("WriteDouble", GLSockBuffer::WriteDouble);
			SetMember("ReadDouble", GLSockBuffer::ReadDouble);
			SetMember("WriteFloat", GLSockBuffer::WriteFloat);
			SetMember("ReadFloat", GLSockBuffer::ReadFloat);
			SetMember("WriteLong", GLSockBuffer::WriteLong);
			SetMember("ReadLong", GLSockBuffer::ReadLong);
			SetMember("WriteShort", GLSockBuffer::WriteShort);
			SetMember("ReadShort", GLSockBuffer::ReadShort);
			SetMember("WriteByte", GLSockBuffer::WriteByte);
			SetMember("ReadByte", GLSockBuffer::ReadByte);

			SetMember("Size", GLSockBuffer::Size);
			SetMember("Tell", GLSockBuffer::Tell);
			SetMember("Seek", GLSockBuffer::Seek);
			SetMember("EOB", GLSockBuffer::EOB);
			SetMember("Empty", GLSockBuffer::Empty);
			SetMember("Clear", GLSockBuffer::Clear);
		}
		LUA->RawSet(-3);

		SetMember("__gc", GLSockBuffer::__delete);
		SetMember("__eq", GLSockBuffer::__eq);
		SetMember("__tostring", GLSockBuffer::__tostring);
		SetMember("__index", GLSockBuffer::__index);
		SetMember("__newindex", GLSockBuffer::__newindex);

		#undef SetMember
	}
	
	LuaSetGlobal(state, "GLSockBuffer", GLSockBuffer::__new);

	// Seek Methods
	LuaSetGlobal(state, "GLSOCKBUFFER_SEEK_SET", SOCKBUFFER_SEEK_SET);
	LuaSetGlobal(state, "GLSOCKBUFFER_SEEK_CUR", SOCKBUFFER_SEEK_CUR);
	LuaSetGlobal(state, "GLSOCKBUFFER_SEEK_END", SOCKBUFFER_SEEK_END);
}

void Cleanup( lua_State *state )
{
}

}

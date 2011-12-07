#include "Common.h"
#include "BindingGLSockBuffer.h"
#include "GLSockBuffer.h"
#include "BufferMgr.h"
#include "SockMgr.h"

namespace GLSockBuffer {

static int __new(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	CGLSockBuffer* pBuffer = g_pBufferMgr->Create();
	pBuffer->Reference();

	CAutoUnRef MetaTable = Lua()->GetMetaTable(GLSOCKBUFFER_NAME, GLSOCKBUFFER_TYPE);
	Lua()->PushUserData(MetaTable, static_cast<void*>(pBuffer));

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

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
		return 0;

	if( pBuffer->Unreference() <= 0 )
	{
		Lua()->FreeReference(pBuffer->m_nTableRef);
	}

	return 0;
}

static int __index(lua_State* L)
{
	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

#if defined(GetObject)
#undef GetObject
#endif

	ILuaObject* pMember = NULL;

	CAutoUnRef MetaTable = Lua()->GetMetaTable(GLSOCKBUFFER_NAME, GLSOCKBUFFER_TYPE);
	{
		ILuaObject* pFunctions = MetaTable->GetMember("__functions");
		pMember = pFunctions->GetMember(Lua()->GetObject(2));
		if( pMember )
		{
			Lua()->Push(pMember);
			return 1;
		}
	}

	if( pBuffer->m_nTableRef == 0 )
		return 0;

	Lua()->PushReference(pBuffer->m_nTableRef);
	CAutoUnRef Table(Lua()->GetObject(-1));
	{
		Lua()->Pop();
		pMember = Table->GetMember(Lua()->GetObject(2));
	}

	Lua()->Push(pMember);
	return 1;
}

static int __newindex(lua_State* L)
{
	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);

#if defined(GetObject)
#undef GetObject
#endif

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	if( pBuffer->m_nTableRef == 0 )
	{
		Lua()->NewTable();
		pBuffer->m_nTableRef = Lua()->GetReference(-1, true);
	}

	Lua()->PushReference(pBuffer->m_nTableRef);
	CAutoUnRef Table(Lua()->GetObject(-1));
	{
		Lua()->Pop();
		Table->SetMember(Lua()->GetObject(2), Lua()->GetObject(3));
	}

	return 0;
}

static int __tostring(lua_State* L)
{
	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	std::stringstream ss;
	ss << "GLSockBuffer: " << (void*)pBuffer;

	std::string strType = ss.str();
	Lua()->Push(strType.c_str());

	return 1;
}

static int __eq(lua_State* L)
{
	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);
	Lua()->CheckType(2, GLSOCKBUFFER_TYPE);

	Lua()->Push(Lua()->GetUserData(1) == Lua()->GetUserData(2));
	return 1;
}

static int Write(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);
	Lua()->CheckType(2, GLua::TYPE_STRING);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	if( Lua()->GetType(2) != GLua::TYPE_STRING )
		return 0;
	std::string strData( Lua()->GetString(2), Lua()->StringLength(2) );

	Lua()->PushLong( pBuffer->Write(strData.c_str(), strData.size()) );
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

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);
	Lua()->CheckType(2, GLua::TYPE_NUMBER);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	if( Lua()->GetType(2) != GLua::TYPE_NUMBER )
		return 0;
	unsigned int nReadBytes = Lua()->GetInteger(2);
	
	char* szData = new char[nReadBytes + 1];

	unsigned int nRead = pBuffer->Read(szData, nReadBytes);
	szData[nRead] = '\0';

	if( nReadBytes > 0 )
	{
		Lua()->PushLong(nReadBytes);
		Lua()->PushDataString(szData, nReadBytes);
	}
	else
	{
		Lua()->PushLong(0);
		Lua()->PushNil();
	}

	delete[] szData;

	return 2;
}

static int WriteString(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);
	Lua()->CheckType(2, GLua::TYPE_STRING);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	if( Lua()->GetType(2) != GLua::TYPE_STRING )
		return 0;
	std::string strData( Lua()->GetString(2) );

	Lua()->PushLong( pBuffer->Write(strData.c_str(), strData.size() + 1) );
	return 1;
}

static int ReadString(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);
	
	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

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
		Lua()->PushLong(uStringLength + 1);
		std::string strData( pData + pBuffer->Tell(), uStringLength );
		Lua()->Push(strData.c_str());

		// Update position.
		pBuffer->Seek(uStringLength + 1, SOCKBUFFER_SEEK_CUR);
	}
	else
	{
		Lua()->PushLong(0);
		Lua()->PushNil();
	}

	return 2;
}

static int WriteDouble(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);
	Lua()->CheckType(2, GLua::TYPE_NUMBER);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	if( Lua()->GetType(2) != GLua::TYPE_NUMBER )
		return 0;
	double nValue = Lua()->GetDouble(2);

	if( Lua()->GetStackTop() >= 3 )
	{
		Lua()->CheckType(3, GLua::TYPE_BOOL);
		if( Lua()->GetType(3) == GLua::TYPE_BOOL )
		{
			bool bSwap = Lua()->GetBool(3);
			if( bSwap )
				pBuffer->SwapEndian(nValue);
		}
	}

	Lua()->PushLong( pBuffer->Write(nValue) );
	return 1;
}

static int ReadDouble(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	double nValue = 0;
	unsigned int nRead = pBuffer->Read(nValue);

	if( Lua()->GetStackTop() >= 2 )
	{
		Lua()->CheckType(2, GLua::TYPE_BOOL);
		if( Lua()->GetType(2) == GLua::TYPE_BOOL )
		{
			bool bSwap = Lua()->GetBool(2);
			if( bSwap )
				pBuffer->SwapEndian(nValue);
		}
	}

	if( nRead == 0 )
	{
		Lua()->PushLong(0);
		Lua()->PushNil();
	}
	else
	{
		Lua()->PushLong(nRead);
		Lua()->PushDouble(nValue);
	}

	return 2;
}

static int WriteFloat(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);
	Lua()->CheckType(2, GLua::TYPE_NUMBER);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	if( Lua()->GetType(2) != GLua::TYPE_NUMBER )
		return 0;
	float nValue = Lua()->GetNumber(2);

	if( Lua()->GetStackTop() >= 3 )
	{
		Lua()->CheckType(3, GLua::TYPE_BOOL);
		if( Lua()->GetType(3) == GLua::TYPE_BOOL )
		{
			bool bSwap = Lua()->GetBool(3);
			if( bSwap )
				pBuffer->SwapEndian(nValue);
		}
	}

	Lua()->PushLong( pBuffer->Write(nValue) );
	return 1;
}

static int ReadFloat(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	float nValue = 0;
	unsigned int nRead = pBuffer->Read(nValue);

	if( Lua()->GetStackTop() >= 2 )
	{
		Lua()->CheckType(2, GLua::TYPE_BOOL);
		if( Lua()->GetType(2) == GLua::TYPE_BOOL )
		{
			bool bSwap = Lua()->GetBool(2);
			if( bSwap )
				pBuffer->SwapEndian(nValue);
		}
	}

	if( nRead == 0 )
	{
		Lua()->PushLong(0);
		Lua()->PushNil();
	}
	else
	{
		Lua()->PushLong(nRead);
		Lua()->Push(nValue);
	}

	return 2;
}

static int WriteLong(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);
	Lua()->CheckType(2, GLua::TYPE_NUMBER);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	if( Lua()->GetType(2) != GLua::TYPE_NUMBER )
		return 0;
	unsigned int nValue = Lua()->GetInteger(2);

	if( Lua()->GetStackTop() >= 3 )
	{
		Lua()->CheckType(3, GLua::TYPE_BOOL);
		if( Lua()->GetType(3) == GLua::TYPE_BOOL )
		{
			bool bSwap = Lua()->GetBool(3);
			if( bSwap )
				pBuffer->SwapEndian(nValue);
		}
	}

	Lua()->PushLong( pBuffer->Write(nValue) );
	return 1;
}

static int ReadLong(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	unsigned int nValue = 0;
	unsigned int nRead = pBuffer->Read(nValue);

	if( Lua()->GetStackTop() >= 2 )
	{
		Lua()->CheckType(2, GLua::TYPE_BOOL);
		if( Lua()->GetType(2) == GLua::TYPE_BOOL )
		{
			bool bSwap = Lua()->GetBool(2);
			if( bSwap )
				pBuffer->SwapEndian(nValue);
		}
	}

	if( nRead == 0 )
	{
		Lua()->PushLong(0);
		Lua()->PushNil();
	}
	else
	{
		Lua()->PushLong(nRead);
		Lua()->PushLong(nValue);
	}

	return 2;
}

static int WriteShort(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);
	Lua()->CheckType(2, GLua::TYPE_NUMBER);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	if( Lua()->GetType(2) != GLua::TYPE_NUMBER )
		return 0;
	unsigned short nValue = (unsigned short)Lua()->GetInteger(2);

	if( Lua()->GetStackTop() >= 3 )
	{
		Lua()->CheckType(3, GLua::TYPE_BOOL);
		if( Lua()->GetType(3) == GLua::TYPE_BOOL )
		{
			bool bSwap = Lua()->GetBool(3);
			if( bSwap )
				pBuffer->SwapEndian(nValue);
		}
	}

	Lua()->PushLong( pBuffer->Write(nValue) );
	return 1;
}

static int ReadShort(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	unsigned short nValue = 0;
	unsigned int nRead = pBuffer->Read(nValue);

	if( Lua()->GetStackTop() >= 2 )
	{
		Lua()->CheckType(2, GLua::TYPE_BOOL);
		if( Lua()->GetType(2) == GLua::TYPE_BOOL )
		{
			bool bSwap = Lua()->GetBool(2);
			if( bSwap )
				pBuffer->SwapEndian(nValue);
		}
	}

	if( nRead == 0 )
	{
		Lua()->PushLong(0);
		Lua()->PushNil();
	}
	else
	{
		Lua()->PushLong(nRead);
		Lua()->PushLong(nValue);
	}

	return 2;
}

static int WriteByte(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);
	Lua()->CheckType(2, GLua::TYPE_NUMBER);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	if( Lua()->GetType(2) != GLua::TYPE_NUMBER )
		return 0;
	unsigned char nValue = (unsigned char)Lua()->GetInteger(2);

	Lua()->PushLong( pBuffer->Write(nValue) );
	return 1;
}

static int ReadByte(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	unsigned char nValue = 0;
	unsigned int nRead = pBuffer->Read(nValue);

	if( nRead == 0 )
	{
		Lua()->PushLong(0);
		Lua()->PushNil();
	}
	else
	{
		Lua()->PushLong(nRead);
		Lua()->PushLong(nValue);
	}

	return 2;
}

static int Size(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	Lua()->PushLong(pBuffer->Size());
	return 1;
}

static int Tell(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	Lua()->PushLong(pBuffer->Tell());
	return 1;
}

static int Seek(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);
	Lua()->CheckType(2, GLua::TYPE_NUMBER);
	Lua()->CheckType(3, GLua::TYPE_NUMBER);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	if( Lua()->GetType(2) != GLua::TYPE_NUMBER )
		return 0;
	unsigned int nPos = Lua()->GetInteger(2);

	if( Lua()->GetType(3) != GLua::TYPE_NUMBER )
		return 0;
	unsigned int nMethod = Lua()->GetInteger(3);

	Lua()->Push( pBuffer->Seek(nPos, nMethod) );
	return 1;
}

static int EOB(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	Lua()->Push( pBuffer->EOB() );
	return 1;
}

static int Empty(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	Lua()->Push( pBuffer->Empty() );
	return 1;
}

static int Clear(lua_State* L)
{
	// SCOPED_LOCK(g_pSockMgr->Mutex());
	if( !L )
		return 0;

#if defined(_DEBUG)
	Lua()->Msg("%s()\n", __FUNCTION__);
#endif

	Lua()->CheckType(1, GLSOCKBUFFER_TYPE);
	Lua()->CheckType(2, GLua::TYPE_NUMBER);
	Lua()->CheckType(3, GLua::TYPE_NUMBER);

	CGLSockBuffer* pBuffer = reinterpret_cast<CGLSockBuffer*>( Lua()->GetUserData(1) );
	if( !g_pBufferMgr->ValidHandle(pBuffer) )
	{
		Lua()->LuaError("Invalid buffer handle", 1);
		return 0;
	}

	if( Lua()->GetType(2) != GLua::TYPE_NUMBER )
		return 0;
	unsigned int nPos = Lua()->GetInteger(2);

	if( Lua()->GetType(3) != GLua::TYPE_NUMBER )
		return 0;
	unsigned int cSize = Lua()->GetInteger(3);

	Lua()->Push( pBuffer->Clear(nPos, cSize) );

	return 1;
}

void Startup( lua_State* L )
{
	CAutoUnRef MetaTable = Lua()->GetMetaTable(GLSOCKBUFFER_NAME, GLSOCKBUFFER_TYPE);
	{
		CAutoUnRef Index = Lua()->GetNewTable();

		Index->SetMember("Write", GLSockBuffer::Write);
		Index->SetMember("Read", GLSockBuffer::Read);

		Index->SetMember("WriteString", GLSockBuffer::WriteString);
		Index->SetMember("ReadString", GLSockBuffer::ReadString);
		Index->SetMember("WriteDouble", GLSockBuffer::WriteDouble);
		Index->SetMember("ReadDouble", GLSockBuffer::ReadDouble);
		Index->SetMember("WriteFloat", GLSockBuffer::WriteFloat);
		Index->SetMember("ReadFloat", GLSockBuffer::ReadFloat);
		Index->SetMember("WriteLong", GLSockBuffer::WriteLong);
		Index->SetMember("ReadLong", GLSockBuffer::ReadLong);
		Index->SetMember("WriteShort", GLSockBuffer::WriteShort);
		Index->SetMember("ReadShort", GLSockBuffer::ReadShort);
		Index->SetMember("WriteByte", GLSockBuffer::WriteByte);
		Index->SetMember("ReadByte", GLSockBuffer::ReadByte);

		Index->SetMember("Size", GLSockBuffer::Size);
		Index->SetMember("Tell", GLSockBuffer::Tell);
		Index->SetMember("Seek", GLSockBuffer::Seek);
		Index->SetMember("EOB", GLSockBuffer::EOB);
		Index->SetMember("Empty", GLSockBuffer::Empty);
		Index->SetMember("Clear", GLSockBuffer::Clear);

		MetaTable->SetMember("__gc", GLSockBuffer::__delete);
		MetaTable->SetMember("__eq", GLSockBuffer::__eq);
		MetaTable->SetMember("__tostring", GLSockBuffer::__tostring);
		MetaTable->SetMember("__index", GLSockBuffer::__index);
		MetaTable->SetMember("__newindex", GLSockBuffer::__newindex);
		MetaTable->SetMember("__functions", Index);
	}

	Lua()->SetGlobal("GLSockBuffer", GLSockBuffer::__new);

	// Seek Methods
	Lua()->SetGlobal("GLSOCKBUFFER_SEEK_SET", (float)SOCKBUFFER_SEEK_SET);
	Lua()->SetGlobal("GLSOCKBUFFER_SEEK_CUR", (float)SOCKBUFFER_SEEK_CUR);
	Lua()->SetGlobal("GLSOCKBUFFER_SEEK_END", (float)SOCKBUFFER_SEEK_END);
}

void Cleanup( lua_State* L )
{
}

}

#ifndef LUAHELPER_H
#define LUAHELPER_H

#ifdef _MSC_VER
#pragma once
#endif 

#include "Common.h"

class CAutoUnRef
{
private:
	lua_State *state;
	int _Ref;
public:
	CAutoUnRef(lua_State *L)
	{
		state = L;
		_Ref = LUA->ReferenceCreate();
	}
	~CAutoUnRef()
	{
		LUA->ReferenceFree(_Ref);
	}
	void Push()
	{
		LUA->ReferencePush(_Ref);
	}
};

inline void LuaPush(lua_State *state, const char *val)
{
	LUA->PushString(val);
}

inline void LuaPush(lua_State *state, unsigned int val)
{
	LUA->PushNumber(static_cast<double>(val));
}

inline void LuaPush(lua_State *state, int val)
{
	LUA->PushNumber(static_cast<double>(val));
}

inline void LuaPush(lua_State *state, float val)
{
	LUA->PushNumber(static_cast<double>(val));
}

inline void LuaPush(lua_State *state, double val)
{
	LUA->PushNumber(val);
}

inline void LuaPush(lua_State *state, bool val)
{
	LUA->PushBool(val);
}

inline void LuaPush(lua_State *state, GarrysMod::Lua::CFunc val)
{
	LUA->PushCFunction(val);
}

template<typename T, typename V>
inline void LuaSetGlobal(lua_State *state, const T key, const V val)
{
	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
	LuaPush(state, key);
	LuaPush(state, val);
	LUA->RawSet(-3);
	LUA->Pop();
}

#endif // LUAHELPER_H
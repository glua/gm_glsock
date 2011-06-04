#if !defined(PROXYGLSOCK_H)
#define PROXYGLSOCK_H

#if defined(WIN32)
#pragma once
#endif

#define NO_SDK
#include "GMLuaModule.h"

#include "Lock.h"

namespace GLSock {

void Startup(lua_State* L);
void Cleanup(lua_State* L);

} // GLSock

#endif // PROXYGLSOCK_H
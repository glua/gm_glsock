#if !defined(PROXYGLSOCK_H)
#define PROXYGLSOCK_H

#if defined(WIN32)
#pragma once
#endif

#include "Common.h"
#include "GLSock.h"

namespace GLSock {

CGLSock* CheckSocket(lua_State *state, int idx);
void PushSocket(lua_State *state, CGLSock *pSock);

void Startup(lua_State *state);
void Cleanup(lua_State *state);

} // GLSock

#endif // PROXYGLSOCK_H

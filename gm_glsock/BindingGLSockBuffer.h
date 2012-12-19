#if !defined(PROXYGLSOCKBUFFER_H)
#define PROXYGLSOCKBUFFER_H

#if defined(WIN32)
#pragma once
#endif

#include "Common.h"
#include "BufferMgr.h"

namespace GLSockBuffer {

class CGLSockBuffer;

CGLSockBuffer* CheckBuffer(lua_State *state, int idx);
void PushBuffer(lua_State *state, CGLSockBuffer *pBuffer);

void Startup( lua_State *state );
void Cleanup( lua_State *state );

}

#endif // PROXYGLSOCKBUFFER_H

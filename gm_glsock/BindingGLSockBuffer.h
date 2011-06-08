#if !defined(PROXYGLSOCKBUFFER_H)
#define PROXYGLSOCKBUFFER_H

#if defined(WIN32)
#pragma once
#endif

#define NO_SDK
#include "GMLuaModule.h"

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include "Lock.h"

namespace GLSockBuffer {

void Startup( lua_State* L );
void Cleanup( lua_State* L );

}

#endif // PROXYGLSOCKBUFFER_H

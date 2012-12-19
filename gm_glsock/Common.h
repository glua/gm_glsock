#if !defined(GLSOCK_COMMON_H)
#define GLSOCK_COMMON_H

#if defined(_WIN32)
#pragma once
#endif

#define UNREFERENCED_PARAM(x) x

#include "Lua/LuaBase.h"
#include "Lua/Interface.h"
#include "Lua/Types.h"
#include "Lua/UserData.h"
#include "LuaHelper.h"

#include "Lock.h"

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <sstream>
#include <vector>
#include <string>

#endif // GLSOCK_COMMON_H

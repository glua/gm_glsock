#define NO_SDK

// Boost
#include <boost/asio.hpp>
#include <boost/thread.hpp>

// GMod
#include "GMLuaModule.h"

#include "BindingGLSock.h"
#include "BindingGLSockBuffer.h"

GMOD_MODULE(Startup, Cleanup);

int Startup(lua_State* L)
{
	GLSock::Startup(L);
	GLSockBuffer::Startup(L);

	return 0;
}

int Cleanup(lua_State* L)
{
	GLSock::Cleanup(L);
	GLSockBuffer::Cleanup(L);

	return 0;
}

/*
#if defined(WIN32)
int WINAPI DllMain(_In_ HANDLE _HDllHandle, _In_ DWORD _Reason, _In_opt_ LPVOID _Reserved)
{
	switch(_Reason)
	{
	case DLL_PROCESS_DETACH:
		{
			// Debug Purpose, ignore me
		}
		break;
	}
	return TRUE;
}
#endif
*/

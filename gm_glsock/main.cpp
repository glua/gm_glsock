#define NO_SDK

// Boost
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "BindingGLSock.h"
#include "BindingGLSockBuffer.h"
#include "SockMgr.h"

GMOD_MODULE_OPEN()
{
	g_pSockMgr->Startup();

	GLSock::Startup(state);
	GLSockBuffer::Startup(state);

	return 0;
}

GMOD_MODULE_CLOSE()
{
	GLSock::Cleanup(state);
	GLSockBuffer::Cleanup(state);

	g_pSockMgr->Cleanup();

	return 0;
}
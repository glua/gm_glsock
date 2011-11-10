#if !defined(BUFFERMGR_H)
#define BUFFERMGR_H

#if defined(WIN32)
#pragma once
#endif

#include "GLSockBuffer.h"
#include <vector>

#include <boost/thread.hpp>

class CBufferMgr
{
	typedef boost::recursive_mutex Mutex_t;

private:
	std::vector<GLSockBuffer::CGLSockBuffer*> m_vecBuffer;
	Mutex_t m_Mutex;

public:
	~CBufferMgr(void);

	GLSockBuffer::CGLSockBuffer* Create(void);
	GLSockBuffer::CGLSockBuffer* Create(const char* pData, unsigned int cubData);

	bool Remove(GLSockBuffer::CGLSockBuffer* pBuffer);
	bool ValidHandle(GLSockBuffer::CGLSockBuffer* pBuffer);
};

extern CBufferMgr* g_pBufferMgr;

#endif // BUFFERMGR_H

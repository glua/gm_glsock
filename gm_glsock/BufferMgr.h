#if !defined(BUFFERMGR_H)
#define BUFFERMGR_H

#if defined(WIN32)
#pragma once
#endif

#include "GLSockBuffer.h"

#include <vector>

class CBufferMgr
{
private:
	std::vector<GLSockBuffer::CGLSockBuffer*> m_vecBuffer;

public:
	~CBufferMgr(void);

	GLSockBuffer::CGLSockBuffer* Create(void);
	GLSockBuffer::CGLSockBuffer* Create(const char* pData, unsigned int cubData);

	bool Remove(GLSockBuffer::CGLSockBuffer* pBuffer);
	bool ValidHandle(GLSockBuffer::CGLSockBuffer* pBuffer);
};

extern CBufferMgr* g_pBufferMgr;

#endif // BUFFERMGR_H

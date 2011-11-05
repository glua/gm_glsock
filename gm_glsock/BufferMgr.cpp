#include "Common.h"
#include "BufferMgr.h"

static CBufferMgr s_BufferMgr;
CBufferMgr* g_pBufferMgr = &s_BufferMgr;

CBufferMgr::~CBufferMgr( void )
{
	std::vector<GLSockBuffer::CGLSockBuffer*>::iterator itr;
	for( itr = m_vecBuffer.begin(); itr != m_vecBuffer.end(); itr++ )
		delete *itr;

	m_vecBuffer.clear();
}

GLSockBuffer::CGLSockBuffer* CBufferMgr::Create( void )
{
	GLSockBuffer::CGLSockBuffer* pBuffer = new GLSockBuffer::CGLSockBuffer();
	m_vecBuffer.push_back(pBuffer);

	return pBuffer;
}

GLSockBuffer::CGLSockBuffer* CBufferMgr::Create( const char* pData, unsigned int cubData )
{
	GLSockBuffer::CGLSockBuffer* pBuffer = new GLSockBuffer::CGLSockBuffer(pData, cubData);
	m_vecBuffer.push_back(pBuffer);

	return pBuffer;
}

bool CBufferMgr::Remove( GLSockBuffer::CGLSockBuffer* pBuffer )
{
	std::vector<GLSockBuffer::CGLSockBuffer*>::iterator itr;
	for( itr = m_vecBuffer.begin(); itr != m_vecBuffer.end(); itr++ )
	{
		if( *itr == pBuffer )
		{
			m_vecBuffer.erase(itr);
			return true;
		}
	}

	return false;
}

bool CBufferMgr::ValidHandle( GLSockBuffer::CGLSockBuffer* pBuffer )
{
	std::vector<GLSockBuffer::CGLSockBuffer*>::iterator itr;
	for( itr = m_vecBuffer.begin(); itr != m_vecBuffer.end(); itr++ )
	{
		if( *itr == pBuffer )
			return true;
	}

	return false;
}

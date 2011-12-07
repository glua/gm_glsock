#include "Common.h"
#include "BufferMgr.h"

static CBufferMgr s_BufferMgr;
CBufferMgr* g_pBufferMgr = &s_BufferMgr;

CBufferMgr::~CBufferMgr( void )
{
	std::vector<GLSockBuffer::CGLSockBuffer*>::iterator itr;
	for( itr = m_vecBuffer.begin(); itr != m_vecBuffer.end(); itr++ )
		delete *itr;
}

GLSockBuffer::CGLSockBuffer* CBufferMgr::Create( void )
{
	Mutex_t::scoped_lock lock(m_Mutex);

	GLSockBuffer::CGLSockBuffer* pBuffer = new GLSockBuffer::CGLSockBuffer();
	pBuffer->m_nTableRef = 0;
	m_vecBuffer.push_back(pBuffer);

	return pBuffer;
}

GLSockBuffer::CGLSockBuffer* CBufferMgr::Create( const char* pData, unsigned int cubData )
{
	Mutex_t::scoped_lock lock(m_Mutex);

	GLSockBuffer::CGLSockBuffer* pBuffer = new GLSockBuffer::CGLSockBuffer(pData, cubData);
	pBuffer->m_nTableRef = 0;
	m_vecBuffer.push_back(pBuffer);

	return pBuffer;
}

bool CBufferMgr::Remove( GLSockBuffer::CGLSockBuffer* pBuffer )
{
	Mutex_t::scoped_lock lock(m_Mutex);

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
	Mutex_t::scoped_lock lock(m_Mutex);

	std::vector<GLSockBuffer::CGLSockBuffer*>::iterator itr;
	for( itr = m_vecBuffer.begin(); itr != m_vecBuffer.end(); itr++ )
	{
		if( *itr == pBuffer )
			return true;
	}

	return false;
}

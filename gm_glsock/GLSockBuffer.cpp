#include "GLSockBuffer.h"
#include "BufferMgr.h"

namespace GLSockBuffer {

CGLSockBuffer::CGLSockBuffer( const char* pData, unsigned int cubBuffer )
{
	m_pData = (char*)pData;
	m_cubData = cubBuffer;
	m_nOffset = 0;
	m_fOwner = false;	// Won't delete on dtor
	m_nReferences = 0;
}

CGLSockBuffer::CGLSockBuffer( void )
{
	m_pData = 0;
	m_cubData = 0;
	m_nOffset = 0;
	m_fOwner = true;	// Will delete on dtor
	m_nReferences = 0;
}

CGLSockBuffer::~CGLSockBuffer( void )
{
	if( m_fOwner )
	{
		if( m_pData )
			delete[] m_pData;
	}
}

unsigned int CGLSockBuffer::Write( const char* pData, unsigned int cubBuffer )
{
	if( !m_fOwner )
		return 0;

	if( !m_pData )
	{
		m_pData = new char[cubBuffer + 1];
		m_cubData = cubBuffer;
	}
	else
	{
		unsigned int nSpaceAvailable = m_cubData - m_nOffset;
		unsigned int nAllocSize = cubBuffer;

		if( nSpaceAvailable > cubBuffer )
			nAllocSize = 0;
		else
			nAllocSize = cubBuffer - nSpaceAvailable;

		if( nAllocSize > 0 )
		{
			char* pBuffer = new char[m_cubData + nAllocSize + 1];

			for(unsigned int i = 0; i < m_cubData; i++)
				pBuffer[i] = m_pData[i];

			delete[] m_pData;
			m_pData = pBuffer;

			m_cubData += nAllocSize;
		}
	}

	for(unsigned int i = 0; i < cubBuffer; i++)
		m_pData[m_nOffset + i] = pData[i];

	m_nOffset += cubBuffer;
	m_pData[m_cubData] = '\0';	// Terminate by 0

	return cubBuffer;
}

unsigned int CGLSockBuffer::Read( char* pData, unsigned int cubBuffer, bool bMatchSize)
{
	if( !m_pData )
		return 0;

	unsigned int nRead = cubBuffer;
	if( m_cubData - m_nOffset < nRead )
	{
		if( bMatchSize )
			return 0;

		nRead = m_cubData - m_nOffset;
	}

	for(unsigned int i = 0; i < nRead; i++)
		pData[i] = m_pData[m_nOffset + i];

	m_nOffset += nRead;
	return nRead;
}

bool CGLSockBuffer::Seek( unsigned int nOffset, unsigned int nMethod )
{
	switch(nMethod)
	{
	case SOCKBUFFER_SEEK_SET:
		{
			if( nOffset > m_cubData )
				return false;

			m_nOffset = nOffset;
		}
		break;
	case SOCKBUFFER_SEEK_CUR:
		{
			if( m_nOffset + nOffset > m_cubData )
				return false;

			m_nOffset += nOffset;
		}
		break;
	case SOCKBUFFER_SEEK_END:
		{
			if( nOffset > m_cubData )
				return false;

			m_nOffset = m_cubData - nOffset;
		}
		break;
	default:
		return false;
	}

	return true;
}

unsigned int CGLSockBuffer::Tell( void )
{
	return m_nOffset;
}

unsigned int CGLSockBuffer::Size( void )
{
	return m_cubData;
}

const char* CGLSockBuffer::Buffer( void )
{
	return (const char*)m_pData;
}

bool CGLSockBuffer::EOB()
{
	return (m_nOffset >= m_cubData);
}

bool CGLSockBuffer::Empty()
{
	return m_cubData == 0;
}

void CGLSockBuffer::Reference()
{
	m_nReferences++;
}

void CGLSockBuffer::Unreference()
{
	m_nReferences--;
	if( m_nReferences <= 0 )
	{
		g_pBufferMgr->Remove(this);
		delete this;
	}
}

} // GLSockBuffer
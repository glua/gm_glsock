#if !defined(GLSOCKBUFFER_H)
#define GLSOCKBUFFER_H

#if defined(WIN32)
#pragma once
#endif

#define SOCKBUFFER_SEEK_SET		1
#define SOCKBUFFER_SEEK_END		2
#define SOCKBUFFER_SEEK_CUR		3

#define GLSOCKBUFFER_NAME		"GLSockBuffer"
#define GLSOCKBUFFER_TYPE		0x93841 + 10

namespace GLSockBuffer {

class CGLSockBuffer
{
private:
	char* m_pData;
	unsigned int m_cubData;
	unsigned int m_nOffset;
	bool m_fOwner;
	int m_nReferences;

public:
	CGLSockBuffer(const char* pData, unsigned int cubBuffer);
	CGLSockBuffer(void);
	~CGLSockBuffer(void);

	unsigned int Write(const char* pData, unsigned int cubBuffer);
	unsigned int Read(char* pData, unsigned int cubBuffer, bool bMatchSize = false);

	template<typename T>
	unsigned int Write(const T& Value)
	{
		return Write((const char*)&Value, sizeof(T));
	}

	template<typename T>
	unsigned int Read(T& Value)
	{
		return Read((char*)&Value, sizeof(T), true);
	}

	bool Seek(unsigned int nOffset, unsigned int nMethod);
	unsigned int Tell(void);
	unsigned int Size(void);

	const char* Buffer(void);

	bool EOB();
	bool Empty();

	void Reference();
	void Unreference();
};

} // GLSockBuffer

#endif // GLSOCKBUFFER_H

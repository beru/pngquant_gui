#pragma once

#include "IFile.h"

class MemoryFile : public IFile
{
public:
	MemoryFile(void* pBuff, size_t buffSize);

	bool Read(void* pBuffer, size_t nNumberOfBytesToRead, size_t& nNumberOfBytesRead);
	
	bool Write(const void* pBuffer, size_t nNumberOfBytesToWrite, size_t& nNumberOfBytesWritten);
	
	size_t Seek(long lDistanceToMove, size_t dwMoveMethod);
	
	size_t Tell() const;
	
	size_t Size() const;
	
	bool Flush();

	bool HasBuffer() const { return true; }
	virtual const void* GetBuffer() const { return pBuff_; }
	
private:
	void* pBuff_;
	DWORD buffSize_;
	
	DWORD curPos_;
};


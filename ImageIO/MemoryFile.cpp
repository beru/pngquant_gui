#include "stdafx.h"
#include "MemoryFile.h"

#include <assert.h>

MemoryFile::MemoryFile(void* pBuff, size_t buffSize)
	:
	pBuff_(pBuff),
	buffSize_(buffSize)
{
}

bool MemoryFile::Read(void* pBuffer, size_t nNumberOfBytesToRead, size_t& nNumberOfBytesRead)
{
	DWORD fileSize = Size();
	nNumberOfBytesRead = (fileSize < curPos_+nNumberOfBytesToRead) ? (fileSize - curPos_) : nNumberOfBytesToRead;
	memcpy(pBuffer, (BYTE*)pBuff_+curPos_, nNumberOfBytesRead);
	Seek(nNumberOfBytesRead, FILE_CURRENT);
	return true;
}

bool MemoryFile::Write(const void* pBuffer, size_t nNumberOfBytesToWrite, size_t& nNumberOfBytesWritten)
{
	memcpy((BYTE*)pBuff_+curPos_, pBuffer, nNumberOfBytesToWrite);
	nNumberOfBytesWritten = nNumberOfBytesToWrite;
	Seek(nNumberOfBytesToWrite, FILE_CURRENT);
	return true;
}

size_t MemoryFile::Seek(long lDistanceToMove, size_t dwMoveMethod)
{
	switch (dwMoveMethod) {
	case FILE_BEGIN:
		curPos_ = lDistanceToMove;
		break;
	case FILE_CURRENT:
		curPos_ += lDistanceToMove;
		break;
	case FILE_END:
		curPos_ += buffSize_ + lDistanceToMove;
		break;
	}
	assert(0 <= curPos_ && curPos_ <= buffSize_);
	return curPos_;
}

size_t MemoryFile::Tell() const
{
	return curPos_;
}

size_t MemoryFile::Size() const
{
	return buffSize_;
}

bool MemoryFile::Flush()
{
	return true;
}


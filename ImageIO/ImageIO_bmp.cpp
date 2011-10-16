#include "stdafx.h"
#include "ImageIO_bmp.h"

#include <stdio.h>
#define TRACE printf
#include <assert.h>

#include <windows.h>
#include "arrayutil.h"

static
bool Read_BITMAPINFOHEADER(IFile& file, BITMAPINFOHEADER& bmih)
{
	size_t fileSize = file.Size();
	if (fileSize <= 54) {
		TRACE("file size <= 54 bytes.");
		return false;
	}
	BITMAPFILEHEADER bmpFileHeader;
	size_t readBytes = 0;
	file.Read(&bmpFileHeader, 14, readBytes);
	assert(readBytes == 14);
	if (bmpFileHeader.bfType != 19778) {
		TRACE("file hedear bfType != 19778");
		return false;
	}
	
	file.Read(&bmih, 40, readBytes);
	if (readBytes != 40) {
		TRACE("bmih shortage.");
		return false;
	}
	
	return true;
}

static
bool getImageInfo(ImageInfo& info, const BITMAPINFOHEADER& bmih)
{
	info.width = bmih.biWidth;
	info.height = bmih.biHeight;
	switch (bmih.biBitCount) {
	case 1:
	case 2:
	case 4:
		return false;
		break;
	case 8:
		info.bitsPerSample = 8;
		info.samplesPerPixel = 1;
		break;
	case 16:
		return false;
	case 24:
		info.bitsPerSample = 8;
		info.samplesPerPixel = 3;
		break;
	case 32:
		info.bitsPerSample = 8;
		info.samplesPerPixel = 4;
		break;
	}
	return true;
}

bool ReadImageInfo_bmp(IFile& file, ImageInfo& info)
{
	BITMAPINFOHEADER bmih;
	if (!Read_BITMAPINFOHEADER(file, bmih)) {
		return false;
	}
	if (!getImageInfo(info, bmih)) {
		return false;
	}
	return true;
}

bool ReadImage_bmp(IFile& file, ImageInfo& info, void* dest, int lineOffset, void* palettes)
{
	file.Seek(0, FILE_BEGIN);

	BITMAPINFOHEADER bmih;
	if (!Read_BITMAPINFOHEADER(file, bmih)) {
		return false;
	}
	if (!getImageInfo(info, bmih)) {
		return false;
	}
	size_t bmiColorsLen = 0;
	switch (bmih.biBitCount) {
	case 1:
		bmiColorsLen = 2;
		break;
	case 4:
		bmiColorsLen = 16;
		break;
	case 8:
		bmiColorsLen = 256;
		break;
	case 16:
		TRACE("16 bit BMP not supported.");
		return false;
		break;
	case 32:
		if (bmih.biCompression == BI_BITFIELDS) {
			bmiColorsLen = 3;
		}
	}
	size_t readBytes = 0;
	if (bmiColorsLen) {
		size_t needBytes = /*sizeof(RGBQUAD)*/4*bmiColorsLen;
		file.Read(palettes, needBytes, readBytes);
		if (readBytes != needBytes) {
			TRACE("bmiColors read failed.");
			return false;
		}
	}
	const int lineBytes = ((((bmih.biWidth * bmih.biBitCount) + 31) & ~31) >> 3);
	const int lineIdx = (bmih.biHeight < 0) ? 0 : (bmih.biHeight-1);
	OffsetPtr(dest, lineOffset * lineIdx);
	lineOffset *= (bmih.biHeight < 0) ? 1 : -1;

	const size_t height = abs(bmih.biHeight);
	for (size_t y=0; y<height; ++y) {
		file.Read(dest, lineBytes, readBytes);
		if (readBytes != lineBytes) {
			TRACE("read bytes != lineBytes.");
			return false;
		}
		OffsetPtr(dest, lineOffset);
	}
	
	return true;
}

bool WriteImage_bmp(IFile& file, const ImageInfo& info, const void* data, int lineOffset)
{
	file.Seek(0, FILE_BEGIN);
	
	if (info.bitsPerSample % 8) {
		return false;
	}
	
	const size_t bitsPerPixel = info.bitsPerSample * info.samplesPerPixel;
	const size_t imageSize = info.width * info.height * (bitsPerPixel / 8);
	size_t nBytesWritten;
	{
		BITMAPFILEHEADER bfh;
		bfh.bfType = 19778;	// BM
		bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + imageSize;
		bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		file.Write(&bfh, sizeof(bfh), nBytesWritten);
	}
	
	{
		BITMAPINFOHEADER bmih;
		bmih.biSize = sizeof(bmih);
		bmih.biWidth = info.width;
		bmih.biHeight = info.height;
		bmih.biPlanes = 1;
		bmih.biBitCount = bitsPerPixel;
		bmih.biCompression = BI_RGB;
		bmih.biSize = imageSize;
		bmih.biXPelsPerMeter = bmih.biYPelsPerMeter = 0;
		bmih.biClrUsed = 0;
		bmih.biClrImportant = 0;
		file.Write(&bmih, sizeof(bmih), nBytesWritten);
	}
	
	const void* pLine = ((const uint8_t*)data + lineOffset * (info.height-1));
	const size_t lineBytes = ((((info.width * bitsPerPixel) + 31) & ~31) >> 3);
	for (size_t y=0; y<info.height; ++y) {
		file.Write(pLine, lineBytes, nBytesWritten);
		if (nBytesWritten != lineBytes) {
			TRACE("nBytesWritten != lineBytes.");
			return false;
		}
		OffsetPtr(pLine, -lineOffset);
	}
	
	return true;

}


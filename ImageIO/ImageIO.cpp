#include "stdafx.h"

#include "ImageIO_bmp.h"

bool ReadImageInfo(IFile& file, ImageInfo& info)
{
	return ReadImageInfo_bmp(file, info);
}

bool ReadImage(IFile& file, ImageInfo& info, void* dest, int lineOffset, void* palettes)
{
	return ReadImage_bmp(file, info, dest, lineOffset, palettes);
}

bool WriteImage(IFile& file, const ImageInfo& info, const void* data, int lineOffset)
{
	return WriteImage_bmp(file, info, data, lineOffset);
}


#pragma once

#include "IFile.h"

enum ImageFileFormatType {
	ImageFileFormatType_Unknown,
	ImageFileFormatType_BMP,
	ImageFileFormatType_PNG,
};

struct ImageInfo
{
	ImageFileFormatType fileFormatType;
	size_t width;
	size_t height;
	size_t bitsPerSample;
	size_t samplesPerPixel;
};

bool ReadImageInfo(IFile& file, ImageInfo& info);
bool ReadImage(IFile& file, ImageInfo& info, void* dest, int lineOffset, void* palettes);

bool WriteImage(IFile& file, const ImageInfo& info, const void* data, int lineOffset);


#pragma once

#include "ImageIO.h"

bool ReadImageInfo_bmp(IFile& file, ImageInfo& info);
bool ReadImage_bmp(IFile& file, ImageInfo& info, void* dest, int lineOffset, void* palettes);

bool WriteImage_bmp(IFile& file, const ImageInfo& info, const void* data, int lineOffset);


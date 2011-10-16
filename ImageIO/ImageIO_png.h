#pragma once

#include "ImageIO.h"

bool ReadImageInfo_png(IFile& file, ImageInfo& info);
bool ReadImage_png(IFile& file, ImageInfo& info, void* dest, int lineOffset, void* palettes);

bool WriteImage_png(IFile& file, const ImageInfo& info, const void* data, int lineOffset);


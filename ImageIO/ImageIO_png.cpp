#include "stdafx.h"
#include "ImageIO_png.h"

#include "../libpng/png.h"
#include <vector>

namespace {

struct Reader
{
	Reader(IFile& file)
		:
		file_(file)
	{
		file.Seek(0, FILE_BEGIN);
	}
	
	void Read(uint8_t* data, size_t readSize)
	{
		size_t nBytesRead = 0;
		file_.Read(data, readSize, nBytesRead);
	}
	
	IFile& file_;
};

static void ReadDataFunc(png_structp png_ptr, png_bytep data, png_size_t length)
{
	Reader* pReader = (Reader*) png_get_io_ptr(png_ptr);
	pReader->Read(data, length);
}

bool sys_is_little_endian(){
#if defined(__LITTLE_ENDIAN__)
	return true;
#elif defined(__BIG_ENDIAN__)
	return false;
#else
	int i = 1;
	return (bool)(*(char*)&i);
#endif
}

void parseInfo(
	IFile& file,
	ImageInfo& info,
	png_structp pPNG,
	png_infop pINFO
	)
{
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_method;
	int compression_method, filter_method;
	png_get_IHDR(
		pPNG, pINFO,
		&width, &height, &bit_depth, &color_type,
		&interlace_method, &compression_method, &filter_method
	);
	
	png_set_bgr(pPNG);
	bool hasAlpha = false;
	bool isGrayscale = false;
	
	switch (color_type) {
	case PNG_COLOR_TYPE_GRAY:
		isGrayscale = true;
		if (bit_depth < 8) {
			png_set_expand(pPNG);
		}
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		hasAlpha = true;
		break;
	case PNG_COLOR_TYPE_PALETTE:
		png_set_palette_to_rgb(pPNG);
		break;
	}
	
	if (png_get_valid(pPNG, pINFO, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(pPNG);
		hasAlpha = true;
	}
	if (hasAlpha && isGrayscale) {
		png_set_gray_to_rgb(pPNG);
		isGrayscale = false;
	}

	if (!hasAlpha) {
		png_set_add_alpha(pPNG, 0xff, PNG_FILLER_AFTER);
		hasAlpha = true;
	}
	
	info.width = width;
	info.height = height;
	
#if 0
	if (bit_depth == 16) {
		png_set_strip_16(pPNG);
	}
	info.bitsPerSample = 8;
#else
	info.bitsPerSample = (bit_depth <= 8) ? 8 : 16;
	if (sys_is_little_endian()) {
		png_set_swap(pPNG);		// little endian
	}
#endif
	if (isGrayscale) {
		info.samplesPerPixel = 1;
	}else {
		if (hasAlpha) {
			info.samplesPerPixel = 4;
		}else {
			info.samplesPerPixel = 3;
		}
	}
}

} // namespace


bool ReadImageInfo_png(IFile& file, ImageInfo& info)
{
	info.fileFormatType = ImageFileFormatType_PNG;
	
	png_structp pPNG = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!pPNG) {
		return false;
	}
	png_infop pINFO = png_create_info_struct(pPNG);
	if (!pINFO) {
		png_destroy_read_struct(&pPNG, NULL, NULL);
		return false;
	}
	
	if (setjmp(png_jmpbuf(pPNG))) {
		png_destroy_read_struct(&pPNG, &pINFO, (png_infopp)NULL);
		// TODO: to destruct linePtrs.
		return false;
	}

	Reader reader(file);
	png_set_read_fn(pPNG, &reader, ReadDataFunc);
	png_read_info(pPNG, pINFO);
	
	parseInfo(file, info, pPNG, pINFO);

	png_destroy_read_struct(&pPNG, &pINFO, (png_infopp)NULL);
	return true;
}

bool ReadImage_png(IFile& file, ImageInfo& info, void* dest, int lineOffset, void* palettes)
{
	info.fileFormatType = ImageFileFormatType_PNG;
	
	png_structp pPNG = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!pPNG) {
		return false;
	}
	png_infop pINFO = png_create_info_struct(pPNG);
	if (!pINFO) {
		png_destroy_read_struct(&pPNG, NULL, NULL);
		return false;
	}
	
	if (setjmp(png_jmpbuf(pPNG))) {
		png_destroy_read_struct(&pPNG, &pINFO, (png_infopp)NULL);
		// TODO: to destruct linePtrs.
		return false;
	}

	Reader reader(file);
	png_set_read_fn(pPNG, &reader, ReadDataFunc);
	png_read_info(pPNG, pINFO);
	
	parseInfo(file, info, pPNG, pINFO);
	
	char* pBuffer = (char*)dest;
	std::vector<char*> linePtrs(info.height);
	for (size_t i=0; i<info.height; ++i) {
		linePtrs[i] = pBuffer;
		pBuffer += lineOffset;
	}
	
	png_read_image(pPNG, (png_bytepp)&linePtrs[0]);
	png_read_end(pPNG, pINFO);
	png_destroy_read_struct(&pPNG, &pINFO, (png_infopp)NULL);
	
	return true;
}

bool WriteImage_png(IFile& file, const ImageInfo& info, const void* data, int lineOffset)
{
	return false;
}


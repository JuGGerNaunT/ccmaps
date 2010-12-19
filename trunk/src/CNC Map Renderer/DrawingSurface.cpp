#include "DrawingSurface.h"
#include <jpeglib.h>
#include <png.h>
#include <string.h>
#include <stdexcept>

void DrawingSurface::Draw(const DrawingSurface& ds_src, int sx, int sy, int dx, int dy, int width, int height) {
	for (int y = sy; y < sy + height; y++) {
		unsigned char* dst =  (unsigned char*)(map + (dy + y - sy) * stride + dx * 3);
		unsigned char* src = (unsigned char*)(ds_src.map + y * ds_src.Get_Stride() + sx * 3);
		memcpy(dst, src, width * 3);
	}
}

void DrawingSurface::Draw(const unsigned char* src, int sx, int sy, int src_stride, int dx, int dy, int width, int height) {
	for (int y = sy; y < sy + height; y++) {
		unsigned char* dst = (unsigned char*)(map + (dy + y - sy) * stride + dx * 3);
		unsigned char* src_ = (unsigned char*)(src + y * src_stride + sx * 3);
		memcpy(dst, src_, width * 3);
	}
}

DrawingSurface::DrawingSurface() {}

DrawingSurface::DrawingSurface(int width, int height, int bpp) {
	SetDimensions(width, height, bpp);
}

void DrawingSurface::SetDimensions(int width, int height, int bpp) {
	this->width = width;
	this->height = height;
	this->bpp = bpp;
	this->stride = (width * (bpp / 8) + 3) & ~3;
	this->map = new unsigned char[height * stride];
	this->shadows = new bool[height * width];
	this->zorder = new int[height * width];
	memset(map, 0xFF, height * stride);
	memset(shadows, 0, height * width * sizeof(bool));
	memset(zorder, 0, height * width * sizeof(int));
}

int DrawingSurface::Get_Stride() const {
	return stride;
}

int DrawingSurface::Get_Width() const {
	return width;
}

int DrawingSurface::Get_Height() const {
	return height;
}

const unsigned char* DrawingSurface::Get_Lower_Bound() const {
	return map;
}

const unsigned char* DrawingSurface::Get_Higher_Bound() const {
	return map + height * stride;
}

unsigned char* DrawingSurface::Get_Lower_Bound() {
	return map;
}

unsigned char* DrawingSurface::Get_Higher_Bound() {
	return map + height * stride;
}

bool DrawingSurface::Within_Bounds(unsigned char* w) const {
	bool ret = (w >= map);
	ret = ret && (w < (map + height * stride));
	return ret;
}

bool* DrawingSurface::Get_Shadows() const {
	return shadows;
}

int* DrawingSurface::Get_ZOrders() const {
	return zorder;
}

DrawingSurface::~DrawingSurface() {
	delete[] map;
	delete[] shadows;
	delete[] zorder;
}

void DrawingSurface::SavePNG(const std::string& path, int compression, int left, int top, int width, int height, bool invert_rows) const {
	png_bytep* row_pointers = new png_bytep[height];

	for (int y = 0; y < height; y++) {
		png_byte* row = new png_byte[width * bpp / 8];
		row_pointers[y] = row;
		memcpy(row, map + stride * (invert_rows ? (y + top) : (top + height - y - 1))
			+ left * bpp / 8, width * bpp / 8);
	}

	FILE *fp = fopen(path.c_str(), "wb");
	if (!fp) return;
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) return;
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) return;

	png_init_io(png_ptr, fp);
	if (setjmp(png_jmpbuf(png_ptr))) return;
	png_set_IHDR(png_ptr, info_ptr, width, height,
		8, 
		bpp == 24 ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGBA, 
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_ptr->compression = compression;
	png_write_info(png_ptr, info_ptr);
	if (setjmp(png_jmpbuf(png_ptr))) return;
	for (int i = 0; i < height; i++) {
		png_write_row(png_ptr, row_pointers[i]);
	}
	//png_write_image(png_ptr, row_pointers);
	if (setjmp(png_jmpbuf(png_ptr))) return;
	png_write_end(png_ptr, NULL);
	for (int y = 0; y < height; y++)
		delete[] row_pointers[y];
	delete[] row_pointers;

	fclose(fp);
}

void DrawingSurface::SaveJPEG( const std::string& path, int quality, int left, int top, int width, int height, bool invert_rows) const {
	FILE* outfile = fopen(path.c_str(), "wb");

	if (!outfile)
		throw std::runtime_error("Failed to open output file");

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr       jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width      = width;
	cinfo.image_height     = height;
	cinfo.input_components = 3;
	cinfo.in_color_space   = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	// set the quality [0..100]
	jpeg_set_quality(&cinfo, quality, true);
	jpeg_start_compress(&cinfo, true);

	// allocate buffer to store rows with alpha channel removed in if bpp is 32 (jpeg unsupported)
	unsigned char* row = NULL;
	if (bpp == 32) row = new unsigned char[3 * width];
				
	JSAMPROW row_pointer[1];

	for (int y = 0; y < height; y++) {
		int rownum = invert_rows ? y : height - y - 1;
		if (bpp == 24) 
			row_pointer[0] = (JSAMPROW)(map + stride * (rownum + top) + 3 * left);
		else {// copy row with alpha channels removed
			for (int i = 0; i < width; i++)
				memcpy(row + i * 3, map + stride * (rownum + top) + 4 * (left + i), 3);
			row_pointer[0] = (JSAMPROW)row;
		}
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	if (bpp == 32) delete[] row;

	fclose(outfile);
}
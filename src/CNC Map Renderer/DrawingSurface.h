#pragma once

#include <string>

class DrawingSurface {
private:
	unsigned char* map;
	bool* shadows;
	int* zorder;
	unsigned int stride;
	int width;
	int height;
	int bpp;

public:
	DrawingSurface(int width, int height, int bpp = 24);
	DrawingSurface();
	~DrawingSurface();
	int Get_Stride() const;
	int Get_Width() const;
	int Get_Height() const;

	void Draw(const DrawingSurface& src, int sx, int sy, int dx, int dy, int width, int height);
	void Draw(const unsigned char* src, int sx, int sy, int src_stride, int dx, int dy, int width, int height);

	// void Draw_To_Bitmap(Gdiplus::Bitmap* b, Gdiplus::Rect* src, Gdiplus::Rect* dst) const;
	bool Within_Bounds(unsigned char* w) const;
	bool* Get_Shadows() const;
	const unsigned char* Get_Lower_Bound() const;
	unsigned char* Get_Lower_Bound();
	const unsigned char* Get_Higher_Bound() const;
	unsigned char* Get_Higher_Bound();
	void SavePNG(const std::string& path, int compression, int left, int top, int width, int height, bool invert_rows = true) const;
	void SaveJPEG(const std::string& path, int quality, int left, int top, int width, int height, bool invert_rows = true) const;
	void SetDimensions(int width, int height, int bpp = 24);
	int* Get_ZOrders() const;
};
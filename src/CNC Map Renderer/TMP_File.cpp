#include "TMP_File.h"
#include <algorithm>

unsigned char* get_ofs(unsigned char* w, int x, int y, int stride);
int* get_ofs(int* w, int x, int y, int stride) ;
bool* get_ofs(bool* w, int x, int y, int stride);

void TMP_File::Draw(int sub_tile, int x_offset, int y_offset, int height, DrawingSurface& dst, const unsigned char* palet) {
	if (!initialized) Initialize();
	unsigned char* write = dst.Get_Lower_Bound();
	unsigned int stride = dst.Get_Stride();

	int half_cx = get_header()->cx / 2,
		half_cy = get_header()->cy / 2;
	t_tmp_image_header header;
	if (get_index()[sub_tile] < 0 || get_index()[sub_tile] > tmp_data.size())
		return;
	header = *get_image_header(sub_tile);
	height += header.height;

	// Where the image is stored in memory
	const unsigned char* r = get_image(sub_tile);

	// Where we're gonna start writing
	unsigned char* w = + get_ofs(write,
		x_offset + half_cx - 2,
		y_offset,
		stride);

	int* z = + get_ofs(
		dst.Get_ZOrders(),
		x_offset + half_cx - 2,
		y_offset,
		dst.Get_Width());

	bool* shadows = get_ofs(dst.Get_Shadows(),
		x_offset + half_cx - 2,
		y_offset,
		dst.Get_Width());

	int cx = 0, // Amount of pixel to copy
		y = 0;

	for (;y < half_cy; y++) {
		cx += 4;
		for (unsigned short c = 0; c < cx; c++) {
			int v = *r;
			r++;
			if (dst.Within_Bounds(w)) { // If within boundaries
				memcpy(w, &palet[v*3], 3); // Copy 3 unsigned chars from palet
				if (*shadows && height <= *z) {
					*(w+0) /= 2;
					*(w+1) /= 2;
					*(w+2) /= 2;
				}
				*z = std::max(*z, height);
			}
			w += 3;
			shadows++;
			z++;
		}
		w += stride - 3 * (cx + 2);
		shadows += dst.Get_Width() - cx - 2;
		z += dst.Get_Width() - cx - 2;
	}

	w += 12;
	z += 4;
	shadows += 4;
	for (; y < get_header()->cy; y++) {
		cx -= 4;
		for (unsigned short c = 0; c < cx; c++) {
			int v = *r++;
			if (w >= write) {
				memcpy(w, &palet[v*3], 3);
				if (*shadows && height <= *z) {
					*(w+0) *= 0.5;
					*(w+1) *= 0.5;
					*(w+2) *= 0.5;
				}
				*z = std::max(*z, height);
			}
			w += 3;
			shadows++;
			z++;
		}
		w += stride - 3 * (cx - 2);
		shadows += dst.Get_Width() - cx + 2;
		z += dst.Get_Width() - cx + 2;
	}

	// Extra graphics, used for bridges etc
	// These are not limited to be within tile dimensions
	if (has_extra_graphics(sub_tile)) {
		r += 900;

		unsigned char* w_line = get_ofs(write,
			x_offset + header.x_extra - header.x,
			y_offset + header.y_extra - header.y,
			stride);

		int* z_line = get_ofs(dst.Get_ZOrders(),
			x_offset + header.x_extra - header.x,
			y_offset + header.y_extra - header.y,
			dst.Get_Width());

		bool* shadow_line = get_ofs(dst.Get_Shadows(),
			x_offset + header.x_extra - header.x,
			y_offset + header.y_extra - header.y,
			dst.Get_Width());

		int cx = header.cx_extra;
		int cy = header.cy_extra;

		// Extra graphics are just a square :)
		for (y = 0; y < cy; y++) {
			unsigned char* w = w_line;
			z = z_line;
			shadows = shadow_line;
			for (int x = 0; x < cx; x++) {
				// Checking per line is required because v needs to be checked every time
				int v = *r++;
				if (v && dst.Within_Bounds(w)) {
					memcpy(w, &palet[v*3], 3);
					*z = std::max(*z, height);
				}

				w += 3;
				shadows++;
				z++;
			}
			w_line += stride;
			shadow_line += dst.Get_Width();
			z_line += dst.Get_Width();
		}
	}
}

int TMP_File::get_c_tiles() const {
	// returns the number of tiles
	return get_cblocks_x() * get_cblocks_y();
}

int TMP_File::get_cblocks_x() const {
	// returns a value from the header
	return get_header()->cblocks_x;
}

int TMP_File::get_cblocks_y() const {
	// returns a value from the header
	return get_header()->cblocks_y;
}

const t_tmp_image_header* TMP_File::get_image_header( int i ) const {
	// returns an image header
	return reinterpret_cast<const t_tmp_image_header*>(get_data() + get_index()[i]);
}

int TMP_File::get_x( int i ) const {
	// returns a value from an image header
	return get_image_header(i)->x;
}

int TMP_File::get_y( int i ) const {
	// returns a value from an image header
	return get_image_header(i)->y;
}

int TMP_File::get_x_extra( int i ) const {
	// returns a value from an image header
	return get_image_header(i)->x_extra;
}

int TMP_File::get_y_extra( int i ) const {
	// returns a value from an image header
	return get_image_header(i)->y_extra;
}

int TMP_File::get_cx_extra( int i ) const {
	// returns a value from an image header
	return get_image_header(i)->cx_extra;
}

int TMP_File::get_cy_extra( int i ) const {
	// returns a value from an image header
	return get_image_header(i)->cy_extra;
}

bool TMP_File::has_extra_graphics(int i) const {
	return get_image_header(i)->cx_extra > 0 && get_image_header(i)->cx_extra < 256;
}

bool TMP_File::has_extra_graphics() const {
	for (int i = 0; i < get_c_tiles(); i++) {
		if (get_image_header(i)->has_extra_data) {
			return true;
		}
	}
	return false;
}

const unsigned char* TMP_File::get_image( int i ) const {
	// returns a pointer to the start of an image
	return reinterpret_cast<const unsigned char*>(get_image_header(i) + 1);
}

const int* TMP_File::get_index() const {
	// returns the index
	return reinterpret_cast<const int*>(get_data() + 16/*sizeof(t_tmp_ts_header)*/);
}

const t_tmp_ts_header* TMP_File::get_header() const {
	return reinterpret_cast<const t_tmp_ts_header*>(get_data());
}

const unsigned char* TMP_File::get_data() const {
	return &tmp_data[0];
}

unsigned char* TMP_File::get_data() {
	if (!initialized) Initialize();
	return &tmp_data[0];
}
#include "VXL_File.h"

bool VXL_File::is_valid() const	{
 	int size = get_size();
	const t_vxl_header& h = header();
	if (sizeof(t_vxl_header) > size ||
		strcmp(h.id, vxl_id) ||
		h.one != 1 ||
		h.c_section_headers != h.c_section_tailers ||
		h.size != size - sizeof(t_vxl_header) - h.c_section_headers * sizeof(t_vxl_section_header) - h.c_section_tailers * sizeof(t_vxl_section_tailer) ||
		h.unknown != 0x1f10)
		return false;
	/*
	for (int i = 0; i < header.c_section_headers; i++)
	{
		const t_vxl_section_header& section_header = *get_section_header(i);
		const t_vxl_section_tailer& section_tailer = *get_section_tailer(i);
		if (section_header.one != 1 ||
			section_header.zero ||
			section_tailer.span_end_ofs - section_tailer.span_start_ofs != section_tailer.span_data_ofs - section_tailer.span_end_ofs)
			return false;
	}
	*/
	return true;
}

struct t_vector {
	double x;
	double y;
	double z;
};

const double pi = 3.141592654;

t_vector rotate_x(t_vector v, double a) {
	double l = sqrt(v.y * v.y + v.z * v.z);
	double d_a = atan2(v.y, v.z) + a;
	t_vector r;
	r.x = v.x;
	r.y = l * sin(d_a);
	r.z = l * cos(d_a);
	return r;
}

t_vector rotate_y(t_vector v, double a) {
	double l = sqrt(v.x * v.x + v.z * v.z);
	double d_a = atan2(v.x, v.z) + a;
	t_vector r;
	r.x = l * sin(d_a);
	r.y = v.y;
	r.z = l * cos(d_a);
	return r;
}

void VXL_File::Draw(const int x_off, const int y_off, const int xrot, const int yrot, DrawingSurface& dst, const Palet* p) {
	if (!initialized) Initialize();

	for (int i = 0; i < get_c_section_headers(); i++) {
		const t_vxl_section_tailer& section_tailer = *get_section_tailer(i);
		const int cx = section_tailer.cx;
		const int cy = section_tailer.cy;
		const int cz = section_tailer.cz;
		const int l = ceil(sqrt((cx * cx + cy * cy + cz * cz) / 4.0));
		const int cl = 2 * l;
		
		const double center_x = cx / 2;
		const double center_y = cy / 2;
		const double center_z = cz / 2;

		const int c_pixels = cl * cl;

		unsigned char* image = new unsigned char[c_pixels];
		unsigned char* image_s = new unsigned char[c_pixels];
		char* image_z = new char[c_pixels];
	
		memset(image, 0, c_pixels);
		memset(image_s, 0, c_pixels);
		memset(image_z, CHAR_MIN, c_pixels);
		int j = 0;
		for (int y = 0; y < cy; y++) {
			for (int x = 0; x < cx; x++) {
				const unsigned char* r = get_span_data(i, j);
				if (r) {
					int z = 0;
					while (z < cz) {
						z += *r++;
						int c = *r++;
						while (c--) {
							t_vector s_pixel;
							s_pixel.x = x - center_x;
							s_pixel.y = y - center_y;
							s_pixel.z = z - center_z;
							t_vector d_pixel = rotate_y(rotate_x(s_pixel, xrot * pi / 180.0), yrot * pi / 180.0);
							d_pixel.x += l;
							d_pixel.y += l;
							d_pixel.z += center_z;
							int ofs = static_cast<int>(d_pixel.x) + cl * static_cast<int>(d_pixel.y);
							if (d_pixel.z > image_z[ofs]) {
								if (*r != 0) {
									unsigned char* w = dst.Get_Lower_Bound() + dst.Get_Stride() * ((int)d_pixel.y + y_off - cl - cy) + ((int)d_pixel.x + x_off) * 3;
									if (dst.Within_Bounds(w))
										memcpy(w, p->Get_Colors() + *r * 3 , 3);
								}
								image[ofs] = *r++;
								image_s[ofs] = *r++;
								image_z[ofs] = d_pixel.z;
							}
							else
								r += 2;
							z++;
						}
						r++;
					}
				}
				j++;
			}
		}


		delete[] image_z;
		delete[] image_s;
		delete[] image;
	}
				
}
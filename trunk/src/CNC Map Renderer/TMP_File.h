#pragma once

#include "File.h"
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "DrawingSurface.h"
#include <boost/cstdint.hpp>

using boost::int8_t;
using boost::int16_t;
using boost::int32_t;
using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;

struct t_tmp_image_header {
	int32_t x;
	int32_t y;
	int32_t extra_ofs;
	int32_t z_ofs;
	int32_t extra_z_ofs;
	int32_t x_extra;
	int32_t y_extra;
	int32_t cx_extra;
	int32_t cy_extra;
	unsigned int has_extra_data: 1;
	unsigned int has_z_data: 1;
	unsigned int has_damaged_data: 1;
	char height;
	char terrain_type;
	char ramp_type;
	int8_t radar_red_left;
	int8_t radar_green_left;
	int8_t radar_blue_left;
	int8_t radar_red_right;
	int8_t radar_green_right;
	int8_t radar_blue_right;
	int8_t pad[3];
};

struct t_tmp_ts_header {
	int32_t cblocks_x;                  // width of blocks
	int32_t cblocks_y;                  // height in blocks
	int32_t cx;                         // width of each block, always 48
	int32_t cy;                         // height of each block, always 24
};

class TMP_File {
private:

	bool initialized;
	boost::shared_ptr<File> f;
	std::vector<unsigned char> tmp_data;

	unsigned char* get_data();
	const unsigned char* get_data() const;
	const t_tmp_ts_header* get_header() const;
	int get_c_tiles() const;
	int get_cblocks_x() const;
	int get_cblocks_y() const;
	const t_tmp_image_header* get_image_header(int i) const;
	int get_x(int i) const;
	int get_y(int i) const;
	int get_x_extra(int i) const;
	int get_y_extra(int i) const;
	int get_cx_extra(int i) const;
	int get_cy_extra(int i) const;
	bool has_extra_graphics(int i) const;
	bool has_extra_graphics() const;
	const unsigned char* get_image(int i) const;
	const int* get_index() const;

	void Initialize() {
		if (f && !initialized) {
			f->seek_start();
			if (f->read(tmp_data, f->size()) != f->size())
				throw;
			initialized = true;
		}
	}

public:
	TMP_File(boost::shared_ptr<File> f) {
		initialized = false;
		this->f = f;
		if (f->size() < 0) throw;
	}
	void Draw(int sub_tile, int x_offset, int y_offset, int height,
		DrawingSurface& dst, const unsigned char* palet);
};
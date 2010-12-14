#pragma once

#include "File.h"
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "DrawingSurface.h"
#include <boost/cstdint.hpp>
#include "Palet.h"

using boost::int16_t;
using boost::int32_t;
using boost::uint8_t;

#pragma pack(push, 1)

struct t_vxl_header {
	char id[16];
	int32_t one;
	uint32_t c_section_headers;
	uint32_t c_section_tailers;
	uint32_t size;
	int16_t unknown;
	unsigned char palet[768];
};

struct t_vxl_section_header {
	char id[16];
	__int32 section_i;
	__int32 one;
	__int32 zero;
};

struct t_vxl_section_tailer {
    __int32 span_start_ofs;
    __int32 span_end_ofs;
    __int32 span_data_ofs;
	float scale;
	float transform[3][4];
	float x_min_scale;
	float y_min_scale;
	float z_min_scale;
	float x_max_scale;
	float y_max_scale;
	float z_max_scale;
	unsigned __int8 cx;
	unsigned __int8 cy;
	unsigned __int8 cz;
	__int8 unknown;
};

const char vxl_id[] = "Voxel Animation";

#pragma pack(pop)


class VXL_File {
private:

	bool initialized;
	boost::shared_ptr<File> f;
	boost::shared_ptr<File> hva;

	std::vector<unsigned char> vxl_data;

	unsigned char* get_data();
	const unsigned char* get_data() const { return &vxl_data[0]; }

	const int get_size() const {
		return f->size();
	}

	const t_vxl_header& header() const {
		return *reinterpret_cast<const t_vxl_header*>(get_data());
	}

	int get_c_section_headers() const {
		return header().c_section_headers;
	}

	int get_c_spans(int i) const {
		return get_section_tailer(i)->span_end_ofs - get_section_tailer(i)->span_start_ofs >> 2;
	}

	int get_c_section_tailers() const {
		return header().c_section_tailers;
	}

	const unsigned char* get_palet() const {
		return header().palet;
	}

	const unsigned char* get_section_body() const {
		return get_data() + sizeof(t_vxl_header) + sizeof(t_vxl_section_header) * get_c_section_tailers();
	}

	const t_vxl_section_header* get_section_header(int i) const {
		return reinterpret_cast<const t_vxl_section_header*>(get_data() + sizeof(t_vxl_header) + sizeof(t_vxl_section_header) * i);
	}

	const t_vxl_section_tailer* get_section_tailer(int i) const {
		return reinterpret_cast<const t_vxl_section_tailer*>(get_data() + get_size() + sizeof(t_vxl_section_tailer) * (i - get_c_section_tailers()));
	}

	const unsigned char* get_span_data(int i, int j) const {
		if (get_span_start_list(i)[j] == -1)
			return NULL;
		return get_section_body() + get_section_tailer(i)->span_data_ofs + get_span_start_list(i)[j];
	}

	int get_span_size(int i, int j) const {
		return get_span_end_list(i)[j] - get_span_start_list(i)[j] + 1;
	}

	const int* get_span_start_list(int i) const {
		return reinterpret_cast<const int*>(get_section_body() + get_section_tailer(i)->span_start_ofs);
	}

	const int* get_span_end_list(int i) const {
		return reinterpret_cast<const int*>(get_section_body() + get_section_tailer(i)->span_end_ofs);
	}

public:
	VXL_File(boost::shared_ptr<File> f, boost::shared_ptr<File> hva) : f(f), hva(hva) {
		initialized = false;
		Y_Sort = 0;
		X_Offset = 0;
		Y_Offset = 0;
		//if (f->size() < 0) throw;
	}
	int Y_Sort;
	int X_Offset;
	int Y_Offset;
	
	void Set_YSort(int ysort) { this->Y_Sort = ysort; }
	void Set_Offset(int xoff, int yoff) { this->X_Offset = xoff; this->Y_Offset = yoff; }

	void Initialize() {
		if (f && !initialized) {
			f->seek_start();
			if (f->read(vxl_data, f->size()) != f->size())
				throw;
			initialized = true;
		}
	}

	bool is_valid() const;

	void Draw(const int x, const int y, const int xrot, const int yrot, DrawingSurface& dst, const Palet* p);
};
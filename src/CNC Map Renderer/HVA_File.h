#pragma once

#include "File.h"
#include <vector>
#include <boost/shared_ptr.hpp>
#include "DrawingSurface.h"
#include <boost/cstdint.hpp>

using boost::int32_t;

#pragma pack(push, 1)

struct t_hva_header {
	char id[16];
	int32_t c_frames;
	int32_t c_sections;
};

typedef float t_hva_transform_matrix[3][4];

#pragma pack(pop)


class HVA_File {
private:

	bool initialized;
	boost::shared_ptr<File> f;
	std::vector<unsigned char> vxl_data;

	unsigned char* get_data();
	const unsigned char* get_data() const;

	void Initialize() {
		if (f && !initialized) {
			f->seek_start();
			if (f->read(vxl_data, f->size()) != f->size())
				throw;
			initialized = true;
		}
	}

	const t_hva_header& header() const {
		return *reinterpret_cast<const t_hva_header*>(get_data());
	}

	const int get_size() const {
		return f->size();
	}

	int get_c_frames() const {
		return header().c_frames;
	}

	int get_c_sections() const {
		return header().c_sections;
	}

	const char* get_section_id(int i) const {
		return reinterpret_cast<const char*>(get_data() + sizeof(t_hva_header) + 16 * i);
	}

	const float* get_transform_matrix(int i, int j) const {
		return reinterpret_cast<const float*>(get_data() + sizeof(t_hva_header) + 16 * get_c_sections() + (get_c_frames() * i + j) * sizeof(t_hva_transform_matrix));
	}

public:
	HVA_File(boost::shared_ptr<File> f) {
		initialized = false;
		this->f = f;
		if (f->size() < 0) throw;
	}
};
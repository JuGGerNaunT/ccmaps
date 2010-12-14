#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>
#include "CCRC.h"

#include "blowfish.h"
#include "westwood_key.h"
#include "mixarchive.h"

#include <boost/cstdint.hpp>
using std::string;
using std::vector;
using boost::shared_ptr;
using boost::int8_t;
using boost::int16_t;
using boost::int32_t;
using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;


namespace fs = boost::filesystem;

namespace VFS {

unsigned int calc_id(string filename) {
	boost::algorithm::to_upper(filename);
	const int l = filename.length();
	int a = l >> 2;
	if (l & 3) {
		filename += static_cast<char>(l - (a << 2));
		int i = 3 - (l & 3);
		while (i--)	filename += filename[a << 2];
	}
	Ccrc crc;                        // use a normal CRC function
	crc.init();
	crc.do_block(filename.c_str(), filename.length());
	return crc.get_crc();
}



enum MixArchiveType	{
	TD_MIXFILE,
	RA_MIXFILE,
};

// Red Alert flags
const unsigned int RA_MIX_CHECKSUM  = 0x00010000;
const unsigned int RA_MIX_ENCRYPTED = 0x00020000;

MixArchiveType archive_type(unsigned int first_four_bytes) {
	return first_four_bytes == 0 || first_four_bytes == RA_MIX_CHECKSUM ||
		first_four_bytes == RA_MIX_ENCRYPTED ||
		first_four_bytes == (RA_MIX_ENCRYPTED | RA_MIX_CHECKSUM) ?
		RA_MIXFILE : TD_MIXFILE;
}


MixArchive::MixArchive(boost::shared_ptr<File> base, const std::string& name) : base(base) {
	// Parse main header
	parse_header();
}

//-------------------------------------------------------------------------

void MixArchive::parse_header() {
	unsigned int type_header = 0;
	vector<unsigned char> buff;
	base->seek_start();
	base->read(buff, 4);
	type_header = *reinterpret_cast<unsigned int*>(&buff[0]);
	base->seek_cur(-4);
	if (archive_type(type_header) == TD_MIXFILE) {
		parse_td_header();
	} 
	else {
		// This is the sort of assumption that westwood key makes :-(
		int size = sizeof(void*);
		//if (sizeof(void*) != 4) {
		//	throw std::runtime_error("Unsupported architecture");
		//}
		parse_ra_header();
	}
}

void MixArchive::parse_td_header() {
	// Parse header info	
	vector<unsigned char> buff;
	unsigned char header[6];	
	if (base->read(buff, 6) != sizeof(header)) {
		throw std::runtime_error(
			"MixArchive: '" + path() + "': Invalid header");
	}
	memcpy(header, &buff[0], 6);

	unsigned short index_size =
		*reinterpret_cast<uint16_t*>(&header[0]) * 12;
	unsigned int data_size =
		*reinterpret_cast<uint32_t*>(&header[2]);

	// Read file index
	vector<unsigned char> index(index_size);
	if (base->read(index, (int)index.size()) != index.size()) {
		throw std::runtime_error(
			"MixArchive: '" + path() + "': Invalid file index");
	}
	// 6 = index_size + data_size
	build_index(index, 10 + index_size);
}

void MixArchive::parse_ra_header() {
	uint32_t flags;
	vector<unsigned char> buff;
	base->read(buff, 4);
	flags = *reinterpret_cast<unsigned int*>(&buff[0]);
	int base_offset = 4;
	if (flags & RA_MIX_ENCRYPTED) {
		// Decrypt WS key and prepare blowfish decrypter
		uint8_t ww_key[80];
		uint8_t blowfish_key[56];
		if (base->read(buff, 80) != sizeof(ww_key)) {
			throw std::runtime_error(
				"MixArchive: '" + path() + "': Invalid key");
		}
		memcpy(ww_key, &buff[0], sizeof(ww_key));

		base_offset += 80;
		decode_westwood_key(&ww_key[0], &blowfish_key[0]);
		Blowfish bf(&blowfish_key[0], sizeof(blowfish_key));

		// Parse header info (blowfish wants 64 bit chunks,
		// so we read two extra bytes, used later)
		uint8_t header[8];
		if (base->read(buff, 8) != sizeof(header)) {
			throw std::runtime_error(
				"MixArchive: '" + path() + "': Invalid header");
		}
		memcpy(header, &buff[0], sizeof(header));
		base_offset += 8; // sizeof(t_mix_header);
		bf.decipher(&header[0], &header[0], sizeof(header));

		uint16_t index_size =
			*reinterpret_cast<uint16_t*>(&header[0]) * 12;
		uint16_t data_size =
			*reinterpret_cast<uint16_t*>(&header[2]);
		base_offset += index_size + (index_size % 8);

		// Read file index
		vector<uint8_t> index_buf(index_size + 2);
		if (base->read(buff, index_size) != index_size) {
			throw std::runtime_error(
				"MixArchive: '" + path() + "': Invalid file index");
		}
		memcpy(&index_buf[2], &buff[0], index_size);

		// First two bytes of first entry comes from
		// the header_buf (see above)
		index_buf[0] = header[6];
		index_buf[1] = header[7];

		// Decipher index. The last two bytes are discarded, not sure why
		bf.decipher(&index_buf[2], &index_buf[2], index_size); //  + 5 & ~7);
		index_buf.resize(index_buf.size() - 2);

		// 92 = RA flags + index_size + data_size
		//	+ westwood_key + 2 unknown bytes
		build_index(index_buf, base_offset);
	} 
	else {
		// 4 = RA flags
		parse_td_header();
	}
}

void MixArchive::build_index(
	const vector<unsigned char>& index_buf, int base_offset) {

	// Build index
	unsigned int id = 0, offset = 0, size = 0;
	for (Index::size_type i = 0; i < index_buf.size(); i+=12) {
		id     = *reinterpret_cast<const unsigned int*>(&index_buf[i]);
		offset = *reinterpret_cast<const unsigned int*>(&index_buf[i+4])
				 + base_offset;
		size   = *reinterpret_cast<const unsigned int*>(&index_buf[i+8]);

		// 1422054725 = "local mix database.dat"
		std::pair<Index::iterator, bool> res = index.insert(Index::value_type(id, IndexValue(offset, size)));
	}
}

//-------------------------------------------------------------------------

shared_ptr<File> MixArchive::open(const string& filename, bool writable) {
	if (writable) {
		return shared_ptr<File>();
	}

	// Filenames can only be 12 chars in mix files
	if (filename.size() > 14) {
		return shared_ptr<File>();
	}

	// Lookup file
	unsigned int id = calc_id(filename);
	Index::iterator it = index.find(id);
	if (it != index.end()) {
		return shared_ptr<File>(new MixFile(base, filename, it->second.first, it->second.second));
	}
	return shared_ptr<File>();
}

bool MixArchive::Exists(const std::string& filename) const {
	return index.find(calc_id(filename)) != index.end();
}

MixFile::MixFile(shared_ptr<File> base, const string& name, int mix_offset, int size) : base(base) {
	name_ = name;
	size_ = size;
	lower_boundary = mix_offset;
	writable_ = false;
	seek_start();
}

int MixFile::do_read(vector<unsigned char>& buf, int count) {
	count = base->read(buf, count);
	update_state();
	return count;
}

void MixFile::update_state() {
	eof_ = base->pos() >= lower_boundary + size_ || base->eof();
	pos_ = base->pos() - lower_boundary;
}

void MixFile::do_seek(int offset, int orig) {
	switch (orig) {
		case -1: offset += lower_boundary; break;         // From start of file
		case  0: offset += lower_boundary + pos_; break;  // From current position
		case  1: offset += lower_boundary + size_; break; // From end of file
		default: return;
	}
	base->seek_start(offset);
	update_state();
}

}


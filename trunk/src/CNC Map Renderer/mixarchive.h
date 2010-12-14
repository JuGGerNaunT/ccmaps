#pragma once

#include <cstdio>
#include <map>
#include <string>
#include <vector>
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr.hpp>

#include "archive.h"

namespace VFS {
	//-------------------------------------------------------------------------
	// MixArchive
	//-------------------------------------------------------------------------
	class MixArchive : public Archive 	{
		typedef std::pair<unsigned int, unsigned int> IndexValue;
		typedef std::map<unsigned int, IndexValue> Index;
	public:
		MixArchive(boost::shared_ptr<File> file, const std::string& name);
		~MixArchive(){};
		std::string path() { return base->archive(); }
		boost::shared_ptr<File> open(const std::string& filename, bool writable);
		bool Exists(const std::string& filename) const;
	protected:
		void do_flush() {}
		int do_read(std::vector<unsigned char>& buf, int count);
		void do_seek(int pos, int orig);
		int do_write(const std::vector<unsigned char>& buf) { return 0; }

	private:
		void update_state();
		boost::shared_ptr<File> base;
		Index index;

		void parse_header();
		void parse_td_header();
		void parse_ra_header();
		void build_index(const std::vector<unsigned char>& index_buf,
			int base_offset);
	};


	class MixFile : public File {
	public:
		MixFile(boost::shared_ptr<File> base, const std::string& name, int mix_offset, int size);

	protected:
		void do_flush() {}
		int do_read(std::vector<unsigned char>& buf, int count);
		void do_seek(int pos, int orig);
		int do_write(const std::vector<unsigned char>& buf) { return 0; }
		void update_state();
	private:
		boost::shared_ptr<File> base;
		int lower_boundary;
	};
}
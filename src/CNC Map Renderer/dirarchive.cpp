#include <sstream>
#include <stdexcept>
#include <boost/filesystem/operations.hpp>
#include <string.h>
#include "dirarchive.h"

namespace fs = boost::filesystem;
using std::string;
using std::vector;
using boost::shared_ptr;


namespace VFS {

	//-------------------------------------------------------------------------
	// DirArchiveFile
	//-------------------------------------------------------------------------

	//-------------------------------------------------------------------------

	DirFile::DirFile(const string& path, const string& archive, const string& name, bool writable)
	{
		// Info
		archive_ = archive;
		name_ = name;

		eof_ = false;
		pos_ = 0;
		writable_ = writable;

		// Open file
		handle = fopen(path.c_str(), writable ? "wb" : "rb");
		if (!handle) {
			std::ostringstream temp;
			temp << "DirArchive: fopen failed for '" << name_ << "' in '" << archive_ << "': " << errno << ": " << strerror(errno);
			throw std::runtime_error(temp.str());
		}

		// Calculate size
		fseek(handle, 0, SEEK_END);
		size_ = ftell(handle);
		fseek(handle, 0, SEEK_SET);
	}

	DirFile::~DirFile()
	{
		if (handle) {
			fclose(handle);
		}
	}

	void DirFile::update_state()
	{
		eof_ = feof(handle) != 0;
		pos_ = ftell(handle);
	}

	//-------------------------------------------------------------------------

	void DirFile::do_flush()
	{
		fflush(handle);
	}

	int DirFile::do_read(vector<unsigned char>& buf, int count)
	{
		buf.resize(count);
		int bytesread = static_cast<int>(fread(&buf[0], sizeof(char), buf.size(), handle));
		//buf.resize(bytesread);
		update_state();
		return bytesread;
	}

	void DirFile::do_seek(int offset, int orig)
	{
		int origin;
		switch (orig) {
			case -1: origin = SEEK_SET; break;
			case  0: origin = SEEK_CUR; break;
			case  1: origin = SEEK_END; break;
			default: return;
		}
		fseek(handle, offset, origin);
		update_state();
	}

	int DirFile::do_write(const vector<unsigned char>& buf)
	{
		int byteswritten = static_cast<int>(fwrite(&buf[0], sizeof(char), buf.size(), handle));
		update_state();
		return byteswritten;
	}

	//-------------------------------------------------------------------------
	// DirArchive
	//-------------------------------------------------------------------------

	DirArchive::DirArchive(const fs::path& dir) : dir(dir) {}
	DirArchive::~DirArchive() {}

	string DirArchive::path()
	{
		return dir.native_directory_string();
	}

	shared_ptr<File> DirArchive::open(const string& filename, bool writable)
	{
		fs::path filepath(dir / filename);
		// If we open for reading and the file doesnt exist, or if the path is a directory, return 0.
		if ((!writable && !fs::exists(filepath)) || (fs::exists(filepath) && fs::is_directory(filepath))) {
			return shared_ptr<File>();
		}

		return shared_ptr<File>(new DirFile(filepath.native_directory_string(), path(), filename, writable));
	}

	bool DirArchive::Exists(const std::string& filename) const {
		return fs::exists(fs::path(dir / filename));
	}
}

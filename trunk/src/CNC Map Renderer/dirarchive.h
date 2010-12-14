#pragma once 

#include <string>
#include <boost/filesystem/path.hpp>
#include <boost/smart_ptr.hpp>
#include <cerrno>
#include <cstdio>
#include "archive.h"

namespace VFS {
	class DirFile : public File {
	public:
		DirFile(const std::string& path, const std::string& archive, const std::string& name, bool writable);
		~DirFile();

	protected:
		void do_flush();
		int do_read(std::vector<unsigned char>& buf, int count);
		void do_seek(int pos, int orig);
		int do_write(const std::vector<unsigned char>& buf);

	private:
		void update_state();
		FILE* handle;
	};


	class DirArchive : public Archive
	{
	public:
		DirArchive(const boost::filesystem::path& dir);
		~DirArchive();
	    
		std::string path();
		boost::shared_ptr<File> open(const std::string& filename, bool writable);
		bool Exists(const std::string& filename) const;

	private:
		boost::filesystem::path dir;
	};
}
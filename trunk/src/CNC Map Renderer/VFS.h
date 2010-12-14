#pragma once

#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <string>
#include "File.h"
#include "archive.h"
#include "mixarchive.h"
#include "dirarchive.h"

#ifdef WIN32
	#define NOMINMAX
	#include <windows.h>
#endif

namespace fs = boost::filesystem;

namespace VFS {

	class Archive;

	typedef std::vector<boost::shared_ptr<Archive> > ArchiveVector;

	class VFS : private boost::noncopyable {
	public:
		VFS();
		~VFS();

		// Adds a directory or mixfile to the VFS. Directories are searched in
		// the order they are added.
		// Files in the filesystem always takes priority over files in archives.
		bool add(const boost::filesystem::path& pth);

		// Removes all directories.
		void remove_all();

		// Opens a file for reading.
		boost::shared_ptr<File> open(const std::string& filename);

		// Opens a file for writing.
		boost::shared_ptr<File> open_write(const std::string& filename);

		// Tests whether a file exists somewhere in the VFS
		bool exists(const std::string& filename) const;

		// Retrieve the directory string where the game is installed
		std::string get_game_installpath();

		// Initializes the vfs, required registry entries
#ifdef WIN32
		void Initialize(const std::string& mixroot, bool YR = true);
#endif
		void Initialize(bool YR = true);
		void ScanMixDir(fs::path pth, bool yr = true);
	private:
		boost::shared_ptr<File> open(const std::string& filename, bool writable);

		// Contains the list of archives; The first archive of each group is the
		// directory that was added, and the remaining members are archives located
		// in that directory.
		ArchiveVector archives;
	};

}

extern VFS::VFS vfs;

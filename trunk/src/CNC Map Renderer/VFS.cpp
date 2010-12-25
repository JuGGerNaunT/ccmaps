#include "VFS.h"
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>

using std::string;
using std::vector;
using boost::shared_ptr;

VFS::VFS vfs;

int wildcmp(const char *wild, const char *string); 
namespace VFS {

	int wildcmp(const char *wild, const char *string);

	bool VFS::exists(const string& filename) const {
		for (ArchiveVector::const_iterator it = archives.begin(); it != archives.end(); ++it) {
			if ((*it)->Exists(filename))
				return true;
		}
		return false;
	}

	shared_ptr<File> VFS::open(const string& filename, bool writable) {
		for (ArchiveVector::iterator it = archives.begin(); it != archives.end(); ++it) {
			if (!(*it)->Exists(filename)) {
				continue;
			}
			shared_ptr<File> file = (*it)->open(filename, writable);
			if (file) {
				file->set_name(filename);
				return file;
			}
		}
		return shared_ptr<File>();
	}
	bool VFS::add(const boost::filesystem::path& pth) {
		if (fs::is_directory(pth)) {
			archives.push_back(shared_ptr<DirArchive>(new DirArchive(pth)));
			return true;
		}
		else if (fs::is_regular(pth)) {
			if (fs::extension(pth) == ".mix" || fs::extension(pth) == ".mmx" || fs::extension(pth) == ".yro") {
				shared_ptr<File> dirfile(
					new DirFile(pth.native_file_string(), pth.native_file_string(), pth.filename(), false));
				shared_ptr<MixArchive> mia(		
					new MixArchive(dirfile, pth.native_file_string()));
				archives.push_back(mia);
				return true;
			}
		}
		else if (!fs::exists(pth) && (fs::extension(pth) == ".mix" || fs::extension(pth) == ".mmx" || fs::extension(pth) == ".yro")) {
			// attempt to open mix from vfs
			shared_ptr<File> mixfile = vfs.open(pth.string());
			if (mixfile) {
				shared_ptr<MixArchive> mxa(new MixArchive(mixfile, pth.filename()));
				archives.push_back(mxa);
				return true;
			}
		}
		return false;
	}
	void VFS::ScanMixDir(boost::filesystem::path pth, bool yr) {
		// std::cout << "Initializing filesystem on " << pth << std::endl;
		// see http://modenc.renegadeprojects.com/MIX
		add(pth);
		if (yr)	add(pth / "langmd.mix");
		add(pth / "language.mix");	

		// try all expand\d{2}md?\.mix files
		for (int i = 99; i >= 0; i--) {
			std::string file = "expand" + (boost::format("%02d") % i).str() + ".mix";
 			fs::path p = pth / file;
			if (fs::exists(p)) add(p);
			if (yr) {
				file = "expandmd" + (boost::format("%02d") % i).str() + ".mix";
 				p = pth / file;
				if (fs::exists(p)) add(p);
			}
		}
		
		add(pth / "ra2md.mix");
		add(pth / "ra2.mix");	
		if (yr) add("cachemd.mix");
		add("cache.mix");
		if (yr) add("localmd.mix");
		add("local.mix");
		if (yr) add("audiomd.mix");

		fs::directory_iterator end_itr;		
		fs::directory_iterator di(pth);
		while (di != end_itr) {
			string path = di->path().file_string();
			boost::algorithm::to_lower(path);
			if (wildcmp("*ecache*.mix", path.c_str())) {
			    boost::filesystem::path p = *di;
				add(p);
			}
			di++;
		}
		di = fs::directory_iterator(pth);
		while (di != end_itr) {
			string path = di->path().file_string();
			boost::algorithm::to_lower(path);
			if (wildcmp("*elocal*.mix", path.c_str())) {
				boost::filesystem::path p = *di;
				add(p);
			}
			di++;
		}
		di = fs::directory_iterator(pth);
		while (di != end_itr) {
			string path = di->path().file_string();
			boost::algorithm::to_lower(path);
			if (wildcmp("*.mmx", path.c_str())) {
				boost::filesystem::path p = *di;
				add(p);
			}
			if (wildcmp("*.yro", path.c_str())) {
				boost::filesystem::path p = *di;
				add(p);
			}
			di++;
		}
		if (yr) add("conqmd.mix");
		if (yr) add("genermd.mix");
		add("generic.mix");
		if (yr) add("isogenmd.mix");
		add("isogen.mix");
		add("conquer.mix");
		if (yr) add("cameomd.mix");
		add("cameo.mix");
		if (yr) add(pth/"mapsmd03.mix");
		if (yr) add(pth/"multimd.mix");
		if (yr) add(pth/"thememd.mix");
		if (yr) add(pth/"movmd03.mix");
	}

	void VFS::Initialize(const std::string& mixroot, bool YR) {
		ScanMixDir(boost::filesystem::path(mixroot), YR);
	}

	int wildcmp(const char *wild, const char *string) {
		const char *cp = NULL, *mp = NULL;
		while ((*string) && (*wild != '*')) {
			if ((*wild != *string) && (*wild != '?')) return 0;
			wild++;
			string++;
		}
		while (*string) {
			if (*wild == '*') {
				if (!*++wild) return 1;
				mp = wild;
				cp = string+1;
			} 
			else if ((*wild == *string) || (*wild == '?')) {
				wild++;
				string++;
			} 
			else {
				wild = mp;
				string = cp++;
			}
		}
		while (*wild == '*') wild++;
		return !*wild;
	}

	VFS::VFS() {
	}

#ifdef WIN32
	void VFS::Initialize(bool YR) {
		// first go through mods that are in the game install dir
		// they are named expand*.mix or ecache*.mix
		ScanMixDir(fs::path(get_game_installpath()), YR);
	}
#endif
	VFS::~VFS()	{
	}


#ifdef WIN32
	std::string VFS::get_game_installpath() {
		std::string path;
		char cpath[MAX_PATH];

		DWORD dwBufLen = MAX_PATH*sizeof(char);
		HKEY ra2;
		LONG lRet;

		lRet = RegOpenKeyExA( HKEY_LOCAL_MACHINE, "SOFTWARE\\Westwood\\Red Alert 2",
			0, KEY_QUERY_VALUE,	&ra2);
		if (lRet != ERROR_SUCCESS) {
			GetModuleFileNameA(NULL, cpath, MAX_PATH);
			return std::string();
		}

		lRet = RegQueryValueExA(ra2, "InstallPath", NULL,
			NULL, (LPBYTE)cpath, &dwBufLen);
		if (lRet != ERROR_SUCCESS) {
			GetModuleFileNameA(NULL, cpath, MAX_PATH);
			return std::string();
		}

		RegCloseKey(ra2);
		if (strlen(cpath)) {
			std::string ret(cpath);
			return ret.substr(0, ret.find_last_of('\\') + 1);
		}
		GetModuleFileNameA(NULL, cpath, MAX_PATH);
		return std::string(cpath);
	}
#endif

	void VFS::remove_all() {
		ArchiveVector().swap(archives);
	}

	shared_ptr<File> VFS::open(const string& filename) {
		return open(filename, false);
	}

	shared_ptr<File> VFS::open_write(const string& filename) {
		return open(filename, true);
	}

}

#include "config.h"
#include "VFS.h"
#include "RA2_Map.h"
#include "ini_file.h"
#include <boost/filesystem/path.hpp>
// #include "mmgr.h"
#include <string>
#include <iostream>
#include "anyoption.h"
#include "csf.h"

#include "VXL_File.h"
#include "Palet.h"

using std::cout;
using std::string;
using boost::shared_ptr;

void clean_outfile(std::string& s){
	boost::algorithm::erase_all(s, "\\");
	boost::algorithm::erase_all(s, "/");
	boost::algorithm::erase_all(s, "*");
	boost::algorithm::erase_all(s, ":");
	boost::algorithm::erase_all(s, "<");
	boost::algorithm::erase_all(s, ">");
	boost::algorithm::erase_all(s, "?");
	boost::algorithm::erase_all(s, "|");
	boost::algorithm::erase_all(s, "\"");
}
int main(int argc, char* argv[]) {
	AnyOption opt;
	opt.addUsage("Usage: ");
	opt.addUsage("" );
	opt.addUsage(" -i   --infile \"c:\\myMap.mpr\"   Input map file (.mpr, .map, .yrm)");
	opt.addUsage(" -o   --outfile myMap           Output base filename. Read from map if not specified.");
	opt.addUsage(" -d   --outdir \"c:\\\"            Output directory");
	opt.addUsage(" -Y   --force-yr                Force rendering using YR engine");
	opt.addUsage(" -y   --force-ra2               Force rendering using RA2 engine");
	opt.addUsage(" -j   --jpeg                    Produce JPEG file (myMap.jpg)");
	opt.addUsage(" -q   --jpeg-quality [0-100]     JPEG quality (0-100, default 90)");
	opt.addUsage(" -p   --png                     Produce PNG file (myMap.png)" );
	opt.addUsage(" -c   --png-compression [0-9]    PNG compression level (0-9, default 6)" );
	opt.addUsage(" -m   --mixdir \"c:\\westwood\\\"   Mix files location (registry if not specified)");
	opt.addUsage(" -s   --start-pos-tiled         Mark start positions as 4x4 tiled red spots");
	opt.addUsage(" -S   --start-pos-squared       Mark start positions as a large square");
	opt.addUsage(" -r   --mark-ore                Mark ore clearly");
	opt.addUsage(" -F   --force-fullmap           Ignore LocalSize definition");
	opt.addUsage(" -f   --force-localsize         Force usage of localsize");
	opt.addUsage(" -h   --help                    Show this short help text");
	opt.addUsage(" " );

	opt.setOption("infile", 'i');
	opt.setOption("outfile", 'o');
	opt.setOption("outdir", 'd');
	opt.setOption("force-yr", 'Y');
	opt.setOption("force-ra2", 'y');
	opt.setFlag("jpeg", 'j');
	opt.setOption("jpeg-quality", 'q');
	opt.setFlag("png", 'p');
	opt.setOption("png-compression", 'c');
	opt.setOption("mixdir", 'm');
	opt.setFlag("help", 'h');
	opt.setFlag("start-pos-tiled", 's');
	opt.setFlag("start-pos-squared", 'S');
	opt.setFlag("mark-ore", 'r');
	opt.setFlag("force-fullmap", 'F');
	opt.setFlag("force-localsize", 'f');

	opt.processCommandArgs(argc, argv);

	if (!opt.hasOptions() || opt.getFlag("help") || opt.getFlag('h')) {
		opt.printUsage();
		return 0;
	}

	std::string infile;
	if (opt.getValue('i') != NULL) {
		infile = opt.getValue('i');
	}
	if (!boost::filesystem::exists(infile)) {
		cout << "Error: input file invalid";
		return 0;
	}
	boost::filesystem::path pth(infile);
	std::string infile_nopath = pth.filename();
	if (infile_nopath.rfind('.') != std::string::npos) {
		infile_nopath = infile_nopath.substr(0, infile_nopath.rfind('.'));
	}

#ifndef WIN32
	if (opt.getValue('m') == NULL) {
		cout << "Error: mix files location needs to be specified on non-win32 environments." << endl;
		return 0;
	}
#endif

	std::string mixdir;
#ifdef WIN32
	if (opt.getValue('m') == NULL) {
		mixdir = vfs.get_game_installpath();
	}
#endif

	if (opt.getValue('m') != NULL) {
		mixdir = opt.getValue('m');
	}

    if (!boost::filesystem::exists(mixdir)) {
        cout << "Error: mix files not found in \"" << mixdir << "\"." << endl;
        return 0;
    }
	vfs.Initialize(mixdir);

	if (!opt.getFlag('j') && !opt.getFlag('p')) {
		cout << "Error: no output format(s) specified." << endl;
		cout << "Use -j for JPEG or -p for PNG output." << endl;
		return 0;
	}

	std::string outdir;
	if (opt.getValue('d') == NULL) {
		boost::filesystem::path pth(infile);
		outdir = (pth.parent_path()).string() + "/";

		if (outdir != "" && !boost::filesystem::is_directory(outdir)) {
			cout << "Error: output directory \"" << outdir
				<< "\" does not exist." << endl;
			return 0;
		}
	}
	else outdir = opt.getValue('d');

	bool use_localsize = !opt.getFlag("F");

	// ---------------------------
	// ---------------------------
	// ---        tests        ---
	boost::shared_ptr<VXL_File> v(new VXL_File(vfs.open("sam.vxl"), vfs.open("zep.hva")));
	v->Initialize();
	assert(v->is_valid());
	Palet p(vfs.open("unittem.pal"));
	p.Recalculate();
	DrawingSurface ds(200, 200);
	// v.Draw(0, 0, ds, p.Get_Colors());
	/*boost::shared_ptr<SHP_File> s = v->Get_SHP();
	s->Draw(36, 100, 100, ds, p.Get_Colors());
	ds.SaveJPEG("c:\\soms.jpg", 90, 0, 0, 200, 200);
	VFS::DirFile f("c:\\soms2.shp", "", "", true);
	vector<unsigned char> buf(s->get_data(), s->get_data() + s->get_size());
	f.write(buf);*/
	// exit(0);

	// ---------------------------
	// ---------------------------

	cout << "Loading map " << infile << ".." << endl;
	ini_file MapINI(infile);
	Map_Type M = UKN;
	if (opt.getValue('y')) M = YR;
	else if (opt.getValue('l')) M = RA2;
	RA2_Map myMap(MapINI, M);

	std::string outfile;
	// To determine the name of the outfile, priorities are
	// 1) name given on commandline
	if (opt.getValue('o') != NULL) {
		outfile = opt.getValue('o');
	}
	// 2) determine name based on game internals
	else {
		ini_section& basic = MapINI.get_section("Basic");
		if (outfile == "" && basic.read_bool("Official") == true) {
			string pktfile;
			bool isyr = myMap.Get_MapType() == YR;
			std::cout << "Determining mapname" <<isyr<< std::endl;
			bool custom_pkt = false;
			if (boost::filesystem::extension(infile) == ".mmx") {
				pktfile = infile_nopath + ".pkt";
				vfs.add(infile);
				custom_pkt = true;
			}
			else if (isyr)
				pktfile = "missionsmd.pkt";
			else
				pktfile = "missions.pkt";
			std::cout << "Loading pkt file " << pktfile << std::endl;
			ini_file pkt(vfs.open(pktfile));

			// read mapname from csf
			string pkt_mapname;
			if (!custom_pkt) pkt_mapname = infile_nopath;
			else pkt_mapname = pkt.get_key("multimaps", "1");
			boost::algorithm::to_lower(pkt_mapname);

			string csf_entry;
			if (basic.read_bool("MultiplayerOnly")) { // multiplayer map
				csf_entry = pkt.get_key(pkt_mapname, "description");
			}
			else { // mission
				string missionsfile = isyr ? "missionmd.ini" : "mission.ini";
				std::cout << "Loading missions file " << missionsfile << std::endl;
				ini_file mission(vfs.open(missionsfile));
				if (mission.set_current_section(infile_nopath + ".map")) {
					csf_entry = mission.get_key("UIName");
				}
			}
			if (csf_entry != "") {
				boost::algorithm::to_lower(csf_entry);
				string csffile = isyr ? "ra2md.csf" : "ra2.csf";
				std::cout << "Loading csf file " << csffile << std::endl;
				CSF_File csf(vfs.open(csffile));
				if (csf.post_open()) {
					std::cout << "Couldn't open " << csffile << std::endl;
				}
				else {
					if (csf.has_name(csf_entry)) {
						string csf_value = csf.get_converted_value(csf_entry);
						if (csf_value.find(" (") != string::npos)
							csf_value = csf_value.substr(0, csf_value.find(" ("));
						outfile = csf_value;
						clean_outfile(outfile);
						std::cout << "Mapname found " << outfile << std::endl;
					}
				}
			}
		}
		// 3) On unofficial maps, name specified in basic section
		_3:
		if (outfile == "" /*&& basic.read_bool("Official") == false*/) {
			outfile = basic.read_string("Name");
			clean_outfile(outfile);
		}

		if (outfile == "") {
			// 4) Fallback: file name
			outfile = infile_nopath;
			boost::algorithm::to_lower(outfile);
		}
	}

	myMap.LoadMap();

	if (opt.getFlag('s'))
		myMap.Draw_Startpos_Tiles();

	if (opt.getFlag('r'))
		myMap.fuck_Ore();

	myMap.Draw();

	if (opt.getFlag('S'))
		myMap.Draw_Startpos_Squares();

	if (opt.getFlag('j')) {
		int quality = 90;
		char* jpeg_q = opt.getValue('q');
		bool valid_q_entered = true;

		if (jpeg_q != NULL) {
			try {
				quality = boost::lexical_cast<int>(jpeg_q);
				if (quality < 0 || quality > 100) valid_q_entered = false;
			}
			catch (const std::exception& exc) {
				valid_q_entered = false;
			}
			if (!valid_q_entered) {
				cout << "Invalid quality specified. Valid qualities are in range [0..100]." << endl;
				cout << "Default quality (90) used instead." << endl;
			}
		}
		cout << "Saving JPEG file " << outfile << ".jpg" << " (quality: " << quality << ").." << endl;
		myMap.SaveJPEG(outdir, outfile, quality);
	}

	if (opt.getFlag('p')) {
		int compression = 6;
		char* png_c = opt.getValue('c');
		bool valid_c_entered = true;

		if (png_c != NULL) {
			try {
				compression = boost::lexical_cast<int>(png_c);
				if (compression < 0 || compression > 9) {
					valid_c_entered = false;
				}
			}
			catch (const std::exception& exc) {
				valid_c_entered = false;
			}
			if (!valid_c_entered) {
				cout << "Invalid PNG compression rate specified. Valid rates are in range [0..9]." << endl;
				cout << "Default compression rate (6) used instead." << endl;
			}
		}
		cout << "Saving PNG file " << outfile << ".png" << " (compression: " << compression << ").." << endl;
		myMap.SavePNG(outdir, outfile, compression);
	}

	std::cout << std::endl << "Program by Frank Razenberg. Copyright (c) 2007-2010." << std::endl;
	std::cout << std::endl << "Thanks go out to Olaf van der Spek for XCC," << std::endl;
	std::cout << "Perry aka BrutalAl for coding assistance, lots of ideas and help," << std::endl;
	std::cout << "Matthias Wagner for Final Alert 2," << std::endl;
	std::cout << "DCoder for reverse engineering LightSources," << std::endl;
	exit(0);
}
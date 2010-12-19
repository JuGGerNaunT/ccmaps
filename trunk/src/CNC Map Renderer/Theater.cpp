
// To access global vfs
#include "VFS.h"
#include "Theater.h"
#include "RA2_Map.h"

#include <iomanip>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

#include "TMP_File.h"
#include "File.h"

using std::string;
using boost::lexical_cast;
using boost::shared_ptr;


std::string zeropad(int num, int zeropad) {
	std::ostringstream ss;
	ss << std::setw(zeropad) << std::setfill('0') << num;
	return ss.str();
}

Theater::Theater(Map_Type M, Theater_Type T) {
	Parse_INIs(M, T);
}

void Theater::Parse_INIs(Map_Type M, Theater_Type T) {	
	// Load Rules and Art
	string rulesini = M == YR ? "rulesmd.ini" : "rules.ini";
	std::cout << "Parsing " << rulesini << std::endl;
	RulesINI = shared_ptr<ini_file>(new ini_file());
	RulesINI->parse(vfs.open(rulesini));
	Parse_INIs(M, T, RulesINI);
}

void Theater::Parse_INIs(Map_Type M, Theater_Type T, shared_ptr<ini_file> rules) {
	this->RulesINI = rules;
	this->TheaterType = T;
	this->MapType = M;
	string artini = M == YR ? "artmd.ini" : "art.ini";
	std::cout << "Parsing " << artini << std::endl;
	RulesINI->parse(vfs.open(artini));
}

int Theater::Get_Tilenum_From_Set(int tileset, int num) const {
	return Get_Tile_C().Get_Tilenum_From_Set(tileset, num);
}

int Theater::Get_Set_From_Tilenum(int num) const {
	return Get_Tile_C().Get_Set_From_Tilenum(num);
}

void Theater::Draw_Tile(Tile* T, DrawingSurface& dst) const {
	if (T->TileNr < 0) {
		// tile specified in general/ClearTile is used to fill holes
		int tileset = tmp_c.Get_Clear_Tile();
		T->TileNr = Get_Tilenum_From_Set(tileset);
		T->SubTile = 0;
	}

	// height threatment on tiles required
	tmp_c.Draw_Tile(T, dst);
}

const TMP_Collection& Theater::Get_Tile_C() const {
	return tmp_c;
}

const Palet& Theater::Get_Palet(Palet_Type P) const {
	switch (P) {
		case P_ANIM: return p_anim;
		case P_ISO:	return p_iso;
		case P_LIB:	return p_lib;
		case P_OVERLAY:	return p_ovl;
		case P_UNIT: return p_unit;
	}
}

void Theater::Override(ini_file& Map) {
	std::cout << "Overriding rules.ini with map INI entries" << std::endl;
	sectionmap::const_iterator it;	
	for (it = Map.begin(); it != Map.end(); it++) {

		// If this section also exists in rules.ini
		if (RulesINI->set_current_section(it->second->get_name())) {
			keymap::const_iterator k_it;
			// Then copy all keys in this section over
			if (it->second->num_keys() <= 0)
				continue;
			for (k_it = it->second->begin(); k_it != it->second->end(); k_it++) {
				RulesINI->get_current_section().set_value(k_it->first, k_it->second);
			}
					}
	}
}

const ini_section& Theater::Get_Object_INI(string section) {
	return RulesINI->get_section(section);
}

void Theater::Recalculate_Palets() {
	p_anim.Recalculate();
	p_iso.Recalculate();
	p_lib.Recalculate();
	p_ovl.Recalculate();
	p_unit.Recalculate();
}

void Theater::Draw_Overlay(Overlay* O, DrawingSurface& dst) {
	OverlayTypes.Draw_SHP(O->num, O->sub, O->X * 30, O->Y * 15, O->Z, 0, dst, &O->P);
}

void Theater::Draw_Overlay_Shadow(Overlay* O, DrawingSurface& dst) {
	OverlayTypes.Draw_SHP_Shadow(O->num, O->sub, O->X * 30, O->Y * 15, O->Z, 0, dst);
}

void Theater::Draw_Overlay_NoShadow(Overlay* O, DrawingSurface& dst) {
	// Real ugly "hack" to ensure floating bridges end up correctly.
	// No idea how to improve this :(
	int y_off = 0;
	if (O->num == 24 || O->num == 25 || O->num == 238 || O->num == 237) {
		y_off = O->sub > 8 ? 16 : 1;
	}
	OverlayTypes.Draw_SHP_NoShadow(O->num, O->sub, O->X * 30, O->Y * 15 - y_off, O->Z, 0, dst,&O->P);
}

void Theater::Draw_Smudge(Smudge* Smu, DrawingSurface& dst) {
	int idx = SmudgeTypes.Get_Image_Index(Smu->name);
	if (idx >= 0) {
		SmudgeTypes.Draw_SHP(idx, 0, Smu->X * 30, Smu->Y * 15, Smu->Z, 0, dst, &Smu->P);
	}
}

void Theater::Draw_Terrain(Terrain* T, DrawingSurface& dst) {
	int idx = TerrainTypes.Get_Image_Index(T->name);
	TerrainTypes.Draw_SHP(idx, 0, T->X * 30, T->Y* 15 + 15, T->Z, 0, dst, &T->P);
}

void Theater::Draw_Structure(Structure* Str, DrawingSurface& dst) {
	int idx = BuildingTypes.Get_Image_Index(Str->name);
	if (idx == -1) {
		idx = OverlayTypes.Get_Image_Index(Str->name);
		if (idx != -1) {
			if (Str->health > 128)
				OverlayTypes.Draw_SHP(idx, 0, Str->X * 30, Str->Y * 15, Str->Z, Str->direction, dst, &Str->P);
			else
				BuildingTypes.Draw_Damaged_SHP(idx, 1, Str->X * 30, Str->Y * 15, Str->Z, Str->direction, dst, &Str->P);
		}
	}
	else {
		if (Str->health > 128)
			BuildingTypes.Draw_SHP(idx, 0, Str->X * 30, Str->Y * 15, Str->Z, Str->direction, dst, &Str->P);
		else
			BuildingTypes.Draw_Damaged_SHP(idx, 1, Str->X * 30, Str->Y * 15, Str->Z, Str->direction, dst, &Str->P);
	}
}

void Theater::Draw_Infantry(Infantry* Inf, DrawingSurface& dst) {
	int idx = InfantryTypes.Get_Image_Index(Inf->name);
	if (idx >= 0) {
		InfantryTypes.Draw_SHP(idx, 0, Inf->X * 30, Inf->Y * 15, Inf->Z, Inf->direction, dst, &Inf->P);
	}
}

void Theater::Draw_Unit(Unit* Unit, DrawingSurface& dst) {
	int idx = VehicleTypes.Get_Image_Index(Unit->name);
	if (idx >= 0) {
		VehicleTypes.Draw_SHP(idx, 0, Unit->X * 30, Unit->Y * 15, Unit->Z, Unit->direction, dst, &Unit->P);
	}
}

void Theater::Draw_Aircraft(Aircraft* Aircraft, DrawingSurface& dst) {
	int idx = AircraftTypes.Get_Image_Index(Aircraft->name);
	if (idx >= 0) {
		AircraftTypes.Draw_SHP(idx, 0, Aircraft->X * 30, Aircraft->Y * 15, Aircraft->Z, Aircraft->direction, dst, &Aircraft->P);
	}
}

hsv_color Theater::Get_Country_Remap(std::string house) {
	return remap_colors[house];
}

void Theater::Load_Colours() {
	std::cout << "Loading colors" << std::endl;
	if (RulesINI->set_current_section("Colors")) {
		ini_section& c = RulesINI->get_current_section();
		for (keymap::const_iterator it = c.begin(); it != c.end(); it++) {
			named_colors[it->first] = hsv_color(it->second);
		}
	}
	else {
		std::cout << "Error loading countries" << std::endl;
	}
}

void Theater::Load_SHPs() {
	SHP_Collection* s[] = { 
		&InfantryTypes, &VehicleTypes, &AircraftTypes, &BuildingTypes, &TerrainTypes, &SmudgeTypes, &OverlayTypes 
	};

	for (int i = 0; i < sizeof(s) / sizeof(s[0]); i++) {
		s[i]->Set_Palet(P_ANIM, &p_anim);
		s[i]->Set_Palet(P_ISO, &p_iso);
		s[i]->Set_Palet(P_LIB, &p_lib);
		s[i]->Set_Palet(P_OVERLAY, &p_ovl);
		s[i]->Set_Palet(P_UNIT, &p_unit);
	}

	// Construct SHP collection
	std::cout << "Loading InfantryTypes" << std::endl;
	InfantryTypes.Initialize(S_INFANTRY, TheaterType, RulesINI);

	std::cout << "Loading VehicleTypes" << std::endl;
	VehicleTypes.Initialize(S_VEHICLE, TheaterType, RulesINI);	

	std::cout << "Loading AircraftTypes" << std::endl;
	AircraftTypes.Initialize(S_AIRCRAFT, TheaterType, RulesINI);

	std::cout << "Loading BuildingTypes" << std::endl;
	BuildingTypes.Initialize(S_BUILDING, TheaterType, RulesINI);

	std::cout << "Loading TerrainTypes" << std::endl;
	TerrainTypes.Initialize(S_TERRAIN, TheaterType, RulesINI);

	std::cout << "Loading SmudgeTypes" << std::endl;
	SmudgeTypes.Initialize(S_SMUDGE, TheaterType, RulesINI);

	std::cout << "Loading OverlayTypes" << std::endl;
	OverlayTypes.Initialize(S_OVERLAY, TheaterType, RulesINI);
}

void Theater::RemapMultiplyUsedImages() {
	for (sectionmap::iterator map_it = RulesINI->begin(); map_it != RulesINI->end(); map_it++) {
		// Example: [Bridge1], Image=Bridge.
		// Art ini section is named bridge
		shared_ptr<ini_section> section = map_it->second; 
		std::string artname = section->read_string("Image");
		if (artname != "" && map_it->first != artname) {
			ini_section& art(RulesINI->get_section(artname));
			for (keymap::iterator art_it = art.begin(); art_it != art.end(); art_it++) {
				if (section->read_string(art_it->first) == "")
					section->set_value(art_it->first, art_it->second);
			}
		}
	}
}	

void Theater::Initialize(ini_file& map) {
	RemapMultiplyUsedImages();
	Override(map);
	myLighting.Read(map.get_section("Lighting"));
	Load_Palettes(myLighting);
	Load_Colours();
	Load_Countries();
	Load_Houses(map);

	Load_Tiles();	
	Load_SHPs();
}		

void Theater::Load_Palettes(const Lighting& L) {
	std::cout << "Loading theater global palets" << std::endl;

	Lighting noL;
	// Don't use red/green/blue attributes on other than iso palets
	noL.blue = noL.green = noL.red = noL.ambient = 1.0;
	noL.level = noL.ground = 0;

	// Load palets
	p_anim.Initialize(vfs.open("anim.pal"), L);
	if (TheaterType == T_SNOW || TheaterType == T_SNOW_YR) {
		ext = ".sno";
		p_iso.Initialize(vfs.open("isosno.pal"), L);
		p_lib.Initialize(vfs.open("libsno.pal"), L);
		p_ovl.Initialize(vfs.open("snow.pal"), noL);
		p_unit.Initialize(vfs.open("unitsno.pal"), noL);

		vfs.add("isosnomd.mix");
		vfs.add("snowmd.mix");
		vfs.add("isosnow.mix");
		vfs.add("snow.mix");
		vfs.add("sno.mix");
	}

	else if (TheaterType == T_URBAN || TheaterType == T_URBAN_YR) {
		ext = ".urb";
		p_iso.Initialize(vfs.open("isourb.pal"), L);
		p_lib.Initialize(vfs.open("liburb.pal"), L);
		p_ovl.Initialize(vfs.open("urban.pal"), noL);
		p_unit.Initialize(vfs.open("uniturb.pal"), noL);

		vfs.add("isourbmd.mix");
		vfs.add("isourb.mix");
		vfs.add("urb.mix");
		vfs.add("urban.mix");
	}

	else if (TheaterType == T_TEMPERATE || TheaterType == T_TEMPERATE_YR) {
		ext = ".tem";
		p_iso.Initialize(vfs.open("isotem.pal"), L);
		p_lib.Initialize(vfs.open("libtem.pal"), L);
		p_ovl.Initialize(vfs.open("temperat.pal"), noL);
		p_unit.Initialize(vfs.open("unittem.pal"), noL);	

		vfs.add("isotemp.mix");
		vfs.add("isotemmd.mix");
		vfs.add("temperat.mix");
		vfs.add("tem.mix");
	}
	else if (TheaterType == T_DESERT) {
		ext = ".des";
		p_iso.Initialize(vfs.open("isodes.pal"), L);
		p_lib.Initialize(vfs.open("libdes.pal"), L);
		p_ovl.Initialize(vfs.open("desert.pal"), noL);
		p_unit.Initialize(vfs.open("unitdes.pal"), L);
		
		vfs.add("isodesmd.mix");
		vfs.add("desert.mix");
		vfs.add("des.mix");
		vfs.add("isodes.mix");
	}
	else if (TheaterType == T_LUNAR) {
		ext = ".lun";
		p_iso.Initialize(vfs.open("isolun.pal"), L);
		p_lib.Initialize(vfs.open("liblun.pal"), L);
		p_ovl.Initialize(vfs.open("lunar.pal"), noL);
		p_unit.Initialize(vfs.open("unitlun.pal"), noL);

		vfs.add("isolunmd.mix");
		vfs.add("isolun.mix");
		vfs.add("lun.mix");
		vfs.add("lunar.mix");
	}
	else if (TheaterType == T_NEWURBAN) {
		ext = ".ubn";
		p_iso.Initialize(vfs.open("isoubn.pal"), L);
		p_lib.Initialize(vfs.open("libubn.pal"), L);
		p_ovl.Initialize(vfs.open("urbann.pal"), noL);
		p_unit.Initialize(vfs.open("unitubn.pal"), noL);

		vfs.add("isoubnmd.mix");
		vfs.add("isoubn.mix");
		vfs.add("ubn.mix");
		vfs.add("urbann.mix");
	}
	else 
		throw std::runtime_error("Theater not initialised?");
}


void Theater::Load_Houses(ini_file& map) {
	std::cout << "Loading houses" << std::endl;
	if (map.set_current_section("Houses")) {
		ini_section& c = map.get_current_section();
		for (keymap::const_iterator it = c.begin(); it != c.end(); it++) {
			if (std::find(houses.begin(), houses.end(), it->second) == houses.end()) {
				houses.push_back(it->second);
			}
		}
	}
	else {
		std::cout << "Error loading houses" << std::endl;
	}
	for (std::vector<std::string>::iterator it = houses.begin(); it != houses.end(); it++) {
		if (map.set_current_section(*it)) {
			ini_section& c = map.get_current_section();
			std::string col = c.read_string("Color");
			boost::algorithm::to_lower(col);
			this->remap_colors[*it] = named_colors[col];
		}
		else {
			std::cout << "Error loading house " << *it << std::endl;
			return;
		}
	}
}

void Theater::Load_Tiles() {
	std::cout << "Loading tile collection for theater" << std::endl;
	// Construct TMP collection
	tmp_c.Initialize(TheaterType);
}

const Lighting* Theater::Get_Lighting() const {
	return &myLighting;
}

void Theater::Load_Countries() {
	std::cout << "Loading countries" << std::endl;
	if (RulesINI->set_current_section("Countries")) {
		ini_section& c = RulesINI->get_current_section();
		for (keymap::const_iterator it = c.begin(); it != c.end(); it++) {
			if (std::find(countries.begin(), countries.end(), it->second) == countries.end()) {
				countries.push_back(it->second);
			}
		}
	}
	else {
		std::cout << "Error loading countries" << std::endl;
	}
	for (std::vector<std::string>::iterator it = countries.begin(); it != countries.end(); it++) {
		if (RulesINI->set_current_section(*it)) {
			ini_section& c = RulesINI->get_current_section();
			string col = c.read_string("Color");
			boost::algorithm::to_lower(col);
			this->remap_colors[*it] = named_colors[col];
		}
		else {
			std::cout << "Error loading country " << *it << std::endl;
			return;
		}
	}
}
#define MAX_ORE 127
#define MIN_ORE 102
#define MAX_GEMS 38
#define MIN_GEMS 27

#include "RA2_Map.h"

#include "decode.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <iostream>
#include "CCRC.h"

using std::string;
using std::vector;

/* Coordinate formulas
dx = rx - ry + mapwidth - 1
dy = rx + ry - mapwidth - 1
rx = (dx + dy) / 2 + 1
ry = dy - rx + mapwidth + 1 */

RA2_Map::~RA2_Map() {
	// delete myTheater;
}

RA2_Map::RA2_Map(ini_file& Map, Map_Type M) : MapINI(Map) {
	std::stringstream ss; char g;
	// to not take commas in 0,0,w,has thousands delimiter
	ss.imbue(std::locale("C"));
	ss.clear();
	ss.str(Map.get_key("map", "size"));
	ss >> Size.x >> g >> Size.y >> g >> Size.w >> g >> Size.h;

	// The actual visible part of the map is given by the LocalSize ini entry
	ss.clear();
	ss.str(Map.get_key("localsize"));
	ss >> LocalSize.x >> g >> LocalSize.y >> g >> LocalSize.w >> g >> LocalSize.h;

	ReadObjects();
	if (M == UKN) DetermineMapType();
	rules = boost::shared_ptr<ini_file>(new ini_file(vfs.open(this->M == YR ? "rulesmd.ini" : "rules.ini")));

	DetermineTheaterType();

	// Init theater
	std::cout << "Initializing theater" << std::endl;
	myTheater.Parse_INIs(this->M, this->T, rules);
	myTheater.Initialize(MapINI);
}

bool RA2_Map::All_Objects_RA2() {
	std::cout << "Parsing " << "rules.ini" << std::endl;
	rules = boost::shared_ptr<ini_file>(new ini_file(vfs.open("rules.ini")));
	ini_section& terrain = rules->get_section("TerrainTypes");
	ini_section& buildingtypes = rules->get_section("BuildingTypes");
	ini_section& overlaytypes = rules->get_section("OverlayTypes");
	ini_section& infantry = rules->get_section("InfantryTypes");
	ini_section& aircraft = rules->get_section("AircraftTypes");
	ini_section& vehicles = rules->get_section("VehicleTypes");

	for (int y = 0; y < Size.h * 2; y++) {
		for (int x = 0; x < Size.w * 2; x++) {
			Tile* T = Get_Tile(x, y);
			if (T->Exists) {
			}
			else continue;

			Overlay* Ovl = Get_Overlay(x, y);
			if (Ovl->Exists) {
				if (Ovl->num > 246)
					return false;
			}
			Terrain* Ter = Get_Terrain(x, y);
			if (Ter->Exists) {
				string terr_idx = terrain.lookup(Ter->name);
				if (terr_idx == "" || boost::lexical_cast<int>(terr_idx) > 73)
					return false;
			}

			// no need to test smudge types as no new ones were introduced with yr

			Structure* Str = Get_Structure(x, y);
			if (Str->Exists) {
				string str_idx = buildingtypes.lookup(Str->name);
				string str_idx2 = overlaytypes.lookup(Str->name);
				if (str_idx != ini_section::nullvalue && boost::lexical_cast<int>(str_idx) > 303)
					return false;
				else if (str_idx2 != ini_section::nullvalue && boost::lexical_cast<int>(str_idx2) > 246)
					return false;
				else if (str_idx == ini_section::nullvalue && str_idx2 == ini_section::nullvalue)
					return false;
			}

			Infantry* Inf = Get_Infantry(x, y);
			if (Inf->Exists) {
				string inf_idx = infantry.lookup(Inf->name);
				if (inf_idx == "" || boost::lexical_cast<int>(inf_idx) > 45)
					return false;
			}

			Unit* U = Get_Unit(x, y);
			if (U->Exists) {
				string vehicle_idx = vehicles.lookup(U->name);
				if (vehicle_idx == "" || boost::lexical_cast<int>(vehicle_idx) > 57)
					return false;
			}

			Aircraft* A = Get_Aircraft(x, y);
			if (A->Exists) {
				string aircraft_idx = aircraft.lookup(A->name);
				if (aircraft_idx == "" || boost::lexical_cast<int>(aircraft_idx) > 9)
					return false;
			}
		}
	}
	return true;
}

void RA2_Map::ReadObjects() {
	std::cout << "Reading tiles" << std::endl;
	Read_Tiles();

	std::cout << "Reading map overlay" << std::endl;
	Read_Overlay();

	std::cout << "Reading map structures" << std::endl;
	Read_Structures();

	std::cout << "Reading map overlay objects" << std::endl;
	Read_Terrain();

	std::cout << "Reading map terrain object" << std::endl;
	Read_Smudges();

	std::cout << "Reading infantry on map" << std::endl;
	Read_Infantry();

	std::cout << "Reading vehicles on map" << std::endl;
	Read_Units();

	std::cout << "Reading aircraft on map" << std::endl;
	Read_Aircraft();
}

void RA2_Map::LoadMap() {
	std::cout << "Fixing tiles" << std::endl;
	Fix_Tile_Layer();

	std::cout << "Fixing ore layer" << std::endl;
	Fix_Ore();

	std::cout << "Creating palets for map objects" << std::endl;
	Create_Object_Palets();

	std::cout << "Applying lightsources over map objects" << std::endl;
	Apply_Lamps();

	std::cout << "Recalculating object palets" << std::endl;
	Apply_Palet_Overrides();
}

void RA2_Map::DetermineMapType() {
	std::string ext = this->MapINI.get_name();
	ext = ext.substr(ext.length() - 3);
	boost::algorithm::to_lower(ext);
	string theater = MapINI.get_key("map", "theater");
	boost::algorithm::to_lower(theater);

	// decision based on fa2 key
	if (MapINI.get_section("basic").read_bool("requiredaddon")) M = YR;
	// decision based on theatre
	else if (theater == "lunar") M = YR;
	else if (theater == "newurban") M = YR;
	else if (theater == "desert") M = YR;
	// decision based on overlay/trees/structs
	else if (!All_Objects_RA2()) M = YR;
	// decision based on max tile/threatre
	else if (theater == "temperate") {
		if (max_tile > 838)	M = YR;
		else M = RA2;
	}
	else if (theater == "urban") {
		if (max_tile > 1077) M = YR;
		else M = RA2;
	}
	else if (theater == "snow") {
		if (max_tile > 798)	M = YR;
		else M = RA2;
	}
	// decision based on extension
	else if (ext == "yrm") M = YR;
	else M = RA2;
}

void RA2_Map::DetermineTheaterType() {
	string theater = MapINI.get_key("map", "theater");
	boost::algorithm::to_lower(theater);
	if (theater == "lunar") T = T_LUNAR;
	else if (theater == "newurban") T = T_NEWURBAN;
	else if (theater == "desert") T = T_DESERT;
	else if (theater == "temperate") T = (M == YR ? T_TEMPERATE_YR : T_TEMPERATE);
	else if (theater == "urban") T = (M == YR ? T_URBAN_YR : T_URBAN);
	else if (theater == "snow") T = (M == YR ? T_SNOW_YR: T_SNOW);
	else T = T_TEMPERATE_YR; // most compatible
}

unsigned int RA2_Map::Tile_Index(int x, int y) const {
	return x + y * Size.h * 2;
}

int RA2_Map::Get_Tile_Setnum(int x, int y, int assume) const {
	if (x < Size.w * 2 && x > -1 && y > -1 && y < Size.h * 2)
		return Get_Tile(x, y)->SetNum;
	return assume;
}

void RA2_Map::Read_Tiles() {
	ini_section& TileSection(MapINI.get_section("isomappack5"));
	string concatenate = TileSection.get_concatenated_values();
	int lzo_size = concatenate.length() * 6 / 8;
	unsigned char* d = new unsigned char[lzo_size];
	int test = Decode_64(reinterpret_cast<const unsigned char*>(concatenate.c_str()), d);

	// base 64 padding allows for little offset
	assert(abs(lzo_size - test) <= 3);

	int cells = Size.w * Size.h + (Size.w-1) * Size.h;
	int size_2 = cells * 11 + 11; // 4 extra needed, reason unknown
	unsigned char* IsoMapPack = new unsigned char[size_2];
	test = Decode_5(d, IsoMapPack, test, 5);
	assert(test - size_2 <= 11);

	// assign space for tile information and init
	int TileMapSize = std::max(Size.w, Size.h) * std::max(Size.w, Size.h) * 4;
	TileLayer.resize(TileMapSize);
	OverlayLayer.resize(TileMapSize);
	SmudgeLayer.resize(TileMapSize);
	TerrainLayer.resize(TileMapSize);
	StructLayer.resize(TileMapSize);
	InfantryLayer.resize(TileMapSize);
	UnitLayer.resize(TileMapSize);
	AircraftLayer.resize(TileMapSize);
	for (int i = 0; i < TileMapSize; i++) {
		TileLayer[i].Exists = false;
		OverlayLayer[i].Exists = false;
		SmudgeLayer[i].Exists = false;
		TerrainLayer[i].Exists = false;
		StructLayer[i].Exists = false;
		InfantryLayer[i].Exists = false;
		UnitLayer[i].Exists = false;
		AircraftLayer[i].Exists = false;
	}
	max_tile = -1;
	// Read in all tiles and store them in a rectangular matrix, not like a rhombus
	for (int i = 0; i < cells; i++) {
		Iso_Map_Pack_Entry* r = reinterpret_cast<Iso_Map_Pack_Entry*>(&IsoMapPack[i * 11]);

		int x = r->x - r->y + Size.w - 1;
		int y = r->y + r->x - Size.w - 1;
		if (x < 0 || y < 0 || x > Size.w * 2 || y > Size.h * 2)
			continue; // invalid tile :/
		Tile* T = Get_Tile(x, y);

		// Copy tile details
		T->SetNum = -1;
		T->SubTile = r->sub_tile;
		T->TileNr = r->tile;

		// Squared coordinates
		T->X = x;
		T->Y = y;
		T->Z = r->z;

		// Isometric coordinates
		T->RX = r->x;
		T->RY = r->y;
		T->RZ = r->z;

		order_x.push_back(x);
		order_y.push_back(y);

		// Recalculate tile coordinates
		// Set to initialised
		T->Exists = true;

		// track tile with highest number
		max_tile = std::max(max_tile, (int)r->tile);
	}
}

// Removes LAT system that FinalAlert probably made
void RA2_Map::Fix_Tile_Layer() {
	const TMP_Collection& tile_c(Get_Tile_C());

	for (int y = 0; y < Size.h * 2; y++) {
		for (int x = 0; x < Size.w * 2; x++) {
			Tile* T = Get_Tile(x, y);

			if (!T->Exists)
				continue; // skip unexisting tiles

			T->SetNum = tile_c.Get_Set_From_Tilenum(T->TileNr);

			// If this tile comes from a CLAT (connecting lat) set,
			// then replace it's set and tilenr by corresponding LAT sets'
			if (tile_c.Is_CLAT(T->SetNum)) {
				T->SetNum = tile_c.Get_LAT_Set(T->SetNum);
				T->TileNr = tile_c.Get_Tilenum_From_Set(T->SetNum);
			}
		}
	}

	// Recalculate LAT system (tile connecting)
	for (int y = 0; y < Size.h * 2; y++) {
		for (int x = 0; x < Size.w * 2; x++) {
			Tile* T = Get_Tile(x, y);

			// If this tile is a LAT tile, we might have to connect it
			if (tile_c.Is_LAT(T->SetNum)) {
				// Find Tileset that contains the connecting pieces
				int to_clat = tile_c.Get_CLAT_Set(T->SetNum);
				// Which tile to use from that tileset
				unsigned char lat_transition = 0;

				// Find out setnums of adjacent cells
				int top_right = Get_Tile_Setnum(T->X + 1, T->Y - 1, T->SetNum),
					top_left = Get_Tile_Setnum( T->X - 1, T->Y - 1, T->SetNum),
					bottom_right = Get_Tile_Setnum(T->X + 1, T->Y + 1, T->SetNum),
					bottom_left = Get_Tile_Setnum(T->X - 1, T->Y + 1, T->SetNum);

				if (tile_c.Do_Connect(T->SetNum, top_right))
					lat_transition += 1;

				if (tile_c.Do_Connect(T->SetNum, bottom_right))
					lat_transition += 2;

				if (tile_c.Do_Connect(T->SetNum, bottom_left))
					lat_transition += 4;

				if (tile_c.Do_Connect(T->SetNum, top_left))
					lat_transition += 8;

				if (lat_transition > 0) {
					// Do not change this setnum, as then we could recognize it as
					// a different tileset for later tiles around this one.
					// (T->SetNum = to_clat;)
					T->TileNr = tile_c.Get_Tilenum_From_Set(to_clat, lat_transition);
				}
			}
		}
	}
}

int RA2_Map::Cutoff_Height() {
	int y = Size.h * 2 - 1;
	int highest_height = 0;
	for (int x = y % 2; x < Size.w * 2; x+=2) {
		if (Get_Tile(x, y)->Exists)
			highest_height = std::max(highest_height, Get_Tile(x, y)->Z);
	}
	return highest_height;
}

void RA2_Map::Fix_Ore() {
	for (int y = 0; y < Size.h * 2; y++) {
		for (int x = 0; x < Size.w * 2; x++) {
			Overlay* O = Get_Overlay(x, y);

			if (O->Exists) {
				// The value consists of the sum of all x's with a little magic offsets
				// plus the sum of all y's with also a little magic offset, and also
				// everything is calculated modulo 12

				double y_inc = ((( (y-9) / 2)%12) * (( (y-8) / 2)%12)) % 12;
				double x_inc = ((( (x-13) / 2)%12) * (( (x-12) / 2)%12)) % 12;

				// x_inc may be > y_inc so adding a big number outside of cell bounds
				// will surely keep num positive
				int num = y_inc - x_inc + 120000;
				num %= 12;
				if ((O->num >= MIN_ORE) && (O->num <= MAX_ORE)) {
					// replace ore
					O->num = MIN_ORE + num;
				}

				else if ((O->num >= MIN_GEMS) && (O->num <= MAX_GEMS)) {
					// replace gems
					O->num = MIN_GEMS + num;
				}
			}
		}
	}
}

void RA2_Map::Read_Overlay() {
	// Get OverlayPack section
	string concatenate = MapINI.get_section("overlaypack").get_concatenated_values();

	// decode from base64
	int lzo_size = concatenate.length() * 6 / 8;
	unsigned char* d = new unsigned char[lzo_size];
	int test = Decode_64(reinterpret_cast<const unsigned char*>(concatenate.c_str()), d);
	assert(abs(lzo_size - test) <= 3);

	// decode from lzo
	unsigned char OverlayPack[1 << 18];
	test = Decode_5(d, OverlayPack, test, 80);
	
	assert(test == 1 << 18);
	delete[] d;

	// Get OverlayDataPack section
	concatenate = MapINI.get_section("overlaydatapack").get_concatenated_values();

	// decode from base64
	lzo_size = concatenate.length() * 6 / 8;
	d = new unsigned char[lzo_size];
	test = Decode_64(reinterpret_cast<const unsigned char*>(concatenate.c_str()), d);
	assert(abs(lzo_size - test) <= 3);

	// decode from lzo
	unsigned char OverlayDataPack[1 << 18];
	test = Decode_5(d, OverlayDataPack, test, 80);
	assert(test == 1 << 18);
	delete[] d;

	// destination coordinates for tile (conversion from rhombus)
	for (int y = 0; y < Size.h * 2; y++) {
		for (int x = 0; x < Size.w * 2; x++) {
			Tile* T = Get_Tile(x, y);
			unsigned char* o = OverlayPack + T->RX + 512 * T->RY;
			if (T->Exists && (*o != 0xff)) {
				Overlay ovl;
				ovl.num = *o;
				ovl.sub = *(o - OverlayPack + OverlayDataPack);
				ovl.X = T->X;
				ovl.Y = T->Y;
				ovl.Z = T->Z;
				ovl.RX = T->RX;
				ovl.RY = T->RY;
				ovl.RZ = T->RZ;
				ovl.Exists = true;
				*Get_Overlay(x, y) = ovl;
			}
		}
	}
}

void RA2_Map::Read_Terrain() {
	if (!MapINI.set_current_section("terrain"))
		return;

	ini_section& terrain(MapINI.get_current_section());

	keymap::const_iterator it;

	for (it = terrain.begin(); it != terrain.end(); it++) {
		int pos = boost::lexical_cast<int>(it->first);
		int y = pos / 1000;
		int x = pos - 1000 * y;
		int dx = x - y + Size.w - 1;
		int dy = x + y - Size.w - 1;

		if (dx < 0 || dy < 0 || dx >= Size.w * 2 || dy >= Size.h * 2)
			continue;
		Tile* tl = Get_Tile(dx, dy);
		if (!tl->Exists)
			continue;

		Terrain* T = Get_Terrain(dx, dy);

		T->Exists = true;
		T->X = dx;
		T->Y = dy;
		T->Z = tl->Z;

		T->RX = tl->RX;
		T->RY = tl->RY;
		T->RZ = tl->RZ;
		T->name = it->second;
	}
}

void RA2_Map::Read_Smudges() {
	if (!MapINI.set_current_section("smudge"))
		return;

	ini_section& smudge(MapINI.get_current_section());

	keymap::const_iterator it;

	for (it = smudge.begin(); it != smudge.end(); it++) {
		string line = it->second;

		std::vector<std::string> splitline; // #2: Search for tokens
		boost::algorithm::split(splitline, line, boost::algorithm::is_any_of(","));

		int x = boost::lexical_cast<int>(splitline[1]);
		int y = boost::lexical_cast<int>(splitline[2]);
		int dx = x - y + Size.w - 1;
		int dy = x + y - Size.w - 1;

		if (dx < 0 || dy < 0 || dx >= Size.w * 2 || dy >= Size.h * 2)
			continue;
		Tile* tl = Get_Tile(dx, dy);
		if (!tl->Exists)
			continue;

		Smudge* S = Get_Smudge(dx, dy);
		S->Exists = true;
		S->X = dx;
		S->Y = dy;
		S->Z = tl->Z;
		S->RX = tl->RX;
		S->RY = tl->RY;
		S->RZ = tl->RZ;
		S->name = splitline[0];
	}
}

void RA2_Map::Read_Structures() {
	if (!MapINI.set_current_section("structures"))
		return;

	ini_section& structs(MapINI.get_current_section());

	keymap::const_iterator it;
	for (it = structs.begin(); it != structs.end(); it++) {
		std::string line = it->second;

		std::vector<std::string> splitline; // #2: Search for tokens
		boost::algorithm::split(splitline, line, boost::algorithm::is_any_of(","));

		if (splitline.size() < 5)
			continue;

		int x, y, health;
		try {
			x = boost::lexical_cast<int>(splitline[3]);
			y = boost::lexical_cast<int>(splitline[4]);
			try {
				health = boost::lexical_cast<int>(splitline[2]);
			}
			catch (std::exception& exc) {
				health = 256;
			}
		}
		catch (std::exception& exc) {
			continue;
		}

		int dx = x - y + Size.w - 1;
		int dy = x + y - Size.w - 1;

		if (dx < 0 || dy < 0 || dx >= Size.w * 2 || dy >= Size.h * 2)
			continue;
		Tile* tl = Get_Tile(dx, dy);
		if (!tl->Exists)
			continue;

		std::string structure = splitline[1];

		if (!is_lamp(structure)) {
			Structure* S = Get_Structure(dx, dy);
			S->Exists = true;
			S->X = dx;
			S->Y = dy;
			S->Z = tl->Z;
			S->RX = tl->RX;
			S->RY = tl->RY;
			S->RZ = tl->RZ;
			S->health = health;
			S->name = structure;
			S->owner = splitline[0];
			S->direction = boost::lexical_cast<int>(splitline[5]);
		}
	}
}

bool is_lamp(string lamp) {
	static const string lamps[] = {
		"REDLAMP", "BLUELAMP", "GRENLAMP", "YELWLAMP", "PURPLAMP", "INORANLAMP", "INGRNLMP", "INREDLMP", "INBLULMP", "INGALITE",
		"INYELWLAMP", "INPURPLAMP", "NEGLAMP", "NERGRED", "TEMMORLAMP", "TEMPDAYLAMP", "TEMDAYLAMP", "TEMDUSLAMP", "TEMNITLAMP", "SNOMORLAMP",
		"SNODAYLAMP", "SNODUSLAMP", "SNONITLAMP"
	};

	for (int i = 0; i < sizeof(lamps) / sizeof(string); i++) {
		if (lamp == lamps[i])
			return true;
	}
	return false;
}

void RA2_Map::Create_Object_Palets() {
	for (int y = 0; y < Size.h * 2; y++) {
		for (int x = 0; x < Size.w * 2; x++) {
			Tile* T = Get_Tile(x, y);
			if (T->Exists) {
				Palet_Type pt = P_ISO;
				const Palet& p(myTheater.Get_Palet(pt));
				T->P = p.Get_Copy_Height(T->Z);
			}
			else {
				continue;
			}

			Overlay* Ovl = Get_Overlay(x, y);
			if (Ovl->Exists) {
				Palet_Type pt = myTheater.OverlayTypes.Get_Palet(Ovl->num);
				//const Palet& p(myTheater.Get_Palet(pt));
				Ovl->P = myTheater.Get_Palet(pt).Get_Copy_Height(T->Z + myTheater.OverlayTypes.Get_Height_Offset(Ovl->num));
			} //MIN_ORE-MAX_ORE, MIN_GEMS-MAX_GEMS 102-127, 27-38

			Terrain* Ter = Get_Terrain(x, y);
			if (Ter->Exists) {
				Palet_Type pt = myTheater.TerrainTypes.Get_Palet(Ter->name);
				const Palet& p(myTheater.Get_Palet(pt));
				Ter->P = p.Get_Copy_Height(T->Z + myTheater.TerrainTypes.Get_Height_Offset(Ter->name));
			}

			Smudge* Smu = Get_Smudge(x, y);
			if (Smu->Exists) {
				Palet_Type pt = myTheater.SmudgeTypes.Get_Palet(Smu->name);
				const Palet& p = myTheater.Get_Palet(pt);
				Smu->P = p.Get_Copy_Height(T->Z + myTheater.SmudgeTypes.Get_Height_Offset(Smu->name));
			}

			Structure* Str = Get_Structure(x, y);
			if (Str->Exists) {
				// Building can also be defined in OverlayTypes, but it's probably just BuildingTypes.
				// Not a too great solution here :(
				Palet_Type pt;
				int idx = myTheater.BuildingTypes.Get_Image_Index(Str->name);
				if (idx != -1) {
					 pt = myTheater.BuildingTypes.Get_Palet(idx);
				}
				else {
					idx = myTheater.OverlayTypes.Get_Image_Index(Str->name);
					if (idx != -1)
						pt = myTheater.OverlayTypes.Get_Palet(idx);
				}
				if (idx == -1) {
					Str->Exists = false;
				}
				else {
					const Palet& p = myTheater.Get_Palet(pt);
					Str->P = p.Get_Copy_Height(T->Z + myTheater.BuildingTypes.Get_Height_Offset(Str->name));
					Str->P.Remap(myTheater.Get_Country_Remap(Str->owner));
				}
			}

			Infantry* Inf = Get_Infantry(x, y);
			if (Inf->Exists) {
				Palet_Type pt = myTheater.InfantryTypes.Get_Palet(Inf->name);
				const Palet& p = myTheater.Get_Palet(pt);
				Inf->P = p.Get_Copy_Height(T->Z + myTheater.InfantryTypes.Get_Height_Offset(Inf->name));
				Inf->P.Remap(myTheater.Get_Country_Remap(Inf->owner));
			}

			Unit* Unit = Get_Unit(x, y);
			if (Unit->Exists) {
				Palet_Type pt = myTheater.VehicleTypes.Get_Palet(Unit->name);
				const Palet& p = myTheater.Get_Palet(pt);
				Unit->P = p.Get_Copy_Height(T->Z + myTheater.VehicleTypes.Get_Height_Offset(Unit->name));
				Unit->P.Remap(myTheater.Get_Country_Remap(Unit->owner));
			}

			Aircraft* Aircraft = Get_Aircraft(x, y);
			if (Aircraft->Exists) {
				Palet_Type pt = myTheater.AircraftTypes.Get_Palet(Aircraft->name);
				const Palet& p = myTheater.Get_Palet(pt);
				Aircraft->P = p.Get_Copy_Height(T->Z + myTheater.AircraftTypes.Get_Height_Offset(Aircraft->name));
				Aircraft->P.Remap(myTheater.Get_Country_Remap(Aircraft->owner));
			}
		}
	}
}

bool RA2_Map::within_foundation(int x, int y, int i, int j, int fx, int fy) {
	// returns whether (x,y) lies within the foundation of the object
	// that is located at (i,j)

	// first we'll convert x, y, i and j to the original coordiante system

	// theses are the coordinates to test
	int rx = (x + y) / 2 + 1;
	int ry = y - rx + Size.w + 1;

	// this is the topleft corner of the foundation
	int ri = (i + j) / 2 + 1;
	int rj = j - ri + Size.w + 1;

	// return whether it's in a regular rectangle
	bool ret = ((x >= i) && (x < i + fx) && (y >= j) && (y < j + fy));;
	return ret;
}

bool RA2_Map::Can_Draw_Here(int x, int y) {
	// Checks whether the foundation of terrain of structure
	// should prevent overlay from being drawn here.

	// The largest possible foundation is 4x4 so we only need
	// to search in that area of the map.

	for (int i = x - 4; i <= x + 4; i++) {
		for (int j = y - 4; j <= y + 4; j++) {
			if (i < 0 || j < 0 || i >= Size.w * 2 || j >= Size.h * 2)
				continue;

			Terrain* t = Get_Terrain(i, j);
			if (t->Exists) {
				int fx = myTheater.TerrainTypes.Get_Foundation_X(t->name);
				int fy = myTheater.TerrainTypes.Get_Foundation_Y(t->name);

				if (within_foundation(x, y, i, j, fx, fy))
					return false;
			}

			Structure* s = Get_Structure(i, j);
			if (s->Exists) {
				int fx = myTheater.BuildingTypes.Get_Foundation_X(t->name);
				int fy = myTheater.BuildingTypes.Get_Foundation_Y(t->name);

				if (within_foundation(x, y, i, j, fx, fy))
					return false;
			}
		}
	}
	return true;
}

void RA2_Map::Draw() {
	int width =  Size.w * 60,
		height = Size.h * 30 + 115;

	MapSurface.SetDimensions(width, height);

	//for (int y_ = 0; y_ < (Size.h + Size.w) * 2; y_++) {
	//	int y = 0;
	//	for (int x = Size.w * 2 - y_; x < Size.w * 2; y++) {
	//		x++;
	for (int ry = 0; ry < Size.h * 2; ry++) {
		for (int rx = Size.w * 2 - 1; rx >= 0; rx--) {
			int x = rx - ry + Size.w - 1;
			int y = ry + rx - Size.w - 1;

			if (x < 0 || x >= Size.w * 2 || y < 0 || y >= Size.h * 2)
				continue;
			Tile* T = Get_Tile(x, y);

			// Draw overriding overlay shadows
			Overlay* O = Get_Overlay(x, y);
			bool draw_overlay_later = false;
			if (O->Exists && Can_Draw_Here(x, y)) {
				bool overr = myTheater.OverlayTypes.Get_Overrides(O->num);
				if (overr) {
					myTheater.Draw_Overlay(O, MapSurface);
				}
				else draw_overlay_later = true;
			}

			if (T->Exists) {
				myTheater.Draw_Tile(T, MapSurface);
			}

			if (draw_overlay_later)
				myTheater.Draw_Overlay_NoShadow(O, MapSurface);

			// Draw smudges
			Smudge* Smu = Get_Smudge(x, y);
			if (Smu->Exists) {
				myTheater.Draw_Smudge(Smu, MapSurface);
			}

			// Draw terrain
			Terrain* Ter = Get_Terrain(x, y);
			if (Ter->Exists) {
				myTheater.Draw_Terrain(Ter, MapSurface);
			}

			// Draw infantry
			Infantry* Inf = Get_Infantry(x, y);
			if (Inf->Exists) {
				myTheater.Draw_Infantry(Inf, MapSurface);
			}

			// Draw vehicles
			Unit* Unit = Get_Unit(x, y);
			if (Unit->Exists) {
				myTheater.Draw_Unit(Unit, MapSurface);
			}

			// Draw structures
			Structure* Str = Get_Structure(x, y);
			if (Str->Exists) {
				myTheater.Draw_Structure(Str, MapSurface);
			}

			// Draw aircraft
			Aircraft* Aircraft = Get_Aircraft(x, y);
			if (Aircraft->Exists) {
				myTheater.Draw_Aircraft(Aircraft, MapSurface);
			}
		}
	}
}

Tile* RA2_Map::Get_Tile(int x, int y) {
	return &TileLayer[Tile_Index(x,y)];
}

const Tile* RA2_Map::Get_Tile(int x, int y) const {
	return &TileLayer[Tile_Index(x,y)];
}

Overlay* RA2_Map::Get_Overlay(int x, int y) {
	return &OverlayLayer[Tile_Index(x,y)];
}

const Overlay* RA2_Map::Get_Overlay(int x, int y) const {
	return &OverlayLayer[Tile_Index(x,y)];
}

Smudge* RA2_Map::Get_Smudge(int x, int y) {
	return &SmudgeLayer[Tile_Index(x,y)];
}

const Smudge* RA2_Map::Get_Smudge(int x, int y) const {
	return &SmudgeLayer[Tile_Index(x,y)];
}

Terrain* RA2_Map::Get_Terrain(int x, int y) {
	return &TerrainLayer[Tile_Index(x,y)];
}

const Terrain* RA2_Map::Get_Terrain(int x, int y) const {
	return &TerrainLayer[Tile_Index(x,y)];
}

Structure* RA2_Map::Get_Structure(int x, int y) {
	return &StructLayer[Tile_Index(x,y)];
}

const Structure* RA2_Map::Get_Structure(int x, int y) const {
	return &StructLayer[Tile_Index(x,y)];
}

Infantry* RA2_Map::Get_Infantry(int x, int y) {
	return &InfantryLayer[Tile_Index(x,y)];
}

const Infantry* RA2_Map::Get_Infantry(int x, int y) const {
	return &InfantryLayer[Tile_Index(x,y)];
}

Unit* RA2_Map::Get_Unit(int x, int y) {
	return &UnitLayer[Tile_Index(x,y)];
}

const Unit* RA2_Map::Get_Unit(int x, int y) const {
	return &UnitLayer[Tile_Index(x,y)];
}

Aircraft* RA2_Map::Get_Aircraft(int x, int y) {
	return &AircraftLayer[Tile_Index(x,y)];
}

const Aircraft* RA2_Map::Get_Aircraft(int x, int y) const {
	return &AircraftLayer[Tile_Index(x,y)];
}

// Applies LightSources on every tile
void RA2_Map::Apply_Lamps() {
	// load all maps
	for (int y = 0; y < Size.h * 2; y++) {
		for (int x = 0; x < Size.w * 2; x++) {
			Tile* T = Get_Tile(x, y);
			if (!T->Exists) continue;
			// Lamps only 'work' if they're neutral
			Structure* S = Get_Structure(x, y);
			if (!S->Exists) continue;
			else if (S->owner != "neutral") continue;

			// We have a lamp here
			const Lighting* l = myTheater.Get_Lighting();
			LightSource LS(myTheater.Get_Object_INI(S->name), l);

			LS.Set_Position(T->X, T->Y, false);
			LS.Set_Position(T->RX, T->RY, true);

			LightSources.push_back(LS);
		}
	}

	for (int y = 0; y < Size.h * 2; y++) {
		for (int x = 0; x < Size.w * 2; x++) {
			Tile* T = Get_Tile(x, y);
			if (T->Exists) {
				vector<LightSource>::const_iterator it;
				for (it = LightSources.begin(); it != LightSources.end(); it++) {
					it->Apply_Lamp(T);
				}
			}
/*
			Overlay* Ovl = Get_Overlay(x, y);
			if (Ovl->Exists) {
				vector<LightSource>::const_iterator it;
				for (it = LightSources.begin(); it != LightSources.end(); it++) {
					it->Apply_Lamp(Ovl);
				}
			}

			Terrain* Ter = Get_Terrain(x, y);
			if (Ter->Exists) {
				vector<LightSource>::const_iterator it;
				for (it = LightSources.begin(); it != LightSources.end(); it++) {
					it->Apply_Lamp(Ter);
				}
			}

			Smudge* Smu = Get_Smudge(x, y);
			if (Smu->Exists) {
				vector<LightSource>::const_iterator it;
				for (it = LightSources.begin(); it != LightSources.end(); it++) {
					it->Apply_Lamp(Smu);
				}
			}*/
		}
	}
}

void RA2_Map::Apply_Palet_Overrides() {
	myTheater.Recalculate_Palets();

	for (int y = 0; y < Size.h * 2; y++) {
		for (int x = 0; x < Size.w * 2; x++) {
			Tile* T = Get_Tile(x, y);
			if (T->Exists) {
				T->P.Recalculate();
			}

			Overlay* Ovl = Get_Overlay(x, y);
			if (Ovl->Exists) {
				Ovl->P.Recalculate();
			}

			Terrain* Ter = Get_Terrain(x, y);
			if (Ter->Exists) {
				Ter->P.Recalculate();
			}

			Smudge* Smu = Get_Smudge(x, y);
			if (Smu->Exists) {
				Smu->P.Recalculate();
			}

			Structure* Str = Get_Structure(x, y);
			if (Str->Exists) {
				Str->P.Recalculate();
			}

			Infantry* Inf = Get_Infantry(x, y);
			if (Inf->Exists) {
				Inf->P.Recalculate();
			}

			Unit* Veh = Get_Unit(x, y);
			if (Veh->Exists) {
				Veh->P.Recalculate();
			}

			Aircraft* Aircraft = Get_Aircraft(x, y);
			if (Aircraft->Exists) {
				Aircraft->P.Recalculate();
			}
		}
	}
}

void RA2_Map::SaveJPEG(const std::string& path, const std::string& name, int quality) {
	// only visible part of map
	int left =  std::max(LocalSize.x * 60, 0),
		top  =  std::max(LocalSize.y * 30 - 90, 0);
	int width    =  LocalSize.w * 60;
	int height   =  LocalSize.h * 30 + 135;
	int height2  =  (Size.h - ((Cutoff_Height() / 2 - 1) % 2)) * 30;
	height = std::min(height, height2);

	MapSurface.SaveJPEG(path + name + ".jpg", quality, left, top, width, height);
}

void RA2_Map::SavePNG(const std::string& path, const std::string& name, int quality) {
	// only visible part of map
	int left =  std::max(LocalSize.x * 60, 0),
		top  =  std::max(LocalSize.y * 30 - 90, 0);
	int width    =  LocalSize.w * 60;
	int height   =  LocalSize.h * 30 + 135;
	int height2  =  (Size.h - ((Cutoff_Height() / 2 - 1) % 2)) * 30;
	height = std::min(height, height2);

	MapSurface.SavePNG(path + name + ".png", quality, left, top, width, height);
}

void RA2_Map::Draw_Startpos_Squares() {
	// no startpos for missions
		if (MapINI.get_section("basic").read_bool("MultiplayerOnly") == false) return;
	if (!MapINI.set_current_section("Waypoints")) return;

	ini_section& waypoints(MapINI.get_current_section());

	keymap::const_iterator it;
	for (it = waypoints.begin(); it != waypoints.end(); it++) {
		if (boost::lexical_cast<int>(it->first) >= 8)
			continue;

		int pos = boost::lexical_cast<int>(it->second);
		int y = pos / 1000;
		int x = pos - y * 1000;
		int dx = x - y + Size.w - 1;
		int dy = x + y - Size.w - 1;

		if (dx < 0 || dy < 0 || dx >= Size.w * 2 || dy >= Size.h * 2)
			continue;
		Tile* tl = Get_Tile(dx, dy);
		if (!tl->Exists)
			continue;

		int dest_x = dx * 30;
		int dest_y = dy * 15 - 15 * tl->Z + 15;

		bool vert = Size.h * 2 > Size.w;
		int radius = 0;
		if (vert) {
			radius = 10 * Size.h * 15 / 144;
		}
		else {
			radius = 10 * Size.w * 30 / 133;
		}

		int h = radius, w = radius;
		for (int draw_y = dest_y - h / 2; draw_y < dest_y + h; draw_y++) {
			for (int draw_x = dest_x - w / 2; draw_x < dest_x + w; draw_x++) {
				int red = 0x000000FF;
				unsigned char* w = (unsigned char*)MapSurface.Get_Lower_Bound() + MapSurface.Get_Stride() * draw_y + 3 * draw_x;
				if (MapSurface.Within_Bounds(w))
					memcpy(w, &red, 3);
			}
		}
	}
}

void RA2_Map::Draw_Startpos_Tiles() {
	// no startpos for missions
	if (MapINI.get_section("basic").read_bool("MultiplayerOnly") == false) return;
	if (!MapINI.set_current_section("Waypoints")) return;
	ini_section& waypoints(MapINI.get_current_section());

	static int fourxfour[][2] = {
				{+0, -2},
			{-1, -1}, {+1, -1},
		{-2, +0}, {+0, +0}, {+2, +0},
	{-3, +1}, {-1, +1}, {+1, +1}, {+3, +1},
		{-2, +2}, {+0, +2}, {+2, +2},
			{-1, +3}, {+1, +3},
				{+0, +4}
	};

	Palet red = Palet::MakeRedPalet();
	keymap::const_iterator it;
	for (it = waypoints.begin(); it != waypoints.end(); it++) {
		if (boost::lexical_cast<int>(it->first) >= 8)
			continue;

		int pos = boost::lexical_cast<int>(it->second);
		int y = pos / 1000;
		int x = pos - y * 1000;
		int dx = x - y + Size.w - 1;
		int dy = x + y - Size.w - 1;

		// Draw 4x4 cell around start pos
		for (int i = 0; i < 4 * 4; i++) {
			int xoff = fourxfour[i][0] + dx;
			int yoff = fourxfour[i][1] + dy;
			if (xoff >= 0 && yoff >= 0 && xoff < Size.w * 2 - 1 && yoff < Size.h * 2 - 1) {
				Tile* T = Get_Tile(xoff, yoff);
				T->P = Palet::MergePalets(T->P, red, 0.4);
			}
		}
	}
}

void RA2_Map::fuck_Ore() {
	Palet yellow = Palet::MakeYellowPalet();
	Palet purple = Palet::MakePurplePalet();

	for (int y = 0; y < Size.h * 2; y++) {
		for (int x = 0; x < Size.w * 2; x++) {
			Overlay* O = Get_Overlay(x, y);

			if (!O->Exists)
				continue;

			if ((O->num >= MIN_ORE) && (O->num <= MAX_ORE)) {
				// replace ore
				Tile* T = Get_Tile(x, y);
				T->P = Palet::MergePalets(yellow, T->P, O->sub / 11.0 * 0.6 + 0.1);
			}

			else if ((O->num >= MIN_GEMS) && (O->num <= MAX_GEMS)) {
				// replace gems
				Tile* T = Get_Tile(x, y);
				T->P = Palet::MergePalets(purple, T->P, O->sub / 11.0 * 0.6 + 0.25);
			}
		}
	}
}

void RA2_Map::Read_Infantry() {
	if (!MapINI.set_current_section("Infantry"))
		return;

	ini_section& infantry(MapINI.get_current_section());

	keymap::const_iterator it;
	for (it = infantry.begin(); it != infantry.end(); it++) {
		string line = it->second;

		std::vector<std::string> splitline; // #2: Search for tokens
		boost::algorithm::split(splitline, line, boost::algorithm::is_any_of(","));

		int x = boost::lexical_cast<int>(splitline[3]);
		int y = boost::lexical_cast<int>(splitline[4]);
		int dx = x - y + Size.w - 1;
		int dy = x + y - Size.w - 1;

		if (dx < 0 || dy < 0 || dx >= Size.w * 2 || dy >= Size.h * 2)
			continue;
		Tile* tl = Get_Tile(dx, dy);
		if (!tl->Exists)
			continue;

		Infantry* I = Get_Infantry(dx, dy);
		I->Exists = true;
		I->X = dx;
		I->Y = dy;
		I->Z = tl->Z;
		I->RX = tl->RX;
		I->RY = tl->RY;
		I->RZ = tl->RZ;
		I->owner = splitline[0];
		I->name = splitline[1];
		I->direction = boost::lexical_cast<int>(splitline[7]);
	}
}

void RA2_Map::Read_Units() {
	if (!MapINI.set_current_section("Units"))
		return;

	ini_section& units(MapINI.get_current_section());

	keymap::const_iterator it;
	for (it = units.begin(); it != units.end(); it++) {
		string line = it->second;

		std::vector<std::string> splitline; // #2: Search for tokens
		boost::algorithm::split(splitline, line, boost::algorithm::is_any_of(","));

		int x = boost::lexical_cast<int>(splitline[3]);
		int y = boost::lexical_cast<int>(splitline[4]);
		int dx = x - y + Size.w - 1;
		int dy = x + y - Size.w - 1;

		if (dx < 0 || dy < 0 || dx >= Size.w * 2 || dy >= Size.h * 2)
			continue;
		Tile* tl = Get_Tile(dx, dy);
		if (!tl->Exists)
			continue;

		Unit* U = Get_Unit(dx, dy);
		U->Exists = true;
		U->X = dx;
		U->Y = dy;
		U->Z = tl->Z;
		U->RX = tl->RX;
		U->RY = tl->RY;
		U->RZ = tl->RZ;
		U->owner = splitline[0];
		U->name = splitline[1];
		U->direction = boost::lexical_cast<int>(splitline[5]);
	}
}

void RA2_Map::Read_Aircraft() {
	if (!MapINI.set_current_section("Aircraft"))
		return;

	ini_section& aircraft(MapINI.get_current_section());

	keymap::const_iterator it;
	for (it = aircraft.begin(); it != aircraft.end(); it++) {
		string line = it->second;

		std::vector<std::string> splitline; // #2: Search for tokens
		boost::algorithm::split(splitline, line, boost::algorithm::is_any_of(","));

		int x = boost::lexical_cast<int>(splitline[3]);
		int y = boost::lexical_cast<int>(splitline[4]);
		int dx = x - y + Size.w - 1;
		int dy = x + y - Size.w - 1;

		if (dx < 0 || dy < 0 || dx >= Size.w * 2 || dy >= Size.h * 2)
			continue;
		Tile* tl = Get_Tile(dx, dy);
		if (!tl->Exists)
			continue;

		Aircraft* A = Get_Aircraft(dx, dy);
		A->Exists = true;
		A->X = dx;
		A->Y = dy;
		A->Z = tl->Z;
		A->RX = tl->RX;
		A->RY = tl->RY;
		A->RZ = tl->RZ;
		A->owner = splitline[0];
		A->name = splitline[1];
		A->direction = boost::lexical_cast<int>(splitline[5]);
	}
}

void RA2_Map::Read_StartPos() {
}
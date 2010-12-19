#pragma once

#include "TMP_Collection.h"
#include "SHP_Collection.h"
#include "TheaterDetails.h"
#include "MapTypes.h"
#include "Palet.h"

#include <boost/shared_ptr.hpp>
#include <string>
#include <map>
#include <sstream>

struct Iso_Map_Pack_Entry;
struct Tile;
struct Overlay;

class Theater {

private:
	std::string TheaterName;
	Map_Type MapType;
	Theater_Type TheaterType;

	std::string ext; // extension for tiles

	boost::shared_ptr<ini_file> TheaterINI;
	boost::shared_ptr<ini_file> RulesINI;
	boost::shared_ptr<ini_file> ArtINI;

	Palet p_iso, p_lib, p_unit, p_ovl, p_anim;

	std::map<std::string, hsv_color> named_colors; // <name, color>
	std::vector<std::string> houses; // <houses>
	std::vector<std::string> countries; // <country>
	std::map<std::string, hsv_color> remap_colors; // <country, color>
	
	Lighting myLighting;

public:
	SHP_Collection InfantryTypes;
	SHP_Collection VehicleTypes;
	SHP_Collection AircraftTypes;
	SHP_Collection BuildingTypes;
	SHP_Collection TerrainTypes;
	SHP_Collection SmudgeTypes;
	SHP_Collection OverlayTypes;
	TMP_Collection tmp_c;

	Theater() {}
	Theater(Map_Type M, Theater_Type T);
	// overload 1: call this one when rules.ini isn't loaded yet
	void Parse_INIs(Map_Type M, Theater_Type T);
	// overload 2: call this when a copy of rules.ini has been loaded before
	void Parse_INIs(Map_Type M, Theater_Type T, boost::shared_ptr<ini_file> rules);

	void Initialize(ini_file& map);

	void Load_Tiles();
	void Load_Colours();
	void Load_Countries();
	void Load_Houses(ini_file& map);
	void Load_Palettes(const Lighting& L);
	void Load_SHPs();

	void Override(ini_file& Map);
	const ini_section& Get_Object_INI(std::string section);

	// Draw a tile on surface dst from tmp collection
	void Draw_Tile(Tile* T, DrawingSurface& dst) const;

	// Draw a SHP on surface dst from shp collection OverlayTypes
	void Draw_Overlay(Overlay* O, DrawingSurface& dst);
	void Draw_Overlay_Shadow(Overlay* O, DrawingSurface& dst);
	void Draw_Overlay_NoShadow(Overlay* O, DrawingSurface& dst);

	// Draw a SHP on surface dst from shp collection SmudgeTypes
	void Draw_Smudge(Smudge* Smu, DrawingSurface& dst);

	// Draw a SHP on surface dst from shp collection TerrainTypes
	void Draw_Terrain(Terrain* T, DrawingSurface& dst);

	// Draw a SHP on surface dst from shp collection BuildingTypes
	void Draw_Structure(Structure* T, DrawingSurface& dst);
	
	// Draw a SHP on surface dst from shp collection InfantryTypes
	void Draw_Infantry(Infantry* T, DrawingSurface& dst);

	// Draw a SHP on surface dst from shp collection InfantryTypes
	void Draw_Unit(Unit* T, DrawingSurface& dst);
	
	// Draw a SHP or vxl on surface dst from shp collection InfantryTypes
	void Draw_Aircraft(Aircraft* T, DrawingSurface& dst);

	const TMP_Collection& Get_Tile_C() const;

	// Returns the tile index of the first tile of given tileset + num
	int Get_Tilenum_From_Set(int tileset, int num = 0) const;

	// Finds out tile set from tile index
	int Get_Set_From_Tilenum(int num) const;

	// Sets lighting scenario for global palettes
	void Set_Lighting(const ini_section& Lighting);

	// Returns scenario lighting
	const Lighting* Theater::Get_Lighting() const;

	// Returns one of the global palettes
	const Palet& Get_Palet(Palet_Type P) const;

	// Recalculates the global palettes
	void Recalculate_Palets();

	// Returns the colour to which an object object should have
	// its palette remapped.
	hsv_color Get_Country_Remap(std::string house);
	void RemapMultiplyUsedImages();
};


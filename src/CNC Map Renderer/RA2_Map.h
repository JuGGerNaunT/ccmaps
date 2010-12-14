#pragma once

#include "ini_file.h"
#include "Theater.h"
#include "Lighting.h"
#include "DrawingSurface.h"
#include "LightSource.h"

#include <string>

struct Map_Size {
	int x;
	int y;
	int w;
	int h;
};


class RA2_Map {

private: 
	Map_Type M;;
	Theater_Type T;
	std::vector<int> order_x;
	std::vector<int> order_y;
	
	ini_file &MapINI;
	boost::shared_ptr<ini_file> rules;
	
	Theater myTheater;

	DrawingSurface MapSurface;

	Map_Size Size;
	Map_Size LocalSize;

	int max_tile;

	std::vector<Tile> TileLayer;
	std::vector<Overlay> OverlayLayer;
	std::vector<Smudge> SmudgeLayer;
	std::vector<Terrain> TerrainLayer;
	std::vector<Structure> StructLayer;
	std::vector<Infantry> InfantryLayer;
	std::vector<Unit> UnitLayer;
	std::vector<Aircraft> AircraftLayer;
	std::vector<LightSource> LightSources;

	void ReadObjects();
	void DetermineMapType();
	void DetermineTheaterType();
	bool All_Objects_RA2();

public:
	RA2_Map(ini_file& Map, Map_Type M = UKN);

	~RA2_Map();
	const TMP_Collection& Get_Tile_C() const {
		return myTheater.Get_Tile_C();
	}

	// Initializes the map
	void LoadMap();

	// Draws the whole map
	void Draw();

	// Draw squares at start positions
	void Draw_Startpos_Squares();

	// Draw 2x2 squares at start positions
	void Draw_Startpos_Tiles();

	// Fuck up ore
	void fuck_Ore();

	// Loads tiling
	void Read_Tiles();

	// Loads overlay sections
	void Read_Overlay();

	// Loads infantry section
	void Read_Infantry();

	// Loads infantry section
	void Read_Units();

	// Loads infantry section
	void Read_Aircraft();

	// Loads smudges section
	void Read_Smudges();

	// Loads terrain section
	void Read_Terrain();

	// Loads structure section
	void Read_Structures();

	// Loads start positions
	void Read_StartPos();

	// Recalculates ore and gem patterns
	void Fix_Ore();

	// Returns height of the highest cell on the 'lowest' horizontal lines,
	// used to determine bitmap cutoff point
	int Cutoff_Height();

	// Returns pointer to Tile on map position (x,y)
	Tile* Get_Tile(int x, int y);
	const Tile* Get_Tile(int x, int y) const;

	// Returns pointer to Overlay on map position (x,y)
	const Overlay* Get_Overlay(int x, int y) const;
	Overlay* Get_Overlay(int x, int y);

	// Returns pointer to Terrain on map position (x,y)
	const Terrain* Get_Terrain(int x, int y) const;
	Terrain* Get_Terrain(int x, int y);

	// Returns pointer to Smudge on map position (x,y)
	const Smudge* Get_Smudge(int x, int y) const;
	Smudge* Get_Smudge(int x, int y);

	// Returns pointer to Infantry on map position (x,y)
	Infantry* Get_Infantry(int x, int y);
	const Infantry* Get_Infantry(int x, int y) const;
	
	// Returns pointer to Unit on map position (x,y)
	Unit* Get_Unit(int x, int y);
	const Unit* Get_Unit(int x, int y) const;
	
	// Returns pointer to Aircraft on map position (x,y)
	Aircraft* Get_Aircraft(int x, int y);
	const Aircraft* Get_Aircraft(int x, int y) const;

	// Returns the setnumber of tile at (x,y)
	int Get_Tile_Setnum(int x, int y, int assume) const;

	// Returns index in TileLayer array for tile at (x, y)
	unsigned int Tile_Index(int x, int y) const;
	void Fix_Tile_Layer();
	void Apply_Lamps();
	void Apply_Palet_Overrides();

	// Ignores localsize and uses Size instead. Useful on missions that reveal large map chunks later.
	void OverrideLocalSize() {
		LocalSize = Size;
	}

	void OverrideLocalSize(Map_Size s) {
		this->LocalSize = s;
	}

	void SaveJPEG(const std::string& path, const std::string& name, int quality);
	void SavePNG(const std::string& path, const std::string& name, int quality);
	const Structure* Get_Structure(int x, int y) const;
	Structure* Get_Structure(int x, int y);
	void Create_Object_Palets();
	bool Can_Draw_Here(int x, int y);
	bool within_foundation(int x, int y, int i, int j, int fx, int fy);

	Map_Type Get_MapType() { return M; }
	Theater_Type Get_TheaterType() { return T; }
};

bool is_lamp(std::string lamp);
#pragma once

#include "ini_file.h"
#include "TMP_File.h"
#include "TheaterDetails.h"
#include "Palet.h"
#include "MapTypes.h"

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

class TMP_Collection;

typedef short (TMP_Collection:: * ini_setnum) (void) const;
typedef std::pair<const ini_setnum, const ini_setnum> lat_connection;

struct TileSet {
	std::string SetName;
	std::string FileName;
	int TilesInSet;
};

struct TMP_Randset {
	std::vector< boost::shared_ptr<TMP_File> > TMPs;
	boost::shared_ptr<TMP_File> get_tile() const {
		if (TMPs.size() > 0)
			return TMPs[ rand()	% TMPs.size() ];
	}
};

class TMP_Collection {
private:

	std::vector<TMP_Randset> TMP_Fileset;
	std::vector<int> TileSetToNum;

	short ACliffMMPieces;		short ACliffPieces;			short BlackTile;
	short BridgeBottomLeft1;	short BridgeBottomLeft2;	short BridgeBottomRight1;
	short BridgeBottomRight2;	short BridgeMiddle1;		short BridgeMiddle2;
	short BridgeSet;			short BridgeTopLeft1;		short BridgeTopLeft2;
	short BridgeTopRight1;		short BridgeTopRight2;		short ClearTile;
	short ClearToGreenLat;		short ClearToPaveLat;		short ClearToRoughLat;
	short ClearToSandLat;		short CliffRamps;			short CliffSet;
	short DestroyableCliffs;	short DirtRoadCurve;		short DirtRoadJunction;
	short DirtRoadSlopes;		short DirtRoadStraight;		short DirtTrackTunnels;
	short DirtTunnels;			short GreenTile;			short HeightBase;
	short Ice1Set;				short Ice2Set;				short Ice3Set;
	short IceShoreSet;			short MMRampBase;			short MMWaterCliffAPieces;
	short Medians;				short MiscPaveTile;			short MonorailSlopes;
	short PaveTile;				short PavedRoadEnds;		short PavedRoadSlopes;
	short PavedRoads;			short RampBase;				short RampSmooth;
	short Rocks;				short RoughGround;			short RoughTile;
	short SandTile;				short ShorePieces;			short SlopeSetPieces;
	short SlopeSetPieces2;		short TrackTunnels;			short TrainBridgeSet;
	short Tunnels;				short WaterBridge;			short WaterCaves;
	short WaterCliffAPieces;	short WaterCliffs;			short WaterSet;
	short WaterfallEast;		short WaterfallNorth;		short WaterfallSouth;
	short WaterfallWest;		short WoodBridgeSet;

public:

	TMP_Collection();
	TMP_Collection(Theater_Type T);

	void Initialize(Theater_Type T);
	void Add_Tiles(const TileSet& ts, std::string ext);

	void Draw_Tile(const Tile* T, DrawingSurface& dst) const;

	int Get_Tilenum_From_Set(int tileset, int num = 0) const;
	int Get_Set_From_Tilenum(int num) const;

	short Get_Clear_Tile() const { return ClearTile; }
	short Get_Clear_To_Rough_Lat() const { return ClearToRoughLat; }
	short Get_Sand_Tile() const { return SandTile; }
	short Get_Clear_To_Sand_Lat() const { return ClearToSandLat; }
	short Get_Green_Tile() const { return GreenTile; }
	short Get_Clear_To_Green_Lat() const { return ClearToGreenLat; }
	short Get_Pave_Tile() const { return PaveTile; }
	short Get_Clear_To_Pave_Lat() const { return ClearToPaveLat; }
	short Get_Shore_Tile() const { return ShorePieces; }
	short Get_Waterbridge_Tile() const { return WaterBridge; }
	short Get_Pavedroads_Tile() const { return PavedRoads; }
	short Get_Medians_Tile() const { return Medians; }

	bool Is_LAT(int setnum) const;
	bool Is_CLAT(int setnum) const;
	int Get_LAT_Set(int setnum) const;
	int Get_CLAT_Set(int setnum) const;

	short eval(ini_setnum g) const;
	bool Do_Connect(int Set1, int Set2) const;
};

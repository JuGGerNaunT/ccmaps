#include "TMP_Collection.h"
#include "VFS.h"

using std::string;
using boost::lexical_cast;
using boost::shared_ptr;


short TMP_Collection::eval(ini_setnum g) const {
	return ((*this).*(g))();
} 

// Which tile pieces that are not of the same set are connected?
static const lat_connection ConnectLat[] = {
	lat_connection(&TMP_Collection::Get_Green_Tile, &TMP_Collection::Get_Shore_Tile),
	lat_connection(&TMP_Collection::Get_Green_Tile, &TMP_Collection::Get_Waterbridge_Tile),
	lat_connection(&TMP_Collection::Get_Pave_Tile, &TMP_Collection::Get_Pavedroads_Tile),
	lat_connection(&TMP_Collection::Get_Pave_Tile, &TMP_Collection::Get_Medians_Tile)
};

TMP_Collection::TMP_Collection() {
	srand(time(NULL));
}

TMP_Collection::TMP_Collection(Theater_Type T) {
	srand(time(NULL));
	Initialize(T);
}


void TMP_Collection::Initialize(Theater_Type T) {

	ini_file theater(vfs.open(Get_Theater_Filename(T)));
	ini_section& General(theater.get_section("General"));

	// Load general information
	// Could also get from INI but this caching mechanism will out-do in the end
#pragma region INI keys caching

	ACliffMMPieces 		= General.read_int("ACliffMMPieces", -1);
	ACliffPieces 		= General.read_int("ACliffPieces", -1);
	BlackTile 			= General.read_int("BlackTile", -1);
	BridgeBottomLeft1 	= General.read_int("BridgeBottomLeft1", -1);
	BridgeBottomLeft2 	= General.read_int("BridgeBottomLeft2", -1);
	BridgeBottomRight1 	= General.read_int("BridgeBottomRight1", -1);
	BridgeBottomRight2 	= General.read_int("BridgeBottomRight2", -1);
	BridgeMiddle1 		= General.read_int("BridgeMiddle1", -1);
	BridgeMiddle2 		= General.read_int("BridgeMiddle2", -1);
	BridgeSet 			= General.read_int("BridgeSet", -1);
	BridgeTopLeft1 		= General.read_int("BridgeTopLeft1", -1);
	BridgeTopLeft2 		= General.read_int("BridgeTopLeft2", -1);
	BridgeTopRight1 	= General.read_int("BridgeTopRight1", -1);
	BridgeTopRight2 	= General.read_int("BridgeTopRight2", -1);
	ClearTile 			= General.read_int("ClearTile", -1);
	ClearToGreenLat 	= General.read_int("ClearToGreenLat", -1);
	ClearToPaveLat 		= General.read_int("ClearToPaveLat", -1);
	ClearToRoughLat 	= General.read_int("ClearToRoughLat", -1);
	ClearToSandLat 		= General.read_int("ClearToSandLat", -1);
	CliffRamps 			= General.read_int("CliffRamps", -1);
	CliffSet 			= General.read_int("CliffSet", -1);
	DestroyableCliffs 	= General.read_int("DestroyableCliffs", -1);
	DirtRoadCurve 		= General.read_int("DirtRoadCurve", -1);
	DirtRoadJunction 	= General.read_int("DirtRoadJunction", -1);
	DirtRoadSlopes 		= General.read_int("DirtRoadSlopes", -1);
	DirtRoadStraight 	= General.read_int("DirtRoadStraight", -1);
	DirtTrackTunnels 	= General.read_int("DirtTrackTunnels", -1);
	DirtTunnels 		= General.read_int("DirtTunnels", -1);
	GreenTile 			= General.read_int("GreenTile", -1);
	HeightBase 			= General.read_int("HeightBase", -1);
	Ice1Set 			= General.read_int("Ice1Set", -1);
	Ice2Set 			= General.read_int("Ice2Set", -1);
	Ice3Set 			= General.read_int("Ice3Set", -1);
	IceShoreSet 		= General.read_int("IceShoreSet", -1);
	MMRampBase 			= General.read_int("MMRampBase", -1);
	MMWaterCliffAPieces = General.read_int("MMWaterCliffAPieces", -1);
	Medians 			= General.read_int("Medians", -1);
	MiscPaveTile 		= General.read_int("MiscPaveTile", -1);
	MonorailSlopes 		= General.read_int("MonorailSlopes", -1);
	PaveTile 			= General.read_int("PaveTile", -1);
	PavedRoadEnds 		= General.read_int("PavedRoadEnds", -1);
	PavedRoadSlopes 	= General.read_int("PavedRoadSlopes", -1);
	PavedRoads 			= General.read_int("PavedRoads", -1);
	RampBase 			= General.read_int("RampBase", -1);
	RampSmooth 			= General.read_int("RampSmooth", -1);
	Rocks	 			= General.read_int("Rocks", -1);
	RoughGround 		= General.read_int("RoughGround", -1);
	RoughTile 			= General.read_int("RoughTile", -1);
	SandTile 			= General.read_int("SandTile", -1);
	ShorePieces 		= General.read_int("ShorePieces", -1);
	SlopeSetPieces 		= General.read_int("SlopeSetPieces", -1);
	SlopeSetPieces2 	= General.read_int("SlopeSetPieces2", -1);
	TrackTunnels 		= General.read_int("TrackTunnels", -1);
	TrainBridgeSet 		= General.read_int("TrainBridgeSet", -1);
	Tunnels 			= General.read_int("Tunnels", -1);
	WaterBridge 		= General.read_int("WaterBridge", -1);
	WaterCaves 			= General.read_int("WaterCaves", -1);
	WaterCliffAPieces 	= General.read_int("WaterCliffAPieces", -1);
	WaterCliffs 		= General.read_int("WaterCliffs", -1);
	WaterSet 			= General.read_int("WaterSet", -1);
	WaterfallEast 		= General.read_int("WaterfallEast", -1);
	WaterfallNorth 		= General.read_int("WaterfallNorth", -1);
	WaterfallSouth 		= General.read_int("WaterfallSouth", -1);
	WaterfallWest 		= General.read_int("WaterfallWest", -1);
	WoodBridgeSet 		= General.read_int("WoodBridgeSet", -1);
#pragma endregion

	string ext = Get_Ext(T);
	// Load all tiles for our theater
	for (int i = 0; i < 9999; i++) {
		string section_name = "TileSet" + (boost::format("%04d") % i).str();
		TileSet ts;
		if (theater.set_current_section(section_name)) {
			ini_section& s(theater.get_current_section());
			ts.FileName = s.read_string("FileName");
			ts.SetName = s.read_string("SetName");
			ts.TilesInSet = s.read_int("TilesInSet");
			Add_Tiles(ts, ext);
		}

		else // last tileset found
			break;
	}
}


void TMP_Collection::Add_Tiles(const TileSet& ts, string ext) {
	TileSetToNum.push_back(TMP_Fileset.size());
	for (int i = 1; i <= ts.TilesInSet; i++) {

		TMP_Randset trs;
		for (char r = 'a' - 1; r <= 'z'; r++) {

			if ((r >= 'a') && ts.SetName == "Bridges")
				continue;
			
			// filename = set filename + dd + .tmp/.urb/.des etc
			string filename = ts.FileName + (boost::format("%02d") % i).str();
			if (r >= 'a') {
				filename += r;
			}
			filename += ext;
			shared_ptr<File> myFile = vfs.open(filename);
			if (myFile) {
				trs.TMPs.push_back( shared_ptr<TMP_File>(new TMP_File(myFile)) );
			}
			else {
				break;
			} 
		}
		TMP_Fileset.push_back(trs);
	}
}

void TMP_Collection::Draw_Tile(const Tile* T, DrawingSurface& dst) const {
	if (TMP_Fileset[T->TileNr].TMPs.size() > 0) {
		shared_ptr<TMP_File> Tf = TMP_Fileset[T->TileNr].get_tile();
		Tf->Draw(T->SubTile, T->X * 30, (T->Y - T->Z) * 15, T->Z, dst, T->P.Get_Colors());
	}
}

int TMP_Collection::Get_Set_From_Tilenum(int num) const {
	int find = 0, i = 1, prev = 0;
	while (i < TileSetToNum.size() && TileSetToNum[i] <= num) {
		i++;
	}
	return i - 1;
}

int TMP_Collection::Get_Tilenum_From_Set(int tileset, int num) const {
	return TileSetToNum[tileset] + num;
}

bool TMP_Collection::Is_LAT(int setnum) const {
	if (setnum == RoughTile) {
		return true;
	}

	else if (setnum == SandTile) {
		return true;
	}

	else if (setnum == GreenTile) {
		return true;
	}

	else if (setnum == PaveTile) {
		return true;
	}

	else {
		return false;
	}
}

bool TMP_Collection::Is_CLAT(int setnum) const {
	if (setnum == ClearToRoughLat) {
		return true;
	}

	else if (setnum == ClearToSandLat) {
		return true;
	}

	else if (setnum == ClearToGreenLat) {
		return true;
	}

	else if (setnum == ClearToPaveLat) {
		return true;
	}

	else {
		return false;
	}
}

int TMP_Collection::Get_LAT_Set(int setnum) const {
	if (setnum == ClearToRoughLat) {
		return RoughTile;
	}

	else if (setnum == ClearToSandLat) {
		return SandTile;
	}

	else if (setnum == ClearToGreenLat) {
		return GreenTile;
	}

	else if (setnum == ClearToPaveLat) {
		return PaveTile;
	}

	else {
		return -1;
	}

}

int TMP_Collection::Get_CLAT_Set(int setnum) const {
	if (setnum == RoughTile) {
		return ClearToRoughLat;
	}

	else if (setnum == SandTile) {
		return ClearToSandLat;
	}

	else if (setnum == GreenTile) {
		return ClearToGreenLat;
	}

	else if (setnum == PaveTile) {
		return ClearToPaveLat;
	}

	else {
		return -1;
	}
}

bool TMP_Collection::Do_Connect(int Set1, int Set2) const {
	if (Set1 == Set2)
		return false;

	int size = sizeof(ConnectLat) / sizeof(ConnectLat[0]);
	const lat_connection* it = &ConnectLat[0];
	while (size > 0) {
		int first = eval(it->first),
			second = eval(it->second);

		if ((Set1 == first && Set2 == second) || (Set1 == second && Set2 == first))
			return false;
		it++;
		size--;
	}
	return true;
}

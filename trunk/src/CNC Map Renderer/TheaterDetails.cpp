#include "TheaterDetails.h"
#include <boost/algorithm/string.hpp>

using std::string;


const string get_section_Name(SHP_Type S) {
	switch (S) {

		case S_AIRCRAFT:
			return "AircraftTypes";
			break;

		case S_BUILDING:
			return "BuildingTypes";
			break;

		case S_INFANTRY:
			return "InfantryTypes";
			break;

		case S_OVERLAY:
			return "OverlayTypes";
			break;

		case S_SMUDGE:
			return "SmudgeTypes";
			break;

		case S_TERRAIN:
			return "TerrainTypes";
			break;

		case S_VEHICLE:
			return "VehicleTypes";
			break;

		default:
			return "";
			break;
	}
}

const char Get_Theater_Prefix(Theater_Type T) {
	switch(T) {

		case T_TEMPERATE:
		case T_TEMPERATE_YR:
			return 't';
			break;

		case T_URBAN:
		case T_URBAN_YR:
			return 'u';
			break;

		case T_SNOW:
		case T_SNOW_YR:
			return 'a';
			break;

		case T_LUNAR:
			return 'l';
			break;

		case T_DESERT:
			return 'd';
			break;

		case T_NEWURBAN:
			return 'n';
			break;

		default:
			return 'g';
			break;
	}
}

Palet_Type Get_Palet_Type(SHP_Type S) {
	switch (S) {

		case S_AIRCRAFT:
		case S_BUILDING:
		case S_INFANTRY:
		case S_VEHICLE:
			return P_UNIT;
			break;

		case S_OVERLAY:
			return P_OVERLAY;
			break;

		case S_SMUDGE:
		case S_TERRAIN:
			return P_ISO;
			break;

		default:
			return P_ISO; // most compatible
	}
}




string Get_Ext(Theater_Type T) {

	switch(T) {

		case T_TEMPERATE:
		case T_TEMPERATE_YR:
			return ".tem";
			break;

		case T_URBAN:
		case T_URBAN_YR:
			return ".urb";
			break;

		case T_SNOW:
		case T_SNOW_YR:
			return ".sno";
			break;

		case T_LUNAR:
			return ".lun";
			break;

		case T_DESERT:
			return ".des";
			break;

		case T_NEWURBAN:
			return ".ubn";
			break;

		default:
			return "";
			break;
	}
}

string Get_Ext(Theater_Type T, SHP_Type S) {
	
	switch(S) {
		case S_OVERLAY:
		case S_SMUDGE:
		case S_BUILDING:
		case S_AIRCRAFT:
		case S_INFANTRY:
		case S_VEHICLE:
			return ".shp";
	}

	return Get_Ext(T);

}

string Get_Theater_Filename(Theater_Type T) {

	switch(T) {

		case T_TEMPERATE:
			return "temperat.ini";
			break;

		case T_TEMPERATE_YR:
			return "temperatmd.ini";
			break;

		case T_URBAN:
			return "urban.ini";
			break;

		case T_URBAN_YR:
			return "urbanmd.ini";
			break;

		case T_SNOW:
			return "snow.ini";
			break;

		case T_SNOW_YR:
			return "snowmd.ini";
			break;

		case T_LUNAR:
			return "lunarmd.ini";
			break;

		case T_DESERT:
			return "desertmd.ini";
			break;

		case T_NEWURBAN:
			return "urbannmd.ini";
			break;

		default:
			return "";
			break;
	}
}

string Get_Palet_Name(Theater_Type T, Palet_Type P) {

	string prefix;
	string postfix;

	switch (T) {
		case T_DESERT:
			break;
			
		case T_LUNAR:
			break;

		case T_NEWURBAN:
			break;

		case T_SNOW:
		case T_SNOW_YR:
			break;

		case T_TEMPERATE:
		case T_TEMPERATE_YR:
			break;

		case T_URBAN:
		case T_URBAN_YR:
			break;

	}
	return "";
}

bool Get_Shadow_Assumption(SHP_Type S) {

	switch(S) {
		case S_OVERLAY:
			return true;
		case S_SMUDGE:
			return false;
		case S_BUILDING:
			return true;
		case S_ANIM:
			return false;
		case S_AIRCRAFT:
			return false;
		case S_INFANTRY:
			return true;
		case S_TERRAIN:
			return true;
		case S_VEHICLE:
			return false;
		default:
			return false;
	}

}
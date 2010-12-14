#pragma once

#include <string>

enum Map_Type { 
	TS, RA2, YR, UKN 
};

enum Theater_Type { 
	T_TEMPERATE, T_TEMPERATE_YR, T_URBAN, T_URBAN_YR, T_SNOW, T_SNOW_YR, T_NEWURBAN, T_LUNAR, T_DESERT 
};

enum SHP_Type { 
	S_INFANTRY, S_VEHICLE, S_AIRCRAFT, S_BUILDING, S_TERRAIN, S_SMUDGE, S_OVERLAY, S_ANIM
};

enum Palet_Type { 
	P_ANIM, P_CITY, P_OVERLAY, P_ISO, P_UNIT, P_LIB
};

Palet_Type Get_Palet_Type(SHP_Type S);
const char Get_Theater_Prefix(Theater_Type T);
const std::string get_section_Name(SHP_Type S);
std::string Get_Ext(Theater_Type T);
std::string Get_Ext(Theater_Type T, SHP_Type S);
std::string Get_Theater_Filename(Theater_Type T);
std::string Get_Palet_Name(Theater_Type T);
Map_Type Get_Map_Kind(std::string theater, int max_tile);
bool Get_Shadow_Assumption(SHP_Type S);
#pragma once

#include "Palet.h"
#include "SHP_File.h"
#include <string>

struct Iso_Map_Pack_Entry {
	unsigned short x;
	unsigned short y;
	short tile;
	unsigned char zero1;
	unsigned char zero2;
	unsigned char sub_tile;
	unsigned char z;
	unsigned char zero3;
};

struct RA2Object {
	int X;
	int Y;
	int Z;
	int RX, RY, RZ;
	Palet P;
	bool Exists;
};

struct NameableObject : public RA2Object {
	std::string name;
};

struct OwnableObject : public NameableObject {
	std::string owner;
	int direction; // facing
};

struct Tile : public RA2Object {
	int SetNum;
	int TileNr;
	int SubTile;
};

struct Overlay : public RA2Object {
	int num;
	int sub;
};

struct Structure : public OwnableObject {
	int health;
};

struct Terrain : public NameableObject {
};

struct Smudge : public NameableObject {
};

struct Infantry : public OwnableObject {
	int health;
};

struct Unit : public OwnableObject {
	int health;
};

struct Aircraft : public OwnableObject {
	int health;
};
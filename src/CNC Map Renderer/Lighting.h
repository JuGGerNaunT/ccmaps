#pragma once

#include "ini_file.h"

struct Lighting {
	double level;
	double ambient;
	double red;
	double green;
	double blue;
	double ground;
	Lighting();
	Lighting(const ini_section& ini);
	void Read(const ini_section& ini);
};

/*
[Lighting]
Ambient=0.680000
Red=0.680000
Green=0.680000
Blue=1.000000
Ground=0.000000
Level=0.028000
IonAmbient=0.870000
IonRed=0.300000
IonGreen=0.400000
IonBlue=0.750000
IonGround=0.000000
IonLevel=0.000000
*/
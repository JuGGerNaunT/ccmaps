#include "LightSource.h"
#include <boost/lexical_cast.hpp>
#include "VFS.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <cctype>
#include <math.h>

using boost::lexical_cast;

LightSource::LightSource(const ini_section& lamp, const Lighting* Scenario) {
	Initialize(lamp, Scenario);
}


void LightSource::Initialize(const ini_section& lamp, const Lighting* Scenario){
	// Read and assume default values
	LightVisibility	=	lamp.read_double("lightvisibility", 5000.0);
	LightIntensity 	=	lamp.read_double("lightintensity", 0.0);
	LightRedTint	=	lamp.read_double("lightredtint", 1.0);
	LightGreenTint	=	lamp.read_double("lightgreentint", 1.0);
	LightBlueTint	=	lamp.read_double("lightbluetint", 1.0);

	this->Scenario = Scenario;
}

void LightSource::Apply_Lamp(RA2Object* T) const {

	if (LightIntensity == 0.0)
		return;

	double sqX = (RX - T->RX) * (RX - T->RX);
	double sqY = (RY - (T->RY)) * (RY - (T->RY));

	double distance = sqrt(sqX + sqY);
	// checks whether we're in range
	if ((0 < LightVisibility) && (distance < LightVisibility / 256)) {
		double lsEffect = (LightVisibility - 256 * distance) / LightVisibility;

		double aOff = lsEffect * LightIntensity;
		double rOff = lsEffect * LightRedTint;
		double gOff = lsEffect * LightGreenTint;
		double bOff = lsEffect * LightBlueTint;

		T->P.Adjust_Colors(rOff, gOff, bOff, aOff);
	}
}


void LightSource::Set_Position(int x, int y, bool isometric = false) {
	if (isometric) {
		RX = x;
		RY = y;
	}
	else {
		X = x;
		Y = y;
	}
}

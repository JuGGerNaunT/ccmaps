#include "Lighting.h"

#include <boost/lexical_cast.hpp>

using boost::lexical_cast;

Lighting::Lighting()
{
	red = 1.0;
	green = 1.0;
	blue = 1.0;
	ambient = 1.0;
	ground = 0.0;
	level = 0.00032;
}

Lighting::Lighting(const ini_section& ini) {
	Read(ini);
}

void Lighting::Read(const ini_section& ini) {
	assert(ini.get_name() == "lighting");

	ambient = ini.read_double("ambient", 1.0);
	level = ini.read_double("level", 0.00032);
	red = ini.read_double("red", 1.0);
	green = ini.read_double("green", 1.0);
	blue = ini.read_double("blue", 1.0);
	ground = ini.read_double("ground", 0.0);

}

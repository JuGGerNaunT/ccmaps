#pragma once

#include "File.h"
#include "Lighting.h"
#include "TheaterDetails.h"

#include <vector>
#include <boost/shared_ptr.hpp>

struct hsv_color;
struct rgb_color {
public:
	unsigned char r, g, b;
	rgb_color(std::string rgb);
	rgb_color(unsigned char r, unsigned char g, unsigned char b);
	rgb_color();
	hsv_color to_hsv();
};

struct hsv_color {
	unsigned char h;        // Hue degree between 0 and 255
	unsigned char s;        // Saturation between 0 (gray) and 255
	unsigned char v;        // Value between 0 (black) and 255
	hsv_color(std::string hsv);
	hsv_color();
	hsv_color(unsigned char h, unsigned char s, unsigned char v);
	rgb_color to_rgb();
};

class Palet {
private:
	std::vector<unsigned char> orig;
	std::vector<unsigned char> colors;
	double red_mult, green_mult, blue_mult;
	double ambient, level, ground;
	double aOff, rOff, gOff, bOff;

public:
	Palet();
	Palet(unsigned char* palet);
	Palet(unsigned char* palet, const Lighting& l);
	Palet(boost::shared_ptr<File> f);
	void Initialize(boost::shared_ptr<File> f);
	void Initialize(boost::shared_ptr<File> f, const Lighting& l);

	void Remap(hsv_color remap_base);

	Palet Get_Copy() const;
	void Set_Ambient(double ambient);
	static unsigned char limit(int nr);

	const unsigned char* Get_Colors() const;

	void Adjust_Colors(double rOff, double gOff, double bOff, double aOff);

	void Set_Colors(std::vector<unsigned char> colors);

	void Read_Colors(boost::shared_ptr<File> f, double rOff, double gOff, double bOff);
	void Recalculate();
	Palet Get_Copy_Height(int height) const;
	void Set_Height(int height);

	static Palet MakeRedPalet();
	static Palet MakeYellowPalet();
	static Palet MakePurplePalet();
	static Palet MakePalet(int r, int g, int b);
	static Palet MergePalets(Palet A, Palet B, double opacity);
};
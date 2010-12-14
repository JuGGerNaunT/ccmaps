#include "Palet.h"
#include <algorithm>
#include <math.h>
#include <string.h>

using std::vector;
using boost::shared_ptr;

void Palet::Adjust_Colors(double rOff, double gOff, double bOff, double aOff) {
	this->rOff += rOff;
	this->gOff += gOff;
	this->bOff += bOff;
	this->aOff += aOff;
}

Palet::Palet() {
	red_mult = green_mult = blue_mult = 1.0;
	rOff = gOff = bOff = aOff = 0;
	level = 0.032;
	ambient = 1.0;
}

Palet::Palet(unsigned char* palet) {
	orig.resize(768);
	memcpy(&orig[0], palet, 768);
	red_mult = green_mult = blue_mult = 1.0;
	rOff = gOff = bOff = aOff = 0;
	level = 0.032;
	ambient = 1.0;
}

Palet::Palet(unsigned char* palet, const Lighting& l) {
	orig.resize(768);
	memcpy(&orig[0], palet, 768);
	red_mult = green_mult = blue_mult = 1.0;
	rOff = gOff = bOff = aOff = 0;
	level = 0.032;
	ambient = 1.0;

	ambient = l.ambient;
	level = l.level;
	red_mult = l.red;
	green_mult = l.green;
	blue_mult = l.blue;
	ground = l.ground;
}

Palet::Palet(shared_ptr<File> f) {
	Initialize(f);
}

void Palet::Initialize(shared_ptr<File> f) {
	Lighting l;
	Initialize(f, l);
}

void Palet::Initialize(shared_ptr<File> f, const Lighting& l) {
	// Level is the amount of "more white" that is gained by highening tiles
	// Ambient is the overall amount of light/darkness
	// Red, green and blue define alterations in respective colors

	// Initialize palet from file
	f->read(orig, 768);

	ambient = l.ambient;
	level = l.level;
	red_mult = l.red;
	green_mult = l.green;
	blue_mult = l.blue;
	ground = l.ground;
	rOff = gOff = bOff = aOff = 0;
}

const unsigned char* Palet::Get_Colors() const {
	return &colors[0];
}

unsigned char Palet::limit(int x) {
	if (x > 255)
		return 255;
	if (x < 0)
		return 0;
	return x;
}

void Palet::Recalculate() {
	colors.resize(256 * 3);

	ambient = std::min(std::max(ambient + aOff, -1.6), 1.6);
	red_mult = std::min(std::max(red_mult + rOff, -1.8), 1.8);
	green_mult = std::min(std::max(green_mult + gOff, -1.8), 1.8);
	blue_mult = std::min(std::max(blue_mult + bOff, -1.8), 1.8);

	for (int i = 0; i < 256; i++) {
		colors[i*3+0] = limit((ambient * red_mult) * orig[i*3+0] / 63.0 * 255);
		colors[i*3+1] = limit((ambient * green_mult) * orig[i*3+1] / 63.0 * 255);
		colors[i*3+2] = limit((ambient * blue_mult) * orig[i*3+2] / 63.0 * 255);
	}
}

void Palet::Set_Ambient(double ambient) {
	this->ambient = ambient;
}

Palet Palet::Get_Copy() const {
	return *this;
}

Palet Palet::Get_Copy_Height(int height) const {
	Palet P_New = *this;
	P_New.Set_Height(height);
	P_New.orig = std::vector<unsigned char>(orig.begin(), orig.end());
	return P_New;
}

void Palet::Set_Height(int height) {
	ambient += level * height;
}

void Palet::Set_Colors(std::vector<unsigned char> colors) {
	this->colors = colors;
}

Palet Palet::MakeRedPalet() {
	Palet P;
	std::vector<unsigned char> c;
	c.resize(256 * 3);
	for (int i = 0; i < 3 * 256; i++) {
		if (i % 3 == 0) {
			// red
			c[i] = 255;
		}
		else {
			// green or blue
			c[i] = 0;
		}
	}
	P.Set_Colors(c);
	return P;
}

Palet Palet::MakePalet(int r, int g, int b) {
	Palet P;
	std::vector<unsigned char> c;
	c.resize(256 * 3);
	for (int i = 0; i < 3 * 256;) {
		c[i++] = r;
		c[i++] = g;
		c[i++] = b;
	}
	P.Set_Colors(c);
	return P;
}

Palet Palet::MakeYellowPalet() {
	return MakePalet(255, 255, 0);
}

Palet Palet::MakePurplePalet() {
	return MakePalet(128, 0, 128);
}

Palet Palet::MergePalets(Palet A, Palet B, double opacity) {
	Palet N;
	std::vector<unsigned char> c;
	c.resize(256 * 3);

	for (int i = 0; i < 3 * 256; i++) {
		c[i] = A.colors[i] * opacity + B.colors[i] * (1 - opacity);
	}

	N.Set_Colors(c);
	return N;
}

void Palet::Remap(hsv_color remap_base) {
	// load remap colors if needed

	//	; ******* Color Schemes *******
	//	; Each country must be assigned a color. This lists the various
	//	; colors available. The values represent the 'hue', 'saturation',
	//	; and 'value'. The 'value' component specifies the maximum brightness
	//	; allowed for the color as the color spread is generated. The 'hue'
	//	; component remains constant. The 'saturation' curves through color
	//	; space as the 'value' component changes such that darker colors
	//	; become more saturated.

	rgb_color rgb = remap_base.to_rgb();

	double mults[] = { 0xFC >> 2, 0xEC >> 2, 0xDC >> 2, 0xD0 >> 2,
						0xC0 >> 2, 0xB0 >> 2, 0xA4 >> 2, 0x94 >> 2,
						0x84 >> 2, 0x78 >> 2, 0x68 >> 2, 0x58 >> 2,
						0x4C >> 2, 0x3C >> 2, 0x2C >> 2, 0x20 >> 2 };

	double rmult = rgb.r / 255.0;
	double gmult = rgb.g / 255.0;
	double bmult = rgb.b / 255.0;

	for (int i = 16; i < 32; i++) {
		orig[i*3+0] = limit(rmult * mults[i - 16]);
		orig[i*3+1] = limit(gmult * mults[i - 16]);
		orig[i*3+2] = limit(bmult * mults[i - 16]);
	}
}

rgb_color::rgb_color(std::string rgb) {
	std::istringstream iss(rgb);
	char c;
	int r, g, b;
	iss >> r >> c >> g >> c >> b;
	this->r = Palet::limit(r);
	this->g = Palet::limit(g);
	this->b = Palet::limit(b);
}

rgb_color::rgb_color() {
}

rgb_color::rgb_color(unsigned char r, unsigned char g, unsigned char b) {
	this->r = r;
	this->g = g;
	this->b = b;
}

hsv_color rgb_color::to_hsv() {
	float r = this->r / 255.0;
	float g = this->g / 255.0;
	float b = this->g / 255.0;

	float min = std::min(r, std::min(g, b));
	float max = std::max(r, std::max(g, b));

	hsv_color ret;
	ret.v = max * 255.0;				// v

	float delta = max - min;

	if	(max != 0) {
		ret.s = 255 * (delta / max);		// s
	}
	else {
		// r = g = b = 0		// s = 0, v is undefined
		ret.s = 0;
		ret.h = -1;
		return ret;
	}

	int h_ = 0;
	if (r == max)
		h_ = (g - b) / delta;		// between yellow & magenta
	else if (g == max)
		h_ = 2 + (b - r) / delta;	// between cyan & yellow
	else
		h_ = 4 + (r - g) / delta;	// between magenta & cyan

	h_ *= 60;				// degrees
	if(h_ < 0 )
		h_ += 360;
	ret.h = h_ / 360.0 * 255.0;
}

hsv_color::hsv_color(std::string hsv) {
	std::istringstream iss(hsv);
	char c;
	int h, s, v;
	iss >> h >> c >> s >> c >> v;
	this->h = Palet::limit(h);
	this->s = Palet::limit(s);
	this->v = Palet::limit(v);
}

hsv_color::hsv_color(unsigned char h, unsigned char s, unsigned char v) {
	this->h = h;
	this->s = s;
	this->v = v;
}

hsv_color::hsv_color() {
}

rgb_color hsv_color::to_rgb() {
	float h = this->h / 255.0 * 360.0;
	float s = this->s / 255.0;
	float v = this->v / 255.0;

	int i;
	float f, p, q, t;

	if( s == 0 ) {
		// achromatic (grey)
		return rgb_color(v * 255.0, v * 255.0, v * 255.0);
	}

	h /= 60;			// sector 0 to 5
	i = floor(h);
	f = h - i;			// factorial part of h
	p = v * (1 - s);
	q = v * (1 - s * f);
	t = v * (1 - s * (1 - f));

	rgb_color ret;
	switch( i ) {
	case 0:
		ret.r = v * 255.0;
		ret.g = t * 255.0;
		ret.b = p * 255.0;
		break;
	case 1:
		ret.r = q * 255.0;
		ret.g = v * 255.0;
		ret.b = p * 255.0;
		break;
	case 2:
		ret.r = p * 255.0;
		ret.g = v * 255.0;
		ret.b = t * 255.0;
		break;
	case 3:
		ret.r = p * 255.0;
		ret.g = q * 255.0;
		ret.b = v * 255.0;
		break;
	case 4:
		ret.r = t * 255.0;
		ret.g = p * 255.0;
		ret.b = v * 255.0;
		break;
	default:		// case 5:
		ret.r = v * 255.0;
		ret.g = p * 255.0;
		ret.b = q * 255.0;
		break;
	}
	return ret;
}
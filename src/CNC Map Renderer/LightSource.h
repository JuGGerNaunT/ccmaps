
#include "ini_file.h"
#include "Palet.h"
#include "Lighting.h"
#include "MapTypes.h"

class LightSource {
private:

	double LightVisibility;
	double LightIntensity;
	double LightRedTint;
	double LightGreenTint;
	double LightBlueTint;

	const Lighting* Scenario;

	int X;
	int Y;
	int RX;
	int RY;

public:
	LightSource() { }
	LightSource(const ini_section& lamp, const Lighting* Scenario);
	void Initialize(const ini_section& lamp, const Lighting* Scenario);
	void Apply_Lamp(RA2Object* T) const;
	void Set_Position(int x, int y, bool isometric);
};
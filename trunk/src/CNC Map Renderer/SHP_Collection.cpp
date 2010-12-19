#include "SHP_Collection.h"
#include "VXL_File.h"
#include "HVA_File.h"

#include <boost/lexical_cast.hpp>

using std::string;
using boost::shared_ptr;

const std::string Extra_Images[] = {
	"ActiveAnim",	"ActiveAnimTwo",	"ActiveAnimThree",	"ActiveAnimFour",
	"SpecialAnim",	"SpecialAnimTwo",	"SpecialAnimThree",	"SpecialAnimFour",
	"BibShape", "Turret"
};

SHP_Collection::SHP_Collection() {
}

/*
LightBlueTint=
Specifies the amount of blue light emitted by this BuildingType if it emits light (i.e. its LightIntensity= is greater than the default value of 0). Defaults to 1.0.

LightGreenTint=
Specifies the amount of green light emitted by this BuildingType if it emits light (i.e. its LightIntensity= is greater than the default value of 0). Defaults to 1.0.

LightIntensity=
Specifies the amount light radiated by this BuildingType. Defaults to 0.0.

LightRedTint=
Specifies the amount of red light emitted by this BuildingType if it emits light (i.e. its LightIntensity= is greater than the default value of 0). Defaults to 1.0.

LightVisibility=
Specifies the distance, in leptons, that the light emitted from this BuildingType is visible from. Defaults to 5000.

NoShadow=
Can be set to 'yes' or 'no' and determines whether or not this object should cast a shadow. For SHP objects, it indicates whether or not the shadow images from this object's SHP sequence should be displayed. For VXL units, this sets the flag on the Locomotor= to indicate that the shadow index should be used to display the shadow.
*/

void SHP_Collection::Initialize(SHP_Type S, Theater_Type T,	shared_ptr<ini_file> RulesINI) {
	this->RulesINI = RulesINI;
	string section_name = get_section_Name(S);
	ini_section& Objects(RulesINI->get_section(section_name));
	keymap::const_iterator it;
	int num = 1;
	while (true) {
		string obj = Objects.read_string(boost::lexical_cast<string>(num));
		num++;
		if (obj == "") {
			if (num > Objects.num_keys())
				break;
			else
				continue;
		}
		Add_File_To_Image(obj, S, T);
	}
}

void SHP_Collection::Add_File_To_Image(string ObjName, SHP_Type S, Theater_Type T, int ysort) {
	SHP_Image s_img;

	// Load rules section for this image
	if (!RulesINI->set_current_section(ObjName)) {
	   images.push_back(s_img);
	   return; // skip if unexisting object
	}

	ini_section& ObjRules(RulesINI->get_current_section());
	s_img.set_name(ObjName);

	// Load art section for this image
	std::string art_section_name = ObjRules.read_string("Image");
	if (art_section_name == "")
	   art_section_name = ObjRules.get_name();
	ini_section art_section = RulesINI->get_section(art_section_name);

	bool palet_chosen = false;
	
	// Find filename for image
	string art_image = art_section.read_string("Image");
	if (art_image == "") {
		// last resort
		art_image = ObjRules.get_name();
	}

	string image_filename = art_image;

	// Skip rubbles
	if (ObjRules.read_bool("IsRubble")) {		
		images.push_back(s_img);
		// rubbles aren't drawn: see http://modenc.renegadeprojects.com/IsRubble
		return;
	}


	// Find out what extension to use
	bool is_voxel = art_section.read_bool("Voxel");
	bool TheaterExtension = ObjRules.read_bool("Theater");
	if (is_voxel) image_filename += ".vxl";
	else if (TheaterExtension) {
		image_filename += Get_Ext(T);
		if (S != S_OVERLAY) {
			s_img.Set_Palet(P_ISO);
			palet_chosen = true;
		}
	}
	else image_filename += Get_Ext(T, S);

	// See if a theater-specific image is used
	bool NewTheater = ObjRules.read_bool("NewTheater");
	if (NewTheater) {
		Apply_New_Theater(image_filename, T);
		s_img.Set_Palet(P_UNIT);
		palet_chosen = true;
	}

	// Used palet can be overriden
	std::string nult = ObjRules.read_string("NoUseTileLandType");
	if (nult != "") {
		s_img.Set_Palet(P_ISO);
		palet_chosen = true;
	}

	else if (ObjRules.read_bool("TerrainPalette")) {
	   s_img.Set_Palet(P_ISO);
	   palet_chosen = true;
	}

	else if (ObjRules.read_bool("AnimPalette")) {
	   s_img.Set_Palet(P_ANIM);
	   palet_chosen = true;
	}

	else if (ObjRules.read_bool("AltPalette")) {
		s_img.Set_Palet(P_UNIT);
		palet_chosen = true;
	}

	else if (ObjRules.read_string("Palette") == "lib") {
		s_img.Set_Palet(P_LIB);
		palet_chosen = true;
	}

	if (ObjRules.read_string("AlphaImage") != "") {
		std::string img_filename = ObjRules.read_string("AlphaImage") + ".shp";
		if (vfs.exists(img_filename)) {
			shared_ptr<SHP_File> s(new SHP_File(vfs.open(img_filename)));
			s_img.Set_AlphaImage(s);
		}
	}

	if (!palet_chosen) {
		// Set palet, determine by type of SHP collection
		s_img.Set_Palet(Get_Palet_Type(S));
	}

	bool shadow = Get_Shadow_Assumption(S);
	if (ObjRules.read_string("Shadow") != "") {
		shadow = ObjRules.read_bool("Shadow");
	}

	if (!ObjRules.read_bool("DrawFlat", true)) {
		shadow = true;
	}

	if (ObjRules.read_bool("Immune")) {
		// For example on TIBTRE / Ore Poles
		s_img.Set_Offset(0, -15);
		s_img.Set_Offset_Shadow(0, -15);
		s_img.Set_Palet(P_UNIT);
	}

	if (ObjRules.read_bool("BridgeRepairHut")) {
		s_img.Set_Offset(0, 0);
		s_img.Set_Offset_Shadow(0, 0);
	}

	if (ObjRules.read_string("Land") == "Rock") {
		s_img.Set_Offset(0, 15);
		s_img.Set_Offset_Shadow(0, 15);
	}

	else if (ObjRules.read_string("Land") == "Road") {
		s_img.Set_Offset(0, 15);
		s_img.Set_Offset_Shadow(0, 15);
	}

	if (ObjRules.read_bool("Overrides")) {
		s_img.Set_Height_Offset(4);
		s_img.Set_Overrides(true);
	}

	// Find out foundation
	std::string foundation = ObjRules.read_string("Foundation", "1x1");
	int fx = boost::lexical_cast<int>(foundation[0]);
	int fy = boost::lexical_cast<int>(foundation[2]);
	s_img.Set_Foundation(fx, fy);

	Do_Add(image_filename, s_img, 0, shadow, is_voxel);

	// Buildings often consist of multiple SHP files
	if (S == S_BUILDING) {
		Do_Add_Damaged(image_filename, s_img, 0, shadow);

		int i = sizeof(Extra_Images) / sizeof(Extra_Images[0]);
		while (--i >= 0) {
			std::string img = Extra_Images[i];
			std::string img_damaged = img + "Damaged";
			std::string extra_img = ObjRules.read_string(img);
			std::string extra_img_damaged = ObjRules.read_string(img_damaged);

			if (extra_img != "") {
				ini_section& extra_rules = RulesINI->get_section(extra_img);
				int ysort = extra_rules.read_int("YSort", ObjRules.read_int(img + "YSort"));
				bool extra_shadow = extra_rules.read_bool("Shadow", false); // additional building need shadows listed explicitly

				if (extra_rules.read_string("image") != "") {
					extra_img = extra_rules.read_string("image");
				}
				
				// default to activeanim if activeanimdamaged is not supplied
				if (extra_img_damaged == "")
					extra_img_damaged = extra_img;

				if (TheaterExtension)
					extra_img += Get_Ext(T);
				else
					extra_img += Get_Ext(T, S);

				if (NewTheater)
					Apply_New_Theater(extra_img, T);

				Do_Add(extra_img, s_img, ysort, extra_shadow);
			}

			if (extra_img_damaged != "") {
				ini_section& extra_rules_damaged = RulesINI->get_section(extra_img_damaged);
				int ysort = extra_rules_damaged.read_int("YSort", ObjRules.read_int(img + "YSort"));
				bool extra_shadow = extra_rules_damaged.read_bool("Shadow", false); // additional building need shadows listed explicitly

				if (extra_rules_damaged.read_string("image") != "")
					extra_img_damaged = extra_rules_damaged.read_string("image");

				if (TheaterExtension)
					extra_img_damaged += Get_Ext(T);
				else
					extra_img_damaged += Get_Ext(T, S);

				if (NewTheater)
					Apply_New_Theater(extra_img_damaged, T);

				Do_Add_Damaged(extra_img_damaged, s_img, ysort, extra_shadow);
			}
		}

		// Add fires
		std::string df0 = ObjRules.read_string("DamageFireOffset0");
		if (df0 != "") {
			s_img.Set_Anim_Palet(boost::shared_ptr<Palet>(p_anim));
			int x = boost::lexical_cast<int>(df0.substr(0, df0.find_first_of(',')));
			int y = boost::lexical_cast<int>(df0.substr(df0.find_first_of(',') + 1));
			s_img.Add_Fire("fire01.shp", x, y);
		}
		std::string df1 = ObjRules.read_string("DamageFireOffset1");
		if (df1 != "") {
			int x = boost::lexical_cast<int>(df1.substr(0, df1.find_first_of(',')));
			int y = boost::lexical_cast<int>(df1.substr(df1.find_first_of(',') + 1));
			s_img.Add_Fire("fire02.shp", x, y);
		}
		std::string df2 = ObjRules.read_string("DamageFireOffset2");
		if (df2 != "") {
			int x = boost::lexical_cast<int>(df2.substr(0, df2.find_first_of(',')));
			int y = boost::lexical_cast<int>(df2.substr(df2.find_first_of(',') + 1));
			s_img.Add_Fire("fire03.shp", x, y);
		}

		// Add turrets
		if (ObjRules.read_bool("Turret")) {
			string img = ObjRules.read_string("TurretAnim");
			img += ObjRules.read_bool("TurretAnimIsVoxel") ? ".vxl" : ".shp";
			Do_Add(img, s_img, 0, false, ObjRules.read_bool("TurretAnimIsVoxel"), 
				ObjRules.read_int("TurretAnimX"), ObjRules.read_int("TurretAnimY") - ObjRules.read_double("TurretAnimZAdjust") / 128.0);
		}
	}
	images.push_back(s_img);
}

Palet_Type SHP_Collection::Get_Palet(int img_index) {
	return images[img_index].Get_Palet();
}

Palet_Type SHP_Collection::Get_Palet(std::string img_name) {
	int idx = Get_Image_Index(img_name);
	if (idx >= 0) {
		return images[idx].Get_Palet();
	}
}

void SHP_Collection::Set_Palet(Palet_Type P, Palet* plt) {
	switch (P) {
		case P_ANIM: p_anim = plt; break;
		case P_ISO: p_iso = plt; break;
		case P_LIB: p_lib = plt; break;
		case P_OVERLAY: p_ovl = plt; break;
		case P_UNIT: p_unit = plt; break;
	}
}

const Palet* SHP_Collection::Get_Palet(Palet_Type P) const {
	switch (P) {
		case P_ANIM: return p_anim;
		case P_ISO: return p_iso;
		case P_LIB: return p_lib;
		case P_OVERLAY: return p_ovl;
		case P_UNIT: return p_unit;
	}
}

void SHP_Collection::Do_Add(std::string image_filename, SHP_Image& s_img, int ysort /*= 0*/, bool shadow, bool is_voxel, int x_offset, int y_offset) const {
	if (vfs.exists(image_filename)) {
		shared_ptr<SHP_File> s;
			
		if (!is_voxel) {
			s = shared_ptr<SHP_File>(new SHP_File(vfs.open(image_filename)));
			s->is_voxel = is_voxel;
			s->Set_YSort(ysort);
			s_img.Add_Image(s, shadow);
		}
		else {
			std::string hva = image_filename.substr(0, image_filename.length() - 4) + ".hva";
			boost::shared_ptr<File> file_hva = vfs.open(hva);
			boost::shared_ptr<File> file_vxl = vfs.open(image_filename);
			if (file_hva && file_vxl) {
				boost::shared_ptr<HVA_File> hva(new HVA_File(file_hva));
				boost::shared_ptr<VXL_File> vxl(new VXL_File(file_vxl, hva));
				vxl->Set_YSort(ysort);
				vxl->Set_Offset(x_offset, -y_offset);
				s_img.Add_Voxel(vxl, shadow);
			}
			else { 
				int i = 0; 
			}
		}
	}
}

void SHP_Collection::Do_Add_Damaged(std::string image_filename, SHP_Image& s_img, int ysort /*= 0*/, bool shadow, bool is_voxel, int x_offset, int y_offset) const {
	if (vfs.exists(image_filename)) {
		shared_ptr<SHP_File> s(new SHP_File(vfs.open(image_filename)));
		s->Set_YSort(ysort);
		s_img.Add_Damaged_Image(s, shadow);
	}
}

const SHP_Image& SHP_Collection::Get_Image(std::string image_filename) {
	for (std::vector<SHP_Image>::iterator it = images.begin(); it != images.end(); it++) {
		if (it->get_name() == image_filename) {
			return *it;
		}
	}
}

int SHP_Collection::Get_Image_Index(std::string image_filename) {
	for (int i = 0; i < images.size(); i++) {
		if (images[i].get_name() == image_filename)
			return i;
	}
	return -1;
}

void SHP_Collection::Apply_New_Theater(string &image_filename, Theater_Type T) {
	image_filename[1] = Get_Theater_Prefix(T);
	if (!vfs.exists(image_filename)) {
		image_filename[1] = 'g';
	}
}

void SHP_Collection::Draw_SHP(int num, int sub, int x, int y, int z, const int direction, DrawingSurface& dst, const Palet* p) {
	images[num].Draw(sub, x, y, z, direction, dst, p);
}

void SHP_Collection::Draw_SHP_Shadow(int num, int sub, int x, int y, int z, const int direction, DrawingSurface& dst) {
	images[num].Draw_Shadow(sub, x, y, z, direction, dst);
}

void SHP_Collection::Draw_SHP_NoShadow(int num, int sub, int x, int y, int z, const int direction, DrawingSurface& dst, const Palet* p) {
	images[num].Draw_NoShadow(sub, x, y, z, direction, dst, p);
}

void SHP_Collection::Draw_Damaged_SHP(int num, int sub, int x, int y, int z, const int direction, DrawingSurface& dst, const Palet* p) {
	images[num].Draw_Damaged(sub, x, y, z, direction, dst, p);
}

void SHP_Collection::Draw_Damaged_SHP_Shadow(int num, int sub, int x, int y, int z, const int direction, DrawingSurface& dst) {
	images[num].Draw_Damaged_Shadow(sub, x, y, z, direction, dst);
}

void SHP_Collection::Draw_Damaged_SHP_NoShadow(int num, int sub, int x, int y, int z, const int direction, DrawingSurface& dst, const Palet* p) {
	images[num].Draw_Damaged_NoShadow(sub, x, y, z, direction, dst, p);
}

int SHP_Collection::Get_X_Offset(int idx) {
	if (idx >= 0)
		return images[idx].Get_X_Offset();
	return 0;
}

int SHP_Collection::Get_X_Offset(std::string image) {
	return Get_X_Offset(Get_Image_Index(image));
}

int SHP_Collection::Get_Y_Offset(int idx) {
	if (idx >= 0)
		return images[idx].Get_Y_Offset();
	return 0;
}

int SHP_Collection::Get_Y_Offset(std::string image) {
	return Get_Y_Offset(Get_Image_Index(image));
}

int SHP_Collection::Get_Height_Offset(int idx) {
	if (idx >= 0)
		return images[idx].Get_Height_Offset();
	return 0;
}

int SHP_Collection::Get_Height_Offset(std::string image) {
	return Get_Height_Offset(Get_Image_Index(image));
}

int SHP_Collection::Get_Foundation_X(int idx) {
	if (idx >= 0)
		return images[idx].Get_Foundation_X();
	return 0;
}

int SHP_Collection::Get_Foundation_X(std::string image) {
	return Get_Foundation_X(Get_Image_Index(image));
}

int SHP_Collection::Get_Foundation_Y(int idx) {
	if (idx >= 0)
		return images[idx].Get_Foundation_Y();
	return 0;
}

int SHP_Collection::Get_Foundation_Y(std::string image) {
	return Get_Foundation_Y(Get_Image_Index(image));
}

bool SHP_Collection::Get_Overrides(int idx) {
	if (idx >= 0)
		return images[idx].Get_Overrides();
	return false;
}

bool SHP_Collection::Get_Overrides(std::string image) {
	return Get_Overrides(Get_Image_Index(image));
}
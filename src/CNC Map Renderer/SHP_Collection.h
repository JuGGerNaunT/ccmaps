#pragma once

#include "SHP_File.h"
#include "ini_file.h"
#include "TheaterDetails.h"

#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include "DrawingSurface.h"


class SHP_Collection {
 private:

	 SHP_Type S;
	 std::vector<SHP_Image> images;

	 boost::shared_ptr<ini_file> RulesINI;

	 Palet* p_iso;
	 Palet* p_lib;
	 Palet* p_unit;
	 Palet* p_ovl;
	 Palet* p_anim;

public:

	SHP_Collection();
	void Initialize(SHP_Type S, Theater_Type T, boost::shared_ptr<ini_file> RulesINI);
	void Add_File_To_Image(std::string ObjName, SHP_Type S, Theater_Type T, int ysort = 0);
	void Apply_New_Theater(std::string &image_filename, Theater_Type T);
	void Do_Add(std::string image_filename, SHP_Image &s_img, int ysort = 0, bool shadow = false, bool is_voxel = false, int x_offset = 0, int y_offset = 0) const;
	void Do_Add_Damaged(std::string image_filename, SHP_Image &s_img, int ysort = 0, bool shadow = false, bool is_voxel = false, int x_offset = 0, int y_offset = 0) const;
	
	const Palet* Get_Palet(Palet_Type P) const;
	Palet_Type Get_Palet(int img_index);
	Palet_Type Get_Palet(std::string img_name);
	void Set_Palet(Palet_Type P, Palet* plt);
	const SHP_Image& Get_Image(std::string image_filename);

	void Draw_SHP(int num, int sub, int x, int y, int z, DrawingSurface& dst, const Palet* p);
	void Draw_SHP_Shadow(int num, int sub, int x, int y, int z, DrawingSurface& dst);
	void Draw_SHP_NoShadow(int num, int sub, int x, int y, int z, DrawingSurface& dst, const Palet* p);

	void Draw_Damaged_SHP(int num, int sub, int x, int y, int z, DrawingSurface& dst, const Palet* p);
	void Draw_Damaged_SHP_Shadow(int num, int sub, int x, int y, int z, DrawingSurface& dst);
	void Draw_Damaged_SHP_NoShadow(int num, int sub, int x, int y, int z, DrawingSurface& dst, const Palet* p);

	int Get_Image_Index(std::string image_filename);
	int Get_X_Offset(int idx);
	int Get_X_Offset(std::string image);
	int Get_Y_Offset(int idx);
	int Get_Y_Offset(std::string image);
	int Get_Height_Offset(int idx);
	int Get_Height_Offset(std::string image);
	int Get_Foundation_X(int idx);
	int Get_Foundation_X(std::string image);
	int Get_Foundation_Y(int idx);
	int Get_Foundation_Y(std::string image);
	bool Get_Overrides(int idx);
	bool Get_Overrides(std::string image);
};
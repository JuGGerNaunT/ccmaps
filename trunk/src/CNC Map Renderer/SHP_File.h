#pragma once

#include "File.h"
#include "Palet.h"
#include "VFS.h"
#include "DrawingSurface.h"
#include "VXL_File.h"
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

using boost::int8_t;
using boost::int16_t;
using boost::int32_t;
using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;

struct t_shp_header {
	int16_t c_images;
	int16_t unknown1;
	int16_t unknown2;
	int16_t cx;
	int16_t cy;
	int32_t unknown3;
};

struct t_shp_ts_header {
	int16_t zero;
	int16_t cx;
	int16_t cy;
	int16_t c_images;
};

struct t_shp_ts_image_header {
	int16_t x;
	int16_t y;
	int16_t cx;
	int16_t cy;
	int32_t compression;
	int32_t unknown;
	int32_t zero;
	int32_t offset;
};

class SHP_File {
private:
	std::vector<bool> is_decoded;
	std::vector<unsigned char> shp_data;
	std::vector< std::vector<unsigned char> > decoded_datas;
	int Y_Sort;
	bool initialized; // tracks whether we're not having a unexisting image here
	boost::shared_ptr<File> f;
	SHP_File() { initialized = false; }
public:
	bool is_voxel;

	SHP_File(boost::shared_ptr<File> f);
	SHP_File(std::vector<unsigned char> v);
	void Draw(const int img, const int x, const int y, DrawingSurface& dst, const unsigned char* colors);
	void Draw_Shadow(int img, const int x, const int y, const int height, const int direction, DrawingSurface &dst);
	boost::shared_ptr<SHP_File> Get_SHP();
	void Initialize(); // reads from disk
	void Set_YSort(int ysort);

	// inline functions returning pointers to locations in the file
	const unsigned char* get_data() const {
		return &shp_data[0];
	}

	const t_shp_ts_header* get_ts_header() const {
		return reinterpret_cast<const t_shp_ts_header*>(get_data());
	}

	const t_shp_header* get_header() const {
		return reinterpret_cast<const t_shp_header*>(get_data());
	}

	int get_size() {
		return initialized ? shp_data.size() : (f ? f->size() : 0);
	}

	int get_cx() const 	{
		// returns a value from the header
		return get_ts_header()->cx;
	}

	int num_images() const {
		return get_ts_header()->c_images;
	}

	int get_cy() const {
		// returns a value from the header
		return get_ts_header()->cy;
	}

	const t_shp_ts_image_header* get_image_header(int i) const {
		// returns an image header
		const unsigned char* d = get_index() + sizeof(t_shp_ts_image_header) * i;
		return reinterpret_cast<const t_shp_ts_image_header*>(d);
	}

	int get_x(int i) const {
		// returns a value from an image header
		return get_image_header(i)->x;
	}

	int get_y(int i) const {
		// returns a value from an image header
		return get_image_header(i)->y;
	}

	const unsigned char* get_image(int i) const {
		// returns a pointer to the start of an image
		return reinterpret_cast<const unsigned char*>(get_data() + get_image_header(i)->offset);
	}

	const unsigned char* get_index() const {
		// returns the index
		return reinterpret_cast<const unsigned char*>(get_data() + sizeof(t_shp_ts_header));
	}
	void DrawAlpha(const int img, const int x, const int y, DrawingSurface& dst);
};

struct Fire {
	boost::shared_ptr<SHP_File> shp;
	int x_offset;
	int y_offset;
	void Set_Offset(int x, int y) {
		x_offset = x;
		y_offset = y;
	}
	Fire() {}
};

class SHP_Image {
private:
	std::vector< boost::shared_ptr<SHP_File> > Images;
	std::vector< boost::shared_ptr<VXL_File> > Voxels;
	std::vector< boost::shared_ptr<SHP_File> > DamagedImages;
	boost::shared_ptr<SHP_File> AlphaImage;
	std::vector< bool > Shadows;
	std::vector< bool > DamagedShadows;
	std::vector< Fire > Fires;
	std::string name;
	Palet_Type P;
	int x_offset;
	int y_offset;
	int foundation_x;
	int foundation_y;
	int x_offset_shadow;
	int y_offset_shadow;
	int height_offset;
	bool overrides;

	boost::shared_ptr<Palet> firepalet;

public:
	SHP_Image();

	// Offset in X. As far as I know, this is always 0.
	int Get_X_Offset() const;
	// Ofset in Y. Used to center/align for example rocks and bridges.
	int Get_Y_Offset() const;
	// Offset in height. Used to ensure the correct level is added to the ambient for
	// certain bridges.
	int Get_Height_Offset() const;
	void Set_Offset(int x, int y);
	void Set_Height_Offset(int h);
	void Set_Overrides(bool o);
	bool Get_Overrides();
	void Set_Offset_Shadow(int x, int y);
	int Get_Offset_X_Shadow();
	int Get_Offset_Y_Shadow();

	int Get_Foundation_X();
	int Get_Foundation_Y();
	void Set_Foundation(int x, int y);

	// Looks in the VFS for image named 'filename' and adds to Images
	bool Add_Image(std::string filename);

	// Add given image to Images
	void Add_Image(boost::shared_ptr<SHP_File> f, bool shadow = false);
	
	// Looks in the VFS for image named 'filename' and adds to Images
	bool Add_Damaged_Image(std::string filename);

	// Add given image to Images
	void Add_Damaged_Image(boost::shared_ptr<SHP_File> f, bool shadow = false);
	
	// Add given voxel to Voxels
	void Add_Voxel( boost::shared_ptr<VXL_File> f, bool shadow /*= false*/ );

	// Draw using given pallette. May have been altered by lightsource.
	void Draw(const int img, const int x, const int y, const int height, const int direction, DrawingSurface& dst, const Palet* p);
	void Draw_NoShadow(int img, const int x, const int y, const int height, const int direction, DrawingSurface& dst, const Palet* p);
	void Draw_Shadow(int img, const int x, const int y, const int height, const int direction, DrawingSurface& dst);

	// Draw using given pallette. May have been altered by lightsource.
	void Draw_Damaged(const int img, const int x, const int y, const int height, const int direction, DrawingSurface& dst, const Palet* p);
	void Draw_Damaged_NoShadow(int img, const int x, const int y, const int height, const int direction, DrawingSurface& dst, const Palet* p);
	void Draw_Damaged_Shadow(int img, const int x, const int y, const int height, const int direction, DrawingSurface& dst);

	void Set_Palet(Palet_Type P);
	Palet_Type Get_Palet();
	void set_name(std::string filename);
	const std::string& get_name() const;

	void Set_AlphaImage(boost::shared_ptr<SHP_File> f);
	void Draw_Fires(const int x, const int y, DrawingSurface& dst);

	void Add_Fire(std::string img_name, int x_off, int y_off) {
		Fire f;
		f.shp = boost::shared_ptr<SHP_File>(new SHP_File(vfs.open(img_name)));
		f.Set_Offset(x_off, y_off);
		Fires.push_back(f);
	}

	void Set_Anim_Palet(boost::shared_ptr<Palet> firepalet) {
		this->firepalet = firepalet;
	}
	
	const SHP_File& Get_Image(int idx) const { return *(this->Images[idx]); }
	const int Num_Images() const { return this->Images.size(); }

};
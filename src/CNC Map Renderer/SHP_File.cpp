#include "SHP_File.h"

using std::vector;
using std::string;
using boost::shared_ptr;

// Finds the position in the pixel data array
// for where to write on (x,y)
unsigned char* get_ofs(unsigned char* w, int x, int y, int stride) {
	// 3 bytes per pixel
	return w + 3 * x + y * stride;
}

int* get_ofs(int* w, int x, int y, int stride) {
	// 3 bytes per pixel
	return w + x + y * stride;
}

bool* get_ofs(bool* w, int x, int y, int stride) {
	// 3 bytes per pixel
	return w + x + y * stride;
}

// Most commonly used compression method for RA2 SHP files
int decode3(const unsigned char* s, vector<unsigned char>& d, int cx, int cy) {
	const unsigned char* r = s;
	unsigned char* w = &d[0];
	for (int y = 0; y < cy; y++) {
		int count = *reinterpret_cast<const unsigned short*>(r) - 2;
		r += 2;
		int x = 0;
		while (count-- > 0) {
			int v = *r++;
			if (v) {
				x++;
				*w++ = v;
			}
			else {
				count--;
				v = *r++;
				if (x + v > cx)
					v = cx - x;
				x += v;
				while (v--)
					*w++ = 0;
			}
		}
	}
	return w - &d[0];
}

// Only called when a SHP is about to be drawn the first time
// Decompression happens on the fly
SHP_File::SHP_File(shared_ptr<File> f) {
	initialized = false;
	this->f = f;
}

SHP_File::SHP_File(std::vector<unsigned char> v) {
	shp_data.resize(v.size());
	memcpy(&shp_data[0], &v[0], v.size());
	int c = get_ts_header()->c_images;
	// Vector used to indicate whether an image is already decompressed
	is_decoded.resize(c);
	for (int i = 0; i < c; i++)
		is_decoded[i] = false;
	// Vector<byte*> where pointers to each decompressed image are stored
	decoded_datas.resize(c);
	initialized = true;
}

void SHP_File::Initialize() {
	if (f && !initialized) {
		f->seek_start();
		f->read(shp_data, f->size());
		initialized = true;

		int c = get_ts_header()->c_images;
		// Vector used to indicate whether an image is already decompressed
		is_decoded.resize(c);
		for (int i = 0; i < c; i++)
			is_decoded[i] = false;
		// Vector<byte*> where pointers to each decompressed image are stored
		decoded_datas.resize(c);
		initialized = true;
	}
}

// - img = image in shp to Draw
// - bd = struct containing info on where to Draw on the bitmap
// - stride used to determine where to write (see get_ofs())
// - int* colors points to the palet data on the height of
//   the tile where the shp is to be drawn
void SHP_File::Draw(const int img, const int x, const int y, DrawingSurface& dst, const unsigned char* colors) {
	if (!initialized) Initialize();
	t_shp_ts_header t = *get_ts_header();

	if (img > t.c_images)
		return;

	t_shp_ts_image_header h = *get_image_header(img);
	unsigned int c_px = h.cx * h.cy;
	unsigned int stride = dst.Get_Stride();

	if (c_px <= 0)
		return;

	// This image is not yet decoded, do so now
	if (!is_decoded[img]) {
		decoded_datas[img].resize(c_px);
		// Store the pointer for later reference
		// (if this SHP image gets drawn twice, for example)

		// This image is encoded in format 3
		if (h.compression & 2)
			decode3(get_image(img), decoded_datas[img], h.cx, h.cy);
		else
			memcpy(&decoded_datas[img][0], get_image(img), c_px);

		// Now it is decoded for sure :)
		is_decoded[img] = true;
	}

	// d will point to the decoded image data
	unsigned char* d = &decoded_datas[img][0];

	// Find out where to write
	unsigned char* w = get_ofs(dst.Get_Lower_Bound(),
		x + 30 - t.cx / 2 + h.x, // Pretty stupid formula
		y - t.cy / 2 + h.y,		// that needs the sizes of the entire SHP
		stride);

	bool* shadows = get_ofs(dst.Get_Shadows(),
		x + 30 - t.cx / 2 + h.x,
		y - t.cy / 2 + h.y,
		dst.Get_Width());

	for (int y = 0; y < h.cy; y++) {
		for (int x = 0; x < h.cx; x++) {
			if (*d && dst.Within_Bounds(w)) {
				memcpy(w, &colors[*d * 3], 3);
			}

			d++;	// Up to the next pixel
			w += 3; // Don't recalculate on every pixel,
			// but just assume we're still on the same row...
		}
		w += stride - 3 * h.cx;	// ... and if we're no more on the same row,
		// adjust the writing pointer accordingy
	}
}

// Terrain, infantry and buildings usually have shadows
// No need for a palet
// int* shadows tells us if we "shadowed" data at a certain pixel already,
// This is, so that overlapping don't get darkened twice
void SHP_File::Draw_Shadow(int img, const int x, const int y, const int height, const int direction, DrawingSurface& dst) {
	if (!initialized) Initialize();
	// If there's shadow data, the latter half of all images form the shadows
	// With 20 SHPS, image 9 would have shadow 9 + 20 / 2 = 19
	t_shp_ts_header t = *get_ts_header();
	img += t.c_images / 2;

	if (img >= t.c_images)
		return;

	t_shp_ts_image_header h = *get_image_header(img);
	unsigned int c_px = h.cx * h.cy;
	if (c_px <= 0)
		return;

	unsigned int stride = dst.Get_Stride();

	// Same as for SHP_File::Draw
	// This image is not yet decoded, do so now
	if (!is_decoded[img]) {
		decoded_datas[img].resize(c_px);
		// Store the pointer for later reference
		// (if this SHP image gets drawn twice, for example)

		// This image is encoded in format 3
		if (h.compression & 2)
			decode3(get_image(img), decoded_datas[img], h.cx, h.cy);
		else
			memcpy(&decoded_datas[img][0], get_image(img), c_px);

		// Now it is decoded for sure :)
		is_decoded[img] = true;
	}

	// d will point to the decoded image data
	unsigned char* d = &decoded_datas[img][0];

	// Find out where to write
	unsigned char* w = get_ofs(dst.Get_Lower_Bound(),
		x + 30 - t.cx / 2 + h.x,
		y - t.cy / 2 + h.y, stride);

	bool* shadows = get_ofs(dst.Get_Shadows(),
		x + 30 - t.cx / 2 + h.x,
		y - t.cy / 2 + h.y,
		dst.Get_Width());

	 int* z = get_ofs(dst.Get_ZOrders(),
		x + 30 - t.cx / 2 + h.x,
		y - t.cy / 2 + h.y,
		dst.Get_Width());

	for (int y = 0; y < h.cy; y++) {
		for (int x = 0; x < h.cx; x++) {
			// Calculate which pixel we're talking about,
			// for looking up if it's shadowed already in int* shadows
			if (*d &&				// This pixel is shadowed
				dst.Within_Bounds(w) &&	// Not shadowed yet in bitmap
				(height >= *z) && // This object is above the pixel where we'll cast a shadow
				(*shadows == false)) { // and it's not shadowed yet
					// Half R, G, and B values for this pixel
					*(w+0) /= 2;
					*(w+1) /= 2;
					*(w+2) /= 2;
					*shadows = true;
					*z = std::max(*z, height);
			}
			d++;
			w += 3;
			shadows++;
			z++;
		}
		w += stride - 3 * h.cx;
		z += dst.Get_Width() - h.cx;
		shadows += dst.Get_Width() - h.cx;
	}
}

void SHP_File::DrawAlpha(const int img, const int x, const int y, DrawingSurface& dst) {
	if (!initialized) Initialize();
	t_shp_ts_header t = *get_ts_header();

	if (img > t.c_images)
		return;

	t_shp_ts_image_header h = *get_image_header(img);
	unsigned int c_px = h.cx * h.cy;
	unsigned int stride = dst.Get_Stride();

	if (c_px <= 0)
		return;

	// This image is not yet decoded, do so now
	if (!is_decoded[img]) {
		decoded_datas[img].resize(c_px);
		// Store the pointer for later reference
		// (if this SHP image gets drawn twice, for example)

		// This image is encoded in format 3
		if (h.compression & 2)
			decode3(get_image(img), decoded_datas[img], h.cx, h.cy);
		else
			memcpy(&decoded_datas[img][0], get_image(img), c_px);

		// Now it is decoded for sure :)
		is_decoded[img] = true;
	}

	// d will point to the decoded image data
	unsigned char* d = &decoded_datas[img][0];

	// Find out where to write
	unsigned char* w = get_ofs(dst.Get_Lower_Bound(),
		x + 30 - t.cx / 2 + h.x, // Pretty stupid formula
		y - t.cy / 2 + h.y,		// that needs the sizes of the entire SHP
		stride);

	for (int y = 0; y < h.cy; y++) {
		for (int x = 0; x < h.cx; x++) {
			if (*d && dst.Within_Bounds(w)) {
				double mult = *d / 128.0;
				*(w+0) = Palet::limit(*(w+0) * mult);
				*(w+1) = Palet::limit(*(w+1) * mult);
				*(w+2) = Palet::limit(*(w+2) * mult);
				w += 3;
			}
			d++;
		}
		w += stride - 3 * h.cx;
	}
}

void SHP_File::Set_YSort(int ysort) {
	Y_Sort = ysort;
}

void SHP_Image::Draw(const int img, const int x, const int y, const int height, const int direction, DrawingSurface& dst, const Palet* p) {
	for (vector< shared_ptr<SHP_File> >::iterator i = Images.begin(); i != Images.end(); i++) {
		(*i)->Draw(img, x + x_offset, y + y_offset - height * 15, dst, p->Get_Colors());
	}
	for (vector< shared_ptr<VXL_File> >::iterator i = Voxels.begin(); i != Voxels.end(); i++) {
		int xoff = x + (*i)->X_Offset;
		int yoff = y + - height * 15+ (*i)->Y_Offset;
		(*i)->Draw(xoff, yoff, direction, dst, p);
	}
	Draw_Shadow(img, x, y, height, direction, dst);
	if (AlphaImage)
		AlphaImage->DrawAlpha(0, x + x_offset, y + y_offset - height * 15, dst);
}

void SHP_Image::Draw_Shadow(int img, const int x, const int y, const int height, const int direction, DrawingSurface& dst) {
	for (int i = 0; i < Images.size(); i++) {
		if (Shadows[i]) {
			Images[i]->Draw_Shadow(img, x + x_offset_shadow, y + y_offset_shadow - height * 15, height, direction, dst);
		}
	}
}

void SHP_Image::Draw_NoShadow(int img, const int x, const int y, const int height, const int direction, DrawingSurface& dst, const Palet* p) {
	vector< shared_ptr<SHP_File> >::iterator i;
	Palet p_unit(vfs.open("unittem.pal"));
	for (int i = 0; i < Images.size(); i++) {
		if (Shadows[i]) {
			Images[i]->Draw(img, x + x_offset_shadow, y + y_offset_shadow - height * 15, dst, (Images[i]->is_voxel ? p_unit.Get_Colors() : p->Get_Colors()));
		}
	}
}

void SHP_Image::Draw_Damaged(const int img, const int x, const int y, const int height, const int direction, DrawingSurface& dst, const Palet* p) {
	vector< shared_ptr<SHP_File> >::iterator i;
	for (i = DamagedImages.begin(); i != DamagedImages.end(); i++) {
		(*i)->Draw(img, x + x_offset, y + y_offset - height * 15, dst, p->Get_Colors());
	}
	Draw_Damaged_Shadow(img, x, y, height, direction, dst);
	if (AlphaImage)
		AlphaImage->DrawAlpha(0, x + x_offset, y + y_offset - height * 15, dst);
	Draw_Fires(x, y - height * 15, dst);
}

void SHP_Image::Draw_Damaged_Shadow(int img, const int x, const int y, const int height, const int direction, DrawingSurface& dst) {
	for (int i = 0; i < DamagedImages.size(); i++) {
		if (DamagedShadows[i]) {
			DamagedImages[i]->Draw_Shadow(img, x + x_offset_shadow, y + y_offset_shadow - height * 15, height, direction, dst);
		}
	}
}

void SHP_Image::Draw_Damaged_NoShadow(int img, const int x, const int y, const int height, const int direction, DrawingSurface& dst, const Palet* p) {
	for (int i = 0; i < Images.size(); i++) {
		DamagedImages[i]->Draw(img, x + x_offset_shadow, y + y_offset_shadow - height * 15, dst, p->Get_Colors());
	}
	Draw_Fires(x, y - height * 15, dst);
}

void SHP_Image::Draw_Fires(const int x, const int y, DrawingSurface& dst) {
	for (int i = 0; i < Fires.size(); i++) {
		Fires[i].shp->Initialize();
		Fires[i].shp->Draw(rand() % Fires[i].shp->num_images(),
			x + Fires[i].x_offset,
			y + Fires[i].y_offset,
			dst, firepalet->Get_Colors());
	}
}

void SHP_Image::Add_Image( boost::shared_ptr<SHP_File> f, bool shadow /*= false*/ ) {
	Images.push_back(f);
	Shadows.push_back(shadow);
}

void SHP_Image::Add_Voxel( boost::shared_ptr<VXL_File> f, bool shadow /*= false*/ ) {
	Voxels.push_back(f);
	Shadows.push_back(shadow);
}

void SHP_Image::Add_Damaged_Image( boost::shared_ptr<SHP_File> f, bool shadow /*= false*/ ) {
	DamagedImages.push_back(f);
	DamagedShadows.push_back(shadow);
}

void SHP_Image::set_name(std::string filename) {
	name = filename;
}

void SHP_Image::Set_Palet(Palet_Type P) {
	this->P = P;
}

void SHP_Image::Set_Offset(int x, int y) {
	x_offset = x;
	y_offset = y;
}

void SHP_Image::Set_Offset_Shadow(int x, int y) {
	x_offset_shadow = x;
	y_offset_shadow = y;
}

void SHP_Image::Set_Height_Offset(int h) {
	height_offset = h;
}

void SHP_Image::Set_Overrides(bool o) {
	overrides = o;
}

bool SHP_Image::Get_Overrides() {
	return overrides;
}

SHP_Image::SHP_Image() {
	x_offset = 0;
	y_offset = 0;
	x_offset_shadow = 0;
	y_offset_shadow = 0;
	height_offset = 0;
	overrides = false;
}

const std::string& SHP_Image::get_name() const {
	return name;
}

Palet_Type SHP_Image::Get_Palet() {
	return P;
}

int SHP_Image::Get_X_Offset() const {
	return x_offset;
}

int SHP_Image::Get_Y_Offset() const {
	return y_offset;
}

int SHP_Image::Get_Height_Offset() const {
	return height_offset;
}

int SHP_Image::Get_Offset_X_Shadow() {
	return x_offset_shadow;
}

int SHP_Image::Get_Offset_Y_Shadow() {
	return y_offset_shadow;
}

int SHP_Image::Get_Foundation_X() {
	return foundation_x;
}

int SHP_Image::Get_Foundation_Y() {
	return foundation_y;
}

void SHP_Image::Set_Foundation(int x, int y) {
	foundation_x = x;
	foundation_y = y;
}

void SHP_Image::Set_AlphaImage(boost::shared_ptr<SHP_File> f) {
	AlphaImage = f;
}
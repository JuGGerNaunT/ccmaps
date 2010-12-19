#pragma once

#include "File.h"
#include <vector>
#include <stdint.h>
#include <string>
#include "Palet.h"
#include "DrawingSurface.h"
#include "HVA_File.h"

#define RA2_NUM_NORMALS		244
#define TS_NUM_NORMALS		36

class VXL_File {
private:
	bool initialized;
	boost::shared_ptr<File> vxl;
	boost::shared_ptr<HVA_File> hva;
	std::vector<unsigned char> vxl_data;

public:
	static char const fileTypeText[];
	static float const RA2Normals[RA2_NUM_NORMALS][3];
	static float const TSNormals[TS_NUM_NORMALS][3];

	struct Header {
		uint8_t fileType[16];	/* "Voxel Animation" */
		uint32_t unknown;		/* == 1 */
		uint32_t numLimbs;		/* Number of limbs/bodies/tailers */
		uint32_t numLimbs2;		/* == numLimbs */
		uint32_t bodySize;		/* Total size in bytes of all limb bodies */
		uint8_t startPaletteRemap;	/* (?) Palette remapping for player colours (?) */
		uint8_t endPaletteRemap;		
		std::vector<unsigned char> palette;		/* RGB colour palette */
	};
	struct LimbHeader {
		uint8_t name[16];		/* Limb name (Zero terminated) */
		uint32_t number;		/* Limb number */
		uint32_t unknown;		/* == 1 */
		uint32_t unknown2;		/* == 0 or == 2 (Documentation is contradictory) */
	};
	struct LimbBody {
		int32_t* spanStart;
		int32_t* spanEnd;
		struct Span {
			uint8_t numVoxels;
			struct Voxel {
				uint8_t colour;
				uint8_t normal;
				bool used;
				Voxel() : used(true) { }
			} *voxel;
			Span() : voxel(NULL) { }
			~Span() { delete[] voxel; }
			void alloc() {
				delete[] voxel;
				voxel = new Voxel[numVoxels];
			}
		} *span;
		LimbBody() : spanStart(NULL), spanEnd(NULL), span(NULL) { }
		~LimbBody() {
			delete[] spanStart;
			delete[] spanEnd;
			delete[] span;
		}
		void alloc(unsigned int n) {
			delete[] spanStart;
			spanStart = new int32_t[n];
			delete[] spanEnd;
			spanEnd = new int32_t[n];
			delete[] span;
			span = new Span[n];
		}
	};
	struct LimbTailer {
		uint32_t spanStartOff;		/* Offset into body section to span start list */
		uint32_t spanEndOff;		/* Offset into body section to span end list */
		uint32_t spanDataOff;		/* Offset into body section to span data */
		float scale;			/* Scale factor for the section always seems to be 0.083333 */
		float transform[3][4];		/* Transformation matrix */
		float minBounds[3];		/* Voxel bounding box */
		float maxBounds[3];
		uint8_t xSize;			/* Width of voxel limb */
		uint8_t ySize;			/* Breadth of voxel limb */
		uint8_t zSize;			/* Height of voxel limb */
		uint8_t normalType;		/* 2 == TS Normals, 4 == RA2 Normals */
	};

protected:
	Header header;
	LimbHeader* limbHeaders;
	LimbBody* limbBodies;
	LimbTailer* limbTailers;

	uint32_t currentLimb;

	void readPalette(unsigned int& spos);
	void readLimbHeader(LimbHeader*, unsigned int& spos);
	void readLimbBody(uint32_t, unsigned int& spos);
	void readLimbTailer(LimbTailer*, unsigned int& spos);
	uint8_t decompressVoxels(LimbBody::Span::Voxel*, uint8_t, unsigned int& spos);

public:
	VXL_File(boost::shared_ptr<File> vxl, boost::shared_ptr<HVA_File> hva) : vxl(vxl), hva(hva) {
		initialized = false;
		Y_Sort = 0;
		X_Offset = 0;
		Y_Offset = 0;
		//if (f->size() < 0) throw;
	}
	~VXL_File();
	void Initialize();

	bool getVoxel(uint8_t, uint8_t, uint8_t, LimbBody::Span::Voxel*);
	const std::vector<unsigned char>& getPalette();
	void getXYZNormal(uint8_t, float&, float&, float&);
	void getSize(uint8_t&, uint8_t&, uint8_t&);
	std::string limbName();
	void getBounds(float*, float*);
	void getTotalBounds(int&, int&);
	float getScale();
	void loadGLMatrix(float*);
	uint32_t getNumLimbs();
	void setCurrentLimb(uint32_t);
	void setCurrentLimb(std::string const&);
		
	int Y_Sort;
	int X_Offset;
	int Y_Offset;
	void Set_YSort(int ysort) { this->Y_Sort = ysort; }
	void Set_Offset(int xoff, int yoff) { this->X_Offset = xoff; this->Y_Offset = yoff; }

	void Draw(const int x, const int y, const int direction, DrawingSurface& dst, const Palet* p);
	void setPalette(const Palet* p) {
		// just overwrite old one
		memcpy(&this->header.palette[0], p->Get_Colors(), 768);
	}
};
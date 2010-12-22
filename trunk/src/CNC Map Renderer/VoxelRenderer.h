#pragma once

#include "VXL_File.h"
#include "HVA_File.h"
#include <stdint.h>
#include "DrawingSurface.h"

class VoxelRenderer {
private:
	static float lightPos[4];
	static float lightSpec[4];
	static float lightDiffuse[4];
	static float lightAmb[4];
	bool initialized;
	void SetupGleeFBO(DrawingSurface* dst);
	void SetupMesaFBO(DrawingSurface* dst);
	void SetupFrameRender(DrawingSurface* dst);
	int object_rotation;
	DrawingSurface* frame_renderbuffer;
	
#ifdef GLUT
	unsigned int fbo, depthbuffer, rgb_rb;
#endif

protected:
	void renderSection();
	void renderVoxel(float, float, float, float);

	VXL_File* vxl;
	HVA_File* hva;
	
public:
	uint32_t frame;
	float pitch;

	VoxelRenderer() : frame(0), pitch(0), initialized(false) { }
	~VoxelRenderer();

	DrawingSurface* render(VXL_File* vxl, HVA_File* hva, int object_rotation = 0, const Palet* p = NULL);
	void init();
};

extern VoxelRenderer voxelrenderer;
#include "config.h"
#include <iostream>
#include "VoxelRenderer.h"
#include <exception>

#ifdef GLUT
	#include "GLee.h"
#else
	#include <GL/osmesa.h>
#endif

#include <GL/glut.h>
/*
float VoxelRenderer::lightPos[4] = { 5, 0, 10, 0, };
float VoxelRenderer::lightSpec[4] = { 1, 0.5, 0, 0, };
float VoxelRenderer::lightLight[4] = { 0.5, 0.5, 0.5, 1, };
float VoxelRenderer::lightAmb[4] = { 0.75, 0.75, 0.75, 1, };
*/

float VoxelRenderer::lightPos[4] = { 5, 5, 10, 0 };
float VoxelRenderer::lightSpec[4] = { 1, 0.5, 0, 0 };
float VoxelRenderer::lightDiffuse[4] = { 0.95, 0.95, 0.95, 1 };
float VoxelRenderer::lightAmb[4] = { 0.6, 0.6, 0.6, 1 };

VoxelRenderer voxelrenderer;

VoxelRenderer::~VoxelRenderer() {
	delete frame_renderbuffer;		
#ifdef MESA
	OSMesaDestroyContext(OSMesaGetCurrentContext());
#endif
#ifdef GLUT
	glDeleteFramebuffersEXT(1, &fbo);
	glDeleteRenderbuffersEXT(1, &depthbuffer);
	glDeleteRenderbuffersEXT(1, &rgb_rb);
#endif
}

void VoxelRenderer::init() {
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(0, 0);
	glutCreateWindow("");
	glutHideWindow();	

	frame_renderbuffer = new DrawingSurface(200, 200, 32);
#ifdef MESA
	SetupMesaFBO(frame_renderbuffer);
#endif
#ifdef GLUT
	SetupGleeFBO(frame_renderbuffer);
#endif
	initialized = true;	
}

#ifdef GLUT
void VoxelRenderer::SetupGleeFBO(DrawingSurface* dst) {
	// generate fbo	
	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
	
	// create depthbuffer
	glGenRenderbuffersEXT(1, &depthbuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, dst->Get_Width(), dst->Get_Height());
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthbuffer);
	
	// create a rgba renderbuffer
	glGenRenderbuffersEXT(1, &rgb_rb);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rgb_rb);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA, dst->Get_Width(), dst->Get_Height());
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, rgb_rb);
		
	assert(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT);
}
#endif

#ifdef MESA
void VoxelRenderer::SetupMesaFBO(DrawingSurface* dst) {
	OSMesaContext ctx = OSMesaCreateContext(OSMESA_RGBA, NULL);
	OSMesaMakeCurrent(ctx, dst->Get_Lower_Bound(), GL_UNSIGNED_BYTE, dst->Get_Width(), dst->Get_Height());
}
#endif

void VoxelRenderer::SetupFrameRender(DrawingSurface* dst) {	
	glViewport(0, 0, dst->Get_Width(), dst->Get_Height());

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_BLEND);
	glShadeModel(GL_SMOOTH);		

	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glEnable(GL_LIGHT0);

	glClearColor(0.0, 0.0, 0.0, 0.0); // set some transparent backcolor
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (float)dst->Get_Width() / (float)dst->Get_Height(), 2, dst->Get_Height());
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, -10, 0, 0, 0, 0, 1, 0);
	glTranslatef(0, 0, 10);

	glRotatef(60, 1, 0, 0);
	// flip upside down
	glRotatef(180, 0, 1, 0);

	glRotatef(this->object_rotation, 0, 0, 1);
	glScalef(0.083333, 0.0833336, 0.0833333); // trial and error guess value :D
}

DrawingSurface* VoxelRenderer::render(boost::shared_ptr<VXL_File> vxl, boost::shared_ptr<HVA_File> hva, int object_rotation, const Palet* p) {
	if (!initialized) init();
	this->hva = hva;
	this->vxl = vxl;
	this->object_rotation = object_rotation;
	if (p != NULL)
		vxl->setPalette(p);
	SetupFrameRender(frame_renderbuffer);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	// determine size
	for (uint32_t i = 0; i != vxl->getNumLimbs(); i++) {
		vxl->setCurrentLimb(i);
		if(hva) {
			hva->setCurrentSection(vxl->limbName());
		}
		renderSection();
	}
	
#ifdef GLUT
	glReadPixels(0, 0, frame_renderbuffer->Get_Width(), frame_renderbuffer->Get_Height(), GL_RGBA, GL_UNSIGNED_BYTE, frame_renderbuffer->Get_Lower_Bound());
#endif
	return frame_renderbuffer;
}

void VoxelRenderer::renderSection() {
	glPushMatrix();
	uint8_t xs, ys, zs;
	float min[3], max[3];
	float transform[16];
	float sectionScale[3];

	vxl->getSize(xs, ys, zs);
	vxl->getBounds(min, max);

	/* Calculate the screen units / voxel ratio for scaling */
	max[0] -= min[0];
	max[1] -= min[1];
	max[2] -= min[2];
	sectionScale[0] = max[0] / (float)xs;
	sectionScale[1] = max[1] / (float)ys;
	sectionScale[2] = max[2] / (float)zs;

	/* Load transformation matrix */
	hva->loadGLMatrix(frame, transform);
	/* The HVA transformation matrices have to be scaled */
	transform[12] *= vxl->getScale() * sectionScale[0];
	transform[13] *= vxl->getScale() * sectionScale[1];
	transform[14] *= vxl->getScale() * sectionScale[2];

	/* Apply the transform for this frame */
	glMultMatrixf(transform);

	/* Translate to the bottom left of the section's bounding box */
	glTranslatef(min[0], min[1], min[2]);
	 
	VXL_File::LimbBody::Span::Voxel vx;
	glBegin(GL_QUADS);
	for(unsigned int x = 0; x != xs; x++) {
		for(unsigned int y = 0; y != ys; y++) {
			for(unsigned int z = 0; z != zs; z++) {
				if(vxl->getVoxel(x, y, z, &vx)) {
					const unsigned char* colptr = &(vxl->getPalette()[vx.colour * 3]);
				
					float colour[4];
					colour[0] = (float)(*colptr++ / 255.0f);
					colour[1] = (float)(*colptr++ / 255.0f);
					colour[2] = (float)(*colptr++ / 255.0f);
					colour[3] = 1.0f;
					glColor3fv(colour);
										
					float normal[3];
					vxl->getXYZNormal(vx.normal, normal[0], normal[1], normal[2]);
					glNormal3fv(normal);
					renderVoxel((float)x * sectionScale[0], (float)y * sectionScale[1], (float)z * sectionScale[2], (1 - pitch) / 2);
					glPopMatrix();
				}
			}
		}
	}
	glEnd();
	glPopMatrix();
}

void VoxelRenderer::renderVoxel(float cx, float cy, float cz, float r) {
	float left = cx - r;
	float right = cx + r;
	float base = cy - r;
	float top = cy + r;
	float front = cz - r;
	float back = cz + r;
	/* Base */
	glVertex3f(left, base, front);
	glVertex3f(right, base, front);
	glVertex3f(right, base, back);
	glVertex3f(left, base, back);

	/* Back */
	glVertex3f(left, base, back);
	glVertex3f(right, base, back);
	glVertex3f(right, top, back);
	glVertex3f(left, top, back);

	/* Top */
	glVertex3f(left, top, front);
	glVertex3f(right, top, front);
	glVertex3f(right, top, back);
	glVertex3f(left, top, back);

	/* Right */
	glVertex3f(right, base, front);
	glVertex3f(right, base, back);
	glVertex3f(right, top, back);
	glVertex3f(right, top, front);

	/* Front */
	glVertex3f(left, base, front);
	glVertex3f(right, base, front);
	glVertex3f(right, top, front);
	glVertex3f(left, top, front);

	/* Left */
	glVertex3f(left, base, front);
	glVertex3f(left, base, back);
	glVertex3f(left, top, back);
	glVertex3f(left, top, front);
}
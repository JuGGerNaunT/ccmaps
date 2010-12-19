#ifdef MESA
		#pragma comment(lib, "glut32.lib")
		#pragma comment(lib, "gdi.lib")
		#pragma comment(lib, "glu32.lib")
		#pragma comment(lib, "mesa.lib")
		#pragma comment(lib, "osmesa32.lib")
#endif

#ifdef GLUT
		#pragma comment(lib, "glut32.lib")
		#pragma comment(lib, "opengl32.lib")
		#pragma comment(lib, "glu32.lib")
#endif

#pragma comment(lib, "zlib_mt.lib")
#pragma comment(lib, "libpng_mt.lib")
#pragma comment(lib, "libjpeg_mt.lib")
#ifdef WIN32
	#ifdef _DEBUG
		#ifdef _MT
			#pragma comment(lib, "zlib_mtd.lib")
			#pragma comment(lib, "libpng_mtd.lib")
			#pragma comment(lib, "libjpeg_mtd.lib")
		#else
			//#pragma comment(lib, "zlib_mdd.lib")
			//#pragma comment(lib, "libpng_mdd.lib")
			//#pragma comment(lib, "libjpeg_mdd.lib")
		#endif

	#else
		#ifdef _MT
			#pragma comment(lib, "zlib_mt.lib")
			#pragma comment(lib, "libpng_mt.lib")
			#pragma comment(lib, "libjpeg_mt.lib")
		#else
			//#pragma comment(lib, "zlib_md.lib")
			//#pragma comment(lib, "libpng_md.lib")
			//#pragma comment(lib, "libjpeg_md.lib")
		#endif
	#endif
#endif
#pragma once

#include "File.h"
#include <vector>
#include <boost/shared_ptr.hpp>
#include "DrawingSurface.h"
#include <stdint.h>

class HVA_File {
public:
	struct Header {
		uint8_t fileName[16];
		uint32_t numFrames;
		uint32_t numSections;
	};
	struct Section {
		uint8_t name[16];
		typedef float TMatrix[3][4];
		TMatrix* matrices;

		Section() : matrices(NULL) { }
		~Section() { delete[] matrices; }
		void alloc(uint32_t n) {
			matrices = new TMatrix[n];
		}
	};
protected:
	Header header;
	Section* sections;
	uint32_t currentSection;

public:
	~HVA_File();
	HVA_File(boost::shared_ptr<File> hva);
	void Initialize();

	void loadGLMatrix(uint32_t, float*);
	void setCurrentSection(std::string const&);
	uint32_t numFrames();

	void print();
	bool initialized;
	

private:
	boost::shared_ptr<File> f;
	std::vector<unsigned char> hva_data;

};

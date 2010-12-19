#include "HVA_File.h"
#include <stdint.h>

HVA_File::HVA_File(boost::shared_ptr<File> hva) : f(hva), sections(NULL), currentSection(0) {
	initialized = false;
	this->f = f;
	if (f->size() < 0) 
		throw;
}

HVA_File::~HVA_File() {
	delete[] sections;
}

void HVA_File::Initialize() {
	if (initialized || !f) return;
	initialized = true;
	f->seek_start();
	if (f->read(hva_data, f->size()) != f->size())
		throw;

	int spos = 0;
	memcpy(&header.fileName[0], &hva_data[spos], 16);
	spos += 16;
	memcpy(&header.numFrames, &hva_data[spos], sizeof(header.numFrames));
	spos += sizeof(header.numFrames);
	memcpy(&header.numSections, &hva_data[spos], sizeof(header.numSections));
	spos += sizeof(header.numFrames);

	sections = new Section[header.numSections];

	/* Read in section names */
	for (uint32_t i = 0; i != header.numSections; i++) {	
		memcpy(&sections[i].name[0], &hva_data[spos], 16);
		spos += 16;
		sections[i].alloc(header.numFrames);
	}

	/* Read in matrices.  They are stored packed:
	 * <Frame0/Section0>...<Frame0/SectionN><Frame1/Section0>...<Frame1/SectionN>...<FrameN/SectionN>
	 */
	for (uint32_t i = 0; i != header.numFrames; i++) {
		for(uint32_t j = 0; j != header.numSections; j++) {
			memcpy(&sections[j].matrices[i], &hva_data[spos], sizeof(sections[j].matrices[i]));
			spos += sizeof(sections[j].matrices[i]);
		}
	}
}

void HVA_File::setCurrentSection(std::string const& name) {
	for(uint32_t i = 0; i != header.numSections; i++) {
		if (strcmp(reinterpret_cast<char const*>(&sections[i].name[0]), name.c_str()) == 0) {
			currentSection = i;
			return;
		}
	}
}

uint32_t HVA_File::numFrames() {
	return header.numFrames;
}

void HVA_File::loadGLMatrix(uint32_t frame, float* m) {
	/* OpenGL matrices are laid out thus:
	 * [ 0] [ 4] [ 8] [12]
	 * [ 1] [ 5] [ 9] [13]
	 * [ 2] [ 6] [10] [14]
	 * [ 3] [ 7] [11] [15]
	 *
	 * We need to load the matrix with
	 * [0][0] [0][1] [0][2] [0][3]
	 * [1][0] [1][1] [1][2] [1][3]
	 * [2][0] [2][1] [2][2] [2][3]
	 *    0      0      0      1
	 */
	Section::TMatrix* tm = &sections[currentSection].matrices[frame];
	m[0]  = (*tm)[0][0];
	m[1]  = (*tm)[1][0];
	m[2]  = (*tm)[2][0];
	m[3]  = 0;
	m[4]  = (*tm)[0][1];
	m[5]  = (*tm)[1][1];
	m[6]  = (*tm)[2][1];
	m[7]  = 0;
	m[8]  = (*tm)[0][2];
	m[9]  = (*tm)[1][2];
	m[10] = (*tm)[2][2];
	m[11] = 0;
	m[12] = (*tm)[0][3];
	m[13] = (*tm)[1][3];
	m[14] = (*tm)[2][3];
	m[15] = 1;
}

void HVA_File::print() {
	printf("Contains %u frames for %u sections\n", header.numFrames, header.numSections);
	for(uint32_t i = 0; i != header.numSections; i++) {
		printf("Section %s\n", (char const*)sections[i].name);
		for(uint32_t j = 0; j != header.numFrames; j++) {
			printf("            |% 8.03f  % 8.03f  % 8.03f  % 8.03f|       _  |% 8.03f|\n",
				sections[i].matrices[j][0][0], sections[i].matrices[j][0][1], sections[i].matrices[j][0][2], sections[i].matrices[j][0][3], sections[i].matrices[j][0][3] * 0.083333); 
			printf("  Frame %03u:|% 8.03f  % 8.03f  % 8.03f  % 8.03f| x 0.083  |% 8.03f|\n",
				j, sections[i].matrices[j][1][0], sections[i].matrices[j][1][1], sections[i].matrices[j][1][2], sections[i].matrices[j][1][3], sections[i].matrices[j][1][3] * 0.083333);
			printf("            |% 8.03f  % 8.03f  % 8.03f  % 8.03f|          |% 8.03f|\n",
				sections[i].matrices[j][2][0], sections[i].matrices[j][2][1], sections[i].matrices[j][2][2], sections[i].matrices[j][2][3], sections[i].matrices[j][2][3] * 0.083333);
			printf("             --------------------------------------\n");
		}
	}
}

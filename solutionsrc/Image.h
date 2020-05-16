#include <stdint.h>
#include <cstdio>

enum IMGTYPE {
	PNG,
	JPG,
	BMP,
	TGA
};

struct Image {
	uint8_t* data = NULL;
	size_t size = 0;
	int w = 0;
	int h = 0;
	int channels = 3;
	
	Image(const char* filename);
	Image(int w, int h, int channels);
	Image(const Image& img);
	~Image();


	bool read(const char* filename);
	bool write(const char* filename);

	IMGTYPE getFileType(const char* filename);

	void debug();
};

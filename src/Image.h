#include <stdint.h>
#include <cstdio>

enum ImageType {
	PNG, JPG, BMP, TGA
};

struct Image {
	uint8_t* data = NULL;
	size_t size = 0;
	int w;
	int h;
	int channels;

	Image(const char* filename);
	Image(int w, int h, int channels);
	Image(const Image& img);
	~Image();

	bool read(const char* filename);
	bool write(const char* filename);

	ImageType get_file_type(const char* filename);

	
	Image& convolve_sd(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[]);


	Image& grayscale_avg();
	Image& grayscale_lum();

	Image& color_mask(float r, float g, float b);


};









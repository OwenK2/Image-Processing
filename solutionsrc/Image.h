#include "Kernel.h"
#include <random>

enum ImageType {
	PNG,
	JPG,
	BMP,
	TGA
};

struct Image {
	uint8_t* data = NULL;
	size_t size = 0;
	int w;
	int h;
	int channels;
	
	Image(const char* filename, int channelForce = 0);
	Image(int w, int h, int channels = 3);
	Image(const Image& img);
	~Image();


	bool read(const char* filename, int channelForce = 0);
	bool write(const char* filename);

	ImageType get_file_type(const char* filename);


	Image& diffmap(Image& img);
	Image& diffmap_scale(Image& img, uint8_t scl = 0);

	Image& grayscale_avg();
	Image& grayscale_lum();


	Image& convolve(Kernel& ker);

	Image& medianFilter(Image* imgs, uint8_t numImages);

	Image& inverse();

	Image& colorMask(uint8_t mask);
	Image& colorMask(uint8_t r, uint8_t g, uint8_t b);
	Image& colorMask(float mask);
	Image& colorMask(float r, float g, float b);

	Image& encodeMessage(const char* message);
	const char* decodeMessage();

	

	Image& gaussian_blur();

	Image& tempAdjust(short value);
	Image& grainAdjust(uint8_t value);






	void debug();
};

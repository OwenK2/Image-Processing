#include "Kernel.h"

enum ImageType {
	PNG,
	JPG,
	BMP,
	TGA
};

struct Image {
	uint8_t* data = NULL;
	
	int w; //width
	int h; //height
	int channels; //channels
	size_t size = 0; //w*h*channels

	int pw; //padded width, must be 2^m for some m
	int ph; //padded height, must be 2^n for some n
	size_t psize = 0; //pw*ph*channels
	
	Image(const char* filename, int channelForce = 0);
	Image(int w, int h, int channels);
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

	

	Image& gaussian_blur();







	void debug();
};

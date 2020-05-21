#include "Kernel.h"

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
	
	Image(const char* filename);
	Image(int w, int h, int channels);
	Image(const Image& img);
	~Image();


	bool read(const char* filename);
	bool write(const char* filename);

	ImageType get_file_type(const char* filename);


	Image& diffmap(Image& img);
	Image& diffmap_scale(Image& img, uint8_t scl = 0);

	Image& grayscale_avg();
	Image& grayscale_lum();


	Image& convolve(Kernel& ker);

	

	Image& gaussian_blur();







	void debug();
};

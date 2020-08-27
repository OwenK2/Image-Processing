#include <random>
#include <stdint.h>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <complex>
#include <cmath>
#define _USE_MATH_DEFINES //legacy feature of C

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

	uint32_t ph; //padded pixel height (no channels), can change depending on size of kernel being convolved on image
  	uint32_t pw; //padded pixel width (no channels), can change depending on size of kernel being convolved on image
  	uint64_t psize; //padded pixel size (no channels), can change depending on size of kernel being convolved on image
	
	Image(const char* filename, int channelForce = 0);
	Image(int w, int h, int channels = 3);
	Image(const Image& img);
	~Image();


	bool read(const char* filename, int channelForce = 0);
	bool write(const char* filename);

	ImageType get_file_type(const char* filename);


	static std::complex<double>* recursive_fft(uint32_t n, std::complex<double> x[], std::complex<double>* X, bool inverse);
	static std::complex<double>* dft(uint32_t m, uint32_t n, std::complex<double> y[], std::complex<double>* Y);
	static std::complex<double>* idft(uint32_t m, uint32_t n, std::complex<double> Y[], std::complex<double>* y);

	static std::complex<double>* pointwise_mult(uint64_t len, std::complex<double> a[], std::complex<double> b[], std::complex<double>* p);


	//TODO: add convolve function that chooses from the following based on image and kernel size

	Image& std_convolve_clamp_to_0(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[]);
	Image& fd_convolve_clamp_to_0(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[]);

	Image& std_convolve_clamp_to_border(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[]);
	Image& fd_convolve_clamp_to_border(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[]);

	Image& std_convolve_cyclic(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[]);
	Image& fd_convolve_cyclic(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[]);

	
	Image& gaussian_blur();



	Image& diffmap(Image& img);
	Image& diffmap_scale(Image& img, uint8_t scl = 0);

	Image& grayscale_avg();
	Image& grayscale_lum();

	Image& medianFilter(Image* imgs, uint8_t numImages);

	Image& inverse();

	Image& colorMask(uint8_t mask);
	Image& colorMask(uint8_t r, uint8_t g, uint8_t b);
	Image& colorMask(float mask);
	Image& colorMask(float r, float g, float b);

	Image& encodeMessage(const char* message);
	const char* decodeMessage();

	Image& tempAdjust(short value);
	Image& grainAdjust(uint8_t value);






	void debug();
};

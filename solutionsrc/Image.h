#include <random>
#include <stdint.h>
#include <cstring>
#include <cstdio>
#include <complex>
#include <cmath>

#include "schrift.h"

#define _USE_MATH_DEFINES //legacy feature of C

#define STEG_HEADER_SIZE sizeof(uint32_t) * 8

enum ImageType {
	PNG,
	JPG,
	BMP,
	TGA
};

struct Font;

struct Image {
	uint8_t* data = NULL;
	
	int w; //width
	int h; //height
	int channels; //channels
	size_t size = 0; //w*h*channels
	
	Image(const char* filename, int channelForce = 0);
	Image(int w, int h, int channels = 3);
	Image(const Image& img);
	~Image();


	bool read(const char* filename, int channel_force = 0);
	bool write(const char* filename);

	ImageType get_file_type(const char* filename);

	Image& std_convolve_clamp_to_0(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc);
	Image& std_convolve_clamp_to_border(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc);
	Image& std_convolve_cyclic(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc);

	static uint32_t rev(uint32_t n, uint32_t a);
	static void bit_rev(uint32_t n, std::complex<double> a[], std::complex<double>* A);
	
	static void fft(uint32_t n, std::complex<double> x[], std::complex<double>* X);
	static void ifft(uint32_t n, std::complex<double> X[], std::complex<double>* x);
	static void dft_2D(uint32_t m, uint32_t n, std::complex<double> x[], std::complex<double>* X);
	static void idft_2D(uint32_t m, uint32_t n, std::complex<double> X[], std::complex<double>* x);

	static inline void pointwise_product(uint64_t l, std::complex<double> a[], std::complex<double> b[], std::complex<double>* p);

	static void pad_kernel(uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc, uint32_t pw, uint32_t ph, std::complex<double>* pad_ker);

	Image& fd_convolve_clamp_to_0(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc);
	Image& fd_convolve_clamp_to_border(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc);
	Image& fd_convolve_cyclic(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc);
	
	Image& convolve_linear(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc);
	Image& convolve_clamp_to_border(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc);
	Image& convolve_cyclic(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc);


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
	Image& decodeMessage(char* buffer, size_t* messageLength);

	Image& tempAdjust(short value);
	Image& grainAdjust(uint8_t value);



	Image& flipX();
	Image& flipY();


	Image& overlay(const Image& img, int x, int y);
	Image& overlayText(const char* text, const Font& font, int x, int y, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255);

	Image& resize(uint16_t nw, uint16_t nh);
	Image& resizeNN(uint16_t nw, uint16_t nh); //nearest neighbor

	Image& crop(uint16_t x, uint16_t y, uint16_t w, uint16_t h);



	void debug();
};

// For details about the format:
// https://developer.apple.com/fonts/TrueType-Reference-Manual/
struct Font {
	SFT sft = {NULL, 12, 12, 0, 0, SFT_DOWNWARD_Y|SFT_RENDER_IMAGE};
	Font(const char* filepath, uint16_t size) {
	  if((sft.font = sft_loadfile(filepath)) == NULL) {
	    printf("\e[31m[ERROR] Failed to load %s\e[0m\n", filepath);
	  }
	  setSize(size);
	};
	~Font() {
		sft_freefont(sft.font);
	}
	void setSize(uint16_t size) {
		sft.xScale = size;
		sft.yScale = size;
	}
};


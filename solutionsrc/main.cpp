#include "Image.h"


int main(int argc, char** argv) {
	Image test("test1.jpg");

	Image gray_avg = test;
	gray_avg.grayscale_avg();
	gray_avg.write("gray_avg.png");

	Image gray_lum = test;
	gray_lum.grayscale_lum();
	gray_lum.write("gray_lum.png");
}

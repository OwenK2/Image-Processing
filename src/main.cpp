#include "Image.h"


int main(int argc, char** argv) {
	Image test("imgs/test1.jpg");

	double ker[] = {0.0625, 0.125, 0.0625, 0.125, 0.25, 0.125, 0.0625, 0.125, 0.0625};
	test.convolve_sd(0, 3, 3, ker);
	test.convolve_sd(1, 3, 3, ker);
	test.convolve_sd(2, 3, 3, ker);
	test.write("result.png");

	return 0;
}

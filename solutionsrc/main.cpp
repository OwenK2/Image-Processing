#include "Image.h"

int main(int argc, char** argv) {
	Image test("imgs/test1.jpg");

	// double ker[] = {-2/9.0, -1/9.0, 0, -1/9.0, 1/9.0, 1/9.0, 0, 1/9.0, 2/9.0}; //emboss
	double ker[] = {1/16.0, 2/16.0, 1/16.0, 2/16.0, 4/16.0, 2/16.0, 1/16.0, 2/16.0, 1/16.0}; //gaussian blur
	
	Image cSTD = test;
	Image cFD = test;

	printf("starting convolutions...\n");
	cSTD.std_convolve_clamp_to_0(0, 3, 3, ker, 1, 1);
	cSTD.std_convolve_clamp_to_0(1, 3, 3, ker, 1, 1);
	cSTD.std_convolve_clamp_to_0(2, 3, 3, ker, 1, 1);
	printf("halfway!\n");
	cFD.fd_convolve_clamp_to_0(0, 3, 3, ker, 1, 1);
	cFD.fd_convolve_clamp_to_0(1, 3, 3, ker, 1, 1);
	cFD.fd_convolve_clamp_to_0(2, 3, 3, ker, 1, 1);
	printf("finished convolutions!\n");
	
	cSTD.write("imgs/std_conv.png");
	cFD.write("imgs/fd_conv.png");


	Image diff = cSTD;
	diff.diffmap_scale(cFD);
	for(uint64_t k=0; k<diff.size; ++k) {
		if(cSTD.data[k] - cFD.data[k] > 1) { //if difference is less than or equal to 1, we assume rounding error
			printf("Not matching at index %llu: %d =/= %d\n", k, cSTD.data[k], cFD.data[k]);
		}
	}
	diff.write("imgs/con_check.png");

	// Image test("test.jpg");

	// test.resize(1280, 662);

	// Image logo("logo.png");
	// logo.resizeNN(128, 128);

	
	// Font roboto("Roboto-Regular.ttf", 50);
	// Font fira("FiraCode-Regular.ttf", 50);
	// Font times("/System/Library/Fonts/Supplemental/Times New Roman.ttf", 50);

	// test.overlayText("Ooo Colors", times, 40, 60, 255, 0, 0);
	// times.setSize(100);
	// test.overlayText("Alpha Too", times, 100, 100, 0, 128, 255, 175);
	// test.overlay(logo, 100, 100);
	// printf("%d %d\n", test.w, test.h);
	

	// test.write("output.png");

	return 0;
}

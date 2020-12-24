#include "Image.h"


int main(int argc, char** argv) {
	Image test("imgs/test1.jpg");

	double ker[] = { -2/9.0, -1/9.0, 0,
					-1/9.0,  1/9.0, 1/9.0,
					0     ,  1/9.0, 2/9.0 }; //emboss
	// double ker[] = { 1/16.0, 2/16.0, 1/16.0,
	// 				 				 2/16.0, 4/16.0, 2/16.0,
	// 				 				 1/16.0, 2/16.0, 1/16.0 }; //gaussian blur


	








	// Image cSTD = test;
	// Image cFD = test;


	// cSTD.std_convolve_clamp_to_border(0, 3, 3, ker, 1, 1);
	// cSTD.std_convolve_clamp_to_border(1, 3, 3, ker, 1, 1);
	// cSTD.std_convolve_clamp_to_border(2, 3, 3, ker, 1, 1);

	// cFD.fd_convolve_clamp_to_border(0, 3, 3, ker, 1, 1);
	// cFD.fd_convolve_clamp_to_border(1, 3, 3, ker, 1, 1);
	// cFD.fd_convolve_clamp_to_border(2, 3, 3, ker, 1, 1);

	// cSTD.write("imgs/std_conv.png");
	// cFD.write("imgs/fd_conv.png");

	// Image diff = cSTD;
	// diff.diffmap(cFD);
	// for(uint64_t k=0; k<diff.size; ++k) {
	// 	if(cSTD.data[k]-cFD.data[k] > 1) { // >1 because some rounding errors occur
	// 		printf("Not matching at index %llu: %d =/= %d\n", k, cSTD.data[k], cFD.data[k]);
	// 	}
	// }
	// diff.write("imgs/con_check.png");

	return 0;
}

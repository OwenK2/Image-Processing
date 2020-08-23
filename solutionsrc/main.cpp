#include "Image.h"

int main(int argc, char** argv) {
	Image test("imgs/test1.jpg");

	double ker[] = {0.0625, 0.125, 0.0625, 0.125, 0.25, 0.125, 0.0625, 0.125, 0.0625};
	
	Image cSTD = test;
	Image cFD = test;

	cSTD.convolve_sd(0, 3, 3, ker);
	cSTD.convolve_sd(1, 3, 3, ker);
	cSTD.convolve_sd(2, 3, 3, ker);

	cFD.convolve_fd(0, 3, 3, ker);
	cFD.convolve_fd(1, 3, 3, ker);
	cFD.convolve_fd(2, 3, 3, ker);

	cSTD.write("imgs/std_conv.png");
	cFD.write("imgs/fd_conv.png");

	Image diff = cSTD;
	diff.diffmap(cFD);
	for(uint64_t k=0; k<diff.size; ++k) {
		if(diff.data[k]!=0) {
			printf("Not matching at index %llu: %d =/= %d\n", k, cSTD.data[k], cFD.data[k]);
		}
	}
	diff.write("imgs/ConCheck.png");

	return 0;
}

#include "Image.h"

#include <cstdlib>
#include <chrono>


int main(int argc, char** argv) {

	Image test("imgs/stone.png", 4);
	for(uint32_t i=1; i<test.h-1; ++i) {
		for(uint32_t j=1; j<test.w-1; ++j) {
			test.data[(i*test.w+j)*test.channels] = 0;
			test.data[(i*test.w+j)*test.channels+1] = 0;
			test.data[(i*test.w+j)*test.channels+2] = 0;
			test.data[(i*test.w+j)*test.channels+3] = 255;
		}
	}

	test.write("stone.png");

	// double ker[] = { -2/9.0, -1/9.0, 0,
	// 				 -1/9.0,  1/9.0, 1/9.0,
	// 				  0    ,  1/9.0, 2/9.0 }; //emboss
	// double ker[] = { 1/16.0, 2/16.0, 1/16.0,
	// 				 2/16.0, 4/16.0, 2/16.0,
	// 				 1/16.0, 2/16.0, 1/16.0 }; //gaussian blur
	// double ker[225]; //15x15 box blur
	// for(uint16_t i=0; i<225; ++i) {ker[i]=1.0/225;}
	
	
	// test.convolve_linear(0, 15, 15, ker, 7, 7);
	// test.convolve_linear(1, 15, 15, ker, 7, 7);
	// test.convolve_linear(2, 15, 15, ker, 7, 7);

	// test.write("imgs/c.png");




	// Image cFD = test;
	// Image cSTD = test;

	// auto fd_conv_start = std::chrono::system_clock::now();
	// cFD.fd_convolve_cyclic(0, 15, 15, ker, 7, 7);
	// cFD.fd_convolve_cyclic(1, 15, 15, ker, 7, 7);
	// cFD.fd_convolve_cyclic(2, 15, 15, ker, 7, 7);
	// auto fd_conv_end = std::chrono::system_clock::now();

	// auto std_conv_start = std::chrono::system_clock::now();
	// cSTD.std_convolve_cyclic(0, 15, 15, ker, 7, 7);
	// cSTD.std_convolve_cyclic(1, 15, 15, ker, 7, 7);
	// cSTD.std_convolve_cyclic(2, 15, 15, ker, 7, 7);
	// auto std_conv_end = std::chrono::system_clock::now();

	// printf("std method took %lldns\n", std::chrono::duration_cast<std::chrono::nanoseconds>(std_conv_end-std_conv_start).count());
	// printf(" fd method took %lldns\n", std::chrono::duration_cast<std::chrono::nanoseconds>( fd_conv_end- fd_conv_start).count());


	// cFD.write("imgs/fd_conv.png");
	// cSTD.write("imgs/std_conv.png");


	

	// Image diff = cSTD;
	// diff.diffmap_scale(cFD);
	// for(uint64_t k=0; k<diff.size; ++k) {
	// 	if(cSTD.data[k] - cFD.data[k] > 1) { //if difference is less than or equal to 1, we assume rounding error
	// 		printf("Non negligible difference at index %llu: %d =/= %d\n", k, cSTD.data[k], cFD.data[k]);
	// 	}
	// }
	// diff.write("imgs/con_check.png");

	
	return 0;
}

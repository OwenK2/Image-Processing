#include "Image.h"


int main(int argc, char** argv) {
	Image test("imgs/test1.jpg");

	double ker[] = { -2/9.0, -1/9.0, 0,
									 -1/9.0,  1/9.0, 1/9.0,
									 0     ,  1/9.0, 2/9.0 }; //emboss
	// double ker[] = { 1/16.0, 2/16.0, 1/16.0,
	// 				 				 2/16.0, 4/16.0, 2/16.0,
	// 				 				 1/16.0, 2/16.0, 1/16.0 }; //gaussian blur



	return 0;
}

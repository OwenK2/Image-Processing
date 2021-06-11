#include "Image.h"


// void make_random_complex_arr(uint32_t len, std::complex<double>* z) {
// 	for(uint32_t i=0; i<len; ++i) {
// 		z[i] = std::complex<double>(rand()%100, rand()%100);
// 	}
// }
// void print_complex_arr(uint32_t len, std::complex<double> z[]) {
// 	for(uint32_t i=0; i<len; ++i) {
// 		printf("%f+%fj,\n", z[i].real(), z[i].imag());
// 	}
// 	printf("\n");
// }

int main(int argc, char** argv) {





	Image character("char.png");
	Image sky("aurora.jpg");

	sky.resizeNN(1000, 1000);
	character.resizeNN(1000, 1000);


	sky.write("outputsky.png");
	character.write("outputchar.png");





	return 0;
}

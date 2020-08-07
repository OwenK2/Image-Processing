#include "Image.h"


int main(int argc, char** argv) {
	





	Image test("imgs/test3.jpg");

	std::complex<double>* FDR = new std::complex<double>[test.pw*test.ph];
	test.dft(0, test.ph, test.pw, FDR);
	std::complex<double>* FDG = new std::complex<double>[test.pw*test.ph];
	test.dft(1, test.ph, test.pw, FDG);
	std::complex<double>* FDB = new std::complex<double>[test.pw*test.ph];
	test.dft(2, test.ph, test.pw, FDB);
	
	Image recoveredTest(test.w, test.h, 3);
	recoveredTest.idft(0, test.ph, test.pw, FDR);
	recoveredTest.idft(1, test.ph, test.pw, FDG);
	recoveredTest.idft(2, test.ph, test.pw, FDB);

	recoveredTest.write("imgs/recovered.png");

	Image diff = test;
	diff.diffmap_scale(recoveredTest);
	for(uint64_t k=0; k<diff.size; ++k) {
		if(diff.data[k]!=0) {
			printf("Original image and one recovered from fourier domain do not match.\n");
		}
	}
	//diff.write("FTCheck.png");

	delete[] FDR;
	delete[] FDG;
	delete[] FDB;

	return 0;
}

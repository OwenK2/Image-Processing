#include "Image.h"

int main(int argc, char** argv) {
	Image test("imgs/test1.jpg");

	double ker[] = {0.0625, 0.125, 0.0625, 0.125, 0.25, 0.125, 0.0625, 0.125, 0.0625};
	
	Image cSTD = test;
	Image cFD = test;

	cSTD.std_convolve_clamp_to_0(0, 3, 3, ker);
	cSTD.std_convolve_clamp_to_0(1, 3, 3, ker);
	cSTD.std_convolve_clamp_to_0(2, 3, 3, ker);

	cFD.fd_convolve_clamp_to_0(0, 3, 3, ker);
	cFD.fd_convolve_clamp_to_0(1, 3, 3, ker);
	cFD.fd_convolve_clamp_to_0(2, 3, 3, ker);

	cSTD.write("imgs/std_conv.png");
	cFD.write("imgs/fd_conv.png");





	//testing
	// std::complex<double>* img_std = new std::complex<double>[cSTD.psize];
	// for(uint32_t i=0; i<cSTD.h; ++i) {
 //    for(uint32_t j=0; j<cSTD.w; ++j) {
 //      img_std[i*cSTD.pw+j] = std::complex<double>(cSTD.data[(i*cSTD.w+j)*cSTD.channels+0],0);
 //    }
 //  }
	// std::complex<double>* res_fd_std = new std::complex<double>[cSTD.psize];
	// Image::dft(cSTD.h, cSTD.w, img_std, res_fd_std);

	// std::complex<double>* img_fd = new std::complex<double>[cFD.psize];
	// for(uint32_t i=0; i<cFD.h; ++i) {
 //    for(uint32_t j=0; j<cFD.w; ++j) {
 //      img_fd[i*cFD.pw+j] = std::complex<double>(cFD.data[(i*cFD.w+j)*cFD.channels+0],0);
 //    }
 //  }
	// std::complex<double>* res_fd_fd = new std::complex<double>[cFD.psize];
	// Image::dft(cFD.h, cFD.w, img_fd, res_fd_fd);

	// for(uint64_t k=0; k<test.psize; ++k) {
	// 	if(res_fd_fd[k] != res_fd_std[k]) {
	// 		printf("%llu: %f+%fi =/= %f+%fi\n", k, res_fd_fd[k].real(), res_fd_fd[k].imag(), res_fd_std[k].real(), res_fd_std[k].imag());
	// 	}
	// }


	Image diff = cSTD;
	diff.diffmap_scale(cFD);
	for(uint64_t k=0; k<diff.size; ++k) {
		if(diff.data[k]!=0) {
			printf("Not matching at index %llu: %d =/= %d\n", k, cSTD.data[k], cFD.data[k]);
		}
	}
	diff.write("imgs/ConCheck.png");

	return 0;
}

#include "Image.h"

#include <cstdlib>
#include <cmath>
#include <chrono>

// void make_random_cmplx_arr(uint32_t len, std::complex<double>* z) {
// 	for(uint32_t i=0; i<len; ++i) {
// 		z[i] = std::complex<double>(rand()%100, rand()%100);
// 	}
// }
// void print_cmplx_arr(uint32_t len, std::complex<double> z[]) {
// 	for(uint32_t i=0; i<len; ++i) {
// 		printf("%f+%fj,\n", z[i].real(), z[i].imag());
// 	}
// 	printf("\n");
// }

int main(int argc, char** argv) {
	
	Image img((argc < 2) ? "imgs/test6.png" : argv[1]);

	//grayscale
	img.grayscale_avg();
	Image res(img.w, img.h, 1);
	for(uint64_t k=0; k<img.w*img.h; ++k) {
		res.data[k] = img.data[img.channels*k];
	}
	
	//gaussian blur
	double gauss[9] = {1/16., 2/16., 1/16., 2/16., 4/16., 2/16., 1/16., 2/16., 1/16.};
	res.convolve_linear(0, 3, 3, gauss, 1, 1);
	
	
	//sobel (TODO: put this into image struct)
	double* tempx = new double[res.size];
	double* tempy = new double[res.size];
	double* gx = new double[res.size];
	double* gy = new double[res.size];
	//0 out to make sure endpoints are 0
	memset(tempx, 0, sizeof(double)*res.size);
	memset(tempy, 0, sizeof(double)*res.size);
	memset(gx, 0, sizeof(double)*res.size);
	memset(gy, 0, sizeof(double)*res.size);
	
	for(uint32_t c=1; c<res.w-1; ++c) {//changed these to not include endpoints
		for(uint32_t r=0; r<res.h; ++r) {
			tempx[r*res.w+c] = 0;
			tempy[r*res.w+c] = 2*res.data[r*res.w+c];
			//if(c!=0) {
				tempx[r*res.w+c] -= res.data[r*res.w+c-1];
				tempy[r*res.w+c] += res.data[r*res.w+c-1];
			//}
			//if(c!=res.w-1) {
				tempx[r*res.w+c] += res.data[r*res.w+c+1];
				tempy[r*res.w+c] += res.data[r*res.w+c+1];
			//}
		}
	}

	for(uint32_t r=1; r<res.h-1; ++r) {//changed these to not include endpoints
		for(uint32_t c=1; c<res.w-1; ++c) {//dont include endpoints here because everything is 0 anyway
			gx[r*res.w+c] = 2*tempx[r*res.w+c];
			gy[r*res.w+c] = 0;
			//if(r!=0) {
				gx[r*res.w+c] += tempx[(r-1)*res.w+c];
				gy[r*res.w+c] -= tempy[(r-1)*res.w+c];
			//}
			//if(r!=res.h-1) {
				gx[r*res.w+c] += tempx[(r+1)*res.w+c];
				gy[r*res.w+c] += tempy[(r+1)*res.w+c];
			//}
		}
	}
	delete[] tempx;
	delete[] tempy;

	//make intermediate gx, gy examples
	double mxx = -INFINITY,
		   mnx = INFINITY,
		   mxy = -INFINITY,
		   mny = INFINITY;
	for(uint64_t k=0; k<res.size; ++k) {
		mxx = fmax(mxx, gx[k]);
		mnx = fmin(mnx, gx[k]);
		mxy = fmax(mxy, gy[k]);
		mny = fmin(mny, gy[k]);
	}
	Image Gx(res.w, res.h, 1);
	Image Gy(res.w, res.h, 1);
	for(uint64_t k=0; k<res.size; ++k) {
		Gx.data[k] = (uint8_t)(255*(gx[k]-mnx)/(mxx-mnx));
		Gy.data[k] = (uint8_t)(255*(gy[k]-mny)/(mxy-mny));
	}
	Gx.write("imgs/Gx.png");
	Gy.write("imgs/Gy.png");

	double threshold = 0.1;
	double* g = new double[res.size];
	double* theta = new double[res.size];
	for(uint64_t k=0; k<res.size; ++k) {
		double x = gx[k];
		double y = gy[k];
		g[k] = sqrt(x*x+y*y);
		theta[k] = atan2(y, x);
	}
	//make intermediate example image with dirs&mags
	double mx = -INFINITY,
		   mn = INFINITY;
	for(uint64_t k=0; k<res.size; ++k) {
		mx = fmax(mx, g[k]);
		mn = fmin(mn, g[k]);
	}
	Image Gt(res.w, res.h, 3);
	Image G(res.w, res.h, 1);
	for(uint64_t k=0; k<res.size; ++k) {
		double h = theta[k]*180./M_PI + 180.;
		double s,l;
		double v;
		if(mx == mn) {
			v = 0;
		}
		else {
			v = ((g[k]-mn)/(mx-mn) > threshold) ? (g[k]-mn)/(mx-mn) : 0;
		}
		s=l=v;

		double c = (1-abs(2*l-1))*s;
		double x = c*(1-abs(fmod((h/60),2)-1));
		double m = l-c/2;

		double rt, gt, bt;
		rt=gt=bt=0;
		if(h < 60) {
			rt = c;
			gt = x;
		}
		else if(h < 120) {
			rt = x;
			gt = c;
		}
		else if(h < 180) {
			gt = c;
			bt = x;
		}
		else if(h < 240) {
			gt = x;
			bt = c;
		}
		else if(h < 300) {
			rt = x;
			bt = c;
		}
		else {
			rt = c;
			bt = x;
		}

		uint8_t red, green, blue;
		red = (uint8_t)((rt+m)*255);
		green = (uint8_t)((gt+m)*255);
		blue = (uint8_t)((bt+m)*255);

		Gt.data[k*3] = red;
		Gt.data[k*3+1] = green;
		Gt.data[k*3+2] = blue;

		G.data[k] = (uint8_t)(255*v);
	}
	Gt.write("imgs/Gt.png");
	G.write("imgs/G.png");

	res.write("imgs/result.png");

	delete[] gx;
	delete[] gy;
	delete[] g;
	delete[] theta;







	// Image i("imgs/horiz.png");
	// Image o(i.w, i.h);
	// std::complex<double>* o_data = new std::complex<double>[i.w*i.h];
	// std::complex<double>* i_data = new std::complex<double>[o.w*o.h];

	// for(uint64_t k=0; k<i.w*i.h; ++k) {
	// 	i_data[k] = std::complex<double>((double)i.data[k*i.channels], 0.0);
	// }
	
	// Image::dft_2D(128,128, i_data, o_data);
	
	// for(uint64_t k=0; k<o.w*o.h; ++k) {
	// 	o.data[o.channels*k] = (uint8_t)round(o_data[k].real());
	// 	o.data[o.channels*k+1] = (uint8_t)round(o_data[k].real());
	// 	o.data[o.channels*k+2] = (uint8_t)round(o_data[k].real());
	// }
	// o.write("imgs/fd.png");

	// Image::idft_2D(128,128, o_data, o_data);

	// for(uint64_t k=0; k<o.w*o.h; ++k) {
	// 	o.data[o.channels*k] = (uint8_t)round(o_data[k].real());
	// 	o.data[o.channels*k+1] = (uint8_t)round(o_data[k].real());
	// 	o.data[o.channels*k+2] = (uint8_t)round(o_data[k].real());
	// }
	// o.write("imgs/td.png");



	// delete[] i_data;
	// delete[] o_data;





	// const uint32_t len = 4;
	// std::complex<double>* a = new std::complex<double>[len];
	// make_random_cmplx_arr(len, a);
	// print_cmplx_arr(len, a);
	
	// std::complex<double>* A = new std::complex<double>[len];
	// std::complex<double>* a_recovered = new std::complex<double>[len];

	// auto fft1start = std::chrono::system_clock::now();
	// Image::fft(len, a, A);
	// auto fft1end = std::chrono::system_clock::now();
	// // std::complex<double>* A_f = new std::complex<double>[len];
	// // Image::bit_rev(len, A, A_f);
	// print_cmplx_arr(len, A);
	// // print_cmplx_arr(len, A_f);

	// auto ifft1start = std::chrono::system_clock::now();
	// Image::ifft(len, A, a_recovered);
	// auto ifft1end = std::chrono::system_clock::now();
	// print_cmplx_arr(len, a_recovered);
	

	
	// printf("fft took %lldns\n", std::chrono::duration_cast<std::chrono::nanoseconds>(fft1end-fft1start).count());
	// printf("ifft took %lldns\n", std::chrono::duration_cast<std::chrono::nanoseconds>(ifft1end-ifft1start).count());


	// delete[] a;
	// delete[] A;
	// delete[] a_recovered;






	/*

	//smaller size image: ker-break even = 16
	
	//large size image: ker-break even = 14

	Image test("imgs/test1.jpg");

	// double ker[] = {-2/9.0, -1/9.0, 0, -1/9.0, 1/9.0, 1/9.0, 0, 1/9.0, 2/9.0}; //emboss
	double ker[] = {1/16.0, 2/16.0, 1/16.0, 2/16.0, 4/16.0, 2/16.0, 1/16.0, 2/16.0, 1/16.0}; //gaussian blur
	// double ker[256]; //box blur
	// for(uint16_t i=0; i<256; ++i) {ker[i]=1.0/256;}

	Image cSTD = test;
	Image cFD = test;

	printf("starting convolutions...\n");
	auto fd_conv_start = std::chrono::system_clock::now();
	cFD.fd_convolve_clamp_to_0(0, 3, 3, ker, 1, 1);
	cFD.fd_convolve_clamp_to_0(1, 3, 3, ker, 1, 1);
	cFD.fd_convolve_clamp_to_0(2, 3, 3, ker, 1, 1);
	auto fd_conv_end = std::chrono::system_clock::now();
	printf("halfway!\n");
	auto std_conv_start = std::chrono::system_clock::now();
	cSTD.std_convolve_clamp_to_0(0, 3, 3, ker, 1, 1);
	cSTD.std_convolve_clamp_to_0(1, 3, 3, ker, 1, 1);
	cSTD.std_convolve_clamp_to_0(2, 3, 3, ker, 1, 1);
	auto std_conv_end = std::chrono::system_clock::now();
	printf("finished convolutions!\n");
	
	printf("std took %lldns\n", std::chrono::duration_cast<std::chrono::nanoseconds>(std_conv_end-std_conv_start).count());
	printf("fd took %lldns\n", std::chrono::duration_cast<std::chrono::nanoseconds>(fd_conv_end-fd_conv_start).count());

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
	//*/









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
	
	// test.write("precrop.png");
	// test.crop(0, 0, 1400, 100);

	// test.write("output.png");

	






	return 0;
}

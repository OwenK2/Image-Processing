#include "Image.h"


int main(int argc, char** argv) {
	Image test1("test1.jpg");
	Image test2("test2.jpg");
	Image test3("test3.jpg");

	Image diff = test1;
	diff.diffmap(test2);
	diff.write("diff.png");

	Image avg = test1;
	avg.grayscale_avg();
	avg.write("gray_avg.png");

	Image lum = test1;
	lum.grayscale_lum();
	lum.write("gray_lum.png");

	Image diff2 = avg;
	diff2.diffmap(lum);
	diff2.write("diff2.png");

	Image diff3 = avg;
	diff3.diffmap_scale(lum);
	diff3.write("diff3.png");

	double m[] = {1, 2, 1, 2, 4, 2, 1, 2, 1};
	Kernel k(3, m);
	lum.convolve(k).write("blur.png");
}

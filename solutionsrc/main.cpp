#include "Image.h"


int main(int argc, char** argv) {
	Image f1("f1.png", 3);
	Image f2("f2.png", 3);
	Image f3("f3.png", 3);
	Image f4("f4.png", 3);
	Image f5("f5.png", 3);
	Image test("test.jpg", 3);

	Image imgs[6] = {f1, f2, f3, f4, f5, test};
	test.medianFilter(imgs, 6);
	test.write("blended.png");
}

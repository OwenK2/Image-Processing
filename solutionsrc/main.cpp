#include "Image.h"


int main(int argc, char** argv) {
	Image test("test.jpg");
	test.grayscale().write("gray.png");
	return 0;
}

#include "Image.h"


int main(int argc, char** argv) {
	Image test1("imgs/test1.jpg");
	Image test2("imgs/test2.jpg");

	Image diff = test1;
	diff.diffmap(test2);
	diff.write("imgs/diff.png");



	return 0;
}

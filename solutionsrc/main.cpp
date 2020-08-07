#include "Image.h"

int main(int argc, char** argv) {
	Image img("test.jpg");


	img.grainAdjust(255);


	img.write("grain.png");
}

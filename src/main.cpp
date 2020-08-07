#include "Image.h"


int main(int argc, char** argv) {
	Image test("test.jpg");

	test.encodeMessage("Hello, World!");
	test.write("SecretMessage.png");
	test.decodeMessage();

	return 0;
}

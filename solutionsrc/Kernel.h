#include <stdint.h>
#include <cstring>
#include <cstdio>
#include <cmath>

struct Kernel {
	uint8_t side = 0;
	double* matrix = NULL;

	uint16_t center;
	uint8_t cx;
	uint8_t cy;

	Kernel(uint8_t s, uint16_t c);
	Kernel(uint8_t s);
	Kernel(uint8_t s, uint16_t c, double* m);
	Kernel(uint8_t s, double* m);

	~Kernel();

	double& operator()(int i, int j);

	void print();
};
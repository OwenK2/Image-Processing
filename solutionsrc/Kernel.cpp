#include "Kernel.h"

Kernel::Kernel(uint8_t s, uint16_t c) : side(s) {
	matrix = new double[s*s];
	center = c > s*s-1 ? s*s-1 : c;
	cx = c%s;
	cy = c/s;
}
Kernel::Kernel(uint8_t s) : Kernel(s, s*s/2) {}
Kernel::Kernel(uint8_t s, uint16_t c, double* m) : Kernel(s, c) {
	memcpy(matrix, m, s*s*sizeof(double));

	double sum = 0;
	for(int i=0; i<s*s; ++i) {
		sum+=matrix[i];
	}
	for(int i=0; i<s*s; ++i) {
		matrix[i] /= sum;
	}
}
Kernel::Kernel(uint8_t s, double* m) : Kernel(s, s*s/2, m) {}


Kernel::~Kernel() {
	delete[] matrix;
}



double& Kernel::operator()(int i, int j) {
	return matrix[center+i*side+j];
}


void Kernel::print() {
	printf("\n");
	for(int i=0; i<side; ++i) {
		for(int j=0; j<side; ++j) {
			printf(" %f ", matrix[i*side+j]);
		}
		printf("\n");
	}
}
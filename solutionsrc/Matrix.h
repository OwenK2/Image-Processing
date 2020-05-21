#include <stdint.h>
#include <cstring>
#include <cstdio>
#include <cmath>

struct Matrix {
	uint8_t w;
	uint8_t h;
	uint16_t size;
	double* entries;

	Matrix(uint8_t w, uint8_t h);
	Matrix(uint8_t w, uint8_t h, double* entries);
	Matrix(const Matrix& m);

	~Matrix();


	double& operator()(uint8_t i, uint8_t j);
};
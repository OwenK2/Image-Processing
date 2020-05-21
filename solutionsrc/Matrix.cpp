#include "Matrix.h"

Matrix::Matrix(uint8_t w, uint8_t h) : w(w), h(h) {
	size = w*h;
	entries = new double[size];
}
Matrix::Matrix(uint8_t w, uint8_t h, double* e) : Matrix(w, h) {
	memcpy(this->entries, e, size);
}
Matrix::Matrix(const Matrix& m) : Matrix(m.w, m.h, m.entries) {}

Matrix::~Matrix() {
	delete[] entries;
}

double& Matrix::operator()(uint8_t i, uint8_t j) {
	return entries[i*w + j];
}
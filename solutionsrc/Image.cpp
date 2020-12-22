#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define BYTE_BOUND(value) value < 0 ? 0 : (value > 255 ? 255 : value)
#include "stb_image.h"
#include "stb_image_write.h"
#include "Image.h"

Image::Image(const char* filename, int channel_force) {
  if(read(filename, channel_force)) {
    printf("Read %s\n", filename);
    size = w*h*channels;
    pw = pow(2, ceil(log2(w)));
    ph = pow(2, ceil(log2(h)));
    psize = pw*ph;
  }
  else {
    printf("Failed to read %s\n", filename);
  }
}
Image::Image(int w, int h, int channels) : w(w), h(h), channels(channels), pw(pow(2, ceil(log2(w)))), ph(pow(2, ceil(log2(h)))) {
	size = w*h*channels;
  psize = pw*ph;
	data = new uint8_t[size];
}
Image::Image(const Image& img) : w(img.w), h(img.h), channels(img.channels), size(img.size), pw(img.pw), ph(img.ph), psize(img.psize) {
  data = new uint8_t[size];
  memcpy(data,img.data,size);
}

Image::~Image() {
  stbi_image_free(data);
}



bool Image::read(const char* filename, int channel_force) {
  data = stbi_load(filename, &w, &h, &channels, channel_force);
  channels = channel_force == 0 ? channels : channel_force;
  return data != NULL;
}

bool Image::write(const char* filename) {
	ImageType type = get_file_type(filename);
  int success;
  switch (type) {
    case PNG:
      success = stbi_write_png(filename, w, h, channels, data, w*channels);
      break;
    case BMP:
      success = stbi_write_bmp(filename, w, h, channels, data);
      break;
    case JPG:
      success = stbi_write_jpg(filename, w, h, channels, data, 100);
      break;
    case TGA:
      success = stbi_write_tga(filename, w, h, channels, data);
      break;
  }
  if(success != 0) {
    printf("\e[32mWrote \e[36m%s\e[0m, %d, %d, %d, %zu\n", filename, w, h, channels, size);
    return true;
  }
  else {
    printf("\e[31;1m Failed to write \e[36m%s\e[0m, %d, %d, %d, %zu\n", filename, w, h, channels, size);
    return false;
  }
}

ImageType Image::get_file_type(const char* filename) {
	const char* ext = strrchr(filename, '.');
	if(ext != nullptr) {
		if(strcmp(ext, ".png") == 0) {
			return PNG;
		}
		else if(strcmp(ext, ".jpg") == 0) {
			return JPG;
		}
		else if(strcmp(ext, ".tga") == 0) {
			return TGA;
		}
		else if(strcmp(ext, ".bmp") == 0) {
			return BMP;
		}
	}
	return PNG;
}



//doesn't divide for inverse
std::complex<double>* Image::recursive_fft(uint32_t n, std::complex<double> x[], std::complex<double>* X, bool inverse) {
  if(n == 1) {
    X[0] = x[0];
  }
  else {
    //w_n is an nth root of unity
    std::complex<double>* w = new std::complex<double>[n];
    for(uint64_t i=0; i<n; ++i) {
      double theta = inverse ? -2*M_PI*i/n : 2*M_PI*i/n; //inverse decided here
      w[i] = std::complex<double>(cos(theta), sin(theta));
    }
      
    //decompose x into even indices and odd indices
    std::complex<double>* x_e = new std::complex<double>[n/2] ; //even
    std::complex<double>* x_o = new std::complex<double>[n/2]; //odd
    //get coefficents out of x
    for(uint64_t i=0; i<n/2; ++i) {
      x_e[i] = x[2*i];
      x_o[i] = x[2*i+1];
    }

    //recursive call
    std::complex<double>* X_e = new std::complex<double>[n/2];
    recursive_fft(n/2, x_e, X_e, inverse);

    std::complex<double>* X_o = new std::complex<double>[n/2];
    recursive_fft(n/2, x_o, X_o, inverse);

    //combine results: X[k] = X_e[k] + w_n^+-k X_o[k]
    //           X[k+t/2] = X_e[k] - w_n^+-k X_o[k]
    for(uint64_t k = 0; k < n/2; ++k) {
      X[k] = X_e[k] + w[k]*X_o[k];
      X[k+n/2] = X_e[k] - w[k]*X_o[k];
    }
    delete[] w;
    delete[] x_o;
    delete[] x_e;
    delete[] X_o;
    delete[] X_e;
  }
  return X;
}
std::complex<double>* Image::dft(uint32_t m, uint32_t n, std::complex<double> y[], std::complex<double>* Y) {
  for(uint32_t i=0; i<m; ++i) {
    std::complex<double>* x = new std::complex<double>[n];
    for(uint32_t e=0; e<n; ++e) {
      x[e] = y[i*n+e];
    }
    std::complex<double>* X = new std::complex<double>[n];
    recursive_fft(n, x, X, false);
    delete[] x;
    for(uint32_t e=0; e<n; ++e) {
      Y[i*n+e] = X[e];
    }
    delete[] X;
  }
  for(uint32_t j=0; j<n; ++j) {
    std::complex<double>* x = new std::complex<double>[m];
    for(uint32_t e=0; e<m; ++e) {
      x[e] = Y[e*n+j];
    }
    std::complex<double>* X = new std::complex<double>[m];
    recursive_fft(m, x, X, false);
    delete[] x;
    for(uint32_t e=0; e<m; ++e) {
      Y[e*n+j] = X[e];
    }
    delete[] X;
  }
  return Y;
}
std::complex<double>* Image::idft(uint32_t m, uint32_t n, std::complex<double> Y[], std::complex<double>* y) {
  memset(y, 0, sizeof(std::complex<double>)*m*n);
  for(uint32_t j=0; j<n; ++j) {
    std::complex<double>* x = new std::complex<double>[m];
    for(uint32_t e=0; e<m; ++e) {
      x[e] = Y[e*n+j];
    }
    std::complex<double>* X = new std::complex<double>[m];
    recursive_fft(m, x, X, true);
    delete[] x;
    for(uint32_t e=0; e<m; ++e) {
      y[e*n+j] = X[e]/(double)m;
    }
    delete[] X;
  }
  for(uint32_t i=0; i<m; ++i) {
    std::complex<double>* x = new std::complex<double>[n];
    for(uint32_t e=0; e<n; ++e) {
      x[e] = y[i*n+e];
    }
    std::complex<double>* X = new std::complex<double>[n];
    recursive_fft(n, x, X, true);
    delete[] x;
    for(uint32_t e=0; e<n; ++e) {
      y[i*n+e] = X[e]/(double)n;
    }
    delete[] X;
  }
  return y;
}


std::complex<double>* Image::pointwise_mult(uint64_t len, std::complex<double> a[], std::complex<double> b[], std::complex<double>* p) {
  for(uint64_t k=0; k<len; ++k) {
    p[k] = a[k]*b[k];
  }
  return p;
}

std::complex<double>* Image::pad_kernel(uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc, std::complex<double>* pad_ker) {
  for(long i=-((long)cr); i<(long)ker_h-cr; ++i) {
    for(long j=-((long)cc); j<(long)ker_w-cc; ++j) {
      uint32_t r = i<0 ? i+ph : i;
      uint32_t c = j<0 ? j+pw : j;
      pad_ker[r*pw+c] = std::complex<double>(ker[(i+cr)*ker_w+(j+cc)],0);
    }
  }
  return pad_ker;
}

//TODO: go through all these functions and see if optimization is possible
Image& Image::std_convolve_clamp_to_0(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc) {
  uint8_t new_data[w*h];
  uint64_t center = cr*ker_w + cc;
  for(uint64_t k=channel; k<size; k+=channels) {
    double c = 0;

    for(long i = -((long)cr); i < (long)ker_h-cr; ++i) {
      long row = ((long)k/channels)/w-i; //-i because kernel is flipped as convolved
      if((row < 0) || (row > h-1)) {
        continue;
      }
      for(long j = -((long)cc); j < (long)ker_w-cc; ++j) {
        long col = ((long)k/channels)%w-j; //-j because kernel is flipped as convolved
        if((col < 0) || (col > w-1)) {
          continue;
        }
        c += ker[center+i*(long)ker_w+j]*data[(row*w+col)*channels+channel];
      }
    }
    new_data[k/channels] = (uint8_t)BYTE_BOUND(round(c));
  }
  for(uint64_t k=channel; k<size; k+=channels) {
    data[k] = new_data[k/channels];
  }
  return *this;
}
Image& Image::fd_convolve_clamp_to_0(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc) {
  //TODO: pad to next power of 2 if less than 1024, otherwise pad to multiple of 512 (how to implement this?)
  pw = pow(2, ceil(log2(w+ker_w-1)));
  ph = pow(2, ceil(log2(h+ker_h-1)));
  psize = pw*ph;

  std::complex<double>* img = new std::complex<double>[ph*pw];
  for(uint32_t i=0; i<h; ++i) {
    for(uint32_t j=0; j<w; ++j) {
      img[i*pw+j] = std::complex<double>(data[(i*w+j)*channels+channel],0);
    }
  }
  std::complex<double>* _ker = new std::complex<double>[ph*pw];
  pad_kernel(ker_w, ker_h, ker, cr, cc, _ker);

  std::complex<double>* imgFD = new std::complex<double>[ph*pw];
  std::complex<double>* kerFD = new std::complex<double>[ph*pw];
  
  dft(ph, pw, img, imgFD);
  dft(ph, pw, _ker, kerFD);

  delete[] img;
  delete[] _ker;

  std::complex<double>* resFD = new std::complex<double>[ph*pw];
  pointwise_mult(ph*pw, imgFD, kerFD, resFD);

  delete[] imgFD;
  delete[] kerFD;

  std::complex<double>* res = new std::complex<double>[ph*pw];
  idft(ph, pw, resFD, res);

  delete[] resFD;

  for(uint32_t i=0; i<h; ++i) {
    for(uint32_t j=0; j<w; ++j) {
      data[(i*w+j)*channels+channel] = BYTE_BOUND((uint8_t)round(res[i*pw+j].real()));
    }
  }

  delete[] res;

  return *this;
}

Image& Image::std_convolve_clamp_to_border(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc) {
  uint8_t new_data[w*h];
  uint64_t center = cr*ker_w + cc;
  for(uint64_t k=channel; k<size; k+=channels) {
    double c = 0;
    for(long i = -((long)cr); i < (long)ker_h-cr; ++i) {
      long row = ((long)k/channels)/w-i; //-i because kernel is flipped as convolved
      if(row < 0) {
        row = 0;
      }
      else if(row > h-1) {
        row = h-1;
      }
      for(long j = -((long)cc); j < (long)ker_w-cc; ++j) {
        long col = ((long)k/channels)%w-j; //-j because kernel is flipped as convolved
        if(col < 0) {
          col = 0;
        }
        else if(col > w-1) {
          col = w-1;
        }
        c += ker[center+i*(long)ker_w+j]*data[(row*w+col)*channels+channel];
      }
    }
    new_data[k/channels] = (uint8_t)BYTE_BOUND(round(c));
  }
  for(uint64_t k=channel; k<size; k+=channels) {
    data[k] = new_data[k/channels];
  }
  return *this;
}
Image& Image::fd_convolve_clamp_to_border(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc) {
  //TODO: pad to next power of 2 if less than 1024, otherwise pad to multiple of 512 (how to implement this?)
  pw = pow(2, ceil(log2(w+ker_w-1)));
  ph = pow(2, ceil(log2(h+ker_h-1)));
  psize = pw*ph;

  std::complex<double>* img = new std::complex<double>[ph*pw];
  for(uint32_t i=0; i<ph; ++i) {
    for(uint32_t j=0; j<pw; ++j) {
      uint32_t r = i<h ? i : (i<h+cr ? h-1 : 0);
      uint32_t c = j<w ? j : (j<w+cc ? w-1 : 0);
      img[i*pw+j] = std::complex<double>(data[(r*w+c)*channels+channel],0);
    }
  }
  std::complex<double>* _ker = new std::complex<double>[ph*pw];
  pad_kernel(ker_w, ker_h, ker, cr, cc, _ker);

  std::complex<double>* imgFD = new std::complex<double>[ph*pw];
  std::complex<double>* kerFD = new std::complex<double>[ph*pw];
  
  dft(ph, pw, img, imgFD);
  dft(ph, pw, _ker, kerFD);

  delete[] img;
  delete[] _ker;

  std::complex<double>* resFD = new std::complex<double>[ph*pw];
  pointwise_mult(ph*pw, imgFD, kerFD, resFD);

  delete[] imgFD;
  delete[] kerFD;

  std::complex<double>* res = new std::complex<double>[ph*pw];
  idft(ph, pw, resFD, res);

  delete[] resFD;

  for(uint32_t i=0; i<h; ++i) {
    for(uint32_t j=0; j<w; ++j) {
      data[(i*w+j)*channels+channel] = BYTE_BOUND((uint8_t)round(res[i*pw+j].real()));
    }
  }

  delete[] res;

  return *this;
}

Image& Image::std_convolve_cyclic(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc) {
  uint8_t new_data[w*h];
  uint64_t center = cr*ker_w + cc;
  for(uint64_t k=channel; k<size; k+=channels) {
    double c = 0;

    for(long i = -((long)cr); i < (long)ker_h-cr; ++i) {
      long row = ((long)k/channels)/w-i; //-i because kernel is flipped as convolved
      if(row < 0) {
        row = row%h + h;
      }
      else if(row > h-1) {
        row %= h;
      }
      for(long j = -((long)cc); j < (long)ker_w-cc; ++j) {
        long col = ((long)k/channels)%w-j; //-j because kernel is flipped as convolved
        if(col < 0) {
          col = col%w + w;
        }
        else if(col > w-1) {
          col %= w;
        }
        c += ker[center+i*(long)ker_w+j]*data[(row*w+col)*channels+channel];
      }
    }
    new_data[k/channels] = (uint8_t)BYTE_BOUND(round(c));
  }
  for(uint64_t k=channel; k<size; k+=channels) {
    data[k] = new_data[k/channels];
  }
  return *this;
}
Image& Image::fd_convolve_cyclic(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc) {
  //TODO: pad to next power of 2 if less than 1024, otherwise pad to multiple of 512 (how to implement this?)
  pw = pow(2, ceil(log2(w+ker_w-1)));
  ph = pow(2, ceil(log2(h+ker_h-1)));
  psize = pw*ph;

  std::complex<double>* img = new std::complex<double>[ph*pw];
  for(uint32_t i=0; i<ph; ++i) {
    for(uint32_t j=0; j<pw; ++j) {
      uint32_t r = i<h ? i : (i<h+cr ? i%h : i+h-ph);
      uint32_t c = j<w ? j : (j<w+cc ? j%w : j+w-pw);
      img[i*pw+j] = std::complex<double>(data[(r*w+c)*channels+channel],0);
    }
  }
  std::complex<double>* _ker = new std::complex<double>[ph*pw];
  pad_kernel(ker_w, ker_h, ker, cr, cc, _ker);

  std::complex<double>* imgFD = new std::complex<double>[ph*pw];
  std::complex<double>* kerFD = new std::complex<double>[ph*pw];
  
  dft(ph, pw, img, imgFD);
  dft(ph, pw, _ker, kerFD);

  delete[] img;
  delete[] _ker;

  std::complex<double>* resFD = new std::complex<double>[ph*pw];
  pointwise_mult(ph*pw, imgFD, kerFD, resFD);

  delete[] imgFD;
  delete[] kerFD;

  std::complex<double>* res = new std::complex<double>[ph*pw];
  idft(ph, pw, resFD, res);

  delete[] resFD;

  for(uint32_t i=0; i<h; ++i) {
    for(uint32_t j=0; j<w; ++j) {
      data[(i*w+j)*channels+channel] = BYTE_BOUND((uint8_t)round(res[i*pw+j].real()));
    }
  }

  delete[] res;

  return *this;
}




Image& Image::diffmap(Image& img) {
  int compare_width = fmin(w,img.w);
  int compare_height = fmin(h,img.h);
  int compare_channels = fmin(channels,img.channels);
  for(uint32_t i=0; i<compare_height; ++i) {
    for(uint32_t j=0; j<compare_width; ++j) {
      for(uint8_t k=0; k<compare_channels; ++k) {
        data[(i*w+j)*channels+k] = BYTE_BOUND(abs(data[(i*w+j)*channels+k]-img.data[(i*img.w+j)*img.channels+k]));
      }
    }
  }
  return *this;
}
Image& Image::diffmap_scale(Image& img, uint8_t scl) {
  int compare_width = fmin(w,img.w);
  int compare_height = fmin(h,img.h);
  int compare_channels = fmin(channels,img.channels);
  uint8_t largest = 0;
  for(uint32_t i=0; i<compare_height; ++i) {
    for(uint32_t j=0; j<compare_width; ++j) {
      for(uint8_t k=0; k<compare_channels; ++k) {
        data[(i*w+j)*channels+k] = BYTE_BOUND(abs(data[(i*w+j)*channels+k]-img.data[(i*img.w+j)*img.channels+k]));
        largest = fmax(largest, data[(i*w+j)*channels+k]);
      }
    }
  }
  scl = 255/fmax(1, fmax(scl, largest));
    for(int i = 0; i < size; ++i) {
      data[i] *= scl;
  }
  return *this;
}


Image& Image::grayscale_avg() {
  //if there are less than 3 channels, the image is already grayscale
  if(channels < 3) {
    printf("Image %p has less than 3 channels, it is assumed to already be grayscale.", this);
  }
  else {
    for(int i = 0; i < size; i+=channels) {
      int gray = (data[i] + data[i+1] + data[i+2])/3;
      memset(data+i, gray, 3);
    }
  }

  return *this;
}
Image& Image::grayscale_lum() {
  //if there are less than 3 channels, the image is already grayscale
  if(channels < 3) {
    printf("Image %p has less than 3 channels, it is assumed to already be grayscale.", this);
  }
  else {
    for(int i = 0; i < size; i+=channels) {
      int gray = 0.2126*data[i] + 0.7152*data[i+1] + 0.0722*data[i+2];
      memset(data+i, gray, 3);
    }
  }

  return *this;
}



Image& Image::medianFilter(Image* imgs, uint8_t numImgs) {
  //Error Checking
  for(int i=0; i<numImgs; ++i) {
    if(imgs[i].w != w || imgs[i].h != h) {
      printf("\e[31m[ERROR] Median Filter requires all images be the same size (%d x %d)\e[0m\n", w, h);
      return *this;
    }
    else if(imgs[i].channels != channels) {
      printf("\e[31m[ERROR] Median Filter requires all images to have the same number of color channels \n\tShould have \e[1m%d\e[0m\e[31m channels but found \e[1m%d\e[0m\e[31m channels\e[0m\n", channels, imgs[i].channels);
      return *this;
    }
  }

  //Algorithm
  uint8_t* temp = new uint8_t[numImgs];
  for(int i = 0;i < size;i++) {
    int outOfRange = 0;
    for(int j = 0;j < numImgs;++j) {
      if(i > imgs[j].size) {
        temp[j] = 0;
        outOfRange ++;
      }
      else {
        temp[j] = imgs[j].data[i];
      }
      
    }
    std::sort(temp, temp+numImgs);
    data[i] = temp[(numImgs + outOfRange)/2];
  }

  delete[] temp;
  return *this;
}

Image& Image::inverse() {
  uint8_t effectiveChannels = channels > 3 ? 3 : channels;

  for(int i = 0;i < w*h;++i) {
    for(int j = 0;j < effectiveChannels;++j) {
      data[i*channels+j] = 255 - data[i*channels+j];
    }
  }
  return *this;
}


Image& Image::colorMask(float mask) {
  return colorMask(mask, mask, mask);
}
Image& Image::colorMask(uint8_t mask) {
  return colorMask(mask, mask, mask);
}
Image& Image::colorMask(uint8_t r, uint8_t g, uint8_t b) {
  return colorMask(r/255.0f, g/255.0f, b/255.0f);
}
Image& Image::colorMask(float r, float g, float b) {
  if(channels < 3) {
    printf("\e[31m[ERROR] Color Mask requires at least 3 color channels (this image has %d)\e[0m\n", channels);
  }
  else if(r < 0 || r > 1 || g < 0 || g > 1 || b < 0 || b > 1) {
    printf("\e[31m[ERROR] Color Mask values must bebetween 0-1\e[0m\n");
  }
  else {
    for(int i = 0;i < size;i+=channels) {
      data[i] *= r;
      data[i+1] *= g;
      data[i+2] *= b;
    }
  }

  return *this;
}



Image& Image::encodeMessage(const char* message) {
  uint32_t len = strlen(message) * 8;
  if(len + STEG_HEADER_SIZE > size) {
    printf("\e[31m[ERROR] This message is too large (%lu bits / %zu bits)\e[0m\n", len+STEG_HEADER_SIZE, size);
    return *this;
  }

  for(uint8_t i = 0;i < STEG_HEADER_SIZE;++i) {
    data[i] &= 0xFE;
    data[i] |= (len >> (STEG_HEADER_SIZE - 1 - i)) & 1UL;
  }

  for(uint32_t i = 0;i < len;++i) {
    data[i+STEG_HEADER_SIZE] &= 0xFE;
    data[i+STEG_HEADER_SIZE] |= (message[i/8] >> ((len-1-i)%8)) & 1;
  }

  return *this;
}

Image& Image::decodeMessage(char* buffer, size_t* messageLength) {
  uint32_t len = 0;
  for(uint8_t i = 0;i < STEG_HEADER_SIZE;++i) {
    len = (len << 1) | (data[i] & 1);
  }
  *messageLength = len / 8;

  for(uint32_t i = 0;i < len;++i) {
    buffer[i/8] = (buffer[i/8] << 1) | (data[i+STEG_HEADER_SIZE] & 1);
  }


  return *this;
}


Image& Image::tempAdjust(short value) {
  if(channels < 3) {
    printf("\e[31m[ERROR] Warm filter requires at least 3 color channels (this image only has %d channels)\e[0m\n", channels);
  }
  else {
    for(int i = 0;i < size;i += channels) {
      data[i] = BYTE_BOUND(data[i] + value);
      data[i+2] = BYTE_BOUND(data[i+2] - value);
    }
  }

  return *this;
}

Image& Image::grainAdjust(uint8_t level) {
  std::default_random_engine generator;
  std::normal_distribution<double> dist(0,1);
  uint8_t effectiveChannels = channels > 3 ? 3 : channels;
  for(int i = 0;i < w*h;++i) {
    for(int j = 0;j < effectiveChannels;++j) {
      data[i*channels+j] = BYTE_BOUND(data[i*channels+j] + dist(generator));
    }
  }
  return *this;
}


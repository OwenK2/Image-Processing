#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define BYTE_BOUND(value) value < 0 ? 0 : (value > 255 ? 255 : value)
#include "stb_image.h"
#include "stb_image_write.h"
#include "Image.h"

Image::Image(const char* filename, int channelForce) {
  if(!read(filename, channelForce)) {
    printf("Failed to read %s\n", filename);
  }
  else {
  	size = w*h*channels;
    pw = pow(2, ceil(log2(w)));
    ph = pow(2, ceil(log2(h)));
    psize = pw*ph;
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



bool Image::read(const char* filename, int channelForce) {
  data = stbi_load(filename, &w, &h, &channels, channelForce);
  channels = channelForce == 0 ? channels : channelForce;
  return data != NULL;
}

bool Image::write(const char* filename) {
	ImageType type = get_file_type(filename);
	printf("\e[32mWrote \e[36m%s\e[0m, %d, %d, %d, %zu\n", filename, w, h, channels, size);
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
  return success != 0;
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



//doesnt divide, you must do that
std::complex<double>* Image::recursive_fft(uint32_t n, std::complex<double> x[], std::complex<double>* X, bool inverse) {
  if(n == 1) {
    X[0] = x[0];
  }
  else {
    //w_n is an nth root of unity
    std::complex<double> w[n];
    for(uint64_t i=0; i<n; ++i) {
      double theta = inverse ? -2*M_PI*i/n : 2*M_PI*i/n; //inverse decided here
      w[i] = std::complex<double>(cos(theta), sin(theta));
    }
      
    //decompose x into even indices and odd indices
    std::complex<double> x_e[n/2]; //even
    std::complex<double> x_o[n/2]; //odd
    //get coefficents out of x
    for(uint64_t i=0; i<n/2; ++i) {
      x_e[i] = x[2*i];
      x_o[i] = x[2*i+1];
    }

    //recursive call
    std::complex<double> X_e[n/2];
    recursive_fft(n/2, x_e, X_e, inverse);

    std::complex<double> X_o[n/2];
    recursive_fft(n/2, x_o, X_o, inverse);

    //combine results: X[k] = X_e[k] + w_n^+-k X_o[k]
    //           X[k+t/2] = X_e[k] - w_n^+-k X_o[k]
    for(uint64_t k = 0; k < n/2; ++k) {
      X[k] = X_e[k] + w[k]*X_o[k];
      X[k+n/2] = X_e[k] - w[k]*X_o[k];
    }
  }
  return X;
}

std::complex<double>* Image::dft(uint8_t channel, uint32_t m, uint32_t n, std::complex<double>* Y) {
  memset(Y, 0, sizeof(std::complex<double>)*m*n);
  for(uint32_t i=0; i<h; ++i) {
    std::complex<double>* x = new std::complex<double>[n];
    for(uint32_t e=0; e<w; ++e) {
      x[e] = std::complex<double>(data[(i*w+e)*channels+channel]/255.0,0);
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
    for(uint32_t e=0; e<h; ++e) {
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
Image& Image::idft(uint8_t channel, uint32_t m, uint32_t n, std::complex<double> Y[]) {
  std::complex<double>* _Y = new std::complex<double>[n*m];
  for(uint32_t j=0; j<n; ++j) {
    std::complex<double> x[m];
    for(uint32_t e=0; e<m; ++e) {
      x[e] = Y[e*n+j];
    }
    std::complex<double> X[m];
    recursive_fft(m, x, X, true);
    for(uint32_t e=0; e<m; ++e) {
      _Y[e*n+j] = X[e]/(double)m;
    }
  }
  //rows
  for(uint32_t i=0; i<h; ++i) {
    std::complex<double> x[n];
    for(uint32_t e=0; e<n; ++e) {
      x[e] = _Y[i*n+e];
    }
    std::complex<double> X[n];
    recursive_fft(n, x, X, true);
    for(uint32_t e=0; e<w; ++e) {
      data[(i*w+e)*channels+channel] = round(X[e].real()*255/n);
    }
  }
  delete[] _Y;
  return *this;
}

/*
std::complex<double>* Image::dft(uint8_t channel, std::complex<double>* Y) {
  std::complex<double>* a = new std::complex<double>[n*m];
  for(uint32_t i=0; i<h; ++i) {
    for(uint32_t j=0; j<w; ++j) {
      a[i*n+j] = std::complex<double>(data[(i*w+j)*channels+channel]/255.0,0);
    }
  }
  std::complex<double>* b = new std::complex<double>[n*m];
  for(uint32_t i=0; i<m; ++i) {
    std::complex<double> x[n];
    for(uint32_t e=0; e<n; ++e) {
      x[e] = a[i*n+e];
    }
    std::complex<double> X[n];
    recursive_fft(n, x, X, false);
    for(uint32_t e=0; e<n; ++e) {
      b[i*n+e] = X[e];
    }
  }
  delete[] a;
  for(uint32_t j=0; j<n; ++j) {
    std::complex<double> x[m];
    for(uint32_t e=0; e<m; ++e) {
      x[e] = b[e*n+j];
    }
    std::complex<double> X[m];
    recursive_fft(m, x, X, false);
    for(uint32_t e=0; e<m; ++e) {
      Y[e*n+j] = X[e];
    }
  }
  delete[] b;
  return Y;
}
Image& Image::idft(uint8_t channel, std::complex<double> Y[]) {
  std::complex<double>* b = new std::complex<double>[n*m];
  for(uint32_t j=0; j<n; ++j) {
    std::complex<double> x[m];
    for(uint32_t e=0; e<m; ++e) {
      x[e] = Y[e*n+j];
    }
    std::complex<double> X[m];
    recursive_fft(m, x, X, true);
    for(uint32_t e=0; e<m; ++e) {
      b[e*n+j] = X[e]/(double)m;
    }
  }
  
  std::complex<double>* a = new std::complex<double>[n*m];
  for(uint32_t i=0; i<m; ++i) {
    std::complex<double> x[n];
    for(uint32_t e=0; e<n; ++e) {
      x[e] = b[i*n+e];
    }
    std::complex<double> X[n];
    recursive_fft(n, x, X, true);
    for(uint32_t e=0; e<n; ++e) {
      a[i*n+e] = X[e]/(double)n;
    }
  }
  delete[] b;
  for(uint32_t i=0; i<h; ++i) {
    for(uint32_t j=0; j<w; ++j) {
      data[(i*w+j)*channels+channel] = (uint8_t)round(a[i*n+j].real()*255);
    }
  }
  delete[] a;
  return *this;
}
*/






Image& Image::diffmap(Image& img) {
  if(img.channels != channels) {
    printf("Images %p and %p do not have the same number of channels, they were not compared.", this, &img);
  }
  else {
    for(int i = 0; i < h; ++i) {
      for(int j = 0; j < w*channels; ++j) {
        data[i*w*channels + j] = abs(data[i*w*channels + j] - img.data[i*w*channels + j]);
      }
    }
  }

  return *this;
}
Image& Image::diffmap_scale(Image& img, uint8_t scl) {
  if(img.channels != channels) {
    printf("Images %p and %p do not have the same number of channels, they were not compared.", this, &img);
  }
  else {
    uint8_t largest = 0;
    for(int i = 0; i < h; ++i) {
      for(int j = 0; j < w*channels; ++j) {
        data[i*w*channels + j] = abs(data[i*w*channels + j] - img.data[i*w*channels + j]);
        largest = fmax(largest, data[i*w*channels + j]);
      }
    }
    scl = 255/fmax(1, fmax(scl, largest));
    for(int i = 0; i < size; ++i) {
      data[i] *= scl;
    }
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
  for(int i = 0;i < numImgs;++i) {
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
  unsigned int messageSize = strlen(message) * 8 + sizeof(unsigned int)*8;
  if(messageSize > size) {
    printf("\e[31m[ERROR] The message is too large for this image, it requires %d bits but image can only encode %zu bits\e[0m\n", messageSize, size);
  }
  else {
    unsigned int textSize = strlen(message);
    for(int i = 0;i < sizeof(unsigned int)*8;++i) {
      if(textSize & 1) {
        data[i] |= 1;
      }
      else {
        data[i] &= 254;
      }
      textSize >>= 1;
    }

    for(int i = 0;i < strlen(message);++i) {
      for(int j = 0;j < 8;++j) {
        if((message[i] >> j) & 1) {
          data[i*8+j+sizeof(unsigned int)*8] |= 1;
        }
        else {
          data[i*8+j+sizeof(unsigned int)*8] &= 254;
        }
      }
    }
  }

  return *this;
}

const char* Image::decodeMessage() {
  unsigned int messageSize = 0;
  for(int i = 0;i < sizeof(unsigned int)*8;++i) {
    if(data[i] & 1) {
      messageSize |= 1 << i;
    }
    else {
      messageSize &= ~(1 << i);
    }
  }

  char* message = new char[messageSize];
  for(int i = 0;i < messageSize;++i) {
    for(int j = 0;j < 8;++j) {
      if(data[i*8+j+sizeof(unsigned int)*8] & 1) {
        message[i] |= 1 << j;
      }
      else {
        message[i] &= ~(1 << j);
      }
    }
  }
  return (const char*) message;
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

=======


Image& Image::convolve_sd(Kernel& ker) {
  uint8_t new_data[size];
  for(int k = 0; k < size; k+=channels) {
    double r = 0;
    double g = 0;
    double b = 0;

    for(int i = -ker.cy; i < ker.side-ker.cy; ++i) {
      for(int j = -ker.cx; j < ker.side-ker.cx; ++j) {
        if((k/(channels*w)+i < 0) || (k/(channels*w)+i > h-1)) {
          break;
        }
        if(((k/channels)%w+j < 0) || ((k/channels)%w+j > w-1)) {
          continue;
        }
        else {
          r += ker(i, j)*data[k+(i*ker.side+j)];
          g += ker(i, j)*data[k+(i*ker.side+j)+1];
          b += ker(i, j)*data[k+(i*ker.side+j)+2];
        }
      }
    }
    new_data[k] = (uint8_t)r;
    new_data[k+1] = (uint8_t)g;
    new_data[k+2] = (uint8_t)b;
  }
  memcpy(data, new_data, size);
  return *this;
}
>>>>>>> dd74a608be78717999e1bfda42c3517fafbcabd4

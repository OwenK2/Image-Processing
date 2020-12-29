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
  }
  else {
    printf("Failed to read %s\n", filename);
  }
}
Image::Image(int w, int h, int channels) : w(w), h(h), channels(channels) {
	size = w*h*channels;
	data = new uint8_t[size];
}
Image::Image(const Image& img) : w(img.w), h(img.h), channels(img.channels), size(img.size) {
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



uint32_t Image::rev(uint32_t n, uint32_t a) {
  uint8_t max_bits = (uint8_t)ceil(log2(n));
  uint32_t reverse_a = 0;
  for(uint8_t i=0; i<max_bits; ++i) {
    if(a & (1<<i)) {
      reverse_a |= (1<<(max_bits-1-i));
    }
  }
  return reverse_a;
}
void Image::bit_rev(uint32_t n, std::complex<double> a[], std::complex<double>* A) {
  for(uint32_t i=0; i<n; ++i) {
    A[rev(n, i)] = a[i];
  }
}

void Image::fft(uint32_t n, std::complex<double> x[], std::complex<double>* X) {
  //x in standard order
  if(x != X) {
    memcpy(X, x, n*sizeof(std::complex<double>));
  }

  //GS butterfly
  uint32_t sub_probs = 1;
  uint32_t sub_prob_size = n;
  uint32_t half;
  uint32_t j_begin;
  uint32_t i;
  uint32_t j_end;
  uint32_t j;
  std::complex<double> w_step;
  std::complex<double> w;
  std::complex<double> tmp1, tmp2;
  while(sub_prob_size>1) {
    half = sub_prob_size>>1;
    w_step = std::complex<double>(cos(-2*M_PI/sub_prob_size), sin(-2*M_PI/sub_prob_size));
    for(i=0; i<sub_probs; ++i) {
      j_begin = i*sub_prob_size;
      j_end = j_begin+half;
      w = std::complex<double>(1,0);
      for(j=j_begin; j<j_end; ++j) {
        tmp1 = X[j];
        tmp2 = X[j+half];
        X[j] = tmp1+tmp2;
        X[j+half] = (tmp1-tmp2)*w;
        w *= w_step;
      }
    }
    sub_probs <<= 1;
    sub_prob_size = half;
  }
  //X in bit reversed order
}
void Image::ifft(uint32_t n, std::complex<double> X[], std::complex<double>* x) {
  //X in bit reversed order
  if(x != X) {
    memcpy(x, X, n*sizeof(std::complex<double>));
  }

  //CT butterfly
  uint32_t sub_probs = n>>1;
  uint32_t sub_prob_size;
  uint32_t half = 1;
  uint32_t j_begin;
  uint32_t i;
  uint32_t j_end;
  uint32_t j;
  std::complex<double> w_step;
  std::complex<double> w;
  std::complex<double> tmp1, tmp2;
  while(half<n) {
    sub_prob_size = half<<1;
    w_step = std::complex<double>(cos(2*M_PI/sub_prob_size), sin(2*M_PI/sub_prob_size));
    for(i=0; i<sub_probs; ++i) {
      j_begin = i*sub_prob_size;
      j_end = j_begin+half;
      w = std::complex<double>(1,0);
      for(j=j_begin; j<j_end; ++j) {
        tmp1 = x[j];
        tmp2 = w*x[j+half];
        x[j] = tmp1+tmp2;
        x[j+half] = tmp1-tmp2;
        w *= w_step;
      }
    }
    sub_probs >>= 1;
    half = sub_prob_size;
  }
  for(uint32_t i=0; i<n; ++i) {
    x[i] /= n;
  }
  //x in standard order
}

void Image::dft_2D(uint32_t m, uint32_t n, std::complex<double> x[], std::complex<double>* X) {
  std::complex<double>* intermediate = new std::complex<double>[m*n];
  for(uint32_t i=0; i<m; ++i) {
    fft(n, x+i*n, X+i*n);
    for(uint32_t j=0; j<n; ++j) {
      intermediate[j*m+i] = X[i*n+j];
    }
  }
  for(uint32_t j=0; j<n; ++j) {
    fft(m, intermediate+j*m, intermediate+j*m);
    for(uint32_t i=0; i<m; ++i) {
      X[i*n+j] = intermediate[j*m+i];
    }
  }
  delete[] intermediate;
}
void Image::idft_2D(uint32_t m, uint32_t n, std::complex<double> X[], std::complex<double>* x) {
  std::complex<double>* intermediate = new std::complex<double>[m*n];
  for(uint32_t j=0; j<n; ++j) {
    for(uint32_t i=0; i<m; ++i) {
      intermediate[j*m+i] = X[i*n+j];
    }
    ifft(m, intermediate+j*m, intermediate+j*m);
  }
  for(uint32_t i=0; i<m; ++i) {
    for(uint32_t j=0; j<n; ++j) {
      X[i*n+j] = intermediate[j*m+i];
    }
    ifft(n, X+i*n, x+i*n);
  }
  delete[] intermediate;
}

void Image::pointwise_product(uint64_t l, std::complex<double> a[], std::complex<double> b[], std::complex<double>* p) {
  for(uint64_t k=0; k<l; ++k) {
    p[k] = a[k]*b[k];
  }
}

void Image::pad_kernel(uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc, uint32_t pw, uint32_t ph, std::complex<double>* pad_ker) {
  for(long i=-((long)cr); i<(long)ker_h-cr; ++i) {
    for(long j=-((long)cc); j<(long)ker_w-cc; ++j) {
      uint32_t r = i<0 ? i+ph : i;
      uint32_t c = j<0 ? j+pw : j;
      pad_ker[r*pw+c] = std::complex<double>(ker[(i+cr)*ker_w+(j+cc)],0);
    }
  }
}


Image& Image::fd_convolve_clamp_to_0(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc) {
  uint32_t pw = 1<<((uint8_t)ceil(log2(w+ker_w-1)));
  uint32_t ph = 1<<((uint8_t)ceil(log2(h+ker_h-1)));
  uint64_t psize = ph*pw;

  std::complex<double>* pad_img = new std::complex<double>[psize];
  for(uint64_t i=0; i<h; ++i) {
    for(uint64_t j=0; j<w; ++j) {
      pad_img[i*pw+j] = std::complex<double>(data[(i*w+j)*channels+channel],0);
    }
  }
  std::complex<double>* pad_ker = new std::complex<double>[psize];
  pad_kernel(ker_w, ker_h, ker, cr, cc, pw, ph, pad_ker);

  dft_2D(ph, pw, pad_img, pad_img);
  dft_2D(ph, pw, pad_ker, pad_ker);
  pointwise_product(psize, pad_img, pad_ker, pad_img);
  idft_2D(ph, pw, pad_img, pad_img);

  for(uint64_t i=0; i<h; ++i) {
    for(uint64_t j=0; j<w; ++j) {
      data[(i*w+j)*channels+channel] = BYTE_BOUND(round(pad_img[i*pw+j].real()));
    }
  }

  delete[] pad_img;
  delete[] pad_ker;

  return *this;
}
Image& Image::fd_convolve_clamp_to_border(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc) {
  uint32_t pw = 1<<((uint8_t)ceil(log2(w+ker_w-1)));
  uint32_t ph = 1<<((uint8_t)ceil(log2(h+ker_h-1)));
  uint64_t psize = ph*pw;

  std::complex<double>* pad_img = new std::complex<double>[psize];
  for(uint32_t i=0; i<ph; ++i) {
    for(uint32_t j=0; j<pw; ++j) {
      uint32_t r = i<h ? i : (i<h+cr ? h-1 : 0);
      uint32_t c = j<w ? j : (j<w+cc ? w-1 : 0);
      pad_img[i*pw+j] = std::complex<double>(data[(r*w+c)*channels+channel],0);
    }
  }

  std::complex<double>* pad_ker = new std::complex<double>[psize];
  pad_kernel(ker_w, ker_h, ker, cr, cc, pw, ph, pad_ker);

  dft_2D(ph, pw, pad_img, pad_img);
  dft_2D(ph, pw, pad_ker, pad_ker);
  pointwise_product(psize, pad_img, pad_ker, pad_img);
  idft_2D(ph, pw, pad_img, pad_img);

  for(uint64_t i=0; i<h; ++i) {
    for(uint64_t j=0; j<w; ++j) {
      data[(i*w+j)*channels+channel] = BYTE_BOUND(round(pad_img[i*pw+j].real()));
    }
  }

  delete[] pad_img;
  delete[] pad_ker;

  return *this;
}
Image& Image::fd_convolve_cyclic(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc) {
  uint32_t pw = 1<<((uint8_t)ceil(log2(w+ker_w-1)));
  uint32_t ph = 1<<((uint8_t)ceil(log2(h+ker_h-1)));
  uint64_t psize = ph*pw;

  std::complex<double>* pad_img = new std::complex<double>[psize];
  for(uint32_t i=0; i<ph; ++i) {
    for(uint32_t j=0; j<pw; ++j) {
      uint32_t r = i<h ? i : (i<h+cr ? i%h : i+h-ph);
      uint32_t c = j<w ? j : (j<w+cc ? j%w : j+w-pw);
      pad_img[i*pw+j] = std::complex<double>(data[(r*w+c)*channels+channel],0);
    }
  }

  std::complex<double>* pad_ker = new std::complex<double>[psize];
  pad_kernel(ker_w, ker_h, ker, cr, cc, pw, ph, pad_ker);

  dft_2D(ph, pw, pad_img, pad_img);
  dft_2D(ph, pw, pad_ker, pad_ker);
  pointwise_product(psize, pad_img, pad_ker, pad_img);
  idft_2D(ph, pw, pad_img, pad_img);

  for(uint64_t i=0; i<h; ++i) {
    for(uint64_t j=0; j<w; ++j) {
      data[(i*w+j)*channels+channel] = BYTE_BOUND(round(pad_img[i*pw+j].real()));
    }
  }

  delete[] pad_img;
  delete[] pad_ker;

  return *this;
}


Image& Image::convolve_linear(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc) {
  if(ker_w*ker_h > 224) {
    fd_convolve_clamp_to_0(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc);
  }
  else {
    std_convolve_clamp_to_0(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc);
  }
}
Image& Image::convolve_clamp_to_border(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc) {
  if(ker_w*ker_h > 224) {
    fd_convolve_clamp_to_border(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc);
  }
  else {
    std_convolve_clamp_to_border(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc);
  }
}
Image& Image::convolve_cyclic(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc) {
  if(ker_w*ker_h > 224) {
    fd_convolve_cyclic(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc);
  }
  else {
    std_convolve_cyclic(uint8_t channel, uint32_t ker_w, uint32_t ker_h, double ker[], uint32_t cr, uint32_t cc);
  }
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


Image& Image::flipX() { 
  uint8_t tmp[4];
  uint8_t* px1;
  uint8_t* px2;
  for(int y = 0;y < h;++y) {
    for(int x = 0;x < w/2;++x) {
      px1 = &data[(y * w + x) * channels];
      px2 = &data[(y * w + (w - 1 - x)) * channels];
      memcpy(tmp, px1, channels);
      memcpy(px1, px2, channels);
      memcpy(px2, tmp, channels);
    }
  }
  return *this;
}
Image& Image::flipY() {
  uint8_t tmp[4];
  uint8_t* px1;
  uint8_t* px2;
  for(int x = 0;x < w;++x) {
    for(int y = 0;y < h/2;++y) {
      px1 = &data[(y * w + x) * channels];
      px2 = &data[((h - 1 - y) * w + x) * channels];
      memcpy(tmp, px1, channels);
      memcpy(px1, px2, channels);
      memcpy(px2, tmp, channels);
    }
  }
  return *this;
}


Image& Image::overlay(const Image& img, int offsetX, int offsetY) {
  uint8_t* srcPx;
  uint8_t* dstPx;
  uint8_t* alpha;
  for(int y = 0;y < img.h;++y) {
    if(y + offsetY < 0) {continue;}
    else if(y + offsetY >= h) {break;}

    for(int x = 0;x < img.w;++x) {
      if(x + offsetX < 0) {continue;}
      else if(x + offsetX >= w) {break;}
      dstPx = &data[(w * (y + offsetY) + (x + offsetX)) * channels];
      srcPx = &img.data[(img.w * y + x) * img.channels];

      float dstAlpha = channels < 4 ? 1 : dstPx[3] / 255.f; 
      float srcAlpha = img.channels < 4 ? 1 : srcPx[3] / 255.f;
      if(dstAlpha > .9999 && srcAlpha > .9999) {
        if(img.channels >= channels) {
          memcpy(dstPx, srcPx, channels);
        }
        else {
          memset(dstPx, srcPx[0], channels);
        }
      }
      else if(srcAlpha > 0) {
        float outAlpha = dstAlpha * (1 - srcAlpha) + srcAlpha;
        if(outAlpha == 0) {
          memset(dstPx, 0, channels);
        }
        else {
          for(uint8_t chnl = 0;chnl < channels;++chnl) {
            dstPx[chnl] = (uint8_t)BYTE_BOUND((srcPx[chnl]/255.f * srcAlpha + dstPx[chnl]/255.f * dstAlpha * (1 - srcAlpha)) / outAlpha * 255);
          }
          if(channels > 3) {dstPx[3] = (uint8_t)(outAlpha * 255);}
        }
        
      }
    }
  }
  return *this;
}

Image& Image::overlayText(const char* text, const Font& font, int offsetX, int offsetY, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  size_t len = strlen(text);
  SFT_Char c;
  double kerning[2] = {0};
  for(size_t i = 0;i < len;++i) {
    if(sft_char(&font.sft, text[i], &c) != 0) {
      printf("\e[31m[ERROR]Failed to get character '%c' (%d)\e[0m\n", text[i], text[i]);
      continue;
    }
    if(i > 0 && sft_kerning(&font.sft, text[i-1], text[i], kerning) != 0) {kerning[0] = 0;kerning[1] = 0;}

    // printf("\e[36mDrawing \e[1m'%c'\e[0m: %3d %3d, Off: %3d %3d, Adv: %f, Kerning: %3f %3f\n", text[i], c.width, c.height, c.x, c.y, c.advance, kerning[0], kerning[1]);


    int32_t dx, dy;
    uint8_t srcPx;
    uint8_t* dstPx;
    uint8_t color[4] = {r, g, b, a};
    for(uint16_t y = 0;y < c.height;++y) {
      dy = y + offsetY + (int)kerning[1] + c.y;
      if(dy < 0) {continue;}
      else if(dy >= h) {break;}
      for(uint16_t x = 0;x < c.width;++x) {
        dx = x + offsetX + (int)kerning[0] + c.x;
        if(dx < 0) {continue;}
        else if(dx >= w) {break;}
        srcPx = c.image[y * c.width + x];
        dstPx = &data[(dy * w + dx) * channels];
        if(srcPx != 0) { 
          if(srcPx == 255 && a == 255) {
            memcpy(dstPx, color, channels);
          } 
          else {
            float srcAlpha = (srcPx / 255.f) * (a / 255.f);
            float dstAlpha = channels < 4 ? 1.f : dstPx[3] / 255.f;
            float outAlpha = dstAlpha * (1 - srcAlpha) + srcAlpha;
            if(outAlpha == 0) {
              memset(dstPx, 0, channels);
            }
            else {
              for(uint8_t chnl = 0;chnl < channels;++chnl) {
                dstPx[chnl] = (uint8_t)BYTE_BOUND((color[chnl]/255.f * srcAlpha + dstPx[chnl]/255.f * dstAlpha * (1 - srcAlpha)) / outAlpha * 255);
              }
              if(channels > 3) {dstPx[3] = (uint8_t)(outAlpha * 255);}
            }
          }
        }
      }
    }
    offsetX += c.advance;
    free(c.image);
  }
  return *this;
}

float lerp(uint8_t s, uint8_t e, float t) {return s+(e-s)*t;}
#define blerp(c00, c01, c10, c11, tx, ty) (lerp(lerp(c00, c01, tx), lerp(c10, c11, tx), ty))

Image& Image::resize(uint16_t nw, uint16_t nh) {
  uint8_t* newData = new uint8_t[nw*nh*channels];

  float xScale = (float)nw / (w - 1);
  float yScale = (float)nh / (h - 1);

  float sx, sy, alpha, finalAlpha;
  uint16_t sxi, syi;
  uint8_t* c00, *c01, *c10, *c11;
  
  for(uint16_t y = 0;y < nh;++y) {
    sy = (y / yScale);
    syi = (uint16_t)sy;
    for(uint16_t x = 0;x < nw;++x) {
      sx = (x / xScale);
      sxi = (uint16_t)sx;
      c00 = &data[(sxi + syi * w) * channels];
      c01 = &data[(1 + sxi + syi * w) * channels];
      c10 = &data[(sxi + (syi + 1) * w) * channels];
      c11 = &data[(1 + sxi + (syi + 1) * w) * channels];

      alpha = 1;
      finalAlpha = 1;
      for(int8_t chnl = channels-1;chnl > -1;--chnl) {

        newData[(x + y * nw)*channels + chnl] = finalAlpha > 0 ? (uint8_t)BYTE_BOUND(blerp(c00[chnl]*alpha, c01[chnl]*alpha, c10[chnl]*alpha, c11[chnl]*alpha, sx - sxi, sy - syi) / finalAlpha) : 0;
        if(chnl == 3) {
          finalAlpha = newData[(x + y * nw)*channels + chnl] / 255.f;
        }
      }
    }
  }

  // Fix image buffer
  w = nw;
  h = nh;
  size = w * h * channels;
  delete[] data;
  data = newData;

  return *this;
}


Image& Image::resizeNN(uint16_t nw, uint16_t nh) {
  uint8_t* newData = new uint8_t[nw*nh*channels];

  float xScale = (float)nw / (w - 1);
  float yScale = (float)nh / (h - 1);

  uint16_t sx, sy;
  
  for(uint16_t y = 0;y < nh;++y) {
    sy = (uint16_t)(y / yScale);
    for(uint16_t x = 0;x < nw;++x) {
      sx = (uint16_t)(x / xScale);
      memcpy(&newData[(x + y * nw) * channels], &data[(sx + sy * w) * channels], channels);
    }
  }

  // Fix image buffer
  w = nw;
  h = nh;
  size = w * h * channels;
  delete[] data;
  data = newData;

  return *this;
}



Image& Image::crop(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch) {
  uint8_t* newData = new uint8_t[cw*ch*channels];
  memset(newData, 0, cw*ch*channels);

  uint16_t sx, sy;
  for(uint16_t dy = 0;dy < ch;++dy) {
    sy = cy + dy;
    if(sy >= h) {break;}
    for(uint16_t dx = 0;dx < cw;++dx) {
      sx = cx + dx;
      if(sx >= w) {break;}
      memcpy(&newData[(dx + cw * dy) * channels], &data[(sx + w * sy) * channels], channels);
    }
  }

  w = cw;
  h = ch;
  size = w * h * channels;

  delete[] data;
  data = newData;
  newData = nullptr;

  return *this;
}
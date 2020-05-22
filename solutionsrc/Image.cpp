#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include "Image.h"

Image::Image(const char* filename) {
  if(!read(filename)) {
    printf("Failed to read %s\n", filename);
  }
  else {
  	size = w*h*channels;
  }
}
Image::Image(int w, int h, int channels) : w(w), h(h), channels(channels) {
	size = w*h*channels;
	data = new uint8_t[size];
}
Image::Image(const Image& img) : Image(img.w, img.h, img.channels) {
	memcpy(data,img.data,size);
}

Image::~Image() {
  stbi_image_free(data);
}



bool Image::read(const char* filename) {
  data = stbi_load(filename, &w, &h, &channels, 0);
  return data != NULL;
}

bool Image::write(const char* filename) {
	ImageType type = get_file_type(filename);
	printf("%s, %d, %d, %d, %zu\n", filename, w, h, channels, size);
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





Image& Image::diffmap(Image& img) {
  if(img.channels != channels) {
    printf("Images %p and %p do not have the same number of channels, they were not compared.", this, &img);
  }
  else {
    for(int j = 0; j < h; ++j) {
      for(int i = 0; i < w*channels; ++i) {
        data[j*w*channels + i] = abs(data[j*w*channels + i] - img.data[j*w*channels + i]);
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
    for(int j = 0; j < h; ++j) {
      for(int i = 0; i < w*channels; ++i) {
        data[j*w*channels + i] = abs(data[j*w*channels + i] - img.data[j*w*channels + i]);
        largest = fmax(largest, data[j*w*channels + i]);
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




Image& Image::convolve(Kernel& ker) {
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

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
	ImageType type = getFileType(filename);
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

ImageType Image::getFileType(const char* filename) {
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




Image& Image::grayscale_avg() {
  //maximum channels we will overwrite is 3 (rgb)
  int channels_to_overwrite = channels > 3 ? 3 : channels;

  //loop through data
  for(int i = 0; i < size; i+=channels) {
    //get sum of color values
    int sum = 0;
    for(int j = 0; j < channels_to_overwrite; ++j) {
      sum += data[i+j]
    }
    //compute average color value and set pixels to that gray shade
    int gray = sum/channels_to_overwrite;
    for(int j = 0; j < channels_to_overwrite; ++j) {
      data[i+j] = gray;
    }
  }

  return *this;
}

Image& Image::grayscale_lum() {
  
}




#include "Image.h"
#include <cassert>
#include <sstream>
#include <string.h>

namespace suite {

  Image::Image(int width, int height, uint8_t* buffer,
               int size, int x_offset, int y_offset, uint frameTime,
               ImageUpdateStats stats)
    : width_(width), height_(height), x_offset_(x_offset), y_offset_(y_offset),
      stats_(stats), size_(size), frameTime_(frameTime), buffer_(buffer)
  {
  }

  Image::~Image()
  {
    delete [] buffer_;
  }

  Image &Image::operator+=(Pixel const &pixel)
  {
    assert(pixelCount * 4 <= size_);
    memcpy(&buffer_[pixelCount * 4], &pixel, 4);
    pixelCount++;
    return *this;
  }

  uint8_t* Image::getBuffer() const
  {
    return buffer_;
  }

  std::istringstream& operator>>(std::istringstream &in, Pixel& pixel)
  {
    int r, g, b;
    in >> r;
    in >> g;
    in >> b;

    pixel.r = r;
    pixel.g = g;
    pixel.b = b;
    return in;
  }

  void Image::setBuffer(uint8_t *buffer, int size)
  {
    delete [] this->buffer_;
    buffer_ = buffer;
    size_ = size;
  }
}

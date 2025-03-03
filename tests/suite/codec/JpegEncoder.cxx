#include "JpegEncoder.h"
#include "ImageEncoder.h"
#include <rfb/JpegCompressor.h>
#include <rfb/JpegDecompressor.h>
#include <rfb/ServerParams.h>
#include <rfb/TightJPEGEncoder.h>
#include <fstream>

namespace suite {
  static rfb::PixelFormat pf(32, 24, false, true,
                      255, 255, 255, 0, 8, 16);

  JpegEncoder::JpegEncoder(int quality, int subsampling)
    : ImageEncoder(JPEG), quality_(quality), subsampling_(subsampling)
  {
  }

  JpegEncoder::~JpegEncoder()
  {
  }

  void JpegEncoder::encodeImageTofile(const uint8_t *data, int width,
                                      int height, std::string filename)
  {
    rfb::Rect rect;
    std::ofstream of;
    rfb::JpegCompressor jc;

    rect.tl.x = 0;
    rect.tl.y = 0;
    rect.br.x = width;
    rect.br.y = height;

    jc.clear();
    jc.compress(data, width, rect, pf, quality_, subsampling_);

    of.open(filename);
    of.write((char*)jc.data(), jc.length());
    of.close();
  }

  Image *JpegEncoder::encodeImageToMemory(const uint8_t *data,
                                          int width, int height,
                                          int x_offset, int y_offset)
  {
    rfb::Rect rect;
    uint8_t *bufferCopy;
    rfb::JpegCompressor jc;
    Image* image;

    rect.tl.x = x_offset;
    rect.tl.y = y_offset;
    rect.br.x = x_offset + width;
    rect.br.y = y_offset + height;

    jc.clear();
    jc.compress(data, width, rect, pf, quality_, subsampling_);

    bufferCopy = new uint8_t[jc.length()];
    memcpy(bufferCopy, jc.data(), jc.length());

#ifdef _DEBUG
    measurePixelRate(width, height, 4);
#endif // _DEBUG

    image = new Image(width, height, bufferCopy, jc.length(),x_offset,
                      y_offset);
    return image;
  }
}

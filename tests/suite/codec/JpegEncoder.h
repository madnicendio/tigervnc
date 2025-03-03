#ifndef __SUITE_JPEG_ENCODER_H__
#define __SUITE_JPEG_ENCODER_H__

#include "ImageEncoder.h"
#include <rfb/JpegCompressor.h>
#include <rfb/JpegDecompressor.h>
#include <rfb/TightDecoder.h>

namespace suite {

  class JpegEncoder : public ImageEncoder
  {
  public:
    JpegEncoder(int quality = 100, int subsampling = 0);
    ~JpegEncoder();
    void encodeImageTofile(const uint8_t* data, int width, int height,
                           std::string filename) override;

    Image* encodeImageToMemory(const uint8_t* data, int width,
                               int height, int x_offset = 0,
                               int y_offset = 0) override;
  private:
    int quality_;
    int subsampling_;
  };
}

#endif // __SUITE_JPEG_ENCODER_H__

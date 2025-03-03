#ifndef __SUITE_JPEG_DECODER_H__
#define __SUITE_JPEG_DECODER_H__

#include "ImageDecoder.h"
#include <rfb/JpegCompressor.h>
#include <rfb/JpegDecompressor.h>
#include <rfb/TightDecoder.h>

namespace suite {

  class JpegDecoder : public ImageDecoder
  {
  public:
    JpegDecoder(int quality = 100, int subsampling = 0);
    ~JpegDecoder();

    Image *decodeImageFromFile(std::string filename) override;
    Image* decodeImageFromMemory(uint8_t* data, int width, int height,
                                 int size, int x_offest = 0,
                                 int y_offset = 0) override;
  private:
    int quality_;
    int subsampling_;
  };
}

#endif // __SUITE_JPEG_DECODER_H__

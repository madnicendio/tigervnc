#ifndef __SUITE_PPM_DECODER_H__
#define __SUITE_PPM_DECODER_H__

#include "ImageDecoder.h"

namespace suite {
  class PPMDecoder : public ImageDecoder
  {
  public:
    PPMDecoder();
    ~PPMDecoder();

    Image *decodeImageFromFile(std::string filename) override;
    void encodeImageTofile(Image *image, std::string filename);
    void encodeImageTofile(const uint8_t* data, int width, int height,
                           std::string filename) override;

    Image* decodeImageFromMemory(uint8_t* data, int width, int height,
                                 int size, int x_offset = 0,
                                 int y_offset = 0) override;

    Image* encodeImageToMemory(const uint8_t* data, int width,
                               int height, int offset_x = 0,
                               int offset_y = 0) override;
  };
}

#endif // __SUITE_PPM_DECODER_H__

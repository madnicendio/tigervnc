#ifndef __SUITE_PPM_ENCODER_H__
#define __SUITE_PPM_ENCODER_H__

#include "ImageEncoder.h"

namespace suite {
  class PPMEncoder : public ImageEncoder
  {
  public:
    PPMEncoder();
    ~PPMEncoder();

    void encodeImageTofile(Image *image, std::string filename);
    void encodeImageTofile(const uint8_t* data, int width, int height,
                           std::string filename) override;
    Image* encodeImageToMemory(const uint8_t* data, int width,
                               int height, int offset_x = 0,
                               int offset_y = 0) override;
  };
}

#endif // __SUITE_PPM_ENCODER_H__

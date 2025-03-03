#ifndef __SUITE_IMAGE_ENCODER_H__
#define __SUITE_IMAGE_ENCODER_H__

#include "Image.h"

namespace suite {

  class ImageEncoder
  {
  public:
    ImageEncoder(EncoderEnum type_);
    virtual ~ImageEncoder() {};

    virtual void encodeImageTofile(const uint8_t* data, int width, int height,
                                   std::string filename) = 0;
    virtual Image* encodeImageToMemory(const uint8_t* data, int width,
                                       int height, int offset_x = 0,
                                       int offset_y  = 0) = 0;
    const EncoderEnum type;
    const std::string name;
  protected:
#ifdef _DEBUG
    // Measures encoding performance.
    void measurePixelRate(int width, int height, int channels);
#endif // _DEBUG
  };

}

#endif // __SUITE_IMAGE_ENCODER_H__

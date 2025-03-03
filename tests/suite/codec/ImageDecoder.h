#ifndef __SUITE_IMAGE_DECODER_H__
#define __SUITE_IMAGE_DECODER_H__

#include "Image.h"

namespace suite {

  class ImageDecoder
  {
  public:
    ImageDecoder(EncoderEnum type_)
      : type(type_), name(encoderTypeToString(type_))
    {
    };
    virtual ~ImageDecoder() {};

    virtual Image *decodeImageFromFile(std::string filename) = 0;
    virtual Image* decodeImageFromMemory(uint8_t* data, int width, int height,
                                         int size, int x_offest = 0,
                                         int y_offset = 0) = 0;
    const EncoderEnum type;
    const std::string name;
  protected:
#ifdef _DEBUG
    // Measures encoding/decoding performance.
    void measurePixelRate(int width, int height, int channels);
#endif // _DEBUG
  };
}

#endif // __SUITE_IMAGE_DECODER_H__

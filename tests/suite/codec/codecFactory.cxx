#include "codecFactory.h"
#include "PPMEncoder.h"
#include "JpegEncoder.h"
#include "PPMDecoder.h"
#include "JpegDecoder.h"

namespace suite {
  ImageDecoder* constructDecoder(EncoderEnum decoder)
  {
    switch (decoder) {
    case PPM:
      return (ImageDecoder*) new PPMDecoder();
    case JPEG:
      return (ImageDecoder*) new JpegDecoder();
    default:
      throw std::logic_error("decoder not imlpemented");
    }
  }

  ImageDecoder* constructDecoder(std::string decoder)
  {
    return constructDecoder(encodersMap[decoder]);
  }

  ImageEncoder* constructEncoder(EncoderEnum encoder)
  {
    switch (encoder) {
    case PPM:
      return (ImageEncoder*) new PPMEncoder();
    case JPEG:
      return (ImageEncoder*) new JpegEncoder();
    default:
      throw std::logic_error("encoder not imlpemented");
    }
  }

  ImageEncoder* constructEncoder(std::string encoder)
  {
    return constructEncoder(encodersMap[encoder]);
  }
}

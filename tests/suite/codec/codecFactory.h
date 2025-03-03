#ifndef __SUITE_DECODER_FACTORY_H__
#define __SUITE_DECODER_FACTORY_H__

#include "ImageDecoder.h"
#include "ImageEncoder.h"

namespace suite {
  ImageDecoder* constructDecoder(EncoderEnum decoder);
  ImageDecoder* constructDecoder(std::string decoder);
  ImageEncoder* constructEncoder(EncoderEnum decoder);
  ImageEncoder* constructEncoder(std::string decoder);
}

#endif // __SUITE_DECODER_FACTORY_H__
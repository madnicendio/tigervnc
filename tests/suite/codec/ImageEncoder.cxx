#include "ImageEncoder.h"
#include <chrono>

static int pixelCount = 0;
static std::chrono::system_clock::time_point start =
  std::chrono::system_clock::now();

namespace suite {

  ImageEncoder::ImageEncoder(EncoderEnum type_)
    : type(type_), name(encoderTypeToString(type_))
  {
  }

  #ifdef _DEBUG
  void ImageEncoder::measurePixelRate(int width, int height, int channels)
  {
    // FIXME: This method has to be called explicitly by derived classes.
    // Is there a way to do this automatically?
    pixelCount += (width * height * channels);
    auto now = std::chrono::system_clock::now();
    std::chrono::duration<double> time = now - start;
    if (time.count() > 1) {
      printf("Encoding %.2f Mpx/s\n", pixelCount / (time.count() * 10e6));
      start = now;
      pixelCount = 0;
    }
  }
  #endif
}

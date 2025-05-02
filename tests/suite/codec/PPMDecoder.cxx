#include "PPMDecoder.h"

#include <fstream>
#include <sstream>
#include <string>

namespace suite {
  PPMDecoder::PPMDecoder() : ImageDecoder(PPM)
  {
  }


  PPMDecoder::~PPMDecoder()
  {
  }

  // Leaves ownership of Image
  Image *PPMDecoder::decodeImageFromFile(std::string filename)
  {
    unsigned int width;
    unsigned int height;
    std::string version;
    int maxVal;

    std::ifstream is(filename.c_str());
    is >> version; // ignored
    is >> width;
    is >> height;
    is >> maxVal; // ignored

    std::string line;
    std::string pixels;
    // Load entire image into one string, since PPM images don't
    // necessarily have columns of same length.
    while (std::getline(is, line)) {
      pixels += line + " ";
    }
    is.close();

    std::istringstream iss(pixels);
    int bufSize = width * height * 3;
    uint8_t* buf = new uint8_t[bufSize];
    Image *image = new Image(width, height, buf, bufSize);
    // While there is data still in stream,
    while ((iss >> std::ws).peek() != std::char_traits<char>::eof()) {
      Pixel pixel;
      iss >> pixel;
      *image += pixel;
    }

#ifdef _DEBUG
    measurePixelRate(width, height, 3);
#endif // _DEBUG

    return image;
  }

  Image* PPMDecoder::decodeImageFromMemory(uint8_t*, int, int, int, int, int)
  {
    throw std::logic_error("function not implemented");
  }
}

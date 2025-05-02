#include "PPMEncoder.h"

#include <fstream>
#include <sstream>
#include <string>

namespace suite {
  PPMEncoder::PPMEncoder() : ImageEncoder(PPM)
  {
  }


  PPMEncoder::~PPMEncoder()
  {
  }


  void PPMEncoder::encodeImageTofile(Image *image, std::string filename)
  {
    encodeImageTofile(image->getBuffer(), image->width_, image->height_,
                      filename);
  }

  void PPMEncoder::encodeImageTofile(const uint8_t* data, int width,
                                     int height, std::string filename)
  {
    std::ofstream file;
    file.open(filename.c_str());

    if (!file.is_open()) {
      throw std::ofstream::failure("error creating file \"" + filename +
                                   "\"");
    }

    // Header/metadata
    const int MAXVAL = 255;
    file << "P3" << "\n"
         << width << " " << height << "\n"
         << MAXVAL << "\n";

    std::ostringstream oss;
    for (int i = 0; i < width * height * 4; i+=4) {
      uint8_t r = data[i];
      uint8_t g = data[i+1];
      uint8_t b = data[i+2];
      oss << (int)r << " " << (int)g << " " << (int)b << " ";
    }
    file << oss.str();
    file.close();

#ifdef _DEBUG
    measurePixelRate(width, height, 3);
#endif // _DEBUG
  }

  Image* PPMEncoder::encodeImageToMemory(const uint8_t *data, int width,
                                        int height, int offset_x,
                                        int offset_y)
  {
    const std::string MAXVAL = "255";
    std::string header = "P3\n" + std::to_string(width) + " "
      + std::to_string(height) + "\n" + MAXVAL + "\n";
    uint8_t* buf = new uint8_t[(width * height * 3) + header.length()];

    for (int i = 0; i < width * height * 4; i+=4) {
      buf[i - (i/4)] = data[i];
      buf[i + 1 - (i/4)] = data[i + 1];
      buf[i + 2 - (i/4)] = data[i+ 2];
    }

#ifdef _DEBUG
    measurePixelRate(width, height, 3);
#endif // _DEBUG

    Image* image = new Image(width, height, buf,
                             width * height * 3,
                             offset_x, offset_y);
    return image;
  }
}

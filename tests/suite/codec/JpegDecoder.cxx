#include "JpegDecoder.h"
#include "ImageDecoder.h"
#include <rfb/JpegCompressor.h>
#include <rfb/JpegDecompressor.h>
#include <rfb/ServerParams.h>
#include <rfb/TightDecoder.h>

namespace suite {
  static rfb::PixelFormat pf(32, 24, false, true,
                      255, 255, 255, 0, 8, 16);

  JpegDecoder::JpegDecoder(int quality, int subsampling)
    : ImageDecoder(JPEG), quality_(quality), subsampling_(subsampling)
  {
  }

  JpegDecoder::~JpegDecoder()
  {
  }


  Image* JpegDecoder::decodeImageFromFile(std::string)
  {
    throw std::logic_error("method not implemented");
  }

  Image* JpegDecoder::decodeImageFromMemory(uint8_t *data, int width,
                                            int height, int size,
                                            int x_offset, int y_offset)
  {
    rfb::Rect rect;
    uint8_t* buffer;
    Image* image;
    rfb::JpegDecompressor jd;

    rect.tl.x = x_offset;
    rect.tl.y = y_offset;
    rect.br.x = x_offset + width;
    rect.br.y = y_offset + height;

    buffer = new uint8_t[width * height * 4];

    jd.decompress(data, size, buffer, width, rect, pf);

#ifdef _DEBUG
    measurePixelRate(width, height, 4);
#endif // _DEBUG

    image = new Image(width, height, buffer, size, x_offset, y_offset);
    return image;
  }
}

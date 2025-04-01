#include "FrameInStream.h"
#include "../codec/codecFactory.h"
#include <ios>
#include <iostream>
#include <assert.h>

namespace suite {

  FrameInStream::FrameInStream()
    : decoder_(nullptr), headerParsed(false)
  {
  }

  FrameInStream::~FrameInStream()
  {
    delete decoder_;
  }

  Image* FrameInStream::readImage(std::istream& is)
  {
    assert(headerParsed);

    RecorderStats s; // ignored
    return readImage(is, s, -1);
  }

  Image* FrameInStream::readImage(std::istream& is,
                                  RecorderStats& recorderStats,
                                  int imgNr)
  {
    assert(headerParsed);

    int size, width, height, x_offset, y_offset;
    double frameTime; // alltid 0 ??
    ImageUpdateStats stats;

    if (is.peek() == EOF)
      throw std::ios_base::failure("error reading from stream");

    is >> size >> width >> height >> x_offset >> y_offset >> frameTime
       >> stats.lostDataArea >> stats.overDimensionedArea
       >> stats.encodingTime >> stats.margin;
    is.ignore();
/* Den betraktas inte som för liten här? Någonstans på vägen blir den för liten */
// Debugging output
  if (size == 0) {
    std::cerr << "FrameInStream::readImage: "
              << "size=" << size
              << ", width=" << width
              << ", height=" << height
              << ", x_offset=" << x_offset
              << ", y_offset=" << y_offset
              << ", frameTime=" << frameTime
              << ", lostDataArea=" << stats.lostDataArea
              << ", overDimensionedArea=" << stats.overDimensionedArea
              << ", encodingTime=" << stats.encodingTime
              << ", margin=" << stats.margin
              << std::endl;
  }

    recorderStats.stats_.push_back(stats);

    uint8_t* data = new uint8_t[size];
    is.read((char*)data, size);

    if (is.gcount() != size) {
        std::cerr << "Warning: Expected to read " << size
                  << " bytes, but only read " << is.gcount() << " bytes!"
                  << std::endl;
    }

    Image* image = decoder_->decodeImageFromMemory(data, width, height,
                                                  size, x_offset,
                                                  y_offset);
    delete [] data;

    // after decoding

if (!image) {
  // if decoding fails
      std::cerr << "Error: Failed to decode image!" << std::endl;
      throw std::runtime_error("Failed to decode image");
    }

    // Set the frame time and stats
    image->frameTime_ = frameTime;
    image->stats_ = stats;

    // Debugging output after decoding

    // std::cerr << "Decoded image " << imgNr << ": " << "width=" << width
    // << ", height=" << height
    // << ", encodingTime=" << stats.encodingTime
    // << std::endl;

    return image;
  }

  HeaderData FrameInStream::parseHeader(std::istream& is)
  {
    assert(!headerParsed);

    std::string decoder;
    int width;
    int height;
    double interval;
    is >> decoder >> width >> height >> interval;

    HeaderData header {
      .width = width,
      .height = height,
      .interval = interval,
      .decoder = decoder,
    };

    decoder_ = constructDecoder(decoder);
    headerParsed = true;
    return header;
  }
}

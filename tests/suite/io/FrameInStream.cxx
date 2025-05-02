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
    return readImage(is, s);
  }

  Image* FrameInStream::readImage(std::istream& is,
                                  RecorderStats& recorderStats)
  {
    assert(headerParsed);

    int size, width, height, x_offset, y_offset;
    double frameTime;
    ImageUpdateStats stats;

    if (is.peek() == EOF)
      throw std::ios_base::failure("error reading from stream");

    is >> size >> width >> height >> x_offset >> y_offset >> frameTime
       >> stats.lostDataArea >> stats.overDimensionedArea
       >> stats.encodingTime >> stats.margin;
    is.ignore();

    recorderStats.stats_.push_back(stats);

    uint8_t* data = new uint8_t[size];
    is.read((char*)data, size);

    Image* image = decoder_->decodeImageFromMemory(data, width, height,
                                                  size, x_offset,
                                                  y_offset);
    delete [] data;
    image->frameTime_ = frameTime;
    image->stats_ = stats;
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

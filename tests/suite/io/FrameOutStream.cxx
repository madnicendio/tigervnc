#include "FrameOutStream.h"
#include <assert.h>
#include <chrono>

namespace suite {

  FrameOutStream::FrameOutStream(std::string filename, ImageEncoder* encoder)
    : headerWritten(false), encoder_(encoder->name)
  {
    file.open(filename.c_str());
    if (!file.is_open())
      throw std::ofstream::failure("error creating file");
  }

  FrameOutStream::~FrameOutStream()
  {
  }

  void FrameOutStream::addUpdate(ImageUpdate* update)
  {
    assert(headerWritten);

    Image* image = update->image;
    const ImageUpdateStats stats = update->stats;

    // Keep track of time between frames
    auto now = std::chrono::steady_clock::now();
    // FIXME: timeBudget is unused/unnecessary?
    auto timeBudget = std::chrono::duration<double, std::milli>
                                           (now - lastFrameTime);
    lastFrameTime = now;

    lock.lock();
    file << image->size_ << " " << image->width_ << " " << image->height_ << " "
         << image->x_offset_ << " " << image->y_offset_ << " "
         << timeBudget.count() << " " << stats.lostDataArea
         << " " << stats.overDimensionedArea << " "  << stats.encodingTime
         << " " << stats.margin << "\n";
    file.write((char*)image->getBuffer(), image->size_);
    file << "\n";
    lock.unlock();
  }

  void FrameOutStream::addUpdate(uint8_t* data, int width, int height,
                                int x_offset, int y_offset, int size)
  {
    Image* image = new Image(width, height, data, size, x_offset, y_offset);
    ImageUpdate* update = new ImageUpdate(image);
    addUpdate(update);
  }

  void FrameOutStream::writeHeader(int width, int height, double interval)
  {
    assert(!headerWritten);

    file << encoder_ << " " << width << " " << height << " "
         << interval << "\n";
    headerWritten = true;
    lastFrameTime = std::chrono::steady_clock::now();
  }
}

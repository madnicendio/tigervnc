#include "ImageUpdate.h"

namespace suite {
  ImageUpdate::ImageUpdate(Image* image_, ImageUpdateStats stats_)
    : image(image_), stats(stats_)
  {
  }

  ImageUpdate::~ImageUpdate()
  {
    delete image;
  }
}

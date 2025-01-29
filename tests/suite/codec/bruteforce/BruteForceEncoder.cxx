#include "BruteForceEncoder.h"
#include <rfb/Encoder.h>
#include <rfb/PixelBuffer.h>
#include <rfb/RawEncoder.h>
#include <rfb/SConnection.h>
#include <vector>

namespace suite {
  BruteForceEncoder::BruteForceEncoder(rfb::SConnection* sconn,
                                       std::vector<TimedEncoder*> encoders)
    : TimedEncoder(enumEncoder::encoderHextile,
                   new rfb::RawEncoder(sconn), sconn),
      encoders_(encoders)
  {
  }

  BruteForceEncoder::~BruteForceEncoder()
  {
  }

  void BruteForceEncoder::writeRect(const rfb::PixelBuffer* pb,
                                         const rfb::Palette& palette)
  {
    BestEncoder best{};
    best.inputSize = pb->area() * 4;

    for (const auto& encoder : encoders_) {
      startWriteRectTimer();
      encoder->writeRect(pb, palette);
      stopWriteRectTimer(pb);
      EncoderStats* stats = encoder->stats();

      double time = stats->writeRectEncodetime
                  + stats->writeSolidRectEncodetime;
      uint outputSize = stats->outputSizeRects + stats->outputSizeSolidRects;

      bestEncoder(time, outputSize, encoder->encoderClass, &best);

      stats->outputSizeRects = 0;
      stats->outputSizeSolidRects = 0;
      stats->writeRectEncodetime = 0;
      stats->writeSolidRectEncodetime = 0;
    }

    bestEncoders_.push_back(best);
    bestCounter_[best.encoder_]++;
  }

    void BruteForceEncoder::writeSolidRect(int width, int height,
                                                const rfb::PixelFormat& pf,
                                                const uint8_t* colour)
  {
    int inputSize = width * height  *4;
    BestEncoder best{};
    best.inputSize = inputSize;

    for (const auto& encoder : encoders_) {
      startWriteSolidRectTimer();
      encoder->writeSolidRect(width, height, pf, colour);
      stopWriteSolidRectTimer(width, height);
      EncoderStats* stats = encoder->stats();

      uint outputSize = stats->outputSizeRects +  stats->outputSizeSolidRects;
      double time = stats->writeRectEncodetime
                  + stats->writeSolidRectEncodetime;
      bestEncoder(time, outputSize, encoder->encoderClass, &best);

      stats->outputSizeRects = 0;
      stats->outputSizeSolidRects = 0;
      stats->writeRectEncodetime = 0;
      stats->writeSolidRectEncodetime = 0;
    }
    bestEncoders_.push_back(best);
    bestCounter_[best.encoder_]++;
  }

  void BruteForceEncoder::bestEncoder(double time,
                                           uint outputSize,
                                           enumEncoder::EncoderClass encoder,
                                           BestEncoder* best)
  {
    bool firstComparison = best->outputSize == 0;
    bool bestCompression = best->outputSize > outputSize;

    if (firstComparison || bestCompression) {
      best->outputSize = outputSize;
      best->encoder_ = encoder;
      best->encodingTime = time;
    }
  }
}

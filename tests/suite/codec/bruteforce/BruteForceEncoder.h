#ifndef __SUITE_TIMED_BRUTEFORCE_H__
#define __SUITE_TIMED_BRUTEFORCE_H__

#include "../timed/TimedEncoder.h"

#include <rfb/RawEncoder.h>
#include <rfb/SConnection.h>

namespace suite {

  struct BestEncoder {
    enumEncoder::EncoderClass encoder_;
    int inputSize;
    uint outputSize;
    double encodingTime;
  };

  class BruteForceEncoder : public TimedEncoder
  {
  public:
    BruteForceEncoder(rfb::SConnection* sconn,
                           std::vector<TimedEncoder*> encoders);
    ~BruteForceEncoder();

    virtual void writeRect(const rfb::PixelBuffer* pb,
                           const rfb::Palette& palette) override;
    virtual void writeSolidRect(int width, int height,
                                const rfb::PixelFormat& pf,
                                const uint8_t* colour) override;
    void bestEncoder(double time, uint outputSize,
                     enumEncoder::EncoderClass encoder,
                     BestEncoder* best);

    std::vector<BestEncoder> bestEncoders() { return bestEncoders_; }
    std::map<enumEncoder::EncoderClass, int> bestCounter() { return bestCounter_; }
  private:
    std::vector<TimedEncoder*> encoders_;
    std::vector<BestEncoder> bestEncoders_;
    std::map<enumEncoder::EncoderClass, int> bestCounter_;
  };
}

#endif // __SUITE_TIMED_BRUTEFORCE_H__

#include "BruteForce.h"
#include "BruteForceEncoder.h"
#include "../timed/timedEncoderFactory.h"
#include "../../Server.h"
#include "../../io/FrameInStream.h"

#include <rfb/TightEncoder.h>
#include <rfb/TightJPEGEncoder.h>
#include <rfb/ZRLEEncoder.h>
#include <rfb/encodings.h>

#include <fstream>
#include <iostream>

namespace suite {

  BruteForce::BruteForce(std::string filename)
    : Benchmark(filename), server_(new Server(10,10)),
      manager_(new Manager(server_, false))
  {
    setDefaultBruteForceEncoder();
  }

  BruteForce::BruteForce(std::string filename, Server* server,
                         Manager* manager)
    : Benchmark(filename), server_(server), manager_(manager)
  {
    setDefaultBruteForceEncoder();
  }

  BruteForce::~BruteForce()
  {
  }

  void BruteForce::run()
  {
    FrameInStream is =  FrameInStream();
    std::ifstream file(filename_);
    is.parseHeader(file); // FIXME: Don't parse header twice

    std::cout << "Starting brute-force benchmark using \""
              << filename_ << "\"\n";

    RecorderStats recorderStats;  // unused
    while (file.peek() != EOF) {
      const Image* image = is.readImage(file, recorderStats);
      server_->loadImage(image, image->x_offset_, image->y_offset_);
      server_->out->clear();
      delete image;
    }

    std::vector<BestEncoder> bestEncoders =  bruteForceEncoder->bestEncoders();

    unsigned long long totalInput = 0;
    unsigned long long totalOutput = 0;
    double totalTime = 0;
    for (const auto& encoder : bestEncoders) {
      totalInput += encoder.inputSize;
      totalOutput += encoder.outputSize;
      totalTime += encoder.encodingTime;
    }

    double ratio = (double)totalInput / totalOutput;
    std::cout << "Time (ms): " << totalTime << "\n"
              << "Ratio: " << ratio << "\n";

    for (const auto& b : bruteForceEncoder->bestCounter()) {
      std::string encoder = encoderClasstoString(b.first);
      int count = b.second;
      std::cout << encoder << " " << count << "\n";
    }
  }

  void BruteForce::setBruteForceEncoder(BruteForceEncoder *encoder)
  {
    bruteForceEncoder = encoder;
    manager_->setActiveEncoder(bruteForceEncoder);
  }

  void BruteForce::setDefaultBruteForceEncoder()
  {
    std::vector<TimedEncoder*> encoders = {
      constructTimedEncoder(EncoderClass::encoderRaw, server_),
      constructTimedEncoder(EncoderClass::encoderRRE, server_),
      constructTimedEncoder(EncoderClass::encoderHextile, server_),
      constructTimedEncoder(EncoderClass::encoderTight, server_),
      constructTimedEncoder(EncoderClass::encoderTightJPEG, server_),
      constructTimedEncoder(EncoderClass::encoderZRLE, server_),
    };

    // FIXME: don't hardcode values
    encoders[3]->setCompressLevel(2); // TightEncoder
    encoders[4]->setQualityLevel(8);  // TightJPEGEncoder
    encoders[5]->setCompressLevel(6); // ZRLEEncoder

    bruteForceEncoder = new BruteForceEncoder(server_, encoders);
    manager_->setActiveEncoder(bruteForceEncoder);
  }
}

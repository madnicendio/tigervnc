#include "Manager.h"
#include "codec/timed/TimedEncoder.h"
#include "codec/timed/timedEncoderFactory.h"
#include "stats/ManagerStats.h"

#include <chrono>

#include <rfb/EncodeManager.h>
#include <rfb/Encoder.h>
#include <rfb/RawEncoder.h>
#include <rfb/TightJPEGEncoder.h>
#include <rfb/SConnection.h>
#include <rfb/encodings.h>
#include <rfb/UpdateTracker.h>

namespace suite {

  Manager::Manager(rfb::SConnection *conn_, bool debug) : EncodeManager(conn_),
                                                          SINGLE_ENCODER(false),
                                                          currentWriteUpdate(0)
  {
    // If this is a Manager meant for debugging, leave the
    // rfb::EncodeManager as is and keep one TimedEncoder to record
    // stats.
    fprintf(stderr, "Manager::Manager (debug): %d\n", debug);
    // if (debug) {
    //   stats_.encoders.push_back(constructTimedEncoder(encoderTight, conn_));
    //   return;
    // }


    for (int i = 0; i < rfb::encoderClassMax; i++) {
      rfb::EncoderClass klass = static_cast<rfb::EncoderClass>(i);
      fprintf(stderr, "Creating TimedEncoder for encoderClass = %d\n", klass);
      TimedEncoder *e = new TimedEncoder(klass, encoders[klass], conn_);
      encoders[i] = e;
      stats_.encoders.push_back(e);
    }
  }

  Manager::Manager(rfb::SConnection* conn_, EncoderSettings& settings)
                                         : EncodeManager(conn_),
                                           SINGLE_ENCODER(false),
                                           currentWriteUpdate(0)
  {
    fprintf(stderr, "Manager::Manager2\n");
    // For debugging

    for (int i = 0; i < rfb::encoderClassMax; i++) {
      // Note that we only want to create six encoders. CopyRect is merged with Tight or Raw
      rfb::EncoderClass klass = static_cast<rfb::EncoderClass>(i);
      if (!encoders[klass]) {
        fprintf(stderr, "Skipping encoderClass %d because it's nullptr\n", klass);
        continue;
      }
      fprintf(stderr, "\nEncoderClass: %d\n", klass);
      fprintf(stderr, "Actual encoding class: %s\n", encoders[klass]->getName().c_str());
      TimedEncoder* e = new TimedEncoder(klass, encoders[klass], conn_);
      fprintf(stderr, "Created TimedEncoder with %s \n", e->getName().c_str());
      if (!e) {
        fprintf(stderr, "nullptr\n");
      }
      encoders[klass] = e;
      stats_.encoders.push_back(e);
    }
  }

  Manager::~Manager()
  {
    // Ugly hack, EncodeManager's destructor will call delete on all
    // elements in encoders. Since we sometimes use the same pointer for
    // all elements in encoders, we need to avoid a double free.
    if (SINGLE_ENCODER) {
      for (uint i = 1; i < ENCODERS_COUNT; i++) {
        encoders[i] = nullptr;
      }
    }
  }

  void Manager::writeUpdate(const rfb::UpdateInfo& ui,
                            const rfb::PixelBuffer* pb,
                            const rfb::RenderedCursor* renderedCursor)
  {
    // Här ska vi kopiera in EncodeManagern
    EncodeManager::writeUpdate(ui, pb, renderedCursor);
  }


  unsigned int getArea(rfb::Region changed)
  {
    std::vector<rfb::Rect> rects;
    changed.get_rects(&rects);
    unsigned int totalArea = 0;
    for (const rfb::Rect r : rects) {
      totalArea += r.area();
    }
    return totalArea;
  }

  void Manager::writeUpdate(const rfb::UpdateInfo& ui,
                            const rfb::PixelBuffer* pb,
                            const rfb::RenderedCursor* renderedCursor,
                            uint frameTime)
  {
    updateCurrentWriteUpdate();
    auto start = std::chrono::system_clock::now();
    writeUpdate(ui, pb, renderedCursor);
    auto end = std::chrono::system_clock::now();
    auto time = std::chrono::duration<double, std::milli>(end-start);

    // We keep track of the time it takes to encode an entire frame,
    // and how much time there is left until the next frame occurs
    // (as when it was recorded).
    WriteUpdate update {
      .timeRequired = frameTime,
      .timeSpent = time.count(),
      .size = getArea(ui.changed),
    };
    stats_.addWriteUpdate(update);

    // if (pb) {
    //     fprintf(stderr, "Decoded write update %d: size=%d, encoding time=%.4f ms\n",
    //             currentWriteUpdate, update.size, update.timeSpent);
    // }
  }


  void Manager::updateCurrentWriteUpdate()
  {
    for (TimedEncoder* encoder : stats_.encoders) {
      encoder->currentWriteUpdate = currentWriteUpdate;
    }
    // Increment the currentWriteUpdate index so we can keep track
    // of which writeRect() belongs to which writeUpdate.
    currentWriteUpdate++;
  }

  void Manager::setActiveEncoder(TimedEncoder* encoder)
  // Används bara i Bruteforce
  {
    encoders[rfb::encoderRaw] = encoder;
    encoders[rfb::encoderRRE] = encoder;
    encoders[rfb::encoderHextile] = encoder;
    encoders[rfb::encoderTight] = encoder;
    encoders[rfb::encoderTightJPEG] = encoder;
    encoders[rfb::encoderZRLE] = encoder;
  }

  std::vector<rfb::Encoder*> Manager::getEncoders()
  {
    return encoders;
  }
}


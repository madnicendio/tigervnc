#include "timedEncoderFactory.h"
#include "TimedEncoder.h"
#include "rfb/Encoder.h"
#include "rfb/HextileEncoder.h"
#include "rfb/RREEncoder.h"
#include "rfb/RawEncoder.h"
#include "rfb/TightEncoder.h"
#include "rfb/TightJPEGEncoder.h"
#include "rfb/ZRLEEncoder.h"
#include <rfb/SConnection.h>
#include <stdexcept>

namespace suite {

  TimedEncoder* constructTimedEncoder(rfb::EncoderClass encoder,
                                      rfb::SConnection* sconn)
  {
    fprintf(stderr, "Construct encoder = %d\n", encoder);
    rfb::Encoder* e;
    switch(encoder) {
    case rfb::encoderRaw:
      e = new rfb::RawEncoder(sconn);
      break;
    case rfb::encoderRRE:
      e = new rfb::RREEncoder(sconn);
      break;
    case rfb::encoderHextile:
      e = new rfb::HextileEncoder(sconn);
      break;
    case rfb::encoderTight:
      e = new rfb::TightEncoder(sconn);
      break;
    case rfb::encoderTightJPEG:
      e = new rfb::TightJPEGEncoder(sconn);
      break;
    case rfb::encoderZRLE:
      e = new rfb::ZRLEEncoder(sconn);
      break;
    default:
      throw std::logic_error("decoder not implemented");
    }
    return new TimedEncoder(encoder, e, sconn);
  }
}

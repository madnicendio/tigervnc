#ifndef __SUITE_TIMEDENCODER_H__
#define __SUITE_TIMEDENCODER_H__

#include "../../stats/EncoderStats.h"
#include <rdr/InStream.h>
#include <rdr/MemOutStream.h>
#include <rdr/OutStream.h>
#include <rfb/Encoder.h>
#include <rfb/PixelBuffer.h>
#include <rfb/SConnection.h>
#include <rfb/EncodeManager.h>
#include <chrono>
#include <stdexcept>
#include <string>
namespace suite {

  // Copied from EncodeManager.cxx
  namespace enumEncoder {
    // enum EncoderClass {
    //   encoderRaw,
    //   encoderRRE,
    //   encoderHextile,
    //   encoderTight,
    //   encoderTightJPEG,
    //   encoderZRLE,
    //   encoderClassMax,
    // };

    enum PseudoEncodingLevel {
      NONE = -1,
      ZERO,
      ONE,
      TWO,
      THREE,
      FOUR,
      FIVE,
      SIX,
      SEVEN,
      EIGHT,
      NINE
    };

    struct EncoderSettings {
      rfb::EncoderClass encoderClass;
      int *rfbEncoding;
      size_t encodingSize;
      PseudoEncodingLevel quality;
      PseudoEncodingLevel compression;
      std::string name;


    };

    inline std::string encoderClasstoString(rfb::EncoderClass encoderClass)
    {
      fprintf(stderr, "EncoderClass: %d\n", (int)encoderClass);
      switch (encoderClass) {
        case rfb::encoderRaw:
          return "Raw";
        case rfb::encoderRRE:
          return "RRE";
        case rfb::encoderHextile:
          return "Hextile";
        case rfb::encoderTight:
          return "Tight";
        case rfb::encoderTightJPEG:
          return "TightJPEG";
        case rfb::encoderZRLE:
          return "ZRLE";
        default:
          throw std::logic_error("EncoderClass not implemented");
      }
    }
  // Copied from EncodeManager.cxx
  inline const char *encoderClassName(rfb::EncoderClass klass)
  {
    switch (klass) {
    case rfb::encoderRaw:
      return "Raw";
    case rfb::encoderRRE:
      return "RRE";
    case rfb::encoderHextile:
      return "Hextile";
    case rfb::encoderTight:
      return "Tight";
    case rfb::encoderTightJPEG:
      return "Tight (JPEG)";
    case rfb::encoderZRLE:
      return "ZRLE";
    case rfb::encoderClassMax:
      break;
    }

    return "Unknown Encoder Class";
  }

}


  // Forward declarations
  struct WriteUpdate;

  class TimedEncoder : public rfb::Encoder
  {
  public:
    TimedEncoder(rfb::EncoderClass encoderclass,
                 rfb::Encoder* encoder, rfb::SConnection* sconn);
    virtual ~TimedEncoder();

    void startWriteRectTimer();
    void stopWriteRectTimer(const rfb::PixelBuffer* pb);
    void startWriteSolidRectTimer();
    void stopWriteSolidRectTimer(int width, int height);
    const rfb::EncoderClass encoderClass;
    EncoderStats* stats() { return stats_; };

    bool isSupported() override { return true; };
    void writeRect(const rfb::PixelBuffer* pb,
                   const rfb::Palette& palette) override;

    void writeSolidRect(int width, int height,
                        const rfb::PixelFormat& pf,
                        const uint8_t* colour) override;

    void setCompressLevel(int level) override;
    void setQualityLevel(int level) override;
    void setFineQualityLevel(int quality, int subsampling) override;
    int getCompressLevel() override;
    int getQualityLevel() override;
    std::string getName() const override {
      std::string name = "TimedEncoder::" + std::string(encoder_->getName());
      return name;
 }
    unsigned long long medianRectSize() const;

    std::vector<unsigned long long> rectSizes_;

    uint currentWriteUpdate;
  private:
    rdr::MemOutStream *encoderOutstream;
    rdr::OutStream* os;
    rdr::InStream* is;
    rfb::SConnection* conn_;
    EncoderStats* stats_;
    std::chrono::system_clock::time_point writeRectStart;
    std::chrono::system_clock::time_point writeSolidRectStart;
    rfb::Encoder *encoder_;
  };
}

#endif // __SUITE_TIMEDENCODER_H__

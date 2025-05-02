#include "TimedEncoder.h"
#include "rfb/Encoder.h"
#include <rdr/MemOutStream.h>
#include <rfb/PixelBuffer.h>
#include <rfb/SConnection.h>
#include <chrono>

namespace suite {

  /* This class is a wrapper around rfb::Encoder and provides
   * timing information for each encoder. It does this by using
   * its own OutStream and injecting it into the underlying
   * SConnection just before writeRect() and writeSolidRect().
   */

  TimedEncoder::TimedEncoder(enumEncoder::EncoderClass encoderclass,
                             rfb::Encoder* encoder,
                             rfb::SConnection* sconn)
    : rfb::Encoder(sconn, encoder->encoding, encoder->flags,
                   encoder->maxPaletteSize, encoder->losslessQuality),
      encoderClass(encoderclass),
      currentWriteUpdate(0),
      encoderOutstream(new rdr::MemOutStream(10 << 20)),
      conn_(sconn), encoder_(encoder)
  {
    stats_ = new EncoderStats {
      .writeRectEncodetime = 0,
      .writeSolidRectEncodetime = 0,
      .inputSizeRects = 0,
      .outputSizeRects = 0,
      .inputSizeSolidRects = 0,
      .outputSizeSolidRects = 0,
      .nRects = 0,
      .nSolidRects = 0,
      .name = encoderClassName(encoderclass),
      .writeUpdates = std::map<int,WriteRects>{}
    };
  }

  TimedEncoder::~TimedEncoder()
  {
    delete encoderOutstream;
    delete stats_;
    delete encoder_;
  }

void TimedEncoder::writeRect(const rfb::PixelBuffer* pb,
                             const rfb::Palette& palette)
{
  startWriteRectTimer();
  encoder_->writeRect(pb, palette);
  stopWriteRectTimer(pb);
}

void TimedEncoder::writeSolidRect(int width, int height,
                                  const rfb::PixelFormat& pf,
                                  const uint8_t* colour)
{
  startWriteSolidRectTimer();
  encoder_->writeSolidRect(width, height, pf, colour);
  stopWriteSolidRectTimer(width, height);
}

  void TimedEncoder::startWriteRectTimer()
  {
    os = conn_->getOutStream();
    is = conn_->getInStream();
    // Inject our own MemOutStream just before encoding occurs
    conn_->setStreams(is, encoderOutstream);
    writeRectStart = std::chrono::system_clock::now();
  }

  void TimedEncoder::stopWriteRectTimer(const rfb::PixelBuffer* pb)
  {
    std::chrono::time_point<std::chrono::system_clock> now =
      std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> time =
      std::chrono::duration<double, std::milli>(now - writeRectStart);

    // Keep track of rects belonging to the same writeUpdate().
    WriteRect stats{
      .timeSpent = time.count(),
      .pixelCount =
        static_cast<unsigned long long>(pb->width() * pb->height()),
    };
    stats_->writeUpdates[currentWriteUpdate].writeRects
      .push_back(stats);

    stats_->writeRectEncodetime += time.count();
    stats_->inputSizeRects += pb->width() * pb->height() * BPP;
    stats_->outputSizeRects += encoderOutstream->length();
    stats_->nRects++;

    // Return the original OutStream after encoding
    // & reset our MemOutStream
    conn_->setStreams(is, os);
    encoderOutstream->clear();
  }

  void TimedEncoder::startWriteSolidRectTimer()
  {
    os = conn_->getOutStream();
    is = conn_->getInStream();
    // Inject our own MemOutStream just before encoding occurs
    conn_->setStreams(is, encoderOutstream);
    writeSolidRectStart = std::chrono::system_clock::now();
  }

  void TimedEncoder::stopWriteSolidRectTimer(int width, int height)
  {
    std::chrono::time_point<std::chrono::system_clock> now =
      std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> time =
      std::chrono::duration<double, std::milli>(now -
                                                writeSolidRectStart);

    // Keep track of rects belonging to the same writeUpdate().
    WriteRect stats {
      .timeSpent = time.count(),
      .pixelCount = static_cast<unsigned long long>(width * height),
    };
    stats_->writeUpdates[currentWriteUpdate].writeSolidRects
      .push_back(stats);

    stats_->writeSolidRectEncodetime += time.count();
    stats_->inputSizeSolidRects += width * height * 4;
    stats_->outputSizeSolidRects += encoderOutstream->length();
    stats_->nSolidRects++;

    // Return the original OutStream after encoding
    // & reset our MemOutStream
    conn_->setStreams(is, os);
    encoderOutstream->clear();
  }

  void TimedEncoder::setCompressLevel(int level)
  {
    encoder_->setCompressLevel(level);
  }

  void TimedEncoder::setQualityLevel(int level)
  {
    encoder_->setQualityLevel(level);
  }

  void TimedEncoder::setFineQualityLevel(int quality, int subsampling)
  {
    encoder_->setFineQualityLevel(quality, subsampling);
  }

  int TimedEncoder::getCompressLevel()
  {
    return encoder_->getCompressLevel();
  }

  int TimedEncoder::getQualityLevel()
  {
    return encoder_->getQualityLevel();
  }
}


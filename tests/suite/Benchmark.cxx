#include "Benchmark.h"
#include "io/FrameInStream.h"
#include "stats/RecorderStats.h"

#include <exception>
#include <rfb/EncodeManager.h>
#include <rfb/Exception.h>
#include <rfb/encodings.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>

using namespace suite;

Benchmark::Benchmark(std::string filename, const int32_t* encodings,
                     const size_t len)
  : debugServer_(nullptr), filename_(filename), encodings_(encodings),
    encodingsLength_(len)
{
  struct stat buf;
  FrameInStream is;
  std::ifstream file;
  HeaderData header;

  if (stat (filename.c_str(), &buf) != 0)
    throw std::ios_base::failure("file does not exist");

  is =  FrameInStream();
  file = std::ifstream(filename);

  header = is.parseHeader(file);

  width_ = header.width;
  height_ = header.height;
#ifdef _DEBUG
  try {
    debugServer_ = new Server(header.width, header.height, fbPF, true);
  } catch (std::exception &e) {
    throw e;
  }
#endif // _DEBUG
}

Benchmark::~Benchmark()
{
#ifdef _DEBUG
  delete debugServer_;
#endif // _DEBUG
}

void Benchmark::runBenchmark()
{
  EncoderSettings* settings = new EncoderSettings;

  int allEncodings[] = {
    0,  // rfb::encodingRaw
    1,  // rfb::encodingCopyRect
    2,  // rfb::encodingRRE
    4,  // rfb::encodingCoRRE
    5,  // rfb::encodingHextile
    7,  // rfb::encodingTight
    16  // rfb::encodingZRLE
  };
  settings->rfbEncoding = allEncodings;
  settings->encodingSize = sizeof(allEncodings) / sizeof(allEncodings[0]);
  // (start, stop, destination)

  runBenchmark(settings);

  delete settings;
}

void Benchmark::runBenchmark(EncoderSettings* settings)
{
  FrameInStream is;
  std::ifstream file;
  // std::map<EncoderClass, Server*> servers;

  is = FrameInStream();
  file = std::ifstream(filename_);
  is.parseHeader(file); // FIXME: Don't parse header twice

  Server* server = new Server(width(), height(), *settings);

  // Set Tight as preferred encoding (just as with Auto mode)
  server->setEncodings(settings->encodingSize, settings->rfbEncoding);

  std::cout << "Starting benchmark using \"" << filename_ << "\"\n";
  RecorderStats recorderStats;
  int imageNr = 0;
  while (file.peek() != EOF) {
    imageNr++;
    // Extract the next image from the file
    // This function uses recorderStats to record information
    const Image* image = is.readImage(file, recorderStats, imageNr);

    // Load image once (no need to loop through multiple servers)
    server->loadImage(image, image->x_offset_, image->y_offset_);
    // Immediately after loading, the server's output buffer is cleared
    // to be ready for the next image
    server->out->clear();


#ifdef _DEBUG
    debugServer_->loadImage(image, image->x_offset_, image->y_offset_);
#endif // _DEBUG
    delete image;
  }
  std::cout << "Benchmarking complete!\n";

  // Print statistics for the single server

  // Output encoderstats:
  std::vector<rfb::Encoder*> encoders_ = server->manager->getEncoders();

  for (rfb::Encoder* e : encoders_) {
    suite::TimedEncoder* te = dynamic_cast<suite::TimedEncoder*>(e);
    if (te) {
      EncoderStats* es = te->stats();
      fprintf(stderr, "Encoder: %s, with encoding %d", te->getName().c_str(), te->encoding);

      // Compression ratios
      std::cout << "  Compression Ratio (Rectangles): " << es->compressionRatioRects() << "\n";
      std::cout << "  Compression Ratio (Solid Rectangles): " << es->compressionRatioSolidRects() << "\n";
      std::cout << "  Combined Compression Ratio: " << es->compressionRatioCombined() << "\n";
    } else {
      fprintf(stderr, "Dynamic cast failed\n");
      continue;
    }
  }
  exit(0);

  ManagerStats managerStats = server->stats();

  if (managerStats.encoders.empty()) {
    // FIXME: throw/log error?
    throw std::runtime_error("Empty encoder list");
} else {
  fprintf(stderr, "Print managerStats:\n");
    managerStats.print();
}
  delete server;
}

EncoderSettings Benchmark::encoderSettings(rfb::EncoderClass encoderClass,
                                           PseudoEncodingLevel quality,
                                           PseudoEncodingLevel compression)
{
  int* encodings;
  EncoderSettings settings;

  // FIXME: These encodings should maybe match
  // CConnection::updateEncodings().
  encodings = new int[4];
  encodings[0] = rfb::encodingRaw;
  encodings[1] = rfb::pseudoEncodingCompressLevel0 + compression;
  encodings[2] = rfb::encodingCopyRect;
  encodings[3] = rfb::pseudoEncodingLastRect;

  settings = EncoderSettings{
    .encoderClass = encoderClass,
    .rfbEncoding = encodings,
    .encodingSize = 4,
    .quality = quality,
    .compression = compression,
    .name = encoderClasstoString(encoderClass),
  };

  switch (encoderClass) {
  case rfb::encoderRaw:
    encodings[0] = rfb::encodingRaw;
    break;
  case rfb::encoderRRE:
    encodings[0] = rfb::encodingRRE;
    break;
  case rfb::encoderHextile:
    encodings[0] = rfb::encodingHextile;
    break;
  case rfb::encoderTight:
    encodings[0] = rfb::encodingTight;
    break;
  case rfb::encoderTightJPEG:
    if (quality == NONE)
      settings.quality = TWO;
    delete [] encodings;
    encodings = new int[4];
    encodings[0] = rfb::encodingTight;
    encodings[1] = rfb::pseudoEncodingQualityLevel0 + quality;
    encodings[2] = rfb::pseudoEncodingCompressLevel0 + compression;
    encodings[3] = rfb::encodingCopyRect;
    encodings[4] = rfb::pseudoEncodingLastRect;   // denn är out of bounds men det är nog inte så noga
    settings.rfbEncoding = encodings;
    settings.encodingSize = 5;
    break;
  case rfb::encoderZRLE:
    encodings[0] = rfb::encodingZRLE;
    break;
  default:
    throw std::logic_error("EncoderClass not implemented");
  }
  return settings;
}

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
  EncoderSettings settings;
  std::vector<int32_t> encodingList;

  for (uint i = 0; i < ENCODERS_COUNT; i++) {
      EncoderSettings enc = encoderSettings(static_cast<EncoderClass>(i));
      encodingList.insert(encodingList.end(), enc.rfbEncoding, enc.rfbEncoding + enc.encodingSize);
  }

  // Assign the collected encodings to `settings`
  settings.encodingSize = encodingList.size();
  settings.rfbEncoding = new int32_t[settings.encodingSize];
  std::copy(encodingList.begin(), encodingList.end(), settings.rfbEncoding);

  settings.encoderClass = enumEncoder::encoderTight;

  fprintf(stderr, "settings.encoderClass = %d\n", settings.encoderClass);
  // Run the benchmark with the single settings object
  runBenchmark(settings);

  delete [] settings.rfbEncoding;
}

void Benchmark::runBenchmark(EncoderSettings& settings)
{
  FrameInStream is;
  std::ifstream file;
  std::map<EncoderClass, Server*> servers;

  is = FrameInStream();
  file = std::ifstream(filename_);
  is.parseHeader(file); // FIXME: Don't parse header twice
  printf("Running benchmark with settings: %d\n", settings.encoderClass);

  Server* server = new Server(width(), height(), settings);
  server->setEncodings(settings.encodingSize, settings.rfbEncoding);
  servers[settings.encoderClass] = server;


  std::cout << "Starting benchmark using \"" << filename_ << "\"\n";
  RecorderStats recorderStats;
  int imageNr = 0;
  while (file.peek() != EOF) {
    imageNr++;
    const Image* image = is.readImage(file, recorderStats, imageNr);

    // Load image once (no need to loop through multiple servers)
    server->loadImage(image, image->x_offset_, image->y_offset_);
    server->out->clear();


#ifdef _DEBUG
    debugServer_->loadImage(image, image->x_offset_, image->y_offset_);
#endif // _DEBUG
    delete image;
  }
  std::cout << "Benchmarking complete!\n";

// Print statistics for the single server

  // FIXME: Refactor this to a separate function
  std::string encoderRequested = encoderClasstoString(server->settings.encoderClass);
  // Server* server = s.second;
  ManagerStats managerStats = server->stats();

  if (managerStats.encoders.empty()) {
    // FIXME: throw/log error?
    throw std::runtime_error("Empty encoder list");
} else {
    managerStats.print();
}
  delete server;
}

EncoderSettings Benchmark::encoderSettings(EncoderClass encoderClass,
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
  case encoderRaw:
    encodings[0] = rfb::encodingRaw;
    break;
  case encoderRRE:
    encodings[0] = rfb::encodingRRE;
    break;
  case encoderHextile:
    encodings[0] = rfb::encodingHextile;
    break;
  case encoderTight:
    encodings[0] = rfb::encodingTight;
    break;
  case encoderTightJPEG:
    if (quality == NONE)
      settings.quality = TWO;
    delete [] encodings;
    encodings = new int[4];
    encodings[0] = rfb::encodingTight;
    encodings[1] = rfb::pseudoEncodingQualityLevel0 + quality;
    encodings[2] = rfb::pseudoEncodingCompressLevel0 + compression;
    encodings[3] = rfb::encodingCopyRect;
    encodings[4] = rfb::pseudoEncodingLastRect;
    settings.rfbEncoding = encodings;
    settings.encodingSize = 5;
    break;
  case encoderZRLE:
    encodings[0] = rfb::encodingZRLE;
    break;
  default:
    throw std::logic_error("EncoderClass not implemented");
  }
  return settings;
}

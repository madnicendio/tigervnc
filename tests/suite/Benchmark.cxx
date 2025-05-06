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
  EncoderSettings* settings = new EncoderSettings[ENCODERS_COUNT];

  int allEncodings[] = {
    0,  // rfb::encodingRaw
    1,  // rfb::encodingCopyRect
    2,  // rfb::encodingRRE
    4,  // rfb::encodingCoRRE
    5,  // rfb::encodingHextile
    7,  // rfb::encodingTight
    16, // rfb::encodingZRLE
    -224 // rfb::pseudoEncodingLastRect
  };

  settings->rfbEncoding = allEncodings;
  settings->encodingSize = sizeof(allEncodings) / sizeof(allEncodings[0]);

  runBenchmark(settings);

  delete [] settings;
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
  server->setEncodings(settings->encodingSize, settings->rfbEncoding);

  std::cout << "Starting benchmark using \"" << filename_ << "\"\n";
  RecorderStats recorderStats;
  // Extract images from the file
  while (file.peek() != EOF) {
    // This function uses recorderStats to record information
    const Image* image = is.readImage(file, recorderStats);

    server->loadImage(image, image->x_offset_, image->y_offset_);
    server->out->clear();


#ifdef _DEBUG
    debugServer_->loadImage(image, image->x_offset_, image->y_offset_);
#endif // _DEBUG
    delete image;
  }
  std::cout << "Benchmarking complete!\n";
  fprintf(stderr, "\n");
  fprintf(stderr, "==========================================[ STATS: ]=========================================\n\n");

  // Output encoderstats:
  std::vector<rfb::Encoder*> allEncoders = server->manager->getEncoders();

  std::vector<std::pair<std::string, suite::TimedEncoder*>> encoders = {
    {"Tight", dynamic_cast<suite::TimedEncoder*>(allEncoders[encoderTight])},
    {"JPEG", dynamic_cast<suite::TimedEncoder*>(allEncoders[encoderJPEG])}
  };

  long double totalWriteRectTime = 0.0;
  long long unsigned totalEncodedPixels = 0;
  int totalNumberOfRects = 0;
  long long unsigned totalMedianRectSize = 0;
  double totalMPixelsPerSecond = 0.0;
  double totalCompressionRatio = 0.0;

  for (size_t i = 0; i < encoders.size(); ++i) {
    suite::TimedEncoder* encoder = encoders[i].second;
    if (encoder) {
        EncoderStats* stats = encoder->stats();
        totalWriteRectTime += stats->writeRectEncodetime;
        totalEncodedPixels += stats->encodedPixels;
        totalNumberOfRects += stats->nRects;
        totalMedianRectSize += encoder->medianRectSize();
    }
}

// Skriv ut tabell
fprintf(stderr, "+------------+------------+------------+------------+------------+------------+------------+\n");
fprintf(stderr, "| %-10s | %-10s | %-10s | %-10s | %-10s | %-10s | %-10s |\n",
        "Encoder", "Time (ms)", "#Pixels %", "# Rects", "MedianRect", "MPx/s", "Compr.");
fprintf(stderr, "+------------+------------+------------+------------+------------+------------+------------+\n");

for (size_t i = 0; i < encoders.size(); ++i) {
    std::string name = encoders[i].first;
    suite::TimedEncoder* encoder = encoders[i].second;
    if (encoder) {
        EncoderStats* stats = encoder->stats();
        double percentage = (100.0 * stats->encodedPixels) / totalEncodedPixels;
        double mpixelsPerSecond = stats->megaPixelsPerSecondRects();
        double compressionRatio = stats->compressionRatioRects();
        long long unsigned medianRectSize = encoder->medianRectSize();

        fprintf(stderr, "| %-10s | %-10.2Lf | %-10.2f | %-10d | %-10llu | %-10.2f | %-10.2f |\n",
                name.c_str(),
                stats->writeRectEncodetime,
                percentage,
                stats->nRects,
                medianRectSize,
                mpixelsPerSecond,
                compressionRatio);

        totalMPixelsPerSecond += mpixelsPerSecond * percentage/100;
        totalCompressionRatio += compressionRatio * percentage/100;
    }
}

fprintf(stderr, "+------------+------------+------------+------------+------------+------------+------------+\n");
fprintf(stderr, "| %-10s | %-10.2Lf | %-10llu | %-10d | %-10llu | %-10.2f | %-10.2f |\n",
        "Total",
        totalWriteRectTime,
        totalEncodedPixels,
        totalNumberOfRects,
        totalMedianRectSize / encoders.size(),  // Medelvärde
        totalMPixelsPerSecond,  // viktat
        totalCompressionRatio); // viktat
fprintf(stderr, "+------------+------------+------------+------------+------------+------------+------------+\n\n");

// Ta fram hur lång tid en total frame update tar
const ManagerStats& stats = server->stats();
std::vector<WriteUpdate> writeUpdateStats = stats.writeUpdateStats;

// Summera ihop tiden som varje frame Frame har tagit
double sum = 0;
for (WriteUpdate& update : writeUpdateStats)
  sum += update.timeSpent;

fprintf(stderr, "\n+--------------------------------------+------------+\n");
fprintf(stderr, "| %-36s | %10.3f |\n", "Total time spent writing frames (s)", sum);
fprintf(stderr, "+--------------------------------------+------------+\n");





  exit(0);
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

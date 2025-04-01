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
    16, // rfb::encodingZRLE
    -224 // rfb::pseudoEncodingLastRect
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
  // double maxEncodingTime = 0.0;
  // int mostCriticalFrame = -1;
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

    // Check if this frame has the longest encoding time
    // if (imageNr != 1 && image->stats_.encodingTime > maxEncodingTime) {
    //     maxEncodingTime = image->stats_.encodingTime;
    //     mostCriticalFrame = imageNr;
    // }


#ifdef _DEBUG
    debugServer_->loadImage(image, image->x_offset_, image->y_offset_);
#endif // _DEBUG
    delete image;
  }
  std::cout << "Benchmarking complete!\n\n\n";

  // Print statistics for the single server

  // Output encoderstats:
  std::vector<rfb::Encoder*> allEncoders = server->manager->getEncoders();

  std::vector<std::pair<std::string, suite::TimedEncoder*>> encoders = {
    {"Tight", dynamic_cast<suite::TimedEncoder*>(allEncoders[rfb::encoderTight])},
    {"TightJPEG", dynamic_cast<suite::TimedEncoder*>(allEncoders[rfb::encoderTightJPEG])}
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

        totalMPixelsPerSecond += mpixelsPerSecond;
        totalCompressionRatio += compressionRatio;
    }
}

fprintf(stderr, "+------------+------------+------------+------------+------------+------------+------------+\n");
fprintf(stderr, "| %-10s | %-10.2Lf | %-10llu | %-10d | %-10llu | %-10.2f | %-10.2f |\n",
        "Total",
        totalWriteRectTime,
        totalEncodedPixels,
        totalNumberOfRects,
        totalMedianRectSize / encoders.size(),  // Medelvärde
        totalMPixelsPerSecond / encoders.size(),  // Medelvärde
        totalCompressionRatio / encoders.size()); // Medelvärde
fprintf(stderr, "+------------+------------+------------+------------+------------+------------+------------+\n\n");


int criticalFrame = findCriticalFrame(server);
if (criticalFrame != -1) {
    const ManagerStats& stats = server->stats();
    const WriteUpdate& update = stats.writeUpdateStats[criticalFrame - 1];

    // Print header
    fprintf(stderr, "\nMost Critical Frame:\n");
    fprintf(stderr, "+------------+------------+------------+\n");
    fprintf(stderr, "| Frame Nr   | Time (ms)  | #Pixels    |\n");
    fprintf(stderr, "+------------+------------+------------+\n");

    // Print the critical frame details
    fprintf(stderr, "| %-10d | %-10.2f | %-10u |\n",
            criticalFrame, update.timeSpent, update.size);

    fprintf(stderr, "+------------+------------+------------+\n");
} else {
    std::cout << "No frames processed." << std::endl;
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

int Benchmark::findCriticalFrame(Server* server) {
  const ManagerStats& stats = server->stats();

  // Variabler för att hålla reda på den mest kritiska ramen
  double maxTimeSpent = 0.0;
  int criticalFrame = -1;

  // Gå igenom alla uppdateringar och hitta den med största timeSpent
  for (size_t i = 0; i < stats.writeUpdateStats.size(); ++i) {
      const WriteUpdate& update = stats.writeUpdateStats[i];
      if (update.timeSpent > maxTimeSpent) {
          maxTimeSpent = update.timeSpent;
          criticalFrame = i;
      }
  }

  // Because currentWriteUpdate isn't zero-indexed
  return criticalFrame+1;
}
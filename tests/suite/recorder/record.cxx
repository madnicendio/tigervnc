#include "Recorder.h"
#include "../codec/codecFactory.h"

#include <iostream>
#include <string>

using namespace suite;
int main(int argc, char* argv[])
{
  if(argc < 5) {
    std::string encoders;
    for (const std::pair<std::string, EncoderEnum> pair : encodersMap) {
      encoders += pair.first + " ";
    }
    std::cerr << "Error, incorrect arguments\n"
              << "Usage:\n\t"
              << argv[0] << " <X display> <output filename> "
              << "<framerate> <duration> [<encoder (default=JPEG)>] [<delay (default=0)>]\n\t"
              << "Available encoders: " << encoders << std::endl;
    exit(1);
  }

  // FIXME: Validate display
  std::string display = argv[1];
  std::string filename = argv[2];
  int framerate = atoi(argv[3]);
  int duration = atoi(argv[4]);

  ImageEncoder* encoder = nullptr;
  if (argc > 5) {
      encoder = constructEncoder(argv[5]);
  } else {
      encoder = constructEncoder(JPEG);
  }

  // Parse delay
  int delay = 0;
  if (argc > 6) {
      delay = std::atoi(argv[6]);
      if (delay < 0) {
          std::cerr << "Error: Delay cannot be negative." << std::endl;
          return 1;
      }
  }

  Recorder recorder = Recorder(filename, encoder, display, framerate);

  recorder.startRecording(duration, delay);

  // FIXME: Add some way of stopping recording, like a duration argument.
  return 0;
}

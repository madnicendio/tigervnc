#include "Recorder.h"
#include "../codec/codecFactory.h"

#include <iostream>
#include <string>

using namespace suite;
int main(int argc, char* argv[])
{
  if(argc < 4) {
    std::string encoders;
    for (const std::pair<std::string, EncoderEnum> pair : encodersMap) {
      encoders += pair.first + " ";
    }
    std::cerr << "Error, incorrect arguments\n"
              << "Usage:\n\t"
              << argv[0] << " <X display> <output filename> "
              << "<framerate> <encoder (default=JPEG)>\n\t"
              << "Available encoders: " << encoders << std::endl;
    exit(1);
  }

  // FIXME: Validate display
  std::string display = argv[1];
  std::string filename = argv[2];
  int framerate = atoi(argv[3]);

  ImageEncoder* encoder = argc > 4 ? constructEncoder(argv[4])
                                   : constructEncoder(JPEG);

  Recorder recorder = Recorder(filename, encoder, display, framerate);

  recorder.startRecording();

  // FIXME: Add some way of stopping recording, like a duration argument.
  return 0;
}

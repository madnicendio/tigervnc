#include "Server.h"
#include "Manager.h"
#include "codec/timed/TimedEncoder.h"
#include <rfb/SConnection.h>
#include <rfb/SMsgWriter.h>

namespace suite {

  Server::Server(int width, int height, rfb::PixelFormat pf, bool debug)
    : rfb::SConnection(rfb::AccessFull),
      settings(defaultEncoderSettings)
  {
    fprintf(stderr, "Server::Server (debug)\n");
    init(width, height, pf);
    manager = new Manager(this, debug);
  }

  Server::Server(int width, int height, EncoderSettings settings_,
                                        rfb::PixelFormat pf)
    : rfb::SConnection(rfb::AccessFull), settings(settings_)
  {
    init(width, height, pf);
    fprintf(stderr, "Server::Server\n");
    // fprintf(stderr, "settings.encoderClass = %d\n", settings.encoderClass);
    manager = new Manager(this, settings_);
  }

  void Server::init(int width, int height, rfb::PixelFormat pf)
  {
    out = new DummyOutStream();
    in = new DummyInStream();
    setStreams(in, out);
    setWriter(new rfb::SMsgWriter(&client, out));

    this->updates = rfb::SimpleUpdateTracker();

    setPixelFormat(fbPF);
    client.setPF(fbPF);

    pb_ = new rfb::ManagedPixelBuffer(pf, width,  height);
    setEncodings(sizeof(encodings) / sizeof(*encodings), encodings);
  }

  Server::~Server()
  {
    delete out;
    delete in;
    delete manager;
    delete pb_;
  }

  void Server::loadImage(const Image *image, int x, int y)
  {
    // Define the rectangular region where the image will be loaded?
    rfb::Rect rect(x,y, x + image->width_, y + image->height_);

    // Get a pointer to the pixel buffer
    int stride;
    pb_->getBuffer(rect, &stride);

    // Copy the image's pixel data into the pixel buffer at the defined rectangle
    pb_->imageRect(rect, image->getBuffer());

    // Create an UpdateInfo object that will hold details about the update
    rfb::UpdateInfo ui;

    // Define
    const rfb::Region changed(rect);

    updates.add_changed(rect);
    updates.getUpdateInfo(&ui, changed);

    manager->writeUpdate(ui, pb_, nullptr, image->frameTime_);
  }

  void Server::writeUpdate(const rfb::UpdateInfo& ui,
                           const rfb::PixelBuffer* pb)
  {
    manager->writeUpdate(ui, pb, nullptr);
  }

  ManagerStats Server::stats()
  {
    return manager->stats();
  }
}

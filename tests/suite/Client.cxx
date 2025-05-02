#include "Client.h"
#include "codec/PPMEncoder.h"
#include <rfb/CMsgReader.h>
#include <rfb/CMsgWriter.h>

using namespace suite;

Client::Client(int width, int height, rfb::PixelFormat pf)
{
  in_ = new DummyInStream();
  out_ = new DummyOutStream();
  setStreams(in_, out_);

  setState(RFBSTATE_NORMAL);

  reader = new rfb::CMsgReader(this,in_);
  writer = new rfb::CMsgWriter(&server, out_);
  setReader(reader);
  setWriter(writer);

  setPF(fbPF);
  pb_ = new rfb::ManagedPixelBuffer(pf, width, height);
  server.setDimensions(pb_->width(),pb_->height());
  setFramebuffer(pb_);
}

Client::~Client()
{
  delete in_;
  delete out_;
  // reader & writer get deleted in superclass
}

void Client::setInStream(DummyInStream* in)
{
  delete in_;
  in_ = in;
  setStreams(in_, out_);

  delete reader;
  reader = new rfb::CMsgReader(this, in);
  setReader(reader);
}

void Client::copyOutStreamToInStream(DummyOutStream* out)
{
  uint8_t* data = new uint8_t[out->length()];
  memcpy(data, out->dump(), out->length());
  DummyInStream* inStream = new DummyInStream(data, out->length());
  setInStream(inStream);
}


void Client::resizeFramebuffer()
{
  rfb::ModifiablePixelBuffer *pb;
  pb = new rfb::ManagedPixelBuffer(server.pf(), server.width(),
                                   server.height());
  setFramebuffer(pb);
}

void Client::framebufferUpdateStart()
{
  rfb::CConnection::framebufferUpdateStart();
}

void Client::framebufferUpdateEnd()
{
#ifdef _DEBUG
  const std::string DEBUG_OUTPUT = "debug_output.ppm";

  // Dump framebuffer to file after each framebufferUpdate
  rfb::PixelBuffer* pb;
  int stride;
  pb = getFramebuffer();

  const uint8_t* data = pb->getBuffer(pb->getRect(), &stride);

  PPMEncoder e = PPMEncoder();
  e.encodeImageTofile(data, pb->width(), pb->height(), DEBUG_OUTPUT);
#endif // _DEBUG

  rfb::CConnection::framebufferUpdateEnd();
}

bool Client::dataRect(const rfb::Rect&r , int encoding)
{
  return rfb::CConnection::dataRect(r, encoding);
}

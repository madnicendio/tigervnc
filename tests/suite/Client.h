#ifndef __SUITE_CLIENT_H__
#define __SUITE_CLIENT_H__

#include <rfb/CConnection.h>
#include "Server.h"

namespace suite {
  class Client : public rfb::CConnection
  {
  public:
    Client(int width, int height, rfb::PixelFormat pf = fbPF);
    ~Client();

    // Sets InStream and sets a new CMsgReader with the new InStream
    void setInStream(DummyInStream* in);

    // Creates copy of buffer in OutStream and creates a new InStream
    // with copied data
    void copyOutStreamToInStream(DummyOutStream* out);

    virtual void resizeFramebuffer() override;
    virtual void framebufferUpdateStart() override;
    virtual void framebufferUpdateEnd() override;
    virtual bool dataRect(const rfb::Rect&, int) override;

    // Unused
    virtual void getUserPasswd(bool, std::string*,
                               std::string*) override {};

    virtual bool showMsgBox(rfb::MsgBoxFlags, const char*,
                            const char*) override { return false; };
    virtual void initDone() override {};
    virtual void setCursor(int, int, const rfb::Point&, const uint8_t*) override {};
    virtual void setCursorPos(const rfb::Point&) override {};
    virtual void setColourMapEntries(int, int, uint16_t*) override {};
    virtual void bell() override {};

    rfb::SimpleUpdateTracker updates;
  protected:
    rfb::ManagedPixelBuffer* pb_;
  private:
    DummyInStream *in_;
    DummyOutStream *out_;
    rfb::CMsgReader* reader;
    rfb::CMsgWriter* writer;
  };
}

#endif // __SUITE_CLIENT_H__

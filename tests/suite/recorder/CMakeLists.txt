add_executable(recorder record.cxx)

target_sources(recorder PRIVATE
  Recorder.cxx
  util.cxx
  ../io/FrameOutStream.cxx
  ../io/ImageUpdate.cxx
  ../codec/ImageEncoder.cxx
  ../codec/ImageDecoder.cxx
  ../codec/JpegDecoder.cxx
  ../codec/JpegEncoder.cxx
  ../codec/PPMDecoder.cxx
  ../codec/PPMEncoder.cxx
  ../codec/Image.cxx
  ../codec/codecFactory.cxx
  ../../../unix/x0vncserver/Image.cxx
  ../../../unix/x0vncserver/XPixelBuffer.cxx
  ../../../unix/x0vncserver/PollingManager.cxx
  ../../../unix/x0vncserver/Geometry.cxx
)

add_definitions(-DHAVE_XDAMAGE)
target_link_libraries(recorder ${X11_Xdamage_LIB})

target_link_libraries(recorder rfb tx ${X11_LIBRARIES})

target_compile_features(recorder PRIVATE cxx_std_14)
include_directories(${CMAKE_SOURCE_DIR}/common)
include_directories(${CMAKE_SOURCE_DIR}/common/os)
include_directories(${CMAKE_SOURCE_DIR}/unix)
include_directories(${CMAKE_SOURCE_DIR}/unix/common)
include_directories(${X11_INCLUDE_DIR})
include_directories(${FLTK_INCLUDE_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

add_executable(suite suite.cxx)
target_sources(suite PRIVATE
  Manager.cxx
  Server.cxx
  Client.cxx
  Benchmark.cxx
  io/FrameInStream.cxx
  codec/codecFactory.cxx
  codec/Image.cxx
  codec/ImageEncoder.cxx
  codec/ImageDecoder.cxx
  codec/PPMEncoder.cxx
  codec/PPMDecoder.cxx
  codec/JpegEncoder.cxx
  codec/JpegDecoder.cxx
  codec/timed/TimedEncoder.cxx
  codec/timed/timedEncoderFactory.cxx
  codec/bruteforce/BruteForce.cxx
  codec/bruteforce/BruteForceEncoder.cxx
  stats/EncoderStats.cxx
  stats/ManagerStats.cxx
  streams/DummyInStream.cxx
  streams/DummyOutStream.cxx
)

target_compile_features(suite PRIVATE cxx_std_11)
target_link_libraries(suite PRIVATE rfb)

# Recording tool
add_subdirectory(recorder)

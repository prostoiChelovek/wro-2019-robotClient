cmake_minimum_required(VERSION 3.8)
project(videoTrans)

set(CMAKE_CXX_STANDARD 17)

set(RHVOICE_LIBS "-L/usr/src/RHVoice/build/linux/audio -L/usr/src/RHVoice/src/audio -L/usr/src/RHVoice/build/linux/core -L/usr/src/RHVoice/src/core -lRHVoice_core -lRHVoice_audio")
set(RHVOICE_DIR "/usr/local/include/RHVoice")

find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost COMPONENTS system filesystem thread REQUIRED)

include_directories(${Boost_INCLUDE_DIR})

add_subdirectory(modules/speech)
include_directories(modules/speech)
includeSpeechDirs()

set(SOURCE_FILES main.cpp commandProcessing.hpp utils.hpp dataCollector.hpp)
addSpeechSources("modules/speech/" SOURCE_FILES)
add_executable(videoTrans ${SOURCE_FILES})

target_link_libraries(videoTrans ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(videoTrans ${OpenCV_LIBS})
target_link_libraries(videoTrans ${Boost_SYSTEM_LIBRARY})

linkSpeechLibs(videoTrans)

cmake_minimum_required(VERSION 3.7)
project(videoTrans)

set(CMAKE_CXX_STANDARD 11)

set(RHVOICE_LIBS "-L/usr/src/RHVoice/build/linux/audio -L/usr/src/RHVoice/src/audio -L/usr/src/RHVoice/build/linux/core -L/usr/src/RHVoice/src/core -lRHVoice_core -lRHVoice_audio")
set(RHVOICE_DIR "/usr/local/include/RHVoice")

find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost COMPONENTS system filesystem thread REQUIRED)

include_directories(${Boost_INCLUDE_DIR})

add_subdirectory(modules/speech)

set(SOURCE_FILES main.cpp commandProcessing.hpp utils.hpp dataCollector.hpp Speaker.hpp)
add_executable(videoTrans ${SOURCE_FILES})

target_link_libraries(videoTrans ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(videoTrans ${OpenCV_LIBS})
target_link_libraries(videoTrans ${Boost_SYSTEM_LIBRARY})
target_link_libraries(videoTrans speech)
if (${ON_PI})
    message(STATUS "On PI")
    target_compile_definitions(videoTrans PRIVATE IS_PI=1)
    target_link_libraries(videoTrans wiringPi)
endif ()

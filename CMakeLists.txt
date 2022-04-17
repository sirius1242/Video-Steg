cmake_minimum_required(VERSION 3.22)

project(VideoSteg)

set(OWN_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_MODULE_PATH ${OWN_CMAKE_MODULE_PATH})

find_package(OpenCV REQUIRED)
find_package(GSL REQUIRED)
SET(OpenCV_LIBS opencv_core opencv_highgui opencv_imgproc opencv_imgcodecs)
SET(GSL_LIBS gsl)
include_directories(${OpenCV_INCLUDE_DIRS} ${FFMPEG_INCLUDE_DIRS})

include_directories(
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/src
)

file(GLOB COMMON
        "include/ecc.hpp"
        "include/steg.hpp"
        "src/ecc.cpp"
        "src/steg.cpp"
        )

add_executable(steg ${COMMON} "src/image.cpp")
target_link_libraries(steg ${OpenCV_LIBS} ${GSL_LIBS})

find_package(FFMPEG)
if(NOT(FFMPEG_FOUND))
    message(SEND_ERROR "You must install FFMPEG to compile this code")
endif()

add_executable(vsteg ${COMMON} "src/video.cpp")
target_link_libraries(vsteg ${OpenCV_LIBS} ${GSL_LIBS} ${FFMPEG_LIBRARIES})

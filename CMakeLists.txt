cmake_minimum_required(VERSION 3.22)

project(VideoSteg)

find_package(OpenCV REQUIRED)
SET(OpenCV_LIBS opencv_core opencv_highgui opencv_imgproc opencv_imgcodecs)
include_directories(${OpenCV_INCLUDE_DIRS})

include_directories(
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/src
)

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.c"
        )

add_executable(steg ${all_SRCS})
target_link_libraries(steg ${OpenCV_LIBS})

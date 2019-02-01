cmake_minimum_required(VERSION 2.8)

project(easy_label)

add_compile_options(-std=c++11)
set(OpenCV_DIR "/opt/ros/kinetic/share/OpenCV-3.3.1-dev")
find_package( OpenCV REQUIRED )
include_directories(${OpenCV_INCLUDE_DIRS})


add_executable(${PROJECT_NAME} "main.cpp")
target_link_libraries( ${PROJECT_NAME}
${OpenCV_LIBRARIES}
    )

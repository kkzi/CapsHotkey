cmake_minimum_required(VERSION 3.10)
project(CapsHotkey)

set(CMAKE_CXX_STANDARD 20)

aux_source_directory(. SRCS)
find_path(WTL_INCLUDE_DIRS "atlapp.h")
add_executable(${PROJECT_NAME} WIN32 ${SRCS} res/app.rc)
target_include_directories(${PROJECT_NAME} PRIVATE $ENV{SIMPLE_CPP} ${WTL_INCLUDE_DIRS})
target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE)

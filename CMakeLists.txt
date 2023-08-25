cmake_minimum_required(VERSION 3.10)
project(CapsHotkey)

set(CMAKE_CXX_STANDARD 20)

find_package(imgui CONFIG REQUIRED)
aux_source_directory(. SRCS)
add_executable(${PROJECT_NAME} WIN32 ${SRCS} res/app.rc)
#target_include_directories(${PROJECT_NAME} PRIVATE $ENV{SIMPLE_CPP})
target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE)
target_include_directories(${PROJECT_NAME} PRIVATE "C:/Code/simple")
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)

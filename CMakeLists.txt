cmake_minimum_required(VERSION 3.10)
project(CapsHotkey)

set(CMAKE_CXX_STANDARD 20)

#if (MSVC)
#    string(REGEX REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
#    string(REGEX REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
#    add_compile_options($<$<CONFIG:>:/MT> $<$<CONFIG:Debug>:/MTd> $<$<CONFIG:Release>:/MT>)
#endif (MSVC)

add_executable(${PROJECT_NAME} WIN32 main.cpp logo.rc)

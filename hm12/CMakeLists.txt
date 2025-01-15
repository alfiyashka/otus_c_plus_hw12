cmake_minimum_required(VERSION 3.28.3)


set(PATCH_VERSION "1" CACHE INTERNAL "PATCH_VERSION" )
set(PROJECT_VERSION 0.0.${PATCH_VERSION})

project(mapreduce VERSION ${PROJECT_VERSION} LANGUAGES C CXX)

configure_file(version.h.in version.h)

set ( HW12_INCLUDE
  src/MapReduce.hpp
  src/Utils.hpp
)

set ( HW12_SOURCES 
  src/MapReduce.cpp  
)

find_package (Threads)

include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inlude)

add_executable(mapreduce ${HW12_INCLUDE} ${HW12_SOURCES} main.cpp)

target_link_libraries (mapreduce 
   Threads::Threads
)

set_property(TARGET  PROPERTY CXX_STANDARD 23)

install(TARGETS mapreduce DESTINATION . )


include("functions.cmake")
generate_packages(${PROJECT_NAME})











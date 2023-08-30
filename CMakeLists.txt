# CMakeList.txt : CMake project for exr-thumbnail-generator, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("exr-thumbnail-generator")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source to this project's executable.
add_executable (exr-thumbnail-generator "exr-thumbnail-generator.cpp" "exr-thumbnail-generator.h" "argparse.hpp")

# TODO: Add tests and install targets if needed.
find_package(OpenImageIO CONFIG REQUIRED)  
target_link_libraries(exr-thumbnail-generator PRIVATE OpenImageIO::OpenImageIO)
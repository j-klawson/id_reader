cmake_minimum_required(VERSION 3.15)
project(id_reader VERSION 1.0.0 LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(ENABLE_OPENCL "Enable OpenCL acceleration" OFF)
option(ENABLE_CUDA "Enable CUDA acceleration" OFF)

# Include directories
include_directories(include)

# Find required packages
find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(TESSERACT REQUIRED tesseract)

# Find TensorFlow Lite
find_path(TENSORFLOWLITE_INCLUDE_DIR tensorflow/lite/interpreter.h)
find_library(TENSORFLOWLITE_LIB tensorflowlite)

if(NOT TENSORFLOWLITE_INCLUDE_DIR OR NOT TENSORFLOWLITE_LIB)
    message(FATAL_ERROR "TensorFlow Lite not found")
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Platform-specific settings
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
endif()

if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
endif()

# OpenCL support
if(ENABLE_OPENCL)
    find_package(OpenCL REQUIRED)
    add_definitions(-DENABLE_OPENCL)
endif()

# CUDA support
if(ENABLE_CUDA)
    find_package(CUDA REQUIRED)
    add_definitions(-DENABLE_CUDA)
endif()

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

# Create library
add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    ${OpenCV_LIBS}
    ${TESSERACT_LIBRARIES}
    ${TENSORFLOWLITE_LIB}
)

if(ENABLE_OPENCL)
    target_link_libraries(${PROJECT_NAME} ${OpenCL_LIBRARIES})
endif()

if(ENABLE_CUDA)
    target_link_libraries(${PROJECT_NAME} ${CUDA_LIBRARIES})
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${OpenCV_INCLUDE_DIRS}
    ${TESSERACT_INCLUDE_DIRS}
    ${TENSORFLOWLITE_INCLUDE_DIR}
)

if(ENABLE_OPENCL)
    target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCL_INCLUDE_DIRS})
endif()

# Build tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Build examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Install
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Config
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT ${PROJECT_NAME}Config
    FILE ${PROJECT_NAME}Config.cmake
    DESTINATION lib/cmake/${PROJECT_NAME}
)
cmake_minimum_required(VERSION 3.16.3)

project(app
    VERSION 1.0
    DESCRIPTION "testing cmake"
    LANGUAGES CXX CUDA)


set(CMAKE_CUDA_STANDARD 14)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CUDA REQUIRED)


set(CMAKE_CUDA_ARCHITECTURES 89)

set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};
    -O3
    -gencode arch=compute_89,code=sm_89
    --extended-lambda
    --relocatable-device-code=true
    -lineinfo
)

set(CUDA_FILES
    src/cuda/GPUDecoder.cu
)

set(CPP_FILES
    src/main.cpp
    src/decoder/Decoder.cpp
    src/encoder/Encoder.cpp
    src/decoder/DecoderHuffmanTree.cpp
    src/encoder/EncoderHuffmanTree.cpp
    src/utils/BitStream.cpp
)

set(HEADER_FILES
src/decoder/include/Decoder.hpp
src/encoder/include/Encoder.hpp
src/decoder/include/DecoderHuffmanTree.hpp
src/encoder/include/EncoderHuffmanTree.hpp
src/utils/include/BitStream.hpp
    src/cuda/include/GPUDecoder.hpp
)

add_executable(app ${CUDA_FILES} ${CPP_FILES} ${HEADER_FILES})

set_target_properties(app PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CUDA_INCLUDE_DIRS}
)

target_link_libraries(app PUBLIC 
    ${CUDA_LIBRARIES}
    ${CUDA_CUDA_LIBRARY}
    ${CUDA_CUDART_LIBRARY}
)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_library(testing STATIC ${CPP_FILES} ${HEADER_FILES})
target_link_libraries(testing PUBLIC ${JPEG_LIBRARIES})

add_subdirectory(test)
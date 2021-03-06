cmake_minimum_required(VERSION 2.8)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
project(Rapidnet)

set(CMAKE_BUILD_TYPE Release )

ENABLE_LANGUAGE(ASM)
message(${CMAKE_SOURCE_DIR})
message(${CMAKE_CURRENT_SOURCE_DIR})

if(DEBUG)
    add_definitions(-DDEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG -g -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG -g -fPIC")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fPIC")
    if(BUILD_FOR_ANDROID_COMMAND)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s -Wl,--gc-sections")
    endif()
endif()

if(OPENMP_ENABLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
endif()


if(ARMV8)
    set(KERNEL_SRCS
        4x4i8v1.c  
        GEMM4x4Micro.S
    )
    message("ARMV8 ON")
    set(ANDROID_ABI "arm64-v8a")
    if(LINUX)
        set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
        set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
        message("CXX use:" ${CMAKE_C_COMPILER} " " ${CMAKE_CXX_COMPILER})
    endif()
else()
    set(KERNEL_SRCS
        4x4armv7.c  
        GEMM4x4MicroV7.S
    )
    add_definitions( -mfpu=neon )
    set(ANDROID_ABI "armeabi-v7a with NEON")
endif()

set(TEST_SRCS
  compare_matrices.c  
  copy_matrix.c  
  dclock.c  
  print_matrix.c  
  random_matrix.c  
  test_MMult.c
)


link_libraries(m)
link_directories()
include_directories(${CMAKE_SOURCE_DIR})

message(${TEST_SRCS})

add_executable(test ${TEST_SRCS} ${KERNEL_SRCS})
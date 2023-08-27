cmake_minimum_required (VERSION 3.20)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
project(cantp
	VERSION 0.0.1
	LANGUAGES CXX C
	DESCRIPTION "A CanTp Project")
include(CTest)

set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -O3 -Wall")

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map")

# Create a simple configuration header
configure_file(config.h.in config.h)

add_library(${PROJECT_NAME}
    ${SOURCE_DIR}/cantp.c)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${INCLUDE_DIR}
)

add_executable(${PROJECT_NAME}_main
    main.c)

target_include_directories(${PROJECT_NAME}_main
    PRIVATE
        ${INCLUDE_DIR}
    PUBLIC 
        ${PROJECT_BINARY_DIR}
)

if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(test/unit)
endif()

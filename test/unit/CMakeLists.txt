set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)
include(GoogleTest)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 --coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
add_executable(${PROJECT_NAME}_test
    cantp_test.cpp
    ${SOURCE_DIR}/cantp.c
)

target_include_directories(${PROJECT_NAME}_test
    PRIVATE
        ${INCLUDE_DIR}
)


target_link_libraries(${PROJECT_NAME}_test
    gtest
    gmock
    gtest_main
    gcov
)

add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)

cmake_minimum_required(VERSION 3.17)
project(v8ppDemo)

set(CMAKE_CXX_STANDARD 14)

add_compile_options(-DV8_COMPRESS_POINTERS)

INCLUDE_DIRECTORIES(
        .
        v8/include
        v8/include/libplatform
        v8/include/cppgc
)

add_executable(v8ppDemo v8pp/context.cpp v8pp/class.cpp v8pp/convert.cpp v8pp/json.cpp
        v8pp/throw_ex.cpp v8pp/version.cpp main.cpp  )

target_link_libraries(
        v8ppDemo
        ${PROJECT_SOURCE_DIR}/v8/libv8_monolith.a
        ${PROJECT_SOURCE_DIR}/v8/libv8_monolith.a
)
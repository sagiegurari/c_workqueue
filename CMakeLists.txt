
cmake_minimum_required (VERSION 3.7)

project (workqueue C)

# include shared utilities
if(NOT EXISTS "target/cmake-modules/src/utils.cmake")
  execute_process(COMMAND git clone https://github.com/sagiegurari/cmake-modules.git)
endif()
include("target/cmake-modules/src/utils.cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS -pthread)
if(NOT WIN32)
  set(X_CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wall -Wextra -Wcast-align -Wunused -Wshadow -Wpedantic")
endif()

set(X_CMAKE_PROJECT_ROOT_DIR ${CMAKE_BINARY_DIR}/..)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(include)

# define all sources
file(GLOB SOURCES "src/*.c")
file(GLOB HEADER_SOURCES "include/*.h")
file(GLOB TEST_SOURCES "tests/*")
file(GLOB COMMON_TEST_SOURCES "tests/test.*")
file(GLOB EXAMPLE_SOURCES "examples/*.c")

# lint code
find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
if (CMAKE_CXX_CPPCHECK)
  add_custom_target(cppcheck ALL
    ${CMAKE_CXX_CPPCHECK}
    --enable=all --inline-suppr --error-exitcode=1 --suppress=missingIncludeSystem -I ./include/ ./src/*.c
    WORKING_DIRECTORY "${X_CMAKE_PROJECT_ROOT_DIR}")
endif()

# format code
if(DEFINED ENV{X_CMAKE_UNCRUSTIFY_ENABLED})
  add_custom_target(uncrustify ALL
    uncrustify
    -c ${X_CMAKE_PROJECT_ROOT_DIR}/uncrustify.cfg
    --no-backup ${SOURCES} ${HEADER_SOURCES} ${TEST_SOURCES} ${EXAMPLE_SOURCES})
endif()

# create static library
add_library(${CMAKE_PROJECT_NAME} STATIC ${SOURCES})
if(NOT WIN32)
  set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${X_CMAKE_C_FLAGS} -Wconversion")
endif()

# example
add_executable(example examples/example.c)
target_link_libraries(example ${CMAKE_PROJECT_NAME})
set_target_properties(example PROPERTIES COMPILE_FLAGS "${X_CMAKE_C_FLAGS}")

# tests
include(CTest)

utils_setup_c_all_tests(
  ADDITIONAL_SOURCES "${COMMON_TEST_SOURCES}"
  COMPILATION_FLAGS "${X_CMAKE_C_FLAGS}"
  BINARY_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)


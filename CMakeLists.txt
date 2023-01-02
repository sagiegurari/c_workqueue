
cmake_minimum_required(VERSION 3.7)

project(workqueue C)

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
utils_cppcheck(INCLUDE_DIRECTORY "./include/" SOURCES "./src/*.c" WORKING_DIRECTORY "${X_CMAKE_PROJECT_ROOT_DIR}")

# format code
utils_uncrustify(
  CONFIG_FILE "${X_CMAKE_PROJECT_ROOT_DIR}/uncrustify.cfg"
  SOURCES ${SOURCES} ${HEADER_SOURCES} ${TEST_SOURCES} ${EXAMPLE_SOURCES}
)

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

utils_setup_test_lib(
  SOURCES "${COMMON_TEST_SOURCES}"
  COMPILATION_FLAGS "${X_CMAKE_C_FLAGS}"
)
utils_setup_c_all_tests(
  COMPILATION_FLAGS "${X_CMAKE_C_FLAGS}"
  BINARY_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  LIBRARIES "Test"
)


cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
cmake_policy(SET CMP0054 OLD)
cmake_policy(SET CMP0045 OLD)

if (NOT EXISTS ${CMAKE_CACHEFILE_DIR}/cmake)
  execute_process(
    COMMAND git clone https://github.com/uael/cmake-modules.git cmake
    RESULT_VARIABLE result OUTPUT_QUIET
    WORKING_DIRECTORY ${CMAKE_CACHEFILE_DIR}
  )
endif ()

set(CMAKE_MODULE_PATH "${CMAKE_CACHEFILE_DIR}/cmake")
include(ProjectDependency)

set(CMAKE_C_STANDARD 99)
set(BUILD_SHARED_LIBS OFF)

project(uopt C)
project_dependency(uds GIT_REPOSITORY https://github.com/uael/uds.git)

set(uopt_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(uopt_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(uopt_TEST_DIR ${CMAKE_CURRENT_LIST_DIR}/test)

file(GLOB_RECURSE uopt_SRCS ${uopt_SRCS} ${uopt_SRC_DIR}/*.c)
file(GLOB_RECURSE uopt_HDRS ${uopt_HDRS} ${uopt_INC_DIR}/uopt/*.h)
set(uopt_HDR ${uopt_INC_DIR}/uopt.h)

add_library(uopt ${uopt_SRCS} ${uopt_HDRS} ${uopt_HDR})
target_include_directories(uopt PUBLIC ${uopt_INC_DIR})
target_link_libraries(uopt PUBLIC uds)

set(CMAKE_C_FLAGS "")
set(CMAKE_C_FLAGS_DEBUG "")
set(CMAKE_C_FLAGS_RELEASE "")

if (MSVC)
  target_compile_options(uopt
    PUBLIC /MT$<$<CONFIG:Debug>:d> /Oy /Za /W3
    PUBLIC /D_CRT_SECURE_NO_WARNINGS
    PUBLIC /O$<$<CONFIG:Debug>:d>$<$<CONFIG:Release>:x>)
else ()
  target_compile_options(uopt
    PRIVATE -Wall -Werror -Wextra -fomit-frame-pointer -ansi
    PRIVATE -Wno-missing-field-initializers
    PRIVATE -O$<$<CONFIG:Debug>:0 -g3>$<$<CONFIG:Release>:3>)
endif ()

get_directory_property(uopt_PARENT PARENT_DIRECTORY)
if (NOT uopt_PARENT)
  set(uopt_DEVEL TRUE)
elseif (NOT uopt_DEVEL)
  set(uopt_DEVEL FALSE)
endif ()
if (uopt_DEVEL AND EXISTS ${uopt_TEST_DIR})
  project_dependency(cute GIT_REPOSITORY https://github.com/uael/cute.git)
  if (NOT uopt_PARENT)
    enable_testing()
  endif ()

  file(GLOB uopt_TEST_SRCS ${uopt_TEST_SRCS} ${uopt_TEST_DIR}/*.c)
  foreach (uopt_TEST_SRC ${uopt_TEST_SRCS})
    get_filename_component(uopt_TEST_NAME ${uopt_TEST_SRC} NAME_WE)
    add_executable(test_uopt_${uopt_TEST_NAME} ${uopt_TEST_SRC})
    add_dependencies(test_uopt_${uopt_TEST_NAME} uopt cute)
    target_link_libraries(test_uopt_${uopt_TEST_NAME} uopt cute)
    add_test(${uopt_TEST_NAME} test_uopt_${uopt_TEST_NAME})
  endforeach ()
endif ()

install(FILES ${uopt_HDRS}
  DESTINATION include/uopt)
install(FILES ${uopt_HDR}
  DESTINATION include)
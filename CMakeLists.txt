cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
cmake_policy(SET CMP0054 OLD)
cmake_policy(SET CMP0045 OLD)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(ProjectDependency)

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

get_directory_property(uopt_PARENT PARENT_DIRECTORY)
if (NOT ${uopt_PARENT})
  set(uopt_DEVEL TRUE)
elseif (NOT ${uopt_DEVEL})
  set(uopt_DEVEL FALSE)
endif ()
if (${uopt_DEVEL} AND EXISTS ${uopt_TEST_DIR})
  project_dependency(cute GIT_REPOSITORY https://github.com/uael/cute.git)
  if (NOT uopt_PARENT)
    enable_testing()
  endif ()

  file(GLOB uopt_TEST_SRCS ${uopt_TEST_SRCS} ${uopt_TEST_DIR}/*.c)
  foreach (uopt_TEST_SRC ${uopt_TEST_SRCS})
    get_filename_component(uopt_TEST_NAME ${uopt_TEST_SRC} NAME_WE)
    add_executable(test_${uopt_TEST_NAME} ${uopt_TEST_SRC})
    add_dependencies(test_${uopt_TEST_NAME} uopt cute)
    target_link_libraries(test_${uopt_TEST_NAME} uopt cute)
    add_test(${uopt_TEST_NAME} test_${uopt_TEST_NAME})
  endforeach ()
endif ()

install(FILES ${uopt_HDRS}
  DESTINATION include/uopt)
install(FILES ${uopt_HDR}
  DESTINATION include)
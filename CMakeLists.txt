cmake_minimum_required (VERSION 3.0.0)
project (ArchiverGPU
    VERSION 0.1.0
    LANGUAGES CXX
)

set (CMAKE_CXX_STANDARD 20)

set (SRC_DIR src)

#######################
# OpenCL
#######################
find_package (OpenCL REQUIRED)
include_directories (${OpenCL_INCLUDE_DIRS})
link_directories (${OpenCL_LIBRARY})

#######################
# ArchiverGPU
#######################
set (ARCH_DIR ${SRC_DIR}/archiver)
file (GLOB ARCH_DIR_SOURCES ${ARCH_DIR}/*.cpp)
add_executable (ArchiverGPU ${ARCH_DIR_SOURCES} ${SRC_DIR}/main.cpp)

target_include_directories (ArchiverGPU PUBLIC "OpenCL-CLHPP/include")
target_include_directories (ArchiverGPU PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries (ArchiverGPU ${OpenCL_LIBRARY})

file (COPY src/archiver/config/kernels DESTINATION .)

# add_custom_command (TARGET ArchiverGPU POST_BUILD
#                     COMMAND ${CMAKE_COMMAND} -E copy_directory
#                     ${CMAKE_SOURCE_DIR}/src/archiver/config $<TARGET_FILE_DIR:ArchiverGPU>)

#######################
# GTest
#######################
ADD_SUBDIRECTORY (googletest)
enable_testing ()
include_directories ("googletest/googletest/include" ${gtest_SOURCE_DIR})

# #######################
# # Unit Tests ArchiverGPU
# #######################
set (TEST_DIR src/tests)
add_executable (run_tests ${ARCH_DIR_SOURCES} ${TEST_DIR}/test_archiver_cpu.cpp)

#Link test executable against gtest and gtest_main
target_link_libraries (run_tests gtest gtest_main ${OpenCL_LIBRARY})
target_include_directories (run_tests PUBLIC "OpenCL-CLHPP/include")
target_include_directories (run_tests PUBLIC ${SRC_DIR}/archiver)
cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME wgslibserver-varmap)

project(${PROJECT_NAME})

include_directories(${CMAKE_SOURCE_DIR}/varmap)
include_directories(${CMAKE_SOURCE_DIR})

include_directories(${CMAKE_BINARY_DIR}/varmap)


## File to compile
add_library(wgslibserver-varmap SHARED
    fftvarmapcalc.cpp
    fftvarmapcalc.h
	../common.h
)
target_link_libraries(wgslibserver-varmap
	${FFTW_LIBRARIES}
)


INSTALL(TARGETS wgslibserver-varmap DESTINATION "${INSTALL_DIR_LIB}")


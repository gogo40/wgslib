cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME wgslibserver-server)

project(${PROJECT_NAME})

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}/src)

## File to compile
add_library(wgslibserver-dev SHARED
    abstractwgslibstubserver.h    
    wgslibstubclient.h
    
    grid.h
    grid.cpp

    wgslibserver.h
    wgslibserver.cpp
)
target_link_libraries(wgslibserver-dev
    wgslibserver-varmap	
    ${JSONRPC_LIBRARIES}
    ${FFTW_LIBRARIES}
)

INSTALL(TARGETS wgslibserver-dev DESTINATION "${INSTALL_DIR_LIB}")

add_executable(wgslibserver main.cpp)
target_link_libraries(wgslibserver wgslibserver-dev ${JSONRPC_LIBRARIES} ${FFTW_LIBRARIES})

INSTALL(TARGETS wgslibserver DESTINATION "${INSTALL_DIR_BIN}")


cmake_minimum_required(VERSION 2.6)
project(wgslibserver)


if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Release CACHE STRING
       "Choose the type of build, options are: Debug Release"
       FORCE )
endif()


if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -O3 -march=native -fPIC")
  set(CMAKE_CXX_FLAGS_DEBUG "-std=c++11 -ggdb -march=native -fPIC")
elseif(MSVC)
  set(CMAKE_CXX_FLAGS_RELEASE "/MP4")
  # customize flags for Visual Studio
   add_definitions("-DWGSLIB_EXPORT")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

include_directories(${CMAKE_SOURCE_DIR}/varmap)
include_directories(${CMAKE_SOURCE_DIR})

set(INSTALL_DIR_BIN  "/usr/bin" CACHE TYPE PATH)
set(INSTALL_DIR_LIB   "/usr/lib" CACHE TYPE PATH)
set(INSTALL_DIR_INCLUDE  "/usr/include" CACHE TYPE PATH)

SET(MAJOR_VERSION 0)
SET(MINOR_VERSION 1)
SET(PATCH_VERSION 0)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/wgslibserver/version.h.in"
  "${PROJECT_BINARY_DIR}/wgslibserver/src/version.h"
)
install(FILES ${PROJECT_BINARY_DIR}/wgslibserver/version.h DESTINATION ${INSTALL_DIR_INCLUDE}/wgslib)


find_package(FFTW REQUIRED)
include_directories(${FFTW_INCLUDE_DIR})



find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
  message("OPENMP FOUND")
  if (MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
  else (MSVC)
	set(CMAKE_C_FLAGS "-std=c++11 -march=native -fPIC ${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "-std=c++11 -march=native -fPIC ${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
  endif (MSVC)
endif()


add_subdirectory(varmap)


find_package(JSONRPC REQUIRED)
include_directories(${JSONRPC_INCLUDE_DIR})

find_package(JSONCPP REQUIRED)
#include_directories(${JSONCPP_INCLUDE_DIRS})

add_subdirectory(wgslibserver)



IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
    INCLUDE(InstallRequiredSystemLibraries)

    SET(CPACK_PACKAGE_NAME "libwgslib-dev")
    SET(CPACK_SET_DESTDIR "on")


    SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../license.txt")
    SET(CPACK_RESOURCE_FILE_README  "${CMAKE_CURRENT_SOURCE_DIR}/../README.md")
    SET(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/../README.md")

    IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	    SET(CPACK_GENERATOR "PackageMaker")
	    SET(CPACK_PACKAGING_INSTALL_PREFIX "/opt/local")
    ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	    SET(CPACK_GENERATOR "DEB")
	    SET(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
    ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	    SET(CPACK_GENERATOR "NSIS")
    ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

    SET(CPACK_PACKAGE_DESCRIPTION "wgslib-server for C++, including a geostatistics algorithms server.")
    SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "wgslib-server for C++")
    SET(CPACK_PACKAGE_VENDOR "Péricles Lopes Machado")
    SET(CPACK_PACKAGE_CONTACT "Péricles Lopes Machado <pericles.raskolnikoff@gmail.com>")
    SET(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
    SET(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
    SET(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
    SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
    SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")

    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libcurl4-nss-dev")

    SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    SET(CPACK_DEBIAN_PACKAGE_SECTION "libs")
    SET(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

    SET(CPACK_COMPONENTS_ALL wgslibserver-varmap wgslibserver-server wgslibserver-dev)
    INCLUDE(CPack)
ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")


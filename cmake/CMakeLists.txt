cmake_minimum_required (VERSION 3.13)

include_guard(GLOBAL)

message(STATUS "\n")
message(STATUS ":==================================:")
message(STATUS ":setting global cmake configuration:")
message(STATUS ":==================================:\n")

# must be defined before the module get imported
set(INSTALL_BINARY_SUFFIX bin/${CMAKE_BUILD_TYPE})
set(INSTALL_LIBRARY_SUFFIX lib/${CMAKE_BUILD_TYPE})
set(INSTALL_INCLUDE_SUFFIX include)

#install prefix
if(DEFINED CMAKE_INSTALL_PREFIX)
	message(STATUS "installation path set to ${CMAKE_INSTALL_PREFIX}")
else()
	set(CMAKE_INSTALL_PREFIX "${PROJECT_HOME}")
	message(STATUS "defaulting installation path to ${CMAKE_INSTALL_PREFIX}")
endif()

# load the utils module
include("${CMAKE_CURRENT_LIST_DIR}/module/utils.cmake")

# set verbosity
if(CMAKE_VERBOSE)
	message(DEBUG "extra compiler logging...")
	set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "flag to force make/ninja to display compiler commands")
endif()

# conan setup 
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
	message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
	set(CONAN_FILE_URL https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake)
	file(DOWNLOAD "${CONAN_FILE_URL}" "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)
set(CMAKE_PROGRAM_PATH "${CONAN_BIN_PATH};${CMAKE_PROGRAM_PATH}")
conan_check(VERSION 1.12.0 REQUIRED)

# enable the languages & set the dialect
enable_language(C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# enable lto
include(CheckIPOSupported)
check_ipo_supported(RESULT LTO_SUPPORTED)
		
if (${CMAKE_BUILD_TYPE} STREQUAL "Release")      
  if(LTO_SUPPORTED)
		set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
	endif()
endif()

find_package(Threads)
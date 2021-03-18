cmake_minimum_required(VERSION 3.13)

# project options
option(BUILD_DOC          "create the sphinx documentation of the project" 	OFF	)
option(BUILD_TESTS        "create the testsuite of the project" 						ON	)
option(CMAKE_WERROR       "activate tighter compilation checks" 						OFF	)
option(CMAKE_VERBOSE      "enables debug + compiler logging" 								OFF	)
option(ARROW_EXPERIMENTAL "enables arrow experimental module" 							ON	)

set(PROJECT_HOME "${CMAKE_CURRENT_LIST_DIR}")
set(PROJECT_NAME tws11 CACHE STRING "" FORCE)
set(PROJECT_NAMESPACE ${PROJECT_HOME} CACHE STRING "" FORCE)
string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_U)

message(STATUS "*** variable PROJECT_NAME set to: ${PROJECT_NAME} ***")
message(STATUS "*** variable PROJECT_NAMESPACE set to: ${PROJECT_NAMESPACE} ***")
message(STATUS "*** variable PROJECT_HOME set to: ${PROJECT_HOME} ***")

message(STATUS "*** retrieving cmake configuration from ${PROJECT_HOME}/cmake ***")

#####################################
#         include toolsets          #
#####################################

include("${PROJECT_HOME}/cmake/CMakeLists.txt")

#####################################
#       defines cmake project       #
#####################################

set(${PROJECT_NAME_U}_MAJOR_VERSION 0)
set(${PROJECT_NAME_U}_MINOR_VERSION 0)
set(${PROJECT_NAME_U}_BUILD_VERSION 1)

set(${PROJECT_NAME_U}_VERSION
  ${${PROJECT_NAME_U}_MAJOR_VERSION}.${${PROJECT_NAME_U}_MINOR_VERSION}.${${PROJECT_NAME_U}_BUILD_VERSION})

set(PROJECT_VERSION ${${PROJECT_NAME_U}_VERSION})

message(STATUS "project version set to ${PROJECT_VERSION}")

project(
	${PROJECT_NAME}
	LANGUAGES C CXX
	VERSION	${PROJECT_VERSION}
	DESCRIPTION "the ${PROJECT_NAME} project"
)

#####################################
#    import thirdparty packages     #
#####################################

set(GTEST_VERS     "1.8.1" )
set(TWS_VERS       "9.79.1")
set(PYBIND11_VERS  "2.5.0" )
set(ARROW_VERS     "3.0.0" )
set(CPP11_VERS     "0.2.5" )
set(EIGEN_VERS     "3.3.7" )

set(PROJECT_PACKAGES
	tws gtest pybind11 arrow cpp11 eigen
)

# TODO: need to match the pyarrow deps (jemalloc, parquet, etc)
set(CONAN_OPTIONS "arrow:pyarrow=True")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CONAN_OPTIONS "${CONAN_OPTIONS};gbench:lto=False")
	set(CONAN_EXTRA_SETTINGS "${CONAN_EXTRA_SETTINGS};build_type=${CMAKE_BUILD_TYPE}")
else()
	set(CONAN_OPTIONS "${CONAN_OPTIONS};gbench:lto=True")
	set(CONAN_EXTRA_SETTINGS "${CONAN_EXTRA_SETTINGS};build_type=${CMAKE_BUILD_TYPE}")
endif()

message(STATUS "package list has been set to ${PROJECT_PACKAGES}")

# imported packages
load_packages(
	NAME     ${PROJECT_PACKAGES}
	SETTINGS ${CONAN_EXTRA_SETTINGS}
	PROFILE  ${CONAN_PROFILE}
	OPTIONS  ${CONAN_OPTIONS}
)

#####################################
#    generate the documentation     #
#####################################

if (BUILD_DOC)
	add_subdirectory(docs)
endif()

#####################################
#    add the project applications   #
#####################################

add_subdirectory(python/package)
add_subdirectory(R/package)

#####################################
#             ctest setup           #
#####################################

include(CTest)
add_subdirectory(tests)
cmake_minimum_required (VERSION 3.13)

# step 0: configuration
set(TWS11_MODULE_STR "RcppModules: tws11_module")
set(PKG_BUILD_ROOT "${CMAKE_CURRENT_BINARY_DIR}/../build")

include(ProcessorCount)
ProcessorCount(NCORES)

# 1: create the makevars content
set(MAKEVARS_CONTENT "")
r_create_makevars(
	OUT MAKEVARS_CONTENT
	DIALECT 17
	BUILD_TYPE ${CMAKE_BUILD_TYPE}
	PACKAGES cpp11 tws eigen
)

message(DEBUG "MAKEVARS_CONTENT set to: \n ${MAKEVARS_CONTENT}")

# 2: create the package target
r_create_package_target(
	NAME tws11Package
	PACKAGE tws11
	SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	MAKEVARS ${MAKEVARS_CONTENT}
	DESCRIPTION ${CMAKE_CURRENT_SOURCE_DIR}/tws11/ressources/DESCRIPTION.in
	BUILD_DIRECTORY ${PKG_BUILD_ROOT}
)

add_custom_target(
	refreshTimestamps ALL
  COMMAND ${CMAKE_COMMAND} -E env
	sh -c "touch src/cpp11.cpp"
	COMMAND ${CMAKE_COMMAND} -E env
	sh -c "touch NAMESPACE"
	WORKING_DIRECTORY ${PKG_BUILD_ROOT}/tws11
	DEPENDS tws11Package
)

# register C++ functions
add_custom_target(registerCpp ALL
	COMMAND ${CMAKE_COMMAND} -E env "MAKE=${DEVTOOLS_ROOT}/bin/make -j${NCORES}" 
	"PATH=${R_LOCATION_HINT}/bin:$ENV{PATH}"
	${R_EXEC} -e "library(cpp11);cpp11::cpp_register()"
	WORKING_DIRECTORY ${PKG_BUILD_ROOT}/tws11
	DEPENDS refreshTimestamps
	VERBATIM
)

# refresh the package documentation
add_custom_target(buildDocumentation ALL
	COMMAND ${CMAKE_COMMAND} -E env "MAKE=${DEVTOOLS_ROOT}/bin/make -j${NCORES}" 
	"PATH=${R_LOCATION_HINT}/bin:$ENV{PATH}"
	${R_EXEC} -e "library(roxygen2);roxygen2::roxygenize()"
	WORKING_DIRECTORY ${PKG_BUILD_ROOT}/tws11
	DEPENDS registerCpp 
	VERBATIM
)

# build the package
add_custom_target(installPackage ALL
	COMMAND ${CMAKE_COMMAND} -E env "MAKE=${DEVTOOLS_ROOT}/bin/make -j${NCORES}" 
	"PATH=${R_LOCATION_HINT}/bin:$ENV{PATH}"
	${R_EXEC} -e "library(devtools);devtools::install(build=FALSE, dependencies=FALSE)"
	WORKING_DIRECTORY ${PKG_BUILD_ROOT}/tws11
	DEPENDS buildDocumentation
	VERBATIM
)

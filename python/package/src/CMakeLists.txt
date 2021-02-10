cmake_minimum_required (VERSION 3.13)

SET(TARGET_NAME _tws11)

find_package(pybind11 REQUIRED 
	HINTS ${CONAN_PYBIND11_ROOT}/share/cmake/pybind11
)

import_python(HINT ${PYTHON_LOCATION_HINT})

file(GLOB_RECURSE MODULE_HEADERS CONFIGURE_DEPENDS *.h)
file(GLOB_RECURSE MODULE_SOURCES CONFIGURE_DEPENDS *.cpp)

pybind11_add_module(${TARGET_NAME} MODULE 
	${MODULE_HEADERS} ${MODULE_SOURCES}
)

target_link_libraries(${TARGET_NAME} 
	PUBLIC CONAN_PKG::tws
	CONAN_PKG::arrow
	arrow_python
)

install(
	TARGETS ${TARGET_NAME}
	LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIBRARY_SUFFIX}/python/tws11
)

add_custom_command(TARGET _tws11
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:_tws11> ${PYTHON_DOCS_TARGET}/tws11
	DEPENDS copy_python_files
)
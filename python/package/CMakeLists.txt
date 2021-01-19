cmake_minimum_required (VERSION 3.13)

set(CURRENT_DATE "")
get_current_date(
	OUT CURRENT_DATE
	FORMAT "+%Y-%m-%d"
)

# in order to build the documentation in cmake we must gather 
# all the package ressources in the same folder
set(PYTHON_DOCS_TARGET ${CMAKE_BINARY_DIR}/docs/package)

file(MAKE_DIRECTORY ${PYTHON_DOCS_TARGET}/tws11)

add_custom_target(copy_python_files ALL 
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_CURRENT_SOURCE_DIR}/tws11
	${PYTHON_DOCS_TARGET}/tws11
)

install(
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tws11
	DESTINATION ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIBRARY_SUFFIX}/python
)

add_subdirectory(src)

# TODO: setup configure script
#configure_file(
#	${CMAKE_CURRENT_SOURCE_DIR}/otos/setup.py.in
#	${PROJECT_BIN_PATH}/../setup.py
#)

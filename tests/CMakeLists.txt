cmake_minimum_required (VERSION 3.13)

set(PYTHONPATH_TESTSUITE "${CMAKE_INSTALL_PREFIX}/${INSTALL_LIBRARY_SUFFIX}/python")
message(DEBUG "PYTHONPATH_TESTSUITE set to ${PYTHONPATH_TESTSUITE}")

add_test(
	NAME tws11.python.package.import
	COMMAND ${PYTHON_EXECUTABLE} -c "from tws11 import *"
	WORKING_DIRECTORY ${TEST_FILE_DIR}
)

set_tests_properties(tws11.python.package.import
    PROPERTIES 
	    ENVIRONMENT "PYTHONPATH=${PYTHONPATH_TESTSUITE}"
)

function(add_test_py)

  set(options)
	set(oneValueArgs NAME CLASS FILE)
	set(multiValueArgs)

	cmake_parse_arguments(
		ADD_TEST_PY "${options}"
		"${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	add_test(
		NAME python.${ADD_TEST_PY_NAME}
		COMMAND ${PYTHON_EXECUTABLE} -m unittest -v ${ADD_TEST_PY_FILE}.${ADD_TEST_PY_CLASS}.${ADD_TEST_PY_NAME}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

	set_tests_properties(python.${ADD_TEST_PY_NAME}
		PROPERTIES ENVIRONMENT ""
		DEPENDS wotan.python.package.import
	)

	set_property(TEST python.${ADD_TEST_PY_NAME}
		APPEND PROPERTY ENVIRONMENT "PYTHONPATH=${PYTHONPATH_TESTSUITE}"
	)

endfunction(add_test_py)

add_subdirectory(contract)
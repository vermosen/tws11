cmake_minimum_required (VERSION 3.13)

add_test_py(NAME test_create_contract CLASS test_contract FILE test)
add_test_py(NAME test_print_contract  CLASS test_contract FILE test)
add_test_py(NAME test_build_currency  CLASS test_contract FILE test)
add_test_py(NAME test_build_equity    CLASS test_contract FILE test)
add_test_py(NAME test_build_option    CLASS test_contract FILE test)
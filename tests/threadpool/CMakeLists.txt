cmake_minimum_required (VERSION 3.13)

SET(TARGET_NAME test_threadpool)

setup_component(TARGET ${TARGET_NAME})

file(GLOB_RECURSE TARGET_HEADERS "*.h")
file(GLOB_RECURSE TARGET_SOURCES "*.cpp")

add_executable(
	${TARGET_NAME}
	${TARGET_HEADERS}
	${TARGET_SOURCES}
)

add_gtest(
	TARGET ${TARGET_NAME}
	TIMEOUT 10
)

target_link_libraries(
	${TARGET_NAME}
	PUBLIC
	gtest gtest_main
	Threads::Threads
)

target_include_directories(
	${TARGET_NAME}
	PUBLIC "${PROJECT_HOME}/python/package/src"
)

install_binary(TARGET ${TARGET_NAME})



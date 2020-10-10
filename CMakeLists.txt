cmake_minimum_required(VERSION 3.10)

project(8080)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(FlagSettings "-Wall" "-Werror" "-Wextra" 
	"$<$<CONFIG:RELEASE>:-O2>"
	"$<$<CONFIG:DEBUG>:-DVERBOSE -DDEBUG>"
	"$<$<CONFIG:RELWITHDEBINFO>:-DBENCHMARK>")

add_library(SourceFiles
	OBJECT
		src/arithmetic_opcodes.c
		src/cycle_timer.c
		src/branch_opcodes.c
	  	src/data_transfer_opcodes.c
	      	src/opcode_array.c
		src/cpu_thread.c
		src/logical_opcodes.c
		src/other_opcodes.c
)


target_compile_options(SourceFiles PRIVATE ${FlagSettings})

target_include_directories(SourceFiles PRIVATE include)

add_executable(8080 
	src/main.c
	$<TARGET_OBJECTS:SourceFiles>
)

find_package(Threads)
target_link_libraries(8080 ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(8080 PRIVATE include)

target_compile_options(8080 PRIVATE ${FlagSettings})

enable_testing()
find_package(GTest)
add_executable(Tests 
	test/test_main.cpp
	$<TARGET_OBJECTS:SourceFiles>
)
target_include_directories(Tests 
	PRIVATE 
		include
		${GTEST_INCLUDE_DIRS}
)
target_link_libraries(Tests
	PRIVATE
		${CMAKE_THREAD_LIBS_INIT}
		${GTEST_BOTH_LIBRARIES}
)

target_sources(Tests
	PRIVATE
		test/data_opcode_tests.cpp
		test/logical_opcode_tests.cpp
		test/other_opcode_tests.cpp
		test/branch_opcode_tests.cpp
		test/arithmetic_opcode_tests.cpp
)

gtest_discover_tests(Tests)

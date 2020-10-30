cmake_minimum_required(VERSION 3.10)

project(8080)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(FlagSettings "-Wall" "-Werror" "-Wextra" 
	"$<$<CONFIG:DEBUG>:-DVERBOSE -DDEBUG>"
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/hw)
set(ROM_DIR ${CMAKE_BINARY_DIR}/roms)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

#Copy ROMs
file(GLOB ROM_FILES 
	CONFIGURE_DEPENDS
	${CMAKE_SOURCE_DIR}/roms/*)

foreach(rom ${ROM_FILES})
	get_filename_component(name ${rom} NAME)
	configure_file(${rom} ${ROM_DIR}/${name}
		COPYONLY
	)
endforeach()

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

target_include_directories(SourceFiles PRIVATE ${INCLUDE_DIR})

add_executable(8080 
	src/main.c
	$<TARGET_OBJECTS:SourceFiles>
)

find_package(Threads)
target_link_libraries(8080 
	${CMAKE_THREAD_LIBS_INIT}
	${CMAKE_DL_LIBS})
target_include_directories(8080 PRIVATE ${INCLUDE_DIR})

target_compile_options(8080 PRIVATE 
	${FlagSettings}
)

enable_testing()
find_package(GTest)
add_executable(Tests 
	test/test_main.cpp
	$<TARGET_OBJECTS:SourceFiles>
)

set_target_properties(Tests PROPERTIES ENABLE_EXPORTS TRUE)

target_include_directories(Tests 
	PRIVATE 
		include
		${GTEST_INCLUDE_DIRS}
		hardware
		hardware/taito/include
)
target_link_libraries(Tests
	PRIVATE
		${CMAKE_THREAD_LIBS_INIT}
		${GTEST_BOTH_LIBRARIES}
		${CMAKE_DL_LIBS}
)

target_sources(Tests
	PRIVATE
	test/data_opcode_tests.cpp
	test/logical_opcode_tests.cpp
	test/other_opcode_tests.cpp
	test/branch_opcode_tests.cpp
	test/arithmetic_opcode_tests.cpp
	test/hw_funcs_tests.cpp
)

gtest_discover_tests(Tests)

#Hardware libraries

#Find every file contained in the hardware subdirectory.
file(GLOB children 
	LIST_DIRECTORIES true 
	CONFIGURE_DEPENDS
	${CMAKE_CURRENT_LIST_DIR}/hardware/*)
#Iterate over the files we found: if it's a directory, add it.
FOREACH(item ${children})
	IF(IS_DIRECTORY ${item})
		add_subdirectory(${item})
	ENDIF()
ENDFOREACH()

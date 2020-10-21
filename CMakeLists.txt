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

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/hardware)
set(ROM_DIR ${CMAKE_BINARY_DIR}/roms)

#Copy ROMs
file(GLOB ROM_FILES ${CMAKE_SOURCE_DIR}/roms/*)
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

target_include_directories(SourceFiles PRIVATE include)

add_executable(8080 
	src/main.c
	$<TARGET_OBJECTS:SourceFiles>
)

find_package(Threads)
target_link_libraries(8080 
	${CMAKE_THREAD_LIBS_INIT}
	${CMAKE_DL_LIBS})
target_include_directories(8080 PRIVATE include)

target_compile_options(8080 PRIVATE 
	${FlagSettings}
)
set_target_properties(8080 PROPERTIES
	ENABLE_EXPORTS TRUE)

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

#Hardware libraries

#Find every file contained in the hardware subdirectory.
file(GLOB children 
	LIST_DIRECTORIES true 
	CONFIGURE_DEPENDS
	${CMAKE_CURRENT_LIST_DIR}/hardware/*)
#Create an empty list.
SET(hw_sets "")
#Iterate over the files we found: if it's a directory, add it to the list.
FOREACH(item ${children})
	IF(IS_DIRECTORY ${item})
		LIST(APPEND hw_sets ${item})
	ENDIF()
ENDFOREACH()

#Create a target for the library.
FOREACH(item ${hw_sets})
	#Get just the final element of the path we found.
	get_filename_component(name ${item} NAME)
	#Create a shared library target with that name.
	add_library(${name} SHARED)
	#Add the source file to its source list.
	target_sources(${name} PRIVATE ${item}/src/hw_funcs.c)
	#Give it the compiler flags we're using, whatever those are.
	target_compile_options(${name} PRIVATE ${FlagSettings})
	target_include_directories(${name} PRIVATE 
		include
		${item}/include)
ENDFOREACH()

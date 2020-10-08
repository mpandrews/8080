cmake_minimum_required(VERSION 3.16)

project(8080)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(8080 src/main.c)

find_package(Threads)
target_link_libraries(8080 ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(8080 PRIVATE include)

target_sources(8080
	PRIVATE
		src/arithmetic_opcodes.c
		src/cycle_timer.c
		src/branch_opcodes.c
	  	src/data_transfer_opcodes.c
	      	src/opcode_array.c
		src/cpu_thread.c
		src/logical_opcodes.c
		src/other_opcodes.c
)

target_compile_options(8080
		PRIVATE
			-Wall -Werror -Wextra 
			$<$<CONFIG:RELEASE>:-NDEBUG -O2>
			$<$<CONFIG:DEBUG>:-DVERBOSE -DDEBUG -O0>
)

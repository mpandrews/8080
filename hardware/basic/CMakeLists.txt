
add_library(basic MODULE)

file(GLOB srcs
	CONFIGURE_DEPENDS
	${CMAKE_CURRENT_LIST_DIR}/src/*
)

target_sources(basic PRIVATE ${srcs})
target_include_directories(basic PRIVATE
	${INCLUDE_DIR}
	${CMAKE_CURRENT_LIST_DIR}/include
	)
target_compile_options(basic PRIVATE ${FlagSettings})

find_package(Curses REQUIRED)

target_link_libraries(basic ${CURSES_LIBRARIES})

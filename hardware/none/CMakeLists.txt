
add_library(none MODULE)

file(GLOB srcs
	CONFIGURE_DEPENDS
	${CMAKE_CURRENT_LIST_DIR}/src/*
)

target_sources(none PRIVATE ${srcs})
target_include_directories(none PRIVATE
	${INCLUDE_DIR}
	${CMAKE_CURRENT_LIST_DIR}/include
	)
target_compile_options(none PRIVATE ${FlagSettings})

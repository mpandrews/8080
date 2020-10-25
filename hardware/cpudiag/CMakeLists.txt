
add_library(cpudiag MODULE)

file(GLOB srcs
	CONFIGURE_DEPENDS
	${CMAKE_CURRENT_LIST_DIR}/src/*
)

target_sources(cpudiag PRIVATE ${srcs})
target_include_directories(cpudiag PRIVATE
	${INCLUDE_DIR}
	${CMAKE_CURRENT_LIST_DIR}/include
	)
target_compile_options(cpudiag PRIVATE ${FlagSettings})

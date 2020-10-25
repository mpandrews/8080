
add_library(taito SHARED)

find_package(SDL2 REQUIRED)

file(GLOB srcs
	CONFIGURE_DEPENDS
	${CMAKE_CURRENT_LIST_DIR}/src/*
)

target_sources(taito PRIVATE ${srcs})

target_include_directories(taito PRIVATE
	${INCLUDE_DIR}
	${CMAKE_CURRENT_LIST_DIR}/include
	${SDL2_INCLUDE_DIRS}
	)
target_compile_options(taito PRIVATE ${FlagSettings})

target_link_libraries(taito ${SDL2_LIBRARIES})

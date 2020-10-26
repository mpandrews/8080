
add_library(si MODULE)

find_package(SDL2 REQUIRED)

file(GLOB srcs
	CONFIGURE_DEPENDS
	${CMAKE_CURRENT_LIST_DIR}/src/*
)

target_sources(si PRIVATE ${srcs})
target_include_directories(si PRIVATE
	${INCLUDE_DIR}
	${CMAKE_CURRENT_LIST_DIR}/include
	${SDL2_INCLUDE_DIRS}
	$<TARGET_PROPERTY:taito,INTERFACE_INCLUDE_DIRECTORIES>
	)
target_compile_options(si PRIVATE ${FlagSettings})

target_link_libraries(si PRIVATE taito ${SDL2_LIBRARIES})

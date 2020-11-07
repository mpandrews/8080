
add_library(taito SHARED)

find_package(SDL2 REQUIRED)
find_library(SDL_MIXER_LIBRARY
		NAMES SDL2_mixer
		HINTS
		ENV SDLMIXERDIR
		ENV SDLDIR
		PATH_SUFFIXES lib
		)

file(GLOB srcs
	CONFIGURE_DEPENDS
	${CMAKE_CURRENT_LIST_DIR}/src/*
)

target_sources(taito PRIVATE ${srcs})

target_include_directories(taito PRIVATE
	${INCLUDE_DIR}
	${SDL2_INCLUDE_DIRS}
)

target_include_directories(taito PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_options(taito PRIVATE ${FlagSettings})

target_link_libraries(taito 
	SDL2
	SDL2_mixer
	${CMAKE_THREAD_LIBS_INIT}
)

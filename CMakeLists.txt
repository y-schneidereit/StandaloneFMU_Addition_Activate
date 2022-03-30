cmake_minimum_required (VERSION 3.2)

project (simulator)

add_executable(simulator
	src/simulator.c
)

target_include_directories(simulator PUBLIC
	include
	model/binaries/win64
)

if (MSVC)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
	target_link_libraries(simulator shlwapi.lib)
endif ()

if (UNIX)
	target_link_libraries(simulator m ${CMAKE_DL_LIBS})
endif (UNIX)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Private SRCS)
add_executable(main ${SRCS})

target_link_directories(main
PRIVATE
	${MOLTENVK_LIB_MACOS}
	${VULKAN_LIB_MACOS}
	${MOLTENVK_FRAMEWORK_MACOS}
)

target_link_libraries(main
PRIVATE
	glfw
	RenderCore
	spdlog
	libMoltenVK.dylib
)
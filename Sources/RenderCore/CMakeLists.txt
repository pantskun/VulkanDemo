aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Private SRCS)

add_library(RenderCore STATIC ${SRCS})
target_include_directories(RenderCore
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Public
    ${MOLTENVK_INCLUDE}
Private
    ${CMAKE_CURRENT_SOURCE_DIR}/Private
)
target_link_directories(RenderCore
PUBLIC
    ${MOLTENVK_LIB_MACOS}
)
target_link_libraries(RenderCore
PRIVATE
    spdlog
    glm
)

install(TARGETS RenderCore
    EXPORT RenderCoreTargets
    LIBRARY DESTINATION lib/RenderCore
    ARCHIVE DESTINATION lib/RenderCore
)

install(DIRECTORY Public/
    DESTINATION include/RenderCore
)
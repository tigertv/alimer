if (NOT ALIMER_BUILD_EDITOR)
    return()
endif ()

file (GLOB_RECURSE SOURCE_FILES *.cpp *.h *.hpp)

if (WIN32)
    add_executable(Editor WIN32 ${SOURCE_FILES})
else ()
    add_executable(Editor ${SOURCE_FILES})
endif ()
target_link_libraries(Editor alimer)

install(TARGETS Editor
    LIBRARY DESTINATION ${DEST_LIBRARY_DIR_CONFIG}
    RUNTIME DESTINATION ${DEST_BIN_DIR_CONFIG}
    ARCHIVE DESTINATION ${DEST_ARCHIVE_DIR_CONFIG}
    BUNDLE  DESTINATION ${DEST_BIN_DIR_CONFIG}
)

set_property(TARGET Editor PROPERTY FOLDER "Editor")

find_path(REALSENSE_DIR
          NAMES source/rs.h
          HINTS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/realsense
          )
set(REALSENSE_INCLUDE_DIR ${REALSENSE_DIR}/source)
set(REALSENSE_LIBS_DIR ${REALSENSE_DIR}/linux/${ARCH})
set(REALSENSE_LIBS ${REALSENSE_LIBS_DIR}/lib/librealsense2.so.2.58.3)
set(REALSENSE_LIBS_RELEASE_DLL ${REALSENSE_LIBS_DIR})


add_library(realsenselib SHARED IMPORTED)
set_target_properties(realsenselib PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release"
                      IMPORTED_LOCATION_RELEASE ${REALSENSE_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${REALSENSE_LIBS_RELEASE_DLL}
                      )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(realsense REQUIRED_VARS REALSENSE_DIR REALSENSE_LIBS_DIR REALSENSE_INCLUDE_DIR)

# Copy the realsense dynamic linked lib into the build directory
macro(copy_realsense_dll)
    add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:realsenselib>
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:realsenselib>
    )
endmacro()

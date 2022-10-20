include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET realsense)
    find_package(realsense PATHS ${THIRDPARTY_DIR}/librealsense/cmake REQUIRED)
endif()

set(MODULE_NAME_EXTRA_LIBS realsense)

# add includes
target_include_directories(${PROJECT_NAME} PUBLIC ${THIRDPARTY_DIR}/librealsense/include)

if(WIN32)
    # Add post-build step to set copy realsense to bin
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND}
            -E copy
            $<TARGET_FILE:realsense>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>)
elseif(UNIX)
    # Install realsense lib into packaged project
    file(GLOB REALSENSE_DYLIBS ${THIRDPARTY_DIR}/librealsense/bin/librealsense2*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${REALSENSE_DYLIBS} DESTINATION lib)
endif()

# Install realsense license into packaged project
install(FILES ${THIRDPARTY_DIR}/librealsense/LICENSE DESTINATION licenses/librealsense)

# Install data directory
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data DESTINATION modules/mod_naprealsense)
